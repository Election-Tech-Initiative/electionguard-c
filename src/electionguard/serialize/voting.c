#include "serialize/crypto.h"
#include "serialize/voting.h"
#include "serialize/builtins.h"

void Serialize_reserve_ballot_tracker(struct serialize_state *state,
                                      struct ballot_tracker_rep const *data)
{
    Serialize_reserve_uint64(state, &data->id);
}

void Serialize_write_ballot_tracker(struct serialize_state *state,
                                    struct ballot_tracker_rep const *data)
{
    Serialize_write_uint64(state, &data->id);
}

void Serialize_read_ballot_tracker(struct serialize_state *state,
                                   struct ballot_tracker_rep *data)
{
    Serialize_read_uint64(state, &data->id);
}

void Serialize_reserve_ballot_identifier(
    struct serialize_state *state, struct ballot_identifier_rep const *data)
{
    Serialize_reserve_uint64(state, &data->id);
}

void Serialize_write_ballot_identifier(struct serialize_state *state,
                                       struct ballot_identifier_rep const *data)
{
    Serialize_write_uint64(state, &data->id);
}

void Serialize_read_ballot_identifier(struct serialize_state *state,
                                      struct ballot_identifier_rep *data)
{
    Serialize_read_uint64(state, &data->id);
}

bool Serialize_deserialize_register_ballot_message(struct register_ballot_message *ballot_message, 
                                                    struct encrypted_ballot_rep *out_ballot_rep)
{
    struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = ballot_message->len,
            .offset = 0,
            .buf = (uint8_t *)ballot_message->bytes,
        };

    Serialize_read_encrypted_ballot(&state, out_ballot_rep);

    return state.status == SERIALIZE_STATE_READING;
}