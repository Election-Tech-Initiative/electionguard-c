#include "serialize/voting.h"
#include "serialize/builtins.h"

void Serialize_reserve_register_ballot(struct serialize_state *state,
                                       struct register_ballot_rep const *data)
{
    Serialize_reserve_uint64(state, &data->id);
    Serialize_reserve_uint32(state, &data->num_selections);
    for (uint32_t i = 0; i < data->num_selections; i++)
        Serialize_reserve_bool(state, &data->selections[i]);
}

void Serialize_write_register_ballot(struct serialize_state *state,
                                     struct register_ballot_rep const *data)
{
    Serialize_write_uint64(state, &data->id);
    Serialize_write_uint32(state, &data->num_selections);
    for (uint32_t i = 0; i < data->num_selections; i++)
        Serialize_write_bool(state, &data->selections[i]);
}

void Serialize_read_register_ballot(struct serialize_state *state,
                                    struct register_ballot_rep *data)
{
    Serialize_read_uint64(state, &data->id);
    Serialize_read_uint32(state, &data->num_selections);
    for (uint32_t i = 0; i < data->num_selections; i++)
        Serialize_read_bool(state, &data->selections[i]);
}

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
