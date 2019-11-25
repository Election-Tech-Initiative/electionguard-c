#ifndef __API_ENCRYPT_BALLOT_H__
#define __API_ENCRYPT_BALLOT_H__

#include <electionguard/api/config.h>
#include <electionguard/voting/encrypter.h>
#include <electionguard/voting/tracker.h>

/**
 * Return struct of result from encrypting. Contains the encrypted ballot,
 * its id, and the tracker already converted to a displayable string */
struct API_EncryptBallot_results
{
    struct register_ballot_message message;
    uint64_t identifier;
    char *tracker_string;
};

/**
 * Encrypts the ballot selections given as an array of booleans, 
 * the serialized joint public key, and the current number of ballots. */
struct API_EncryptBallot_results API_EncryptBallot(bool const *selections,
                                                   struct api_config config,
                                                   struct joint_public_key joint_key,
                                                   uint64_t *current_num_ballots);

/**
 * Free the bytes allocated by EncryptBallot */
void API_EncryptBallot_free(struct register_ballot_message message,
                            char *tracker_string);                             

#endif /* __API_ENCRYPT_BALLOT_H__ */
