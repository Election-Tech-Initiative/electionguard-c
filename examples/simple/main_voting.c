#include <stdlib.h>
#include <unistd.h>

#include "main_params.h"
#include "main_voting.h"

static bool initialize_encrypters(struct joint_public_key joint_key);

static bool initialize_coordinator(void);

static bool simulate_random_votes(uint32_t encrypter_ix, uint64_t num_ballots);

static Voting_Encrypter *encrypters;
static Voting_Coordinator coordinator;

bool voting(struct joint_public_key joint_key, FILE *out)
{
    bool ok = true;

    if (ok)
    {
        encrypters = malloc(NUM_ENCRYPTERS * sizeof(Voting_Encrypter));
        if (encrypters == NULL)
            ok = false;
    }

    if (ok)
        ok = initialize_encrypters(joint_key);

    if (ok)
        ok = initialize_coordinator();

    if (ok)
    {
        for (uint32_t i = 0; i < NUM_ENCRYPTERS && ok; i++)
            ok = simulate_random_votes(i, 10);
    }

    if (ok)
    {
        enum Voting_Coordinator_status status =
            Voting_Coordinator_export_ballots(coordinator, out);
        if (status != VOTING_COORDINATOR_SUCCESS)
            ok = false;
    }

    if (coordinator != NULL)
    {
        Voting_Coordinator_free(coordinator);
        coordinator = NULL;
    }

    for (uint32_t i = 0; i < NUM_ENCRYPTERS; i++)
        if (encrypters != NULL && encrypters[i] != NULL)
        {
            Voting_Encrypter_free(encrypters[i]);
            encrypters[i] = NULL;
        }

    if (encrypters != NULL)
    {
        free(encrypters);
        encrypters = NULL;
    }

    return ok;
}

bool initialize_encrypters(struct joint_public_key joint_key)
{
    bool ok = true;

    uint8_t id_buf[1];
    struct uid uid = {
        .len = 1,
        .bytes = id_buf,
    };

    for (uint32_t i = 0; i < NUM_ENCRYPTERS && ok; i++)
    {
        id_buf[0] = i;
        struct Voting_Encrypter_new_r result =
            Voting_Encrypter_new(uid, joint_key, NUM_SELECTIONS);

        if (result.status != VOTING_ENCRYPTER_SUCCESS)
            ok = false;
        else
            encrypters[i] = result.encrypter;
    }

    return ok;
}

bool initialize_coordinator(void)
{
    bool ok = true;

    struct Voting_Coordinator_new_r result =
        Voting_Coordinator_new(NUM_SELECTIONS);

    if (result.status != VOTING_COORDINATOR_SUCCESS)
        ok = false;
    else
        coordinator = result.coordinator;

    return ok;
}

static bool random_bit() { return 1 & rand(); }

static void fill_random_ballot(bool *selections)
{
    for (uint32_t i = 0; i < NUM_SELECTIONS; i++)
        selections[i] = random_bit();
}

bool simulate_random_votes(uint32_t encrypter_ix, uint64_t num_ballots)
{
    bool ok = true;

    Voting_Encrypter const encrypter = encrypters[encrypter_ix];

    for (uint64_t i = 0; i < num_ballots && ok; i++)
    {
        bool selections[MAX_SELECTIONS];

        fill_random_ballot(selections);

        struct register_ballot_message message = {.bytes = NULL};
        struct ballot_tracker tracker = {.bytes = NULL};
        struct ballot_identifier identifier = {.bytes = NULL};

        // Encrypt the ballot
        if (ok)
        {
            struct Voting_Encrypter_encrypt_ballot_r result =
                Voting_Encrypter_encrypt_ballot(encrypter, selections);

            if (result.status != VOTING_ENCRYPTER_SUCCESS)
                ok = false;
            else
            {
                message = result.message;
                tracker = result.tracker;
                identifier = result.id;
            }
        }

        // Register the ballot
        if (ok)
        {
            enum Voting_Coordinator_status status =
                Voting_Coordinator_register_ballot(coordinator, message);

            if (status != VOTING_COORDINATOR_SUCCESS)
                ok = false;
        }

        // Randomly cast or spoil the ballot
        if (ok)
        {
            enum Voting_Coordinator_status status;

            if (random_bit())
                status =
                    Voting_Coordinator_cast_ballot(coordinator, identifier);
            else
                status =
                    Voting_Coordinator_spoil_ballot(coordinator, identifier);

            if (status != VOTING_COORDINATOR_SUCCESS)
                ok = false;
        }

        if (message.bytes != NULL)
        {
            free((void *)message.bytes);
            message.bytes = NULL;
        }

        if (tracker.bytes != NULL)
        {
            free((void *)tracker.bytes);
            message.bytes = NULL;
        }

        if (identifier.bytes != NULL)
        {
            free((void *)identifier.bytes);
            identifier.bytes = NULL;
        }
    }

    return ok;
}
