#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/voting/encrypter.h>

#include "crypto_reps.h"
#include "random_source.h"
#include "serialize/crypto.h"
#include "serialize/state.h"
#include "serialize/voting.h"
#include "voting/message_reps.h"
#include "voting/num_ballots.h"

uint64_t Voting_num_ballots = 0;

struct Voting_Encrypter_s
{
    struct uid uid;
    struct joint_public_key_rep joint_key;
    uint32_t num_selections;
    RandomSource source;
};

enum Voting_Encrypter_status Voting_Encrypter_RandomSource_status_convert(enum RandomSource_status status) {
    switch(status) {
        case RANDOM_SOURCE_SUCCESS: return VOTING_ENCRYPTER_SUCCESS;
        case RANDOM_SOURCE_INSUFFICIENT_MEMORY: return VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
        case RANDOM_SOURCE_IO_ERROR: return VOTING_ENCRYPTER_IO_ERROR;
        default: return VOTING_ENCRYPTER_UNKNOWN_ERROR;
    }
}

enum Voting_Encrypter_status Voting_Encrypter_serialize_read_status_convert(enum serialize_status status) {
    switch(status) {
        case SERIALIZE_STATE_READING: return VOTING_ENCRYPTER_SUCCESS;
        case SERIALIZE_STATE_INSUFFICIENT_MEMORY: return VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
        case SERIALIZE_STATE_BUFFER_TOO_SMALL: return VOTING_ENCRYPTER_DESERIALIZE_ERROR;
        default: return VOTING_ENCRYPTER_UNKNOWN_ERROR;
    }
}

struct Voting_Encrypter_new_r
Voting_Encrypter_new(struct uid uid, struct joint_public_key joint_key,
                     uint32_t num_selections)
{
    struct Voting_Encrypter_new_r result;
    result.encrypter = NULL;
    result.status = VOTING_ENCRYPTER_SUCCESS;

    // Allocate the Encrypter
    result.encrypter = malloc(sizeof(struct Voting_Encrypter_s));
    if (result.encrypter == NULL)
        result.status = VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;

    // Clone the uid
    uint8_t *uid_buf = NULL;
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        uid_buf = malloc(uid.len);
        if (uid_buf == NULL)
            result.status = VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
        else
            memcpy(uid_buf, uid.bytes, uid.len);

        result.encrypter->uid = (struct uid){
            .len = uid.len,
            .bytes = uid_buf,
        };
    }

    // Clone the joint key
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct serialize_state state =
            { .status = SERIALIZE_STATE_READING
            , .len = joint_key.len
            , .offset = 0
            , .buf = (uint8_t *)joint_key.bytes // discard const-ness and pray
            };
        Serialize_read_joint_public_key(&state, &result.encrypter->joint_key);
        result.status = Voting_Encrypter_serialize_read_status_convert(state.status);
    }

    // Get a random source
    RandomSource source = NULL;
    if (result.status == VOTING_ENCRYPTER_SUCCESS) {
        struct RandomSource_new_r rs = RandomSource_new();
        result.status = Voting_Encrypter_RandomSource_status_convert(rs.status);
        if(VOTING_ENCRYPTER_SUCCESS == result.status) {
            source = rs.source;
            result.encrypter->source = rs.source;
        }
    }

    if (result.status == VOTING_ENCRYPTER_SUCCESS)
        result.encrypter->num_selections = num_selections;

    if (VOTING_ENCRYPTER_SUCCESS != result.status) {
        if (NULL != source) RandomSource_free(source);
        if (NULL != uid_buf) free(uid_buf);
        if (NULL != result.encrypter) free(result.encrypter);
    }

    return result;
}

void Voting_Encrypter_free(Voting_Encrypter encrypter)
{
    free((void *)encrypter->uid.bytes);
    RandomSource_free(encrypter->source);
    free((void *)encrypter);
}

enum Voting_Encrypter_status Voting_Encrypter_Crypto_status_convert(enum Crypto_status status) {
    switch(status) {
        case CRYPTO_SUCCESS: return VOTING_ENCRYPTER_SUCCESS;
        case CRYPTO_INSUFFICIENT_MEMORY: return VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
        case CRYPTO_IO_ERROR: return VOTING_ENCRYPTER_IO_ERROR;
        default: return VOTING_ENCRYPTER_UNKNOWN_ERROR;
    }
}

struct Voting_Encrypter_encrypt_ballot_r
Voting_Encrypter_encrypt_ballot(Voting_Encrypter encrypter,
                                bool const *selections)
{
    struct Voting_Encrypter_encrypt_ballot_r result;
    result.status = VOTING_ENCRYPTER_SUCCESS;

    // Construct the ballot id
    {
        struct ballot_identifier_rep rep = {.id = Voting_num_ballots};

        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_ballot_identifier(&state, &rep);
        Serialize_allocate(&state);
        Serialize_write_ballot_identifier(&state, &rep);

        if (state.status != SERIALIZE_STATE_WRITING)
            result.status = VOTING_ENCRYPTER_SERIALIZE_ERROR;
        else
        {
            result.id = (struct ballot_identifier){
                .len = state.len,
                .bytes = state.buf,
            };
        }
    }

    // Construct the ballot tracker
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct ballot_tracker_rep rep = {.id = Voting_num_ballots};

        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_ballot_tracker(&state, &rep);
        Serialize_allocate(&state);
        Serialize_write_ballot_tracker(&state, &rep);

        if (state.status != SERIALIZE_STATE_WRITING)
            result.status = VOTING_ENCRYPTER_SERIALIZE_ERROR;
        else
        {
            result.tracker = (struct ballot_tracker){
                .len = state.len,
                .bytes = state.buf,
            };
        }
    }

    // Construct the message
    struct encrypted_ballot_rep encrypted_ballot;
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct Crypto_encrypted_ballot_new_r result = Crypto_encrypted_ballot_new(encrypter->num_selections, Voting_num_ballots);
        encrypted_ballot = result.result;
        result.status = Voting_Encrypter_Crypto_status_convert(result.status);
    }

    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct uint4096_s uint4096_false_s;
        uint4096_zext_o(&uint4096_false_s, (uint8_t[]){1}, 1);
        const_uint4096 uint4096_true = uint4096_generator_default, uint4096_false = &uint4096_false_s;

        for(uint32_t i = 0; i < encrypter->num_selections; i++) {
            Crypto_encrypt(&encrypted_ballot.selections[i], encrypter->source, &encrypter->joint_key,
                           selections[i] ? uint4096_true : uint4096_false);
        }

        struct serialize_state state =
            { .status = SERIALIZE_STATE_RESERVING
            , .len = 0
            , .offset = 0
            , .buf = NULL
            };
        Serialize_reserve_encrypted_ballot(&state, &encrypted_ballot);
        Serialize_allocate(&state);
        Serialize_write_encrypted_ballot(&state, &encrypted_ballot);

        if (state.status != SERIALIZE_STATE_WRITING)
            result.status = VOTING_ENCRYPTER_SERIALIZE_ERROR;
        else
        {
            result.message = (struct register_ballot_message){
                .len = state.len,
                .bytes = state.buf,
            };
        }
    }

    if (result.status == VOTING_ENCRYPTER_SUCCESS)
        Voting_num_ballots++;

    return result;
}
