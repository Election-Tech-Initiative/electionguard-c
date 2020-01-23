#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/decryption/coordinator.h>
#include <electionguard/max_values.h>
#include <electionguard/secure_zero_memory.h>

#include "decryption/message_reps.h"
#include "serialize/decryption.h"

struct Decryption_Coordinator_s
{
    // Election parameters
    uint32_t num_trustees;
    uint32_t threshold;

    // Which trustees have announced
    bool anounced[MAX_TRUSTEES];

    // Record the tallies from any trustee to return
    bool tallies_initialized;
    uint64_t num_tallies;
    struct encryption_rep tallies[MAX_SELECTIONS]; // \Prod_i M_i in the spec

    // Which trustees have responded, and with which decryption_fragments
    bool responded[MAX_TRUSTEES];
    // How many decryption_fragments we have received to compensate for each trustee
    uint32_t num_decryption_fragments[MAX_TRUSTEES];
};

struct Decryption_Coordinator_new_r
Decryption_Coordinator_new(uint32_t num_trustees, uint32_t threshold)
{
    struct Decryption_Coordinator_new_r result;
    secure_zero_memory(&result, sizeof(struct Decryption_Coordinator_new_r));
    result.status = DECRYPTION_COORDINATOR_SUCCESS;

    if (!(1 <= threshold && threshold <= num_trustees &&
          num_trustees <= MAX_TRUSTEES))
        result.status = DECRYPTION_COORDINATOR_INVALID_PARAMS;

    // Allocate the coordinator
    if (result.status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        result.coordinator = malloc(sizeof(struct Decryption_Coordinator_s));
        if (result.coordinator != NULL)
        {
            secure_zero_memory(result.coordinator, sizeof(struct Decryption_Coordinator_s));
        }
        else
        {
            result.status = DECRYPTION_COORDINATOR_INSUFFICIENT_MEMORY;
        }
    }

    // Initialize the coordinator
    if (result.status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        result.coordinator->num_trustees = num_trustees;
        result.coordinator->threshold = threshold;

        memset(result.coordinator->anounced, false,
               result.coordinator->num_trustees * sizeof(bool));

        result.coordinator->tallies_initialized = false;
    }

    return result;
}

void Decryption_Coordinator_free(Decryption_Coordinator c) 
{ 
    free(c); 
}

enum Decryption_Coordinator_status
Decryption_Coordinator_receive_share(Decryption_Coordinator c,
                                     struct decryption_share share)
{
    enum Decryption_Coordinator_status status = DECRYPTION_COORDINATOR_SUCCESS;

    struct decryption_share_rep share_rep;

    for (int i = 0; i < MAX_SELECTIONS; i++)
    {
        Crypto_encryption_rep_new(&share_rep.tally_share[i]);
    }

    // Deserialize the input
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = share.len,
            .offset = 0,
            .buf = (uint8_t *)share.bytes,
        };

        Serialize_read_decryption_share(&state, &share_rep);

        if (state.status != SERIALIZE_STATE_READING)
            status = DECRYPTION_COORDINATOR_DESERIALIZE_ERROR;
    }

    // Check that we haven't already seen this trustee
    if (status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        if (share_rep.trustee_index >= c->num_trustees)
            status = DECRYPTION_COORDINATOR_INVALID_TRUSTEE_INDEX;
        else if (c->anounced[share_rep.trustee_index])
            status = DECRYPTION_COORDINATOR_DUPLICATE_TRUSTEE_INDEX;
    }

    if (status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        c->anounced[share_rep.trustee_index] = true;
        // We check to see if this is the first time through
        if (c->tallies_initialized)
        {
            // If it is, we add in the shares for the new tally
            if (share_rep.num_tallies != c->num_tallies)
            {
                status = DECRYPTION_COORDINATOR_CONFUSED_DECRYPTION_TRUSTEE;
            }
            for (size_t i = 0; i < share_rep.num_tallies &&
                               DECRYPTION_COORDINATOR_SUCCESS == status;
                 i++)
            {
                mul_mod_p(c->tallies[i].nonce_encoding,
                          c->tallies[i].nonce_encoding,
                          share_rep.tally_share[i].nonce_encoding);
                if (!(0 == mpz_cmp(c->tallies[i].message_encoding,
                                   share_rep.tally_share[i].message_encoding)))
                    status = DECRYPTION_COORDINATOR_CONFUSED_DECRYPTION_TRUSTEE;

                // printf("Comparing message encodings\n");
                // print_base16(c->tallies[i].message_encoding);
                // print_base16(share_rep.tally_share[i].message_encoding);
            }
        }
        else
        {
            //If this is the first one, we just copy them over
            c->num_tallies = share_rep.num_tallies;
            for (size_t i = 0; i < share_rep.num_tallies; i++)
            {
                Crypto_encryption_rep_new(&c->tallies[i]);
                mpz_set(c->tallies[i].nonce_encoding,
                        share_rep.tally_share[i].nonce_encoding);
                mpz_set(c->tallies[i].message_encoding,
                        share_rep.tally_share[i].message_encoding);
            }
            c->tallies_initialized = true;
        }
    }

    //free 
    for (uint32_t i = 0; i < share_rep.num_tallies; i++)
    {
        Crypto_encryption_rep_free(&share_rep.tally_share[i]);
        Crypto_cp_proof_free(&share_rep.cp_proofs[i]);
    }

    return status;
}

