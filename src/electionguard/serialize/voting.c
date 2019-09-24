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
