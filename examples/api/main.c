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
#include <electionguard/api/load_ballots.h>
#include <electionguard/api/record_ballots.h>
#include <electionguard/api/tally_votes.h>
#include <electionguard/max_values.h>

// test struct to capture ballot state
struct test_ballot
{
    char external_identifier[MAX_EXTERNAL_ID_LENGTH];
    bool isCast;
    bool isSpoiled;
    char *tracker;
    uint8_t selections[MAX_SELECTIONS];
};

// misc functions
static bool random_bit();
static bool compare_string(char *first, char *second);
static int32_t fill_random_ballot(uint8_t *selections);

// assertion functions
static bool loaded_ballots_match_encrypted_ballots(
                    struct register_ballot_message *loaded_ballots, 
                    struct register_ballot_message *encrypted_ballots, 
                    uint32_t ballot_count);
static bool loaded_ballot_identifiers_match_encrypted_ballots(
                    char **loaded_ballot_identifiers, 
                    struct test_ballot *test_ballots, 
                    uint32_t ballot_count);
static bool result_equals_expected_selections(
                    struct test_ballot *testBallots, 
                    uint32_t actual_tally, 
                    uint32_t selection_index);

// Election Parameters
uint32_t const NUM_TRUSTEES = 3;
uint32_t const THRESHOLD = 2; 
uint32_t const NUM_ENCRYPTERS = 3;
uint32_t const NUM_SELECTIONS = 12;                  // the number of total contest selections for an election
uint32_t const DECRYPTING_TRUSTEES = 2;             // must be >= THRESHOLD && <= NUM_TRUSTEES
uint32_t const NUM_RANDOM_BALLOTS = 6;              // the number of ballots to use when executing the test

