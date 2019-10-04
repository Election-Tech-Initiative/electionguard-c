#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/decryption/trustee.h>
#include <electionguard/secure_zero_memory.h>

#include "crypto_reps.h"
#include "decryption/message_reps.h"
#include "serialize/decryption.h"
#include "serialize/trustee_state.h"
#include "trustee_state_rep.h"

struct Decryption_Trustee_s
{
    uint32_t num_trustees;
    uint32_t threshold;
    uint32_t num_selections;
    uint32_t index;
    struct encryption_rep tallies[MAX_SELECTIONS];
    //@secret the private key must not be leaked from the system
    struct private_key private_key;
    struct hash base_hash;
};

struct Decryption_Trustee_new_r
Decryption_Trustee_new(uint32_t num_trustees, uint32_t threshold,
                       uint32_t num_selections, struct trustee_state message,
                       raw_hash base_hash)
{
    struct Decryption_Trustee_new_r result;
    result.status = DECRYPTION_TRUSTEE_SUCCESS;

    if (!(1 <= threshold && threshold <= num_trustees &&
          num_trustees <= MAX_TRUSTEES))
        result.status = DECRYPTION_TRUSTEE_INVALID_PARAMS;

    struct trustee_state_rep state_rep;

    Crypto_private_key_init(&state_rep.private_key, threshold);
    // Deserialize the input
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = message.len,
            .offset = 0,
            .buf = (uint8_t *)message.bytes,
        };

        Serialize_read_trustee_state(&state, &state_rep);

        if (state.status != SERIALIZE_STATE_READING)
            result.status = DECRYPTION_TRUSTEE_DESERIALIZE_ERROR;
    }

    // Allocate the trustee
    if (result.status == DECRYPTION_TRUSTEE_SUCCESS)
    {
        result.decryptor = malloc(sizeof(struct Decryption_Trustee_s));
        if (result.decryptor != NULL)
        {
            secure_zero_memory(result.decryptor, sizeof(struct Decryption_Trustee_s));
        }
        else
        {
            result.status = DECRYPTION_TRUSTEE_INSUFFICIENT_MEMORY;
        }
    }

    if (result.status == DECRYPTION_TRUSTEE_SUCCESS)
    {
        mpz_init(result.decryptor->base_hash.digest);
        Crypto_hash_reduce(&result.decryptor->base_hash, base_hash);
    }

    // Initialize the trustee
    if (result.status == DECRYPTION_TRUSTEE_SUCCESS)
    {
        result.decryptor->num_trustees = num_trustees;
        result.decryptor->threshold = threshold;
        result.decryptor->num_selections = num_selections;
        result.decryptor->index = state_rep.index;
        for (size_t i = 0; i < MAX_SELECTIONS; i++)
        {
            Crypto_encryption_rep_new(&result.decryptor->tallies[i]);
            Crypto_encryption_homomorphic_zero(&result.decryptor->tallies[i]);
        }
        Crypto_private_key_init(&result.decryptor->private_key, threshold);
        Crypto_private_key_copy(&result.decryptor->private_key,
                                &state_rep.private_key);
    }

    return result;
}

void Decryption_Trustee_free(Decryption_Trustee d)
{
    for (size_t i = 0; i < MAX_SELECTIONS; i++)
    {
        Crypto_encryption_rep_free(&d->tallies[i]);
    }
    Crypto_private_key_free(&d->private_key, d->threshold);
    mpz_clear(d->base_hash.digest);

    free(d);
}

static enum Decryption_Trustee_status
Decryption_Trustee_read_ballot(FILE *in, uint64_t *ballot_id, bool *cast,
                               uint32_t num_selections,
                               struct encryption_rep *selections)
{
    enum Decryption_Trustee_status status = DECRYPTION_TRUSTEE_SUCCESS;

    {
        int cast_tmp;
        int num_read = fscanf(in, "%d\t%" PRIu64, &cast_tmp, ballot_id);
        if (num_read != 2)
            status = DECRYPTION_TRUSTEE_IO_ERROR;
        else
            *cast = cast_tmp;
    }

    for (uint32_t i = 0;
         i < num_selections && status == DECRYPTION_TRUSTEE_SUCCESS; i++)
    {
        int num_read = fscanf(in, "\t(");
        if (0 != num_read) // can this actually happen????
            status = DECRYPTION_TRUSTEE_IO_ERROR;

        if (DECRYPTION_TRUSTEE_SUCCESS == status)
        {
            if (!mpz_t_fscan(in, selections[i].nonce_encoding))
                status = DECRYPTION_TRUSTEE_IO_ERROR;
        }

        if (DECRYPTION_TRUSTEE_SUCCESS == status)
        {
            num_read = fscanf(in, ",");
            if (0 != num_read)
                status = DECRYPTION_TRUSTEE_IO_ERROR;
        }

        if (DECRYPTION_TRUSTEE_SUCCESS == status)
        {
            if (!mpz_t_fscan(in, selections[i].message_encoding))
                status = DECRYPTION_TRUSTEE_IO_ERROR;
        }

        if (DECRYPTION_TRUSTEE_SUCCESS == status)
        {
            num_read = fscanf(in, ")");
            if (0 != num_read)
                status = DECRYPTION_TRUSTEE_IO_ERROR;
        }
    }

    return status;
}

static void Decryption_Trustee_accum_tally(Decryption_Trustee d,
                                           struct encryption_rep *selections)
{
    for (size_t i = 0; i < d->num_selections; i++)
        Crypto_encryption_homomorphic_add(&d->tallies[i], &d->tallies[i],
                                          &selections[i]);
}

