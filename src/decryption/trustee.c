#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "crypto_reps.h"
#include "decryption/message_reps.h"
#include "decryption/trustee.h"
#include "serialize/decryption.h"
#include "serialize/trustee_state.h"
#include "trustee_state_rep.h"

struct Decryption_Trustee_s
{
    uint32_t num_trustees;
    uint32_t threshold;
    uint32_t num_selections;
    uint32_t index;
    uint64_t tallies[MAX_SELECTIONS];
    //@secret the private key must not be leaked from the system
    struct private_key private_key;
};

struct Decryption_Trustee_new_r
Decryption_Trustee_new(uint32_t num_trustees, uint32_t threshold,
                       uint32_t num_selections, struct trustee_state message)
{
    struct Decryption_Trustee_new_r result;
    result.status = DECRYPTION_TRUSTEE_SUCCESS;

    if (!(1 <= threshold && threshold <= num_trustees &&
          num_trustees <= MAX_TRUSTEES))
        result.status = DECRYPTION_TRUSTEE_INVALID_PARAMS;

    struct trustee_state_rep state_rep;

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
        if (result.decryptor == NULL)
            result.status = DECRYPTION_TRUSTEE_INSUFFICIENT_MEMORY;
    }

    // Initialize the trustee
    if (result.status == DECRYPTION_TRUSTEE_SUCCESS)
    {
        result.decryptor->num_trustees = num_trustees;
        result.decryptor->threshold = threshold;
        result.decryptor->num_selections = num_selections;
        result.decryptor->index = state_rep.index;
        memset(result.decryptor->tallies, 0, MAX_SELECTIONS * sizeof(uint64_t));
        Crypto_private_key_copy(&result.decryptor->private_key,
                                &state_rep.private_key);
    }

    return result;
}

void Decryption_Trustee_free(Decryption_Trustee d) { free(d); }

static enum Decryption_Trustee_status
Decryption_Trustee_read_ballot(FILE *in, uint64_t *ballot_id, bool *cast,
                               uint32_t num_selections, bool *selections)
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
        int selection;
        int num_read = fscanf(in, "\t%d", &selection);
        if (num_read != 1)
            status = DECRYPTION_TRUSTEE_IO_ERROR;
        else
            selections[i] = selection;
    }

    return status;
}

static void Decryption_Trustee_accum_tally(Decryption_Trustee d,
                                           bool *selections)
{
    for (size_t i = 0; i < d->num_selections; i++)
        d->tallies[i] += selections[i];
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
        bool selections[MAX_SELECTIONS];

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
        memcpy(share_rep.tallies, d->tallies,
               d->num_selections * sizeof(uint64_t));

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