static uint32_t 
Decryption_Coordinator_num_anounced(Decryption_Coordinator c)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < c->num_trustees; i++)
        if (c->anounced[i])
            count++;

    return count;
}

static void 
Decryption_Coordinator_fill_missing_indices(
    Decryption_Coordinator c, bool *indices)
{
    for (uint32_t i = 0; i < c->num_trustees; i++)
        indices[i] = !c->anounced[i];
}

static bool 
Decryption_Coordinator_fill_requests(
    Decryption_Coordinator c, bool *request_present, 
    struct decryption_fragments_request *requests)
{
    bool ok = true;

    // The number of trustees from who've we requested 
    // the missing trustee decryption_fragments
    uint32_t num_requested = 0;

    for (uint32_t i = 0; i < c->num_trustees && ok; i++)
    {
        if (!c->anounced[i])
            request_present[i] = false;
        else
        {
            // Build the message
            struct decryption_fragments_request_rep request_rep;
            request_rep.num_trustees = c->num_trustees;

            if (num_requested < c->threshold)
            {
                // detmine which trustees have not announced
                // and fill in the 'requested' array
                Decryption_Coordinator_fill_missing_indices(
                    c, request_rep.requested);
                num_requested++;
            }
            else
                // we have met the threshold so we do not need
                // to request fragments from any remaining trustees
                // who have aanounced
                memset(request_rep.requested, false,
                       c->num_trustees * sizeof(bool));

            // Serialize the message
            struct serialize_state state = {
                .status = SERIALIZE_STATE_RESERVING,
                .len = 0,
                .offset = 0,
                .buf = NULL,
            };

            Serialize_reserve_decryption_fragments_request(&state,
                                                           &request_rep);
            Serialize_allocate(&state);
            Serialize_write_decryption_fragments_request(&state, &request_rep);

            if (state.status != SERIALIZE_STATE_WRITING)
                ok = false;
            else
            {
                request_present[i] = true;
                requests[i] = (struct decryption_fragments_request){
                    .len = state.len,
                    .bytes = state.buf,
                };
            }
        }
    }

    // Deallocate everything we allocated if not successful
    if (!ok)
        for (uint32_t i = 0; i < c->num_trustees; i++)
            if (request_present[i])
                free((void *)requests[i].bytes);

    return ok;
}

struct Decryption_Coordinator_all_shares_received_r
Decryption_Coordinator_all_shares_received(Decryption_Coordinator c)
{
    struct Decryption_Coordinator_all_shares_received_r result;
    result.status = DECRYPTION_COORDINATOR_SUCCESS;
    result.num_trustees = c->num_trustees;
    // It is important that the entries of request_present are set to
    // false, so that in case of failure we know which requests to free.
    memset(result.request_present, false, c->num_trustees * sizeof(bool));

    // Count the number of trustees who have announced
    uint32_t const num_anounced = Decryption_Coordinator_num_anounced(c);

    // Check that we have at least the threshold
    if (num_anounced < c->threshold)
        result.status = DECRYPTION_COORDINATOR_MISSING_TRUSTEES;

    // Fill the requests array with the fragment requests for each trustee
    if (result.status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        bool ok = Decryption_Coordinator_fill_requests(
            c, result.request_present, result.requests);
        if (!ok)
            result.status = DECRYPTION_COORDINATOR_INSUFFICIENT_MEMORY;
    }

    // Prepare to receive decryption_fragments
    if (result.status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        memset(c->responded, false, c->num_trustees * sizeof(bool));
        memset(c->num_decryption_fragments, false,
               c->num_trustees * sizeof(uint32_t));
    }

    return result;
}

