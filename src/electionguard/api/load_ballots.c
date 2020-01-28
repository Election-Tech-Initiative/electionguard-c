#include <stdlib.h>

#include <electionguard/api/load_ballots.h>

#include <log.h>

#include "api/base_hash.h"
#include "directory.h"
#include "api/filename.h"

static API_LoadBallots_status initialize_coordinator(uint32_t num_selections);
static API_LoadBallots_status load_ballots(uint64_t start_index, 
                    uint64_t count,
                    uint32_t num_selections,
                    char *import_filepath,
                    char **out_external_identifiers,
                    struct register_ballot_message *out_encrypted_ballots);

// Global state
static Voting_Coordinator _load_coordinator = NULL;

/**
 * Load a range of ballots from a specified file on the file system
 */
API_LoadBallots_status API_LoadBallots(
                    uint64_t start_index, 
                    uint64_t count,
                    uint32_t num_selections,
                    char *import_filepath,
                    char **out_external_identifiers,
                    struct register_ballot_message *out_encrypted_ballots)
{
    enum API_LoadBallots_status status = API_LOADBALLOTS_SUCCESS;

    // Set global variables
    Crypto_parameters_new();
    
    if (_load_coordinator == NULL)
    {
        status = initialize_coordinator(num_selections);
    }

    if (status == API_LOADBALLOTS_SUCCESS)
    {
        status = load_ballots(
            start_index, 
            count, 
            num_selections, 
            import_filepath, 
            out_external_identifiers, 
            out_encrypted_ballots
        );
    }

    // Unlike other API Methods, we do not call
    // Voting_Coordinator_free, because that component
    // is used both when loading ballots and
    // during record/cast/spoil.  

    Crypto_parameters_free();

    return status;
}                     

/**
 * Free the bytes allocated by LoadBallots 
 */
API_LoadBallots_status API_LoadBallots_free(char *output_filename)
{
    enum API_LoadBallots_status status = API_LOADBALLOTS_SUCCESS;

    if (output_filename != NULL)
    {
        free(output_filename);
    }

    if (_load_coordinator != NULL)
    {
        Voting_Coordinator_free(_load_coordinator);
        _load_coordinator = NULL;
    }

    return status;
}

API_LoadBallots_status initialize_coordinator(uint32_t num_selections)
{
    struct Voting_Coordinator_new_r result =
        Voting_Coordinator_new(num_selections);

    if (result.status != VOTING_COORDINATOR_SUCCESS)
        return API_LOADBALLOTS_INITIALIZATION_ERROR;
    else
    {
        _load_coordinator = result.coordinator;
        return API_LOADBALLOTS_SUCCESS;
    }
}

API_LoadBallots_status load_ballots(uint64_t start_index, 
                    uint64_t count,
                    uint32_t num_selections,
                    char *import_filepath,
                    char **out_external_identifiers,
                    struct register_ballot_message *out_encrypted_ballots)
{
    // open the file for read
    FILE *in = fopen(import_filepath, "r");
    if (in == NULL) 
    {
        INFO_PRINT(("API_LoadBallots: load_ballots error accessing file\n"));
        return API_LOADBALLOTS_IO_ERROR;
    }

    int seek_status = fseek(in, 0, SEEK_SET);
    if (seek_status != 0)
    {
        INFO_PRINT(("API_LoadBallots: load_ballots error seeking file\n"));
        fclose(in);
        return API_LOADBALLOTS_IO_ERROR;
    }

    enum Voting_Coordinator_status load_result = VOTING_COORDINATOR_SUCCESS;

    load_result = Voting_Coordinator_import_encrypted_ballots(
        _load_coordinator,
        start_index,
        count,
        num_selections,
        in,
        out_external_identifiers,
        out_encrypted_ballots
    );

    if (in != NULL)
    {
        fclose(in);
        in = NULL;
    }

    switch (load_result)
    {
        case VOTING_COORDINATOR_INSUFFICIENT_MEMORY:
            return API_LOADBALLOTS_INSUFFICIENT_MEMORY;
        // TODO: other cases
        case VOTING_COORDINATOR_SUCCESS:
        default:
            return API_LOADBALLOTS_SUCCESS;
    }
}
