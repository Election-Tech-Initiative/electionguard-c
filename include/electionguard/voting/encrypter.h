#ifndef __VOTING_ENCRYPTER_H__
#define __VOTING_ENCRYPTER_H__

#include <stdbool.h>

#include <electionguard/crypto.h>
#include <electionguard/voting/messages.h>

typedef struct Voting_Encrypter_s *Voting_Encrypter;

enum Voting_Encrypter_status
{
    VOTING_ENCRYPTER_SUCCESS,
    VOTING_ENCRYPTER_INSUFFICIENT_MEMORY,
    VOTING_ENCRYPTER_SERIALIZE_ERROR,
    VOTING_ENCRYPTER_DESERIALIZE_ERROR,
    VOTING_ENCRYPTER_IO_ERROR,
    VOTING_ENCRYPTER_SELECTION_ERROR,
    // It is a bug for the SDK to produce one of these.
    VOTING_ENCRYPTER_UNKNOWN_ERROR
};

/************************** INITIALIZATION & FREEING ***************************/

struct uid
{
    uint64_t len;
    uint8_t const *bytes;
};

/**
 * Create a new encrypter. Does not transfer ownership of the public
 * key, but creates and allocates a new copy. */
struct Voting_Encrypter_new_r
Voting_Encrypter_new(struct uid uid, struct joint_public_key joint_key,
                     uint32_t num_selections, raw_hash base_hash);

struct Voting_Encrypter_new_r
{
    enum Voting_Encrypter_status status;
    Voting_Encrypter encrypter;
};

/** Free a ballot marking device. */
void Voting_Encrypter_free(Voting_Encrypter encrypter);

/***************************** BALLOT ENCRYPTION ******************************/

// @todo jwaksbaum How do we want to represent an unencrypted ballot?
// An array of booleans? Something more efficient? This might depend
// on what format the crypto wants to receive things in.

// @todo Should we take as input only the selections, and the BMD is
// responsible for adding information like time, unique ID, etc? Or
// should the GUI layer provide any of that info?

/**
 * Encrypt an unencrypted ballot, producing an encrypted ballot, a
 * ballot tracker, and a ballot identifier. The caller must free both
 * when done, ie. they own them. */
struct Voting_Encrypter_encrypt_ballot_r
Voting_Encrypter_encrypt_ballot(Voting_Encrypter encrypter,
                                bool const *selections);

struct Voting_Encrypter_encrypt_ballot_r
{
    enum Voting_Encrypter_status status;
    struct register_ballot_message message;
    struct ballot_tracker tracker;
    struct ballot_identifier id;
};
bool Validate_selections(bool const *selections, uint32_t num_selections, uint32_t *selected_count);

#endif /* __VOTING_ENCRYPTER_H__ */
