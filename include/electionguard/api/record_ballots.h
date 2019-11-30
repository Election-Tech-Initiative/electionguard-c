#ifndef __API_RECORD_BALLOTS_H__
#define __API_RECORD_BALLOTS_H__

#include <electionguard/api/config.h>
#include <electionguard/voting/coordinator.h>

/**
 * Performs Ballot Registration and Recording Cast/Spoil for the ballot in bulk.
 * Operates on the array of encrypted ballot messages, the array of casted ballot ids,
 * and the array of spoiled ballot ids. */
bool API_RecordBallots(uint32_t num_selections,
                       uint32_t num_cast_ballots,
                       uint32_t num_spoil_ballots,
                       uint64_t total_num_ballots,
                       uint64_t *cast_ids,
                       uint64_t *spoil_ids,
                       struct register_ballot_message *encrypted_ballots,
                       char *export_path,
                       char *filename_prefix,
                       char **output_filename);                         

/**
 * Free the bytes allocated by RecordBallots */
void API_RecordBallots_free(char *output_filename);    

#endif /* __API_RECORD_BALLOTS_H__ */
