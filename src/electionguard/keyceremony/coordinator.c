#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/keyceremony/coordinator.h>
#include <electionguard/max_values.h>

#include "keyceremony/message_reps.h"
#include "serialize/crypto.h"
#include "serialize/keyceremony.h"

struct KeyCeremony_Coordinator_s
{
    uint32_t num_trustees;
    uint32_t threshold;
    bool seen[MAX_TRUSTEES];
    struct public_key public_keys[MAX_TRUSTEES];
    struct encrypted_key_share shares[MAX_TRUSTEES][MAX_TRUSTEES];
};

/* Set each entry in seen to false. */
static void KeyCeremony_Coordinator_clear_seen(KeyCeremony_Coordinator c)
{
    memset(c->seen, 0, c->num_trustees * sizeof(bool));
}

/* Return KEYCEREMONY_COORDINATOR_INVALID_TRUSTEE_INDEX if index is
   out of range. Return
   KEYCEREMONY_COORDINATOR_DUPLICATE_TRUSTEE_INDEX if ix has been seen
   before. Otherwise return KEYCEREMONY_COORDINATOR_SUCCESS. */
static enum KeyCeremony_Coordinator_status
KeyCeremony_Coordinator_assert_index_unseen(KeyCeremony_Coordinator c,
                                            uint32_t ix)
{
    if (!(ix < c->num_trustees))
        return KEYCEREMONY_COORDINATOR_INVALID_TRUSTEE_INDEX;
    else if (c->seen[ix])
        return KEYCEREMONY_COORDINATOR_DUPLICATE_TRUSTEE_INDEX;
    else
        return KEYCEREMONY_COORDINATOR_SUCCESS;
}

/* Return KEYCEREMONY_COORDINATOR_MISSING_TRUSTEES if not all trustees have been
   seen. Otherwise return KEYCEREMONY_COORDINATOR_SUCCESS. */
static enum KeyCeremony_Coordinator_status
KeyCeremony_Coordinator_assert_all_seen(KeyCeremony_Coordinator c)
{
    enum KeyCeremony_Coordinator_status result =
        KEYCEREMONY_COORDINATOR_SUCCESS;

    for (uint32_t i = 0;
         i < c->num_trustees && result == KEYCEREMONY_COORDINATOR_SUCCESS; i++)
        if (!c->seen[i])
            result = KEYCEREMONY_COORDINATOR_MISSING_TRUSTEES;

    return result;
}

struct KeyCeremony_Coordinator_new_r
KeyCeremony_Coordinator_new(uint32_t num_trustees, uint32_t threshold)
{
    struct KeyCeremony_Coordinator_new_r result = {
        .status = KEYCEREMONY_COORDINATOR_SUCCESS};

    if (!(1 <= threshold && threshold <= num_trustees &&
          num_trustees <= MAX_TRUSTEES))
        result.status = KEYCEREMONY_COORDINATOR_INVALID_PARAMS;

    // Allocate the coordinator
    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
    {
        result.coordinator = malloc(sizeof(struct KeyCeremony_Coordinator_s));
        if (result.coordinator == NULL)
            result.status = KEYCEREMONY_COORDINATOR_INSUFFICIENT_MEMORY;
    }

    // Initialize the coordinator
    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
    {
        result.coordinator->num_trustees = num_trustees;
        result.coordinator->threshold = threshold;

        KeyCeremony_Coordinator_clear_seen(result.coordinator);

        for(int i=0; i<num_trustees; i++){
            Crypto_public_key_new(&result.coordinator->public_keys[i], threshold);
            for(int j=0; j < num_trustees; j++){
                Crypto_encrypted_key_share_init(&result.coordinator->shares[i][j], threshold);
            }
        }
    }
    return result;
}

void KeyCeremony_Coordinator_free(KeyCeremony_Coordinator c) {
    for(int i=0; i<c->threshold; i++){
        Crypto_public_key_free(&c->public_keys[i],c->threshold);
        for(int j=0; j < c->num_trustees; j++){
            Crypto_encrypted_key_share_free(&c->shares[i][j], c->threshold);
        }
    }
    free(c);
}

