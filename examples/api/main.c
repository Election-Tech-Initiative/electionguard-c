#include <assert.h>
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

struct test_ballot
{
    uint64_t ballotId;
    bool isCast;
    bool isSpoiled;
    char *tracker;
    uint8_t selections[MAX_SELECTIONS];
};

static bool random_bit();
static int32_t fill_random_ballot(uint8_t *selections);
static bool result_equals_expected_selections(struct test_ballot *testBallots, uint32_t actual_tally, uint32_t selection_index);

// Election Parameters
uint32_t const NUM_TRUSTEES = 3;
uint32_t const THRESHOLD = 2; 
uint32_t const NUM_ENCRYPTERS = 3;
uint32_t const NUM_SELECTIONS = 6;
uint32_t const DECRYPTING_TRUSTEES = 2;             // must be >= THRESHOLD && <= NUM_TRUSTEES
uint32_t const NUM_RANDOM_BALLOT_SELECTIONS = 6;

int main()
{
    bool ok = true;

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
    printf("\n--- Create Election ---\n\n");

    struct trustee_state trustee_states[MAX_TRUSTEES];
    ok = API_CreateElection(&config, trustee_states);

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        if (trustee_states[i].bytes == NULL)
            ok = false;
    }

    // Encrypt Ballots

    printf("\n--- Encrypt Ballots ---\n\n");
    
    struct register_ballot_message encrypted_ballots[NUM_RANDOM_BALLOT_SELECTIONS];
    struct test_ballot testBallots[NUM_RANDOM_BALLOT_SELECTIONS];

    if (ok)
    {
        uint64_t current_num_ballots = 0;
        for (uint64_t i = 0; i < NUM_RANDOM_BALLOT_SELECTIONS && ok; i++)
        {
            struct test_ballot testBallot;
            struct register_ballot_message encrypted_ballot_message;
            //char *tracker;

            // we're assuming that the returned number of true selections for this ballot
            // is the the correct expected number for this ballot style in order to encrypt it
            uint32_t selected_count = fill_random_ballot(testBallot.selections);

            ok = API_EncryptBallot(testBallot.selections, selected_count, config, &current_num_ballots,
                                   &testBallot.ballotId, &encrypted_ballot_message,
                                   &testBallot.tracker);

            if (ok)
            {
                encrypted_ballots[i] = encrypted_ballot_message;
                // ballot_identifiers[i] = testBallot.ballotId;
                // ballot_trackers[i] = testBallot.tracker;

                testBallots[i] = testBallot;

                // Print id and tracker
                printf("Ballot id: %lu\n%s\n\n", testBallot.ballotId, testBallot.tracker);
            }
        }
    }

    // Register & Record Cast/Spoil Multiple Ballots

    printf("\n--- Randomly Assigning Ballots to be Cast or Spoil Arrays ---\n\n");

    uint32_t current_cast_index = 0;
    uint32_t current_spoiled_index = 0;
    uint64_t casted_ballot_ids[NUM_RANDOM_BALLOT_SELECTIONS];
    uint64_t spoiled_ballot_ids[NUM_RANDOM_BALLOT_SELECTIONS];

    for (uint64_t i = 0; i < NUM_RANDOM_BALLOT_SELECTIONS && ok; i++)
    {
        if (random_bit())
        {
            testBallots[i].isCast = true;
            casted_ballot_ids[current_cast_index] = testBallots[i].ballotId;
            current_cast_index++;

            printf("Ballot Id: %lu - Cast!\n", testBallots[i].ballotId);
        }
        else
        {
            testBallots[i].isSpoiled = true;
            spoiled_ballot_ids[current_spoiled_index] = testBallots[i].ballotId;
            current_spoiled_index++;

            printf("Ballot Id: %lu - Spoiled!\n", testBallots[i].ballotId);
        }
    }

    if ((current_cast_index + current_spoiled_index) != NUM_RANDOM_BALLOT_SELECTIONS)
        ok = false;

    printf("\n--- Record Ballots (Register, Cast, and Spoil) ---\n\n");

    char *ballots_filename;
    char *casted_trackers[current_cast_index];
    char *spoiled_trackers[current_spoiled_index];
    if (ok)
    {
        // Assigning an output_path fails if this folder doesn't already exist
        char *output_path = "./ballots/"; // This outputs to the directy above the cwd.
        char *output_prefix = "ballots-";
        ok = API_RecordBallots(config.num_selections, current_cast_index, current_spoiled_index,
                NUM_RANDOM_BALLOT_SELECTIONS, casted_ballot_ids, spoiled_ballot_ids, encrypted_ballots,
                output_path, output_prefix, &ballots_filename, casted_trackers, spoiled_trackers);
                
        if (ok)
        {
            printf("Casted Ballot Trackers:\n");
            for (uint32_t i = 0; i < current_cast_index; i++)
            {
                uint64_t id = casted_ballot_ids[i];
                assert(testBallots[id].isCast == true);
                printf("\t%ld: %s\n", id, casted_trackers[i]);
            }
            printf("\nSpoiled Ballot Trackers:\n");
            for (uint32_t i = 0; i < current_spoiled_index; i++)
            {
                uint64_t id = spoiled_ballot_ids[i];
                assert(testBallots[id].isSpoiled == true);
                printf("\t%ld: %s\n", id, spoiled_trackers[i]);
            }
            printf("\nBallot registrations and recording of cast/spoil successful!\nCheck output file \"%s\"\n",
                ballots_filename);
        }
    }

    // Tally Votes & Decrypt Results

    printf("\n--- Tally & Decrypt Votes ---\n\n");

    char *tally_filename;
    uint32_t tally_results[config.num_selections];
    if (ok)
    {
        char *output_path = "./tallies/"; // This outputs to the directy above the cwd.
        char *output_prefix = "tally-";

        // copy the threshold number of trustees needed to decrypt
        struct trustee_state threshold_trustee_states[MAX_TRUSTEES];
        for (uint32_t i = 0; i < THRESHOLD && ok; i++)
        {
            threshold_trustee_states[i] = trustee_states[i];
            assert(threshold_trustee_states[i].bytes != NULL);
        }
        
        printf("Tallying with %d of %d trustees \n\n", DECRYPTING_TRUSTEES, config.num_trustees);

        ok = API_TallyVotes(config, threshold_trustee_states, DECRYPTING_TRUSTEES,
                ballots_filename, output_path, output_prefix, &tally_filename, tally_results);

        if (ok)
        {
            printf("Results for: \n");
            for (uint32_t i = 0; i < config.num_selections; i++)
            {
                assert(result_equals_expected_selections(testBallots, tally_results[i], i));
            }
            printf("\nTally from ballots input successful!\nCheck output file \"%s\"\n",
                tally_filename);
        } else {
            printf("\nTally & Decrypt Votes - Error in API_TallyVotes! \n");
        }
    }

    // Cleanup

    API_TallyVotes_free(tally_filename);
    API_RecordBallots_free(ballots_filename, current_cast_index, current_spoiled_index, casted_trackers, spoiled_trackers);
    for (uint64_t i = 0; i < NUM_RANDOM_BALLOT_SELECTIONS && ok; i++)
        API_EncryptBallot_free(encrypted_ballots[i], testBallots[i].tracker);
    API_CreateElection_free(config.joint_key, trustee_states);

    if (ok)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

bool result_equals_expected_selections(struct test_ballot *testBallots, uint32_t actual_tally, uint32_t selection_index) 
{
    uint32_t expected_tally = 0;

    for (uint64_t i = 0; i < NUM_RANDOM_BALLOT_SELECTIONS; i++)
    {
        if (testBallots[i].isCast) {
            expected_tally += testBallots[i].selections[selection_index];
        }
    }
    printf("\tselection: %d: expected: %d actual: %d\n", selection_index, expected_tally, actual_tally);
    return expected_tally == actual_tally;
}

bool random_bit() { return 1 & rand(); }

int32_t fill_random_ballot(uint8_t *selections)
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

    printf("vote created selections: [ ");
    for (uint32_t i = 0; i < NUM_SELECTIONS; i++)
    {
        printf("%d, ", selections[i]);
    }
    printf("]\n");

    return selected_count;
}
