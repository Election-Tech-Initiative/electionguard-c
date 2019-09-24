#ifndef __KEYCEREMONY_COORDINATOR_H__
#define __KEYCEREMONY_COORDINATOR_H__

#include <stdbool.h>
#include <stddef.h>

#include <electionguard/crypto.h>
#include <electionguard/keyceremony/messages.h>

/**
 * Responsible for coordinating communication between the trustees
 * during the key ceremony.
 */
typedef struct KeyCeremony_Coordinator_s *KeyCeremony_Coordinator;

// @todo jwaksbaum Document what these means, maybe even add a way to
// lookup more descriptive error messages using a static array of
// strings

enum KeyCeremony_Coordinator_status
{
    KEYCEREMONY_COORDINATOR_SUCCESS,
    KEYCEREMONY_COORDINATOR_INSUFFICIENT_MEMORY,
    KEYCEREMONY_COORDINATOR_INVALID_PARAMS,
    KEYCEREMONY_COORDINATOR_DUPLICATE_TRUSTEE_INDEX,
    KEYCEREMONY_COORDINATOR_INVALID_TRUSTEE_INDEX,
    KEYCEREMONY_COORDINATOR_MISSING_TRUSTEES,
    KEYCEREMONY_COORDINATOR_TRUSTEE_INVALIDATION,
    KEYCEREMONY_COORDINATOR_SERIALIZE_ERROR,
    KEYCEREMONY_COORDINATOR_DESERIALIZE_ERROR,
};

/************************** INITIALIZATION & FREEING ***************************/

/** Create a new coordinator. */
struct KeyCeremony_Coordinator_new_r
KeyCeremony_Coordinator_new(uint32_t num_trustees, uint32_t threshold);

struct KeyCeremony_Coordinator_new_r
{
    enum KeyCeremony_Coordinator_status status;
    KeyCeremony_Coordinator coordinator;
};

/** Free a coordinator. */
void KeyCeremony_Coordinator_free(KeyCeremony_Coordinator c);

/******************************* KEY_GENERATION ********************************/

// @todo jwaksbaum We need to decide and document precisely when and
// how we will fail in all possible failure circumstances. That
// includes unexpected failures like running out of memory, API
// misuse, and cryptographic proofs that don't verify.
// - Do we fail on receiving a bad message or when finalizing that stage?
// - Do we fail within the coordinator, or do we also produce message
//   that indicate that the trustee should fail as well? If so, we
//   have to make clear that even in case of failure the messages must
//   be communicated to the trustee, or else we might want to delay
//   failing until those messages have already been passed.
// - In the case of failure, do we require that the entire process
//   restart, or do we let them try again from where the left off?

/**
 * Receive a message indicating that a trustee has generated its
 * key-pair, which contains the trustee's public key. Checks the NIZKP
 * of possession of the private key.*/
enum KeyCeremony_Coordinator_status
KeyCeremony_Coordinator_receive_key_generated(
    KeyCeremony_Coordinator c, struct key_generated_message message);

/**
 * Assert that exactly one key_generated_message from each trustee has
 * been received, and generate an all_keys_generated_message
 * containing all of the public keys. */
struct KeyCeremony_Coordinator_all_keys_received_r
KeyCeremony_Coordinator_all_keys_received(KeyCeremony_Coordinator c);

struct KeyCeremony_Coordinator_all_keys_received_r
{
    enum KeyCeremony_Coordinator_status status;
    struct all_keys_received_message message;
};

/****************************** SHARE_GENERATION *******************************/

/**
 * Receive a message containing a trustees encrypted shares of its
 * private key. */
enum KeyCeremony_Coordinator_status
KeyCeremony_Coordinator_receive_shares_generated(
    KeyCeremony_Coordinator c, struct shares_generated_message message);

/**
 * Assert that exactly one shares_generated_message from each trustee
 * has been received, and generate an all_shares_received_message
 * containing all of the encrypted key shares. */
struct KeyCeremony_Coordinator_all_shares_received_r
KeyCeremony_Coordinator_all_shares_received(KeyCeremony_Coordinator c);

struct KeyCeremony_Coordinator_all_shares_received_r
{
    enum KeyCeremony_Coordinator_status status;
    struct all_shares_received_message message;
};

/******************************** VERIFICATION *********************************/

/**
 * Receive a message indicating that a trustee has verified that the
 * key shares it has received are consistent with the commitments in
 * the public keys of each trustee. */
enum KeyCeremony_Coordinator_status
KeyCeremony_Coordinator_receive_shares_verified(
    KeyCeremony_Coordinator c, struct shares_verified_message message);

/**
 * Assert that exactly one shares_verified_message from each trustee
 * has been received, and generate the joint public key to be used to
 * encrypt votes in the election. */
struct KeyCeremony_Coordinator_publish_joint_key_r
KeyCeremony_Coordinator_publish_joint_key(KeyCeremony_Coordinator c);

struct KeyCeremony_Coordinator_publish_joint_key_r
{
    enum KeyCeremony_Coordinator_status status;
    struct joint_public_key key;
};

#endif /* __KEYCEREMONY_COORDINATOR_H__ */
