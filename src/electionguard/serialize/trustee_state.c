#include "serialize/builtins.h"
#include "serialize/crypto.h"
#include "trustee_state_rep.h"

void Serialize_reserve_trustee_state(struct serialize_state *state,
                                     struct trustee_state_rep const *data)
{
    Serialize_reserve_uint32(state, &data->index);
    Serialize_reserve_private_key(state, &data->private_key);
}

void Serialize_write_trustee_state(struct serialize_state *state,
                                   struct trustee_state_rep const *data)
{
    Serialize_write_uint32(state, &data->index);
    Serialize_write_private_key(state, &data->private_key);
}

void Serialize_read_trustee_state(struct serialize_state *state,
                                  struct trustee_state_rep *data)
{
    Serialize_read_uint32(state, &data->index);
    Serialize_read_private_key(state, &data->private_key);
}
