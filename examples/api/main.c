#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#include <electionguard/max_values.h>
#include <electionguard/api/create_election.h>
#include <electionguard/api/encrypt_ballot.h>

void fill_random_ballot(bool *selections);

// Election Parameters
uint32_t const NUM_TRUSTEES = 3;
uint32_t const THRESHOLD = 2;
uint32_t const NUM_ENCRYPTERS = 3;
uint32_t const NUM_SELECTIONS = 3;
uint32_t const DECRYPTING_TRUSTEES = 2;
uint32_t const NUM_RANDOM_BALLOT_SELECTIONS = 6;

// ^The number of trustees that will participate in decryption. Will fail if this is less than THRESHOLD

int main()
{
    bool ok = true;
    // Seed the RNG that we use to generate arbitrary ballots. This
    // relies on the fact that the current implementation of the
    // cryptography does not rely on the built in RNG.
    srand(100);

    struct api_config config = {
        .num_selections = NUM_SELECTIONS,
        .num_trustees = NUM_TRUSTEES,
        .threshold = THRESHOLD,
        .subgroup_order = 0,
        .election_meta = "placeholder",
        .joint_key = {.bytes = NULL},
    };

    // Create Election
    
    printf("\n--- Create Election ---\n");

    struct trustee_state trustee_states[MAX_TRUSTEES];
    ok = API_CreateElection(&config, trustee_states);

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++) {
        if (trustee_states[i].bytes == NULL)
            ok = false;
    }

    // Encrypt Ballots

    printf("\n--- Encrypt Ballots ---\n");

    if (ok) {
        uint64_t current_num_ballots = 0;
        for (uint64_t i = 0; i < NUM_RANDOM_BALLOT_SELECTIONS && ok; i++) {
            
            bool selections[MAX_SELECTIONS];
            fill_random_ballot(selections);

            struct API_EncryptBallot_result encrypt_ballot_result =
                API_EncryptBallot(selections, config, &current_num_ballots);

            if (encrypt_ballot_result.message.bytes == NULL ||
                encrypt_ballot_result.tracker_string == NULL) {
                ok = false;
            } else {
                // Print id and tracker
                printf("Ballot id: %lu\n%s\n", encrypt_ballot_result.identifier, encrypt_ballot_result.tracker_string);
            }
            // TODO: store encrypted ballot and id for next step
        }
    }

    if (ok)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

static bool random_bit() { return 1 & rand(); }

void fill_random_ballot(bool *selections)
{
    bool selected = false;
    for (uint32_t i = 0; i < NUM_SELECTIONS; i++)
    {
        if (!selected)
        {
            selections[i] = random_bit();
        }
        else
        {
            selections[i] = false;
        }
        if (selections[i])
        {
            selected = true;
        }
    }
    if (!selected)
    {
        selections[NUM_SELECTIONS - 1] = true;
    }
    printf("vote created ");
    for (uint32_t i = 0; i < NUM_SELECTIONS; i++)
    {
        printf("%d ", selections[i]);
    }
    printf("\n");
}