enum KeyCeremony_Coordinator_status
KeyCeremony_Coordinator_receive_key_generated(
    KeyCeremony_Coordinator c, struct key_generated_message message)
{
    enum KeyCeremony_Coordinator_status status =
        KEYCEREMONY_COORDINATOR_SUCCESS;

    // Deserialize the message
    struct key_generated_rep message_rep;
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = message.len,
            .offset = 0,
            .buf = (uint8_t *)message.bytes,
        };

        message_rep.public_key.threshold = c->threshold;
        Crypto_public_key_new(&message_rep.public_key, c->threshold);
        Serialize_read_key_generated(&state, &message_rep);

        if (state.status != SERIALIZE_STATE_READING)
            status = KEYCEREMONY_COORDINATOR_DESERIALIZE_ERROR;
    }

    // Check that we haven't already seen this trustee
    if (status == KEYCEREMONY_COORDINATOR_SUCCESS)
        status = KeyCeremony_Coordinator_assert_index_unseen(
            c, message_rep.trustee_index);

    // Check the schnorr proof
    if (status == KEYCEREMONY_COORDINATOR_SUCCESS){
        //We'll need to pass the base hash once that function checks
        //the hash
        Crypto_check_keypair_proof(message_rep.public_key, NULL);
    }


    // Mark message->trustee_index as seen and store the public key
    if (status == KEYCEREMONY_COORDINATOR_SUCCESS)
    {
        c->seen[message_rep.trustee_index] = true;
        Crypto_public_key_copy(&c->public_keys[message_rep.trustee_index],
                               &message_rep.public_key);
        Crypto_public_key_free(&message_rep.public_key,c->threshold); //TODO paradigm for freeing on failure
    }

    return status;
}

struct KeyCeremony_Coordinator_all_keys_received_r
KeyCeremony_Coordinator_all_keys_received(KeyCeremony_Coordinator c)
{
    struct KeyCeremony_Coordinator_all_keys_received_r result;
    result.status = KEYCEREMONY_COORDINATOR_SUCCESS;

    // Check that we've received messages from every trustee
    result.status = KeyCeremony_Coordinator_assert_all_seen(c);

    // Set all in seen to false to prepare for next phase
    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
        KeyCeremony_Coordinator_clear_seen(c);

    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
    {
        // Build the message
        struct all_keys_received_rep message_rep;

        message_rep.num_trustees = c->num_trustees;
        for (uint32_t i = 0; i < c->num_trustees; i++){
            message_rep.public_keys[i].threshold = c->threshold;
            Crypto_public_key_new(&message_rep.public_keys[i],c->threshold);
            Crypto_public_key_copy(&message_rep.public_keys[i],
                                   &c->public_keys[i]);
        }

        // Serialize the message
        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_all_keys_received(&state, &message_rep);
        Serialize_allocate(&state);
        Serialize_write_all_keys_received(&state, &message_rep);

        if (state.status != SERIALIZE_STATE_WRITING)
            result.status = KEYCEREMONY_COORDINATOR_SERIALIZE_ERROR;
        else
        {
            result.message = (struct all_keys_received_message){
                .len = state.len,
                .bytes = state.buf,
            };
        }
         for (uint32_t i = 0; i < c->num_trustees; i++){
            Crypto_public_key_free(&message_rep.public_keys[i], c->threshold);
         }
    }

    return result;
}

enum KeyCeremony_Coordinator_status
KeyCeremony_Coordinator_receive_shares_generated(
    KeyCeremony_Coordinator c, struct shares_generated_message message)
{
    enum KeyCeremony_Coordinator_status status =
        KEYCEREMONY_COORDINATOR_SUCCESS;

    // Deserialize the message
    struct shares_generated_rep message_rep;
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = message.len,
            .offset = 0,
            .buf = (uint8_t *)message.bytes,
        };

        Serialize_read_shares_generated(&state, &message_rep);

        if (state.status != SERIALIZE_STATE_READING)
            status = KEYCEREMONY_COORDINATOR_DESERIALIZE_ERROR;
    }

    // Check that we haven't already seen this trustee
    status = KeyCeremony_Coordinator_assert_index_unseen(
        c, message_rep.trustee_index);

    // Mark message->trustee_index as seen and store the shares
    if (status == KEYCEREMONY_COORDINATOR_SUCCESS)
    {
        c->seen[message_rep.trustee_index] = true;

        // Not doing thresholding
        // for (uint32_t i = 0; i < c->num_trustees; i++)
        //     Crypto_encrypted_key_share_copy(
        //         &c->shares[message_rep.trustee_index][i],
        //         &message_rep.shares[i]);
    }

    return status;
}

