#ifndef __VOTING_COORDINATOR_H__
#define __VOTING_COORDINATOR_H__

#include <stdio.h>

#include <electionguard/voting/messages.h>
#include <electionguard/voting/tracker.h>

// @todo jwaksbaum What sort of assurances do we make about the
// machine being shut off? How does it persist votes?

typedef struct Voting_Coordinator_s *Voting_Coordinator;

enum Voting_Coordinator_status
{
    VOTING_COORDINATOR_SUCCESS,
    VOTING_COORDINATOR_INSUFFICIENT_MEMORY,
    VOTING_COORDINATOR_INVALID_BALLOT_ID,
    VOTING_COORDINATOR_INVALID_BALLOT_INDEX,
    VOTING_COORDINATOR_INVALID_BALLOT,
    VOTING_COORDINATOR_UNREGISTERED_BALLOT,
    VOTING_COORDINATOR_DUPLICATE_BALLOT,
    VOTING_COORDINATOR_TIMED_OUT_BALLOT,
    VOTING_COORDINATOR_END_OF_FILE,
    VOTING_COORDINATOR_IO_ERROR,
    VOTING_COORDINATOR_ERROR_ALREADY_EXISTS,
    VOTING_COORDINATOR_SERIALIZE_ERROR,
    VOTING_COORDINATOR_DESERIALIZE_ERROR,
};

/************************** INITIALIZATION & FREEING ***************************/

// @todo jwaksbaum Do we want to take in information about the ballot
// formats so that we can validate that the ballots we receive are
// well-formed?

/** 
 * Create a new voting coordinator. 
 * 
 * This component mutates the state of votes by handling loading ballots,
 * registering them, marking them as cast, or spoiled, and caching the state.
 * 
 * The Voting Coordinator can track the state of:
 * -- MAX_BALLOTS it can hold in memory and
 * -- MAX_BALLOT_PAYLOAD slections and trackers it can buffer before writing to cache
 * 
 * @param uint32_t num_selections the total number of selections 
 *                                available on the ballot
*/
struct Voting_Coordinator_new_r Voting_Coordinator_new(uint32_t num_selections);

struct Voting_Coordinator_new_r
{
    enum Voting_Coordinator_status status;
    Voting_Coordinator coordinator;
};

/** 
 * Free the Voting Coordinator
 * 
 * Clears the component state including the tracked state of a ballot box
 * and the buffered selections and trackers being held in memory.
 */
void Voting_Coordinator_free(Voting_Coordinator coordinator);

/**
 * Clear the slections buffer and drop instance references to any external id's
 */
enum Voting_Coordinator_status Voting_Coordinator_clear_buffer(Voting_Coordinator coordinator);

/****************** REGISTERING, CASTING & SPOILING BALLOTS *******************/

/**
 * Register a ballot with the coordinator so that it may be cast or
 * spoiled. 
 */
enum Voting_Coordinator_status
Voting_Coordinator_register_ballot(Voting_Coordinator coordinator,
                                   char *external_identifier,
                                   struct register_ballot_message message,
                                   char **out_ballot_tracker);

/** Mark the ballot specified by ballot_id as cast. */
enum Voting_Coordinator_status
Voting_Coordinator_cast_ballot(Voting_Coordinator coordinator,
                               char *external_identifier,
                               char **out_tracker);

/** Mark the ballot specified by ballot_id as spoiled. */
enum Voting_Coordinator_status
Voting_Coordinator_spoil_ballot(Voting_Coordinator coordinator,
                                char *external_identifier,
                                char **out_tracker);

/** Get the tracker string for the given ballot_id */
char *Voting_Coordinator_get_tracker(Voting_Coordinator coordinator,
                                     char *external_identifier);

/***************************** EXPORTING BALLOTS ******************************/

// @todo jwaksbaum Do we want to return the number of bytes written?

// @todo jwaksbaum What format is it writing in?

/** 
 * Write all of the cast and spoiled ballots to out to the specified file.
 * Clears the Voting Coordinator buffer in the process
 */
enum Voting_Coordinator_status
Voting_Coordinator_export_buffered_ballots(Voting_Coordinator coordinator, FILE *out);

/**
 * Import ballots from the specified file.  Expects a file that was build
 * With the Voting Encrypter using the format:
 *      <ballot_id> TAB <encrypted_ballot_message> \n
 * 
 * @see Voting_Encrypter_write_ballot
 * @param Voting_Coordinator coordinator the voting coordinator instance
 * @param uint64_t start_index the start index to import form the file
 * @param uint64_t count the count of ballots to import
 * @param FILE *in the file to read
 * @param char **out_external_identifiers index-based array of external identifiers
 * @param register_ballot_message out_messages index-based collection of register ballot messages
 * @return Voting_Coordinator_status indicating success or failure
 */
enum Voting_Coordinator_status
Voting_Coordinator_import_encrypted_ballots(Voting_Coordinator coordinator, 
                                            uint64_t start_index, 
                                            uint64_t count,
                                            uint32_t num_selections,
                                            FILE *in,
                                            char **out_external_identifiers,
                                            struct register_ballot_message *out_messages);

#endif /* __VOTING_COORDINATOR_H__ */
