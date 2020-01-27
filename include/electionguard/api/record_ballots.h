#ifndef __API_RECORD_BALLOTS_H__
#define __API_RECORD_BALLOTS_H__

#include <electionguard/api/config.h>
#include <electionguard/voting/coordinator.h>

/**
 * Performs Ballot Registration and Recording Cast/Spoil for the ballot in bulk.
 * Operates on the array of encrypted ballot messages, the array of casted ballot ids,
 * and the array of spoiled ballot ids. 
 * uint32_t num_selections,
 * @param uint32_t num_cast_ballots,
 * @param uint32_t num_spoil_ballots,
 * @param uint64_t total_num_ballots,
 * @param char *cast_ids array pointing to the cast id's in external_identifiers
 * @param char *spoil_ids array pointing to the spoiled id's in external_identifiers
 * @param char *external_identifiers index-based array pointing to the external identifiers
 *              associated with encrypted_ballots
 * @param struct register_ballot_message *encrypted_ballots array pointing to serialized encrypted ballots
 * @param char *export_path path to export the ballots file for use when decrypting
 * @param char *filename_prefix previx to use on the file name
 * @param char **output_filename return value of the generated file.  
 *                               when successful, caller is responsible for Freeing output_filename
 * @param char **casted_tracker_strings return value of the trackers that were cast
 * @param char **spoiled_tracker_strings return value of the trackers that were spoiled
 */
bool API_RecordBallots(uint32_t num_selections,
                       uint32_t num_cast_ballots,
                       uint32_t num_spoil_ballots,
                       uint32_t total_num_ballots,
                       char **cast_ids,
                       char **spoil_ids,
                       char **external_identifiers,
                       struct register_ballot_message *encrypted_ballots,
                       char *export_path,
                       char *filename_prefix,
                       char **output_filename,
                       char **casted_tracker_strings,
                       char **spoiled_tracker_strings);                         

/**
 * Free the bytes allocated by RecordBallots */
void API_RecordBallots_free(char *output_filename,
                            uint32_t num_cast_ballots,
                            uint32_t num_spoil_ballots,
                            char **casted_tracker_strings,
                            char **spoiled_tracker_strings);    

#endif /* __API_RECORD_BALLOTS_H__ */