int main()
{
    bool ok = true;

    srand(time(NULL));

    // TODO: name all output artifacts for the test run instance (e.g. by start time)

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
    
    struct register_ballot_message memory_encrypted_ballots[NUM_RANDOM_BALLOTS];
    struct test_ballot testBallots[NUM_RANDOM_BALLOTS];

    char *encrypted_ballots_filename = NULL;
    char *encrypted_output_path = "./ballots_encrypter/"; // This outputs to the directy above the cwd.
    char encrypted_output_prefix[50];
    
    //generate a unique file name
    time_t now = time(NULL);
    struct tm *local_time = 0;
    local_time = localtime(&now);
    if (local_time == 0)
    {
        ok = false;
    }
    sprintf(encrypted_output_prefix, "%s_%d_%d_%d", "encrypted-ballots", 
        local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday);

    if (ok)
    {
        for (uint32_t i = 0; i < NUM_RANDOM_BALLOTS && ok; i++)
        {
            struct test_ballot testBallot;
            sprintf(testBallot.external_identifier, "some_string_value_%d", i);
            
            struct register_ballot_message encrypted_ballot_message;

            // TODO: demonstrate the empty null field

            // for now, we're assuming that the returned number of true selections for this ballot
            // is the the correct expected number for this ballot style in order to encrypt it
            uint32_t selected_count = fill_random_ballot(testBallot.selections);

            ok = API_EncryptBallot(
                testBallot.selections, 
                selected_count, 
                config, 
                testBallot.external_identifier, 
                &encrypted_ballot_message,
                encrypted_output_path,
                encrypted_output_prefix,
                &encrypted_ballots_filename,
                &testBallot.tracker
            );

            if (ok)
            {
                memory_encrypted_ballots[i] = encrypted_ballot_message;
                testBallots[i] = testBallot;

                // Print id and tracker
                printf("Encrypted Ballot id: %s\n%s\n\n", 
                testBallots[i].external_identifier, testBallots[i].tracker);
            } else {
                printf("encrypt ballot failed");
            }
        }
    }

    // TODO: test simulating multiple encrypters or an encrypter being reset

    // [START] OPTIONAL:

    // When running an encrypter on another device, it is possible to import a file
    // and pass it to the tally functions, but this is not strictly necessary
    // from an API Perspective.

    printf("\n--- Load Ballots ---\n\n");

    // load the ballot ID's so we can cast/spoil them
    char *loaded_external_identifiers[NUM_RANDOM_BALLOTS];

    // Load the data from the file that is created
    struct register_ballot_message loaded_encrypted_ballots[NUM_RANDOM_BALLOTS];

    if (ok) 
    {
        ok = API_LoadBallots(
            0, 
            NUM_RANDOM_BALLOTS, 
            NUM_SELECTIONS, 
            encrypted_ballots_filename, 
            loaded_external_identifiers,
            loaded_encrypted_ballots
        ) == API_LOADBALLOTS_SUCCESS;

        printf("\n--- Validate Ballots ---\n\n");

        // verify the loaded ballots match the ones in memory
        assert(
            loaded_ballot_identifiers_match_encrypted_ballots(
                loaded_external_identifiers, 
                testBallots, 
                NUM_RANDOM_BALLOTS)
        );
        assert(
            loaded_ballots_match_encrypted_ballots(
                loaded_encrypted_ballots, 
                memory_encrypted_ballots, 
                NUM_RANDOM_BALLOTS
            )
        );   
    }

    // TODO: test loading ballots in batches

    // free the ballots we loaded from disk
    // since they are not actually used in this test
    for (uint32_t i = 0; i < NUM_RANDOM_BALLOTS && ok; i++)
        API_EncryptBallot_free(loaded_encrypted_ballots[i], NULL);

    // free the external identifiers we loaded for assertions
    // since they are not actually used in this test
    for (uint32_t i = 0; i < NUM_RANDOM_BALLOTS && ok; i++)
        free(loaded_external_identifiers[i]);

    // [END] OPTIONAL:

    // Register & Record Cast/Spoil Multiple Ballots

    printf("\n\n--- Randomly Assigning Ballots to be Cast or Spoil Arrays ---\n\n");

    uint32_t current_cast_index = 0;
    uint32_t current_spoiled_index = 0;
    char *casted_ballot_ids[NUM_RANDOM_BALLOTS];
    char *spoiled_ballot_ids[NUM_RANDOM_BALLOTS];
    char *memory_external_identifiers[NUM_RANDOM_BALLOTS];

    for (uint32_t i = 0; i < NUM_RANDOM_BALLOTS && ok; i++)
    {
        char *id = testBallots[i].external_identifier;
        memory_external_identifiers[i] = id;
        if (random_bit())
        {
            testBallots[i].isCast = true;
            testBallots[i].isSpoiled = false;
            casted_ballot_ids[current_cast_index] = id;
            
            printf("Ballot Id: %s - Cast!\n", casted_ballot_ids[current_cast_index]);
            current_cast_index++;
        }
        else
        {
            testBallots[i].isCast = false;
            testBallots[i].isSpoiled = true;
            spoiled_ballot_ids[current_spoiled_index] = id;

            printf("Ballot Id: %s - Spoiled!\n", spoiled_ballot_ids[current_spoiled_index]);
            current_spoiled_index++;
        }
    }

    if ((current_cast_index + current_spoiled_index) != NUM_RANDOM_BALLOTS)
    {
        ok = false;
    }

    printf("\n--- Record Ballots (Register, Cast, and Spoil) ---\n\n");

    char *cast_and_spoiled_ballots_filename = NULL;
    char *casted_trackers[current_cast_index];
    char *spoiled_trackers[current_spoiled_index];

    if (ok)
    {
        // Assigning an output_path fails if this folder doesn't already exist
        char *output_path = "./ballots/"; // This outputs to the directy above the cwd.
        char output_prefix[50];

        sprintf(output_prefix, "%s_%d_%d_%d", "registered-ballots", 
        local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday);

        ok = API_RecordBallots(
            config.num_selections, 
            current_cast_index, 
            current_spoiled_index,
            NUM_RANDOM_BALLOTS, 
            casted_ballot_ids,
            spoiled_ballot_ids,
            memory_external_identifiers,
            memory_encrypted_ballots,
            output_path, 
            output_prefix, 
            &cast_and_spoiled_ballots_filename, 
            casted_trackers, 
            spoiled_trackers
        );
                
        if (ok)
        {
            // TODO: assert on outputs not testBallots inputs

            printf("\nCast Ballot Trackers:\n");
            for (uint32_t i = 0; i < current_cast_index; i++)
            {
                printf("\t%s: %s\n", casted_ballot_ids[i], casted_trackers[i]);
            }

            printf("\nSpoiled Ballot Trackers:\n");
            for (uint32_t i = 0; i < current_spoiled_index; i++)
            {
                printf("\t%s: %s\n", spoiled_ballot_ids[i], spoiled_trackers[i]);
            }

            printf("\nBallot registrations and recording of cast/spoil successful!\nCheck output file \"%s\"\n",
                cast_and_spoiled_ballots_filename);
        }
    }

    // Tally Votes & Decrypt Results

    printf("\n--- Tally & Decrypt Votes ---\n\n");

    char *tally_filename = NULL;
    uint32_t tally_results[config.num_selections];

    if (ok)
    {
        char *output_path = "./tallies/"; // output to the directy above the cwd.
        char output_prefix[50];

        sprintf(output_prefix, "%s_%d_%d_%d", "tally", 
        local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday);

        // copy the threshold number of trustees needed to decrypt
        struct trustee_state threshold_trustee_states[MAX_TRUSTEES];
        for (uint32_t i = 0; i < THRESHOLD && ok; i++)
        {
            threshold_trustee_states[i] = trustee_states[i];
            assert(threshold_trustee_states[i].bytes != NULL);
        }

        // run the tally

        printf("Tallying with %d of %d trustees \n\n", DECRYPTING_TRUSTEES, config.num_trustees);

        ok = API_TallyVotes(
            config, 
            threshold_trustee_states,
            DECRYPTING_TRUSTEES,
            cast_and_spoiled_ballots_filename,
            output_path, 
            output_prefix, 
            &tally_filename, 
            tally_results
        );

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

    printf("\n--- Cleaning Up Resources ---\n\n");

    API_TallyVotes_free(tally_filename);

    API_RecordBallots_free(
        cast_and_spoiled_ballots_filename, 
        current_cast_index, 
        current_spoiled_index, 
        casted_trackers, 
        spoiled_trackers
    );

    API_LoadBallots_free(encrypted_ballots_filename);

    for (uint32_t i = 0; i < NUM_RANDOM_BALLOTS && ok; i++)
        API_EncryptBallot_free(memory_encrypted_ballots[i], testBallots[i].tracker);

    API_CreateElection_free(config.joint_key, trustee_states);

    printf("\n--- Done! ---\n\n");

    if (ok)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

bool result_equals_expected_selections(
    struct test_ballot *testBallots, uint32_t actual_tally, uint32_t selection_index) 
{
    uint32_t expected_tally = 0;

    for (uint32_t i = 0; i < NUM_RANDOM_BALLOTS; i++)
    {
        if (testBallots[i].isCast) {
            expected_tally += testBallots[i].selections[selection_index];
        }
    }
    printf("\tselection: %d: expected: %d actual: %d\n", 
        selection_index, expected_tally, actual_tally);
    return expected_tally == actual_tally;
}

bool loaded_ballots_match_encrypted_ballots(
    struct register_ballot_message *loaded_ballots, 
    struct register_ballot_message *encrypted_ballots, 
    uint32_t ballot_count)
{
    printf("\nverifying %d ballots \n", ballot_count);

    bool ok = true;
    for (uint32_t i = 0; i < ballot_count && ok; i++)
    {
        ok = Messages_are_equal(&loaded_ballots[i], &encrypted_ballots[i]);
    }

    if (!ok)
    {
        printf("loaded ballots did not match the encrypted ballots!");
    }

    return ok;
}

bool loaded_ballot_identifiers_match_encrypted_ballots(
    char **loaded_ballot_identifiers, struct test_ballot *test_ballots, uint32_t ballot_count)
{
    printf("\nverifying %d ballot identifiers\n", ballot_count);

    bool ok = true;
    for (uint32_t i = 0; i < ballot_count && ok; i++)
    {
        printf("\ncompare_string:\n - expect: %s\n - actual: %s\n", 
            loaded_ballot_identifiers[i], test_ballots[i].external_identifier);
        ok = compare_string(test_ballots[i].external_identifier, loaded_ballot_identifiers[i]);
    }

    if (!ok)
    {
        printf("\nloaded ballot identifiers did not match the encrypted ballots!\n");
    }

    return ok;
}

bool random_bit() { return 1 & rand(); }

int32_t fill_random_ballot(uint8_t *selections)
{
    // TODO: Handle null votes / expected num selected
    uint32_t selected_count = 0;
    for (uint32_t i = 0; i < NUM_SELECTIONS; i++)
    {
        if (random_bit())
        {
            // write true
            selections[i] = 1;
            selected_count++;
        }
        else
        {
            // write false
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


bool compare_string(char *expected, char *actual) 
{
    while (*expected == *actual) 
    {
        if (*expected == '\0' || *actual == '\0')
        {
            break;
        }
        expected++;
        actual++;
    }

    if (*expected == '\0' && *actual == '\0')
    {
        printf("compare_string: success\n");
        return true;
    } 
    else
    {
        printf("compare_string: failed!\n");
        return false;
    }  
}