enum Decryption_Trustee_status
Decryption_Trustee_tally_voting_record(Decryption_Trustee d, FILE *in)
{
    enum Decryption_Trustee_status status = DECRYPTION_TRUSTEE_SUCCESS;

    uint64_t num_ballots;
    {
        int num_read = fscanf(in, "%" PRIu64 "\n", &num_ballots);
        if (num_read != 1)
            status = DECRYPTION_TRUSTEE_IO_ERROR;
    }

    uint64_t num_selections;
    {
        int num_read = fscanf(in, "%" PRIu64 "\n", &num_selections);
        if (num_read != 1)
            status = DECRYPTION_TRUSTEE_IO_ERROR;
        else if (num_selections != d->num_selections)
            status = DECRYPTION_TRUSTEE_MALFORMED_INPUT;
    }

    for (size_t i = 0; i < num_ballots && status == DECRYPTION_TRUSTEE_SUCCESS;
         i++)
    {
        uint64_t ballot_id;
        bool cast;
        struct encryption_rep selections[MAX_SELECTIONS];

        for (int j = 0; j < MAX_SELECTIONS; j++)
        {
            Crypto_encryption_rep_new(&selections[j]);
        }

        status = Decryption_Trustee_read_ballot(in, &ballot_id, &cast,
                                                d->num_selections, selections);

        if (status == DECRYPTION_TRUSTEE_SUCCESS && cast)
            Decryption_Trustee_accum_tally(d, selections);
    }

    return status;
}

struct Decryption_Trustee_compute_share_r
Decryption_Trustee_compute_share(Decryption_Trustee d)
{
    struct Decryption_Trustee_compute_share_r result;
    result.status = DECRYPTION_TRUSTEE_SUCCESS;

    {
        // Build the message
        struct decryption_share_rep share_rep;
        share_rep.trustee_index = d->index;
        share_rep.num_tallies = d->num_selections;
        for (size_t i = 0; i < d->num_selections; i++)
        {
            Crypto_encryption_rep_new(&share_rep.tally_share[i]);
            pow_mod_p(share_rep.tally_share[i].nonce_encoding,
                      d->tallies[i].nonce_encoding,
                      d->private_key.coefficients[0]);
            mpz_set(share_rep.tally_share[i].message_encoding,
                    d->tallies[i].message_encoding);

            //Generate the proof
            Crypto_cp_proof_new(&share_rep.cp_proofs[i]);
            Crypto_generate_decryption_cp_proof(
                &share_rep.cp_proofs[i], d->private_key.coefficients[0],
                share_rep.tally_share[i].nonce_encoding, d->tallies[i],
                d->base_hash);
            //Reconstruct the public key to sanity check the proof
            mpz_t public_key;
            mpz_init(public_key);
            pow_mod_p(public_key, generator, d->private_key.coefficients[0]);
            Crypto_check_decryption_cp_proof(
                share_rep.cp_proofs[i], public_key,
                share_rep.tally_share[i].nonce_encoding, d->tallies[i],
                d->base_hash);
            mpz_clear(public_key);
        }

        //printf("Trustee %d sending 0th\n", d->index);
        // print_base16(share_rep.tally_share[0].nonce_encoding);
        // print_base16(share_rep.tally_share[0].message_encoding);

        // Serialize the message
        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_decryption_share(&state, &share_rep);
        Serialize_allocate(&state);
        Serialize_write_decryption_share(&state, &share_rep);

        if (state.status != SERIALIZE_STATE_WRITING)
            result.status = DECRYPTION_TRUSTEE_SERIALIZE_ERROR;
        else
        {
            result.share = (struct decryption_share){
                .len = state.len,
                .bytes = state.buf,
            };
        }

        for (size_t i = 0; i < d->num_selections; i++)
        {
            Crypto_encryption_rep_free(&share_rep.tally_share[i]);
        }
    }

    return result;
}

struct Decryption_Trustee_compute_fragments_r
Decryption_Trustee_compute_fragments(Decryption_Trustee d,
                                     struct decryption_fragments_request req)
{
    struct Decryption_Trustee_compute_fragments_r result;
    result.status = DECRYPTION_TRUSTEE_SUCCESS;

    struct decryption_fragments_request_rep req_rep;

    // Deserialize the input
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = req.len,
            .offset = 0,
            .buf = (uint8_t *)req.bytes,
        };

        Serialize_read_decryption_fragments_request(&state, &req_rep);

        if (state.status != SERIALIZE_STATE_READING)
            result.status = DECRYPTION_TRUSTEE_DESERIALIZE_ERROR;
    }

    if (result.status == DECRYPTION_TRUSTEE_SUCCESS)
    {
        // Build the message
        struct decryption_fragments_rep decryption_fragments_rep;
        decryption_fragments_rep.trustee_index = d->index;
        decryption_fragments_rep.num_trustees = req_rep.num_trustees;
        memcpy(decryption_fragments_rep.requested, req_rep.requested,
               req_rep.num_trustees * sizeof(bool));

        // Serialize the message
        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_decryption_fragments(&state,
                                               &decryption_fragments_rep);
        Serialize_allocate(&state);
        Serialize_write_decryption_fragments(&state, &decryption_fragments_rep);

        if (state.status != SERIALIZE_STATE_WRITING)
            result.status = DECRYPTION_TRUSTEE_SERIALIZE_ERROR;
        else
        {
            result.fragments = (struct decryption_fragments){
                .len = state.len,
                .bytes = state.buf,
            };
        }
    }

    return result;
}
