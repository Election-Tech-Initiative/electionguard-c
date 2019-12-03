#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#include <electionguard/api/create_election.h>
#include <electionguard/api/encrypt_ballot.h>
#include <electionguard/api/record_ballots.h>
#include <electionguard/api/tally_votes.h>
#include <electionguard/max_values.h>

static bool random_bit();
static void fill_random_ballot(uint8_t *selections);

// Election Parameters
uint32_t const NUM_TRUSTEES = 3;
uint32_t const THRESHOLD = 2;
uint32_t const NUM_ENCRYPTERS = 3;
uint32_t const NUM_SELECTIONS = 6;
uint32_t const DECRYPTING_TRUSTEES = 2;
uint32_t const NUM_RANDOM_BALLOT_SELECTIONS = 6;

// ^The number of trustees that will participate in decryption. Will fail if this is less than THRESHOLD

int main()
{
    bool ok = true;
    // Seed the RNG that we use to generate arbitrary ballots. This
    // relies on the fact that the current implementation of the
    // cryptography does not rely on the built in RNG.
    srand(time(NULL));

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

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        if (trustee_states[i].bytes == NULL)
            ok = false;
    }

    // Encrypt Ballots

    printf("\n--- Encrypt Ballots ---\n");
    
    struct register_ballot_message encrypted_ballots[NUM_RANDOM_BALLOT_SELECTIONS];
    uint64_t ballot_identifiers[NUM_RANDOM_BALLOT_SELECTIONS];
    char *ballot_trackers[NUM_RANDOM_BALLOT_SELECTIONS];

    if (ok)
    {
        uint64_t current_num_ballots = 0;
        for (uint64_t i = 0; i < NUM_RANDOM_BALLOT_SELECTIONS && ok; i++)
        {

            uint8_t selections[MAX_SELECTIONS];
            fill_random_ballot(selections);
            uint64_t ballotId;
            struct register_ballot_message encrypted_ballot_message;
            char *tracker;

            ok = API_EncryptBallot(selections, config, &current_num_ballots,
                                   &ballotId, &encrypted_ballot_message,
                                   &tracker);

            if (ok)
            {
                encrypted_ballots[i] = encrypted_ballot_message;
                ballot_identifiers[i] = ballotId;
                ballot_trackers[i] = tracker;

                // Print id and tracker
                printf("Ballot id: %lu\n%s\n", ballotId, tracker);
            }
        }
    }

    // Register & Record Cast/Spoil Multiple Ballots

    printf("\n--- Randomly Assigning Ballots to be Cast or Spoil Arrays ---\n");

    uint32_t current_cast_index = 0;
    uint32_t current_spoiled_index = 0;
    uint64_t casted_ballot_ids[NUM_RANDOM_BALLOT_SELECTIONS];
    uint64_t spoiled_ballot_ids[NUM_RANDOM_BALLOT_SELECTIONS];

    for (uint64_t i = 0; i < NUM_RANDOM_BALLOT_SELECTIONS && ok; i++)
    {
        if (random_bit())
        {
            casted_ballot_ids[current_cast_index] = ballot_identifiers[i];
            current_cast_index++;

            printf("Cast Ballot Id: %lu\n", ballot_identifiers[i]);
        }
        else
        {
            spoiled_ballot_ids[current_spoiled_index] = ballot_identifiers[i];
            current_spoiled_index++;

            printf("Spoil Ballot Id: %lu\n", ballot_identifiers[i]);
        }
    }

    if ((current_cast_index + current_spoiled_index) != NUM_RANDOM_BALLOT_SELECTIONS)
        ok = false;

    printf("\n--- Record Ballots (Register, Cast, and Spoil) ---\n");

    char *ballots_filename;
    if (ok)
    {
        // Assigning an output_path fails if this folder doesn't already exist
        char *output_path = "../"; // This outputs to the directy above the cwd.
        char *output_prefix = "ballots-";
        ok = API_RecordBallots(config.num_selections, current_cast_index, current_spoiled_index,
                NUM_RANDOM_BALLOT_SELECTIONS, casted_ballot_ids, spoiled_ballot_ids, encrypted_ballots,
                output_path, output_prefix, &ballots_filename);
                
        if (ok)
            printf("Ballot registrations and recording of cast/spoil successful!\nCheck output file \"%s\"\n",
                ballots_filename);
    }

    // Tally Votes & Decrypt Results

    printf("\n--- Tally & Decrypt Votes ---\n");

    char *tally_filename;
    if (ok)
    {
        char *output_path = "../"; // This outputs to the directy above the cwd.
        char *output_prefix = "tally-";
        ok = API_TallyVotes(config, trustee_states, DECRYPTING_TRUSTEES,
                ballots_filename, output_path, output_prefix, &tally_filename);

        if (ok)
            printf("Tally from ballots input successful!\nCheck output file \"%s\"\n",
                tally_filename);
    }

    // Cleanup

    API_TallyVotes_free(tally_filename);
    API_RecordBallots_free(ballots_filename);
    for (uint64_t i = 0; i < NUM_RANDOM_BALLOT_SELECTIONS && ok; i++)
        API_EncryptBallot_free(encrypted_ballots[i], ballot_trackers[i]);
    API_CreateElection_free(config.joint_key, trustee_states);

    if (ok)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

bool random_bit() { return 1 & rand(); }

void fill_random_ballot(uint8_t *selections)
{
    uint32_t selected_count = 0;
    for (uint32_t i = 0; i < NUM_SELECTIONS; i++)
    {
        if (random_bit())
        {
            selections[i] = 1;
            selected_count++;
        }
        else
        {
            selections[i] = 0;
        }
    }

    // ensure we dont have all trues or all falses
    if (selected_count == NUM_SELECTIONS)
        selections[0] = 0;
    else if (selected_count == 0)
        selections[0] = 1;

    printf("vote created ");
    for (uint32_t i = 0; i < NUM_SELECTIONS; i++)
    {
        printf("%d ", selections[i]);
    }
    printf("\n");
}