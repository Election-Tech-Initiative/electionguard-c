#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "crypto_reps.h"
#include "serialize/state.h"
#include "serialize/voting.h"
#include "voting/encrypter.h"
#include "voting/message_reps.h"
#include "voting/num_ballots.h"

uint64_t Voting_num_ballots = 0;

struct Voting_Encrypter_s
{
    struct uid uid;
    struct joint_public_key joint_key;
    uint32_t num_selections;
};

struct Voting_Encrypter_new_r
Voting_Encrypter_new(struct uid uid, struct joint_public_key joint_key,
                     uint32_t num_selections)
{
    struct Voting_Encrypter_new_r result;
    result.status = VOTING_ENCRYPTER_SUCCESS;

    // Allocate the Encrypter
    result.encrypter = malloc(sizeof(struct Voting_Encrypter_s));
    if (result.encrypter == NULL)
        result.status = VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;

    // Clone the uid
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        uint8_t *buf = malloc(uid.len);
        if (buf == NULL)
            result.status = VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
        else
            memcpy(buf, uid.bytes, uid.len);

        result.encrypter->uid = (struct uid){
            .len = uid.len,
            .bytes = buf,
        };
    }

    // Clone the joint key
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        uint8_t *buf = malloc(joint_key.len);
        if (buf == NULL)
            result.status = VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
        else
            memcpy(buf, joint_key.bytes, joint_key.len);

        result.encrypter->joint_key = (struct joint_public_key){
            .len = joint_key.len,
            .bytes = buf,
        };
    }

    if (result.status == VOTING_ENCRYPTER_SUCCESS)
        result.encrypter->num_selections = num_selections;

    return result;
}

void Voting_Encrypter_free(Voting_Encrypter encrypter)
{
    free((void *)encrypter->uid.bytes);
    free((void *)encrypter->joint_key.bytes);
    free((void *)encrypter);
}

struct Voting_Encrypter_encrypt_ballot_r
Voting_Encrypter_encrypt_ballot(Voting_Encrypter encrypter,
                                bool const *selections)
{
    // Suppress compiler warning that encrypter is unused
    (void)encrypter;

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
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct register_ballot_rep rep;
        rep.id = Voting_num_ballots;
        rep.num_selections = encrypter->num_selections;
        memcpy(&rep.selections, selections,
               encrypter->num_selections * sizeof(bool));

        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_register_ballot(&state, &rep);
        Serialize_allocate(&state);
        Serialize_write_register_ballot(&state, &rep);

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
