#ifndef __API_ENCRYPT_BALLOT_H__
#define __API_ENCRYPT_BALLOT_H__

#include <electionguard/api/config.h>
#include <electionguard/voting/encrypter.h>
#include <electionguard/voting/tracker.h>

/**
 * Encrypts the ballot selections given as an array of booleans, 
 * the serialized joint public key, and the current number of ballots.
 * @param uint8_t *selections_byte_array a byte array representing the selections on the ballot
 * @param uint32_t expected_num_selected the expected number of true values in *selections_byte_array
 * @param api_config config the election configuration
 * @param char *external_identifier an arbitrary value meaninful to the library consumer that is associated
 *                                  with each encrypted ballot
 * @param char *export_path
 * @param char *filename
 * @param char **output_filename return value of the generated file.  
 *                               when successful, caller is responsible for Freeing output_filename
 * @param char **tracker_string representation of the ballot's tracking Id after encryption
 */
bool API_EncryptBallot(uint8_t *selections_byte_array,
                       uint32_t expected_num_selected,
                       struct api_config config,
                       char *external_identifier,
                       struct register_ballot_message *encrypted_ballot_message,
                       char *export_path,
                       char *filename,
                       char **output_filename,
                       char **tracker_string);

// TODO: API endpoint that does not use the file system

/**
 * Soft delete a file by renaming it with the current system time
 */
bool API_EncryptBallot_soft_delete_file(char *export_path, char *filename);

/**
 * Free the bytes allocated by EncryptBallot 
 */
void API_EncryptBallot_free(struct register_ballot_message message,
                            char *tracker_string);                             

#endif /* __API_ENCRYPT_BALLOT_H__ */
