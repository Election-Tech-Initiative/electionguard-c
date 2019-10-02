#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/decryption/coordinator.h>
#include <electionguard/decryption/trustee.h>

#include "main_decryption.h"
#include "main_params.h"

static bool initialize_coordinator(void);

static bool initialize_trustees(struct trustee_state *trustee_states);

static bool tally_voting_records(FILE *in);

static bool decrypt_tally_shares(void);

static bool decrypt_tally_decryption_fragments(
    bool *requests_present, struct decryption_fragments_request *requests);

static Decryption_Trustee trustees[MAX_TRUSTEES];
static Decryption_Coordinator coordinator;

bool decryption(FILE *in, FILE *out, struct trustee_state *trustee_states)
{
    bool ok = true;

    if (ok)
        ok = initialize_coordinator();

    if (ok)
        ok = initialize_trustees(trustee_states);

    if (ok)
        ok = tally_voting_records(in);

    if (ok)
        ok = decrypt_tally_shares();

    struct decryption_fragments_request requests[MAX_TRUSTEES];
    bool request_present[MAX_TRUSTEES];
    for (uint32_t i = 0; i < NUM_TRUSTEES; i++)
        requests[i] = (struct decryption_fragments_request){.bytes = NULL};

    if (ok)
    {
        struct Decryption_Coordinator_all_shares_received_r result =
            Decryption_Coordinator_all_shares_received(coordinator);

        if (result.status != DECRYPTION_COORDINATOR_SUCCESS)
            ok = false;
        else
            for (uint32_t i = 0; i < result.num_trustees; i++)
                requests[i] = result.requests[i],
                request_present[i] = result.request_present[i];
    }

    if (ok)
        ok = decrypt_tally_decryption_fragments(request_present, requests);

    if (ok)
    {
        enum Decryption_Coordinator_status status =
            Decryption_Coordinator_all_fragments_received(coordinator, out);

        if (status != DECRYPTION_COORDINATOR_SUCCESS)
            ok = false;
    }

    for (uint32_t i = 0; i < NUM_TRUSTEES; i++)
        if (request_present[i])
        {
            free((void *)requests[i].bytes);
            requests[i].bytes = NULL;
        }

    for (uint32_t i = 0; i < NUM_TRUSTEES; i++)
        if (trustees[i] != NULL)
        {
            Decryption_Trustee_free(trustees[i]);
            trustees[i] = NULL;
        }

    if (coordinator != NULL)
    {
        Decryption_Coordinator_free(coordinator);
        coordinator = NULL;
    }

    return ok;
}

bool initialize_coordinator(void)
{
    bool ok = true;

    struct Decryption_Coordinator_new_r result =
        Decryption_Coordinator_new(NUM_TRUSTEES, THRESHOLD);

    if (result.status != DECRYPTION_COORDINATOR_SUCCESS)
        ok = false;
    else
        coordinator = result.coordinator;

    return ok;
}

bool initialize_trustees(struct trustee_state *trustee_states)
{
    bool ok = true;

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        struct Decryption_Trustee_new_r result = Decryption_Trustee_new(
            NUM_TRUSTEES, THRESHOLD, NUM_SELECTIONS, trustee_states[i], BASE_HASH_CODE);

        if (result.status != DECRYPTION_TRUSTEE_SUCCESS)
            ok = false;
        else
            trustees[i] = result.decryptor;
    }

    return ok;
}

bool tally_voting_records(FILE *in)
{
    bool ok = true;

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        int seek_status = fseek(in, 0L, SEEK_SET);
        if (seek_status != 0)
            ok = false;

        if (ok)
        {
            enum Decryption_Trustee_status status =
                Decryption_Trustee_tally_voting_record(trustees[i], in);

            if (status != DECRYPTION_TRUSTEE_SUCCESS)
                ok = false;
        }
    }

    return ok;
}

bool decrypt_tally_shares(void)
{
    bool ok = true;

    for (uint32_t i = 0; i < DECRYPTING_TRUSTEES && ok; i++)
    {
        struct decryption_share share = {.bytes = NULL};

        {
            struct Decryption_Trustee_compute_share_r result =
                Decryption_Trustee_compute_share(trustees[i]);

            if (result.status != DECRYPTION_TRUSTEE_SUCCESS)
                ok = false;
            else
                share = result.share;
        }

        if (ok)
        {
            enum Decryption_Coordinator_status status =
                Decryption_Coordinator_receive_share(coordinator, share);
            if (status != DECRYPTION_COORDINATOR_SUCCESS)
                ok = false;
        }

        if (share.bytes != NULL)
        {
            free((void *)share.bytes);
            share.bytes = NULL;
        }
    }

    return ok;
}

bool decrypt_tally_decryption_fragments(
    bool *requests_present, struct decryption_fragments_request *requests)
{
    bool ok = true;

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        if (requests_present[i])
        {
            struct decryption_fragments decryption_fragments = {.bytes = NULL};

            {
                struct Decryption_Trustee_compute_fragments_r result =
                    Decryption_Trustee_compute_fragments(trustees[i],
                                                         requests[i]);

                if (result.status != DECRYPTION_TRUSTEE_SUCCESS)
                    ok = false;
                else
                    decryption_fragments = result.fragments;
            }

            if (ok)
            {
                enum Decryption_Coordinator_status status =
                    Decryption_Coordinator_receive_fragments(
                        coordinator, decryption_fragments);

                if (status != DECRYPTION_COORDINATOR_SUCCESS)
                    ok = false;
            }

            if (decryption_fragments.bytes != NULL)
            {
                free((void *)decryption_fragments.bytes);
                decryption_fragments.bytes = NULL;
            }
        }
    }

    return ok;
}
