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

#define NUM_SELECTIONS 3
#define NUM_RANDOM_BALLOTS 5

int main(int argc, char** argv)
{
    if( argc < 2 )
    {
        printf("Must specify the path where the test files are located\n");
        return -1;
    }

    int retval = 0;
    size_t len = strlen(argv[1]);
    char *lpzTestFilename = malloc( len + 32);
    if( lpzTestFilename == NULL )
    {
        printf("Out of Memory\n");
        return -1;
    }

    char* loaded_external_identifiers[NUM_RANDOM_BALLOTS];
    struct register_ballot_message loaded_encrypted_ballots[NUM_RANDOM_BALLOTS];

    bool passed = false;
    sprintf(lpzTestFilename, "%s/%s", argv[1], "valid");
    API_LoadBallots_status status = API_LoadBallots(
        0, 
        MAX_BALLOT_PAYLOAD, 
        NUM_SELECTIONS, 
        lpzTestFilename, //"/Code/electionguard-c/examples/ballot_parser/ballots/valid", 
        loaded_external_identifiers,
        loaded_encrypted_ballots
    );
    passed = (API_LOADBALLOTS_SUCCESS == status);
    retval += (-1 * passed?0:1);
    printf("Test: \"valid ballot\" - status %d - passed: %d\n", status, passed);

    sprintf(lpzTestFilename, "%s/%s", argv[1], "valid2");
    status = API_LoadBallots(
        0, 
        MAX_BALLOT_PAYLOAD, 
        NUM_SELECTIONS, 
        lpzTestFilename, //"/Code/electionguard-c/examples/ballot_parser/ballots/valid2", 
        loaded_external_identifiers,
        loaded_encrypted_ballots
    );
    passed = (API_LOADBALLOTS_SUCCESS == status);
    retval += (-1 * passed?0:1);
    printf("Test: \"valid ballot, 1 row, no additional newline\" - status %d - passed: %d\n", status, passed);

    sprintf(lpzTestFilename, "%s/%s", argv[1], "boundary_names");
    status = API_LoadBallots(
        0, 
        MAX_BALLOT_PAYLOAD, 
        NUM_SELECTIONS, 
        lpzTestFilename, //"/Code/electionguard-c/examples/ballot_parser/ballots/boundary_names", 
        loaded_external_identifiers,
        loaded_encrypted_ballots
    );
    passed = (API_LOADBALLOTS_SUCCESS == status);
    retval += (-1 * passed?0:1);
    printf("Test: \"valid ballot, external ID boundaries\" - status %d - passed: %d\n", status, passed);

    sprintf(lpzTestFilename, "%s/%s", argv[1], "exploit");
    status = API_LoadBallots(
        0, 
        MAX_BALLOT_PAYLOAD, 
        NUM_SELECTIONS, 
        lpzTestFilename, //"/Code/electionguard-c/examples/ballot_parser/ballots/exploit", 
        loaded_external_identifiers,
        loaded_encrypted_ballots
    );
    passed = (API_LOADBALLOTS_INVALID_DATA_ERROR == status);
    retval += (-1 * passed?0:1);
    printf("Test: \"malformed ballot, should fail to parse\" - status %d - passed: %d\n", status, passed);

    sprintf(lpzTestFilename, "%s/%s", argv[1], "exploit2");
    status = API_LoadBallots(
        0, 
        MAX_BALLOT_PAYLOAD, 
        NUM_SELECTIONS, 
        lpzTestFilename, //"/Code/electionguard-c/examples/ballot_parser/ballots/exploit", 
        loaded_external_identifiers,
        loaded_encrypted_ballots
    );
    passed = (VOTING_COORDINATOR_INVALID_BALLOT_INDEX == status);
    retval += (-1 * passed?0:1);
    printf("Test: \"malformed ballot, should fail to parse\" - status %d - passed: %d\n", status, passed);

    return retval; 
}
