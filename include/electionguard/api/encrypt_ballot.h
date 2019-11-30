#ifndef __API_ENCRYPT_BALLOT_H__
#define __API_ENCRYPT_BALLOT_H__

#include <electionguard/api/config.h>
#include <electionguard/voting/encrypter.h>
#include <electionguard/voting/tracker.h>

/**
 * Encrypts the ballot selections given as an array of booleans, 
 * the serialized joint public key, and the current number of ballots. */
bool API_EncryptBallot(uint16_t *selections,
                       struct api_config config,
                       uint64_t *current_num_ballots,
                       uint64_t *identifier,
                       struct register_ballot_message *encrypted_ballot_message,
                       char **tracker_string);

/**
 * Free the bytes allocated by EncryptBallot */
void API_EncryptBallot_free(struct register_ballot_message message,
                            char *tracker_string);                             

#endif /* __API_ENCRYPT_BALLOT_H__ */
