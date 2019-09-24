#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "decryption/coordinator.h"
#include "decryption/message_reps.h"
#include "max_values.h"
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
    uint64_t tallies[MAX_SELECTIONS];

    // Which trustees have responded, and with which fragments
    bool responded[MAX_TRUSTEES];
    // How many fragments we have received to compensate for each trustee
    uint32_t num_fragments[MAX_TRUSTEES];
};

struct Decryption_Coordinator_new_r
Decryption_Coordinator_new(uint32_t num_trustees, uint32_t threshold)
{
    struct Decryption_Coordinator_new_r result = {
        .status = DECRYPTION_COORDINATOR_SUCCESS};

    if (!(1 <= threshold && threshold <= num_trustees &&
          num_trustees <= MAX_TRUSTEES))
        result.status = DECRYPTION_COORDINATOR_INVALID_PARAMS;

    // Allocate the coordinator
    if (result.status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        result.coordinator = malloc(sizeof(struct Decryption_Coordinator_s));
        if (result.coordinator == NULL)
            result.status = DECRYPTION_COORDINATOR_INSUFFICIENT_MEMORY;
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

void Decryption_Coordinator_free(Decryption_Coordinator c) { free(c); }

enum Decryption_Coordinator_status
Decryption_Coordinator_recieve_tally_share(Decryption_Coordinator c,
                                           struct decryption_share share)
{
    enum Decryption_Coordinator_status status = DECRYPTION_COORDINATOR_SUCCESS;

    struct decryption_share_rep share_rep;

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
        if (!(share_rep.trustee_index < c->num_trustees))
            status = DECRYPTION_COORDINATOR_INVALID_TRUSTEE_INDEX;
        else if (c->anounced[share_rep.trustee_index])
            status = DECRYPTION_COORDINATOR_DUPLICATE_TRUSTEE_INDEX;
    }

    if (status == DECRYPTION_COORDINATOR_SUCCESS)
        c->anounced[share_rep.trustee_index] = true;

    // If this is the first trustee we've seen, record the tallies

    // @todo jwaksbaum We could check that everyone agrees, but for
    // this version I don't think it's worth the trouble
    if (!c->tallies_initialized)
    {
        memcpy(c->tallies, share_rep.tallies,
               share_rep.num_tallies * sizeof(uint64_t));
        c->num_tallies = share_rep.num_tallies;
        c->tallies_initialized = true;
    }

    return status;
}

static uint32_t Decryption_Coordinator_num_anounced(Decryption_Coordinator c)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < c->num_trustees; i++)
        if (c->anounced[i])
            count++;

    return count;
}

static void
Decryption_Coordinator_fill_missing_indices(Decryption_Coordinator c,
                                            bool *indices)
{
    for (uint32_t i = 0; i < c->num_trustees; i++)
        indices[i] = !c->anounced[i];
}

static bool
Decryption_Coordinator_fill_requests(Decryption_Coordinator c,
                                     bool *request_present,
                                     struct fragments_request *requests)
{
    bool ok = true;

    // The number of trustees from who've we requested the missing trustee fragments
    uint32_t num_requested = 0;

    for (uint32_t i = 0; i < c->num_trustees && ok; i++)
    {
        if (!c->anounced[i])
            request_present[i] = false;
        else
        {
            // Build the message
            struct fragments_request_rep request_rep;
            request_rep.num_trustees = c->num_trustees;

            if (num_requested < c->threshold)
            {
                Decryption_Coordinator_fill_missing_indices(
                    c, request_rep.requested);
                num_requested++;
            }
            else
                memset(request_rep.requested, false,
                       c->num_trustees * sizeof(bool));

            // Serialize the message
            struct serialize_state state = {
                .status = SERIALIZE_STATE_RESERVING,
                .len = 0,
                .offset = 0,
                .buf = NULL,
            };

            Serialize_reserve_fragments_request(&state, &request_rep);
            Serialize_allocate(&state);
            Serialize_write_fragments_request(&state, &request_rep);

            if (state.status != SERIALIZE_STATE_WRITING)
                ok = false;
            else
            {
                request_present[i] = true;
                requests[i] = (struct fragments_request){
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

struct Decryption_Coordinator_all_tally_shares_received_r
Decryption_Coordinator_all_tally_shares_received(Decryption_Coordinator c)
{
    struct Decryption_Coordinator_all_tally_shares_received_r result;
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

    // Prepare to receive fragments
    if (result.status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        memset(c->responded, false, c->num_trustees * sizeof(bool));
        memset(c->num_fragments, false, c->num_trustees * sizeof(uint32_t));
    }

    return result;
}

enum Decryption_Coordinator_status
Decryption_Coordinator_receive_tally_fragments(Decryption_Coordinator c,
                                               struct fragments fragments)
{
    enum Decryption_Coordinator_status status = DECRYPTION_COORDINATOR_SUCCESS;

    struct fragments_rep fragments_rep;

    // Deserialize the input
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = fragments.len,
            .offset = 0,
            .buf = (uint8_t *)fragments.bytes,
        };

        Serialize_read_fragments(&state, &fragments_rep);

        if (state.status != SERIALIZE_STATE_READING)
            status = DECRYPTION_COORDINATOR_DESERIALIZE_ERROR;
    }

    // Check that this trustee is in range, had previously announced,
    // and has not yet responded
    if (status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        if (!(fragments_rep.trustee_index < c->num_trustees))
            status = DECRYPTION_COORDINATOR_INVALID_TRUSTEE_INDEX;
        else if (!c->anounced[fragments_rep.trustee_index])
            status = DECRYPTION_COORDINATOR_INVALID_TRUSTEE_INDEX;
        else if (c->responded[fragments_rep.trustee_index])
            status = DECRYPTION_COORDINATOR_DUPLICATE_TRUSTEE_INDEX;
    }

    // Mark this trustee as having responded and increment the count
    // of fragments for each of the trustees for whom he is providing
    // a fragment
    if (status == DECRYPTION_COORDINATOR_SUCCESS)
    {
        c->responded[fragments_rep.trustee_index] = true;
        for (uint32_t i = 0; i < c->num_trustees; i++)
            if (fragments_rep.requested[i])
                c->num_fragments[i]++;
    }

    return status;
}

static bool Decryption_Coordinator_all_trustees_seen_or_compensated(
    Decryption_Coordinator c)
{
    bool ok = true;
    for (uint32_t i = 0; i < c->num_trustees; i++)
        if (c->anounced[i])
            ok = ok && c->responded[i];
        else
            ok = ok && (c->num_fragments[i] == c->threshold);

    return ok;
}

enum Decryption_Coordinator_status
Decryption_Coordinator_all_fragments_received(Decryption_Coordinator c,
                                              FILE *out)
{
    enum Decryption_Coordinator_status status = DECRYPTION_COORDINATOR_SUCCESS;

    if (!Decryption_Coordinator_all_trustees_seen_or_compensated(c))
        status = DECRYPTION_COORDINATOR_MISSING_TRUSTEES;

    for (uint64_t i = 0;
         i < c->num_tallies && status == DECRYPTION_COORDINATOR_SUCCESS; i++)
    {
        int io_status = fprintf(out, "%" PRIu64 "\n", c->tallies[i]);
        if (io_status < 0)
            status = DECRYPTION_COORDINATOR_IO_ERROR;
    }

    return status;
}