enum Decryption_Coordinator_status 
Decryption_Coordinator_receive_fragments(
    Decryption_Coordinator c, struct decryption_fragments decryption_fragments)
{
    enum Decryption_Coordinator_status status = DECRYPTION_COORDINATOR_SUCCESS;

    struct decryption_fragments_rep decryption_fragments_rep;

    for (uint32_t i = 0; i < c->num_trustees; i++)
        for (uint64_t j = 0; j < c->num_tallies; j++)
            mpz_init(decryption_fragments_rep.partial_decryption_M[i][j]);

    mpz_init(decryption_fragments_rep.lagrange_coefficient);
    // Deserialize the input
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = decryption_fragments.len,
            .offset = 0,
            .buf = (uint8_t *)decryption_fragments.bytes,
        };

        Serialize_read_decryption_fragments(&state, &decryption_fragments_rep);

        if (state.status != SERIALIZE_STATE_READING)
            status = DECRYPTION_COORDINATOR_DESERIALIZE_ERROR;
    }

    // Check that this trustee is in range, had previously announced,
    // and has not yet responded
    if (status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        if (decryption_fragments_rep.trustee_index >= c->num_trustees)
            status = DECRYPTION_COORDINATOR_INVALID_TRUSTEE_INDEX;
        else if (!c->anounced[decryption_fragments_rep.trustee_index])
            status = DECRYPTION_COORDINATOR_INVALID_TRUSTEE_INDEX;
        else if (c->responded[decryption_fragments_rep.trustee_index])
            status = DECRYPTION_COORDINATOR_DUPLICATE_TRUSTEE_INDEX;
    }

    //TODO

    // Mark this trustee as having responded and increment the count
    // of decryption_fragments for each of the trustees for whom he is providing
    // a fragment
    if (status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        c->responded[decryption_fragments_rep.trustee_index] = true;
        for (uint32_t i = 0; i < c->num_trustees; i++)
            if (decryption_fragments_rep.requested[i])
                c->num_decryption_fragments[i]++;

        mpz_t res;

        mpz_init(res);
        for (uint32_t i = 0; i < c->num_trustees; i++)
        {
            if (decryption_fragments_rep.requested[i])
            {
                for (uint64_t j = 0; j < c->num_tallies; ++j)
                {
                    pow_mod_p(
                        res,
                        decryption_fragments_rep.partial_decryption_M[i][j],
                        decryption_fragments_rep.lagrange_coefficient);
                    mul_mod_p(c->tallies[j].nonce_encoding,
                              c->tallies[j].nonce_encoding, res);
                }
            }
        }
        mpz_clear(res);
    }

    for (uint32_t i = 0; i < c->num_trustees; i++)
        for (uint32_t j = 0; j < decryption_fragments_rep.num_selections; j++)
            mpz_clear(decryption_fragments_rep.partial_decryption_M[i][j]);

    mpz_clear(decryption_fragments_rep.lagrange_coefficient);

    return status;
}

static bool 
Decryption_Coordinator_all_trustees_seen_or_compensated(
    Decryption_Coordinator c)
{
    bool ok = true;
    for (uint32_t i = 0; i < c->num_trustees; i++)
        if (c->anounced[i])
            ok = ok && c->responded[i];
        else
            ok = ok && (c->num_decryption_fragments[i] == c->threshold);

    return ok;
}

enum Decryption_Coordinator_status
Decryption_Coordinator_all_fragments_received(
    Decryption_Coordinator c, FILE *out, uint32_t *tally_results_array)
{
    enum Decryption_Coordinator_status status = DECRYPTION_COORDINATOR_SUCCESS;

    if (!Decryption_Coordinator_all_trustees_seen_or_compensated(c))
        status = DECRYPTION_COORDINATOR_MISSING_TRUSTEES;

    for (uint64_t i = 0;
         i < c->num_tallies && status == DECRYPTION_COORDINATOR_SUCCESS; i++)
    {

        mpz_t M, decrypted_tally;

        mpz_init(M);
        mpz_init(decrypted_tally);

        // At this point, the message encoding is the same for all
        // of the trustees (confirmed, B in the document), and
        // the nonce encoding has been accumulated by product
        // as messages have come from trustees. Each trustee
        // sent their nonce encoding raised to their secret key
        div_mod_p(M, c->tallies[i].message_encoding,
                  c->tallies[i].nonce_encoding);

        //This M should be equal to g^tally
        if (!log_generator_mod_p(decrypted_tally, M))
        {
            status = DECRYPTION_COORDINATOR_IO_ERROR;
        }

        if (status == DECRYPTION_COORDINATOR_SUCCESS)
        {
            tally_results_array[i] = mpz_get_ui(decrypted_tally);
#ifdef DEBUG_PRINT
            printf("Tally %lu: %u \n", i, tally_results_array[i]);
#endif

            const char *preamble_format = "tally %" PRIu64 ": ";
            const int expected_len = snprintf(NULL, 0, preamble_format, i);

            if (fprintf(out, preamble_format, i) < expected_len)
                status = DECRYPTION_COORDINATOR_IO_ERROR;
        }
        if (DECRYPTION_COORDINATOR_SUCCESS == status)
        {
            if (!Crypto_encryption_fprint(out, &c->tallies[i]))
            {
                status = DECRYPTION_COORDINATOR_IO_ERROR;
            }
        }

        if (DECRYPTION_COORDINATOR_SUCCESS == status)
        {
            if (fprintf(out, "\n") < 1)
            {
                status = DECRYPTION_COORDINATOR_IO_ERROR;
            }
        }
    }

    return status;
}
