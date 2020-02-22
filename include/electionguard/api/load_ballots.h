#ifndef __API_LOAD_BALLOTS_H__
#define __API_LOAD_BALLOTS_H__

#include <electionguard/voting/coordinator.h>

typedef enum API_LoadBallots_status
{
    API_LOADBALLOTS_SUCCESS,
    API_LOADBALLOTS_INITIALIZATION_ERROR,
    API_LOADBALLOTS_INSUFFICIENT_MEMORY,
    API_LOADBALLOTS_INVALID_BALLOT_INDEX,
    API_LOADBALLOTS_END_OF_FILE,
    API_LOADBALLOTS_TIMED_OUT,
    API_LOADBALLOTS_IO_ERROR,
    API_LOADBALLOTS_SERIALIZE_ERROR,
    API_LOADBALLOTS_DESERIALIZE_ERROR,
    API_LOADBALLOTS_INVALID_DATA_ERROR,
    API_LOADBALLOTS_UNDEFINED_ERROR,
} API_LoadBallots_status;

/**
 * Load a range of ballots from a specified file on the file system
 */
API_LoadBallots_status API_LoadBallots(
                    uint64_t start_index, 
                    uint64_t count,
                    uint32_t num_selections,
                    char *import_filepath,
                    char **out_external_identifiers,
                    struct register_ballot_message *out_encrypted_ballots);                         

/**
 * Free the bytes allocated by LoadBallots 
 */
API_LoadBallots_status API_LoadBallots_free(char *output_filename);   

#endif /* __API_LOAD_BALLOTS_H__ */