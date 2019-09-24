#ifndef __KEYCEREMONY_TRUSTEE_H__
#define __KEYCEREMONY_TRUSTEE_H__

#include <stdint.h>

#include "crypto.h"
#include "keyceremony/messages.h"
#include "max_values.h"
#include "trustee_state.h"

typedef struct KeyCeremony_Trustee_s *KeyCeremony_Trustee;

enum KeyCeremony_Trustee_status
{
    KEYCEREMONY_TRUSTEE_SUCCESS,
    KEYCEREMONY_TRUSTEE_INSUFFICIENT_MEMORY,
    KEYCEREMONY_TRUSTEE_INVALID_PARAMS,
    KEYCEREMONY_TRUSTEE_PUBLISHED_PUBLIC_KEY_ERROR,
    KEYCEREMONY_TRUSTEE_MISSING_PUBLIC_KEY,
    KEYCEREMONY_TRUSTEE_BAD_NIZKP,
    KEYCEREMONY_TRUSTEE_INVALID_KEY_SHARE,
    KEYCEREMONY_TRUSTEE_SERIALIZE_ERROR,
    KEYCEREMONY_TRUSTEE_DESERIALIZE_ERROR,
};

/************************** INITIALIZATION & FREEING ***************************/

/** Create an new trustee. */
struct KeyCeremony_Trustee_new_r KeyCeremony_Trustee_new(uint32_t num_trustees,
                                                         uint32_t threshold,
                                                         uint32_t index);

struct KeyCeremony_Trustee_new_r
{
    enum KeyCeremony_Trustee_status status;
    KeyCeremony_Trustee trustee;
};

/** Free a trustee. */
void KeyCeremony_Trustee_free(KeyCeremony_Trustee t);

/******************************* KEY_GENERATION ********************************/

/**
 * Generate a key pair and return the key_generated_message to be
 * passed to the coordinator. */
struct KeyCeremony_Trustee_generate_key_r
KeyCeremony_Trustee_generate_key(KeyCeremony_Trustee t);

struct KeyCeremony_Trustee_generate_key_r
{
    enum KeyCeremony_Trustee_status status;
    struct key_generated_message message;
};

/****************************** SHARE_GENERATION *******************************/

/**
 * Verify in_message to ensure:
 *   - that this trustee's public key is present
 *   - that any NIZKPs are valid
 *
 * Then, compute and encrypt the shares of this trustee's private key
 * for the other trustees. */
struct KeyCeremony_Trustee_generate_shares_r
KeyCeremony_Trustee_generate_shares(
    KeyCeremony_Trustee t, struct all_keys_received_message in_message);

struct KeyCeremony_Trustee_generate_shares_r
{
    enum KeyCeremony_Trustee_status status;
    struct shares_generated_message message;
};

/******************************** VERIFICATION *********************************/

/**
 * Verify that the private key shares in in_message match the
 * commitments in the previously received public keys. */
struct KeyCeremony_Trustee_verify_shares_r KeyCeremony_Trustee_verify_shares(
    KeyCeremony_Trustee t, struct all_shares_received_message in_message);

struct KeyCeremony_Trustee_verify_shares_r
{
    enum KeyCeremony_Trustee_status status;
    struct shares_verified_message message;
};

/********************************* STATE EXPORT ********************************/

/**
 * Export the portion of the trustee's state that will be necessary
 * for decryption. */
struct KeyCeremony_Trustee_export_state_r
KeyCeremony_Trustee_export_state(KeyCeremony_Trustee t);

struct KeyCeremony_Trustee_export_state_r
{
    enum KeyCeremony_Trustee_status status;
    struct trustee_state state;
};

#endif /* __KEYCEREMONY_TRUSTEE_H__ */