struct KeyCeremony_Coordinator_all_shares_received_r
KeyCeremony_Coordinator_all_shares_received(KeyCeremony_Coordinator c)
{
    struct KeyCeremony_Coordinator_all_shares_received_r result;
    result.status = KEYCEREMONY_COORDINATOR_SUCCESS;

    // Check that all trustees have been seen
    result.status = KeyCeremony_Coordinator_assert_all_seen(c);

    // Set all in seen to false to prepare for next phase
    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
        KeyCeremony_Coordinator_clear_seen(c);

    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
    {
        // Build the message
        struct all_shares_received_rep message_rep;

        message_rep.num_trustees = c->num_trustees;
        /*for (uint32_t i = 0; i < c->num_trustees; i++){
            for (uint32_t j = 0; j < c->num_trustees; j++){
                Crypto_encrypted_key_share_init(&message_rep.shares[i][j], c->threshold);
                Crypto_encrypted_key_share_copy(&message_rep.shares[i][j],
                                                &c->shares[i][j]);
            }
        }*/

        // Serialize the message
        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_all_shares_received(&state, &message_rep);
        Serialize_allocate(&state);
        Serialize_write_all_shares_received(&state, &message_rep);

        if (state.status != SERIALIZE_STATE_WRITING)
            result.status = KEYCEREMONY_COORDINATOR_SERIALIZE_ERROR;
        else
        {
            result.message = (struct all_shares_received_message){
                .len = state.len,
                .bytes = state.buf,
            };
        }
    }
    //TODO Free message rep
    return result;
}

enum KeyCeremony_Coordinator_status
KeyCeremony_Coordinator_receive_shares_verified(
    KeyCeremony_Coordinator c, struct shares_verified_message message)
{
    enum KeyCeremony_Coordinator_status status =
        KEYCEREMONY_COORDINATOR_SUCCESS;

    // Deserialize the message
    struct shares_verified_rep message_rep;
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = message.len,
            .offset = 0,
            .buf = (uint8_t *)message.bytes,
        };

        Serialize_read_shares_verified(&state, &message_rep);

        if (state.status != SERIALIZE_STATE_READING)
            status = KEYCEREMONY_COORDINATOR_DESERIALIZE_ERROR;
    }

    // Check that we haven't already seen this trustee
    status = KeyCeremony_Coordinator_assert_index_unseen(
        c, message_rep.trustee_index);

    // Mark message->trustee_index as seen and check verification was
    // successful
    if (status == KEYCEREMONY_COORDINATOR_SUCCESS)
        c->seen[message_rep.trustee_index] = true;

    if (status == KEYCEREMONY_COORDINATOR_SUCCESS)
        if (!message_rep.verified)
            status = KEYCEREMONY_COORDINATOR_TRUSTEE_INVALIDATION;

    return status;
}

struct KeyCeremony_Coordinator_publish_joint_key_r
KeyCeremony_Coordinator_publish_joint_key(KeyCeremony_Coordinator c)
{
    struct KeyCeremony_Coordinator_publish_joint_key_r result;
    result.status = KEYCEREMONY_COORDINATOR_SUCCESS;

    // Check that we've received messages from every trustee
    result.status = KeyCeremony_Coordinator_assert_all_seen(c);

    // Allocate aggregate public key
    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
    {
        // Build the message
        struct joint_public_key_rep joint_key;
        Crypto_joint_public_key_init(&joint_key);
        Crypto_generate_joint_public_key(&joint_key, c->public_keys,
                                         c->num_trustees);

        // Serialize the message
        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_joint_public_key(&state, &joint_key);
        Serialize_allocate(&state);
        Serialize_write_joint_public_key(&state, &joint_key);

        if (state.status != SERIALIZE_STATE_WRITING)
            result.status = KEYCEREMONY_COORDINATOR_SERIALIZE_ERROR;
        else
        {
            result.key = (struct joint_public_key){
                .len = state.len,
                .bytes = state.buf,
            };
        }

        Crypto_joint_public_key_free(&joint_key);
    }

    return result;
}
