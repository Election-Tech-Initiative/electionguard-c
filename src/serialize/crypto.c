#include "serialize/crypto.h"
#include "serialize/builtins.h"

void Serialize_reserve_key(struct serialize_state *state,
                           struct key const *data)
{
    for (uint32_t i = 0; i < KEY_SIZE; i++)
        Serialize_reserve_uint8(state, &data->bytes[i]);
}

void Serialize_write_key(struct serialize_state *state, struct key const *data)
{
    for (uint32_t i = 0; i < KEY_SIZE; i++)
        Serialize_write_uint8(state, &data->bytes[i]);
}

void Serialize_read_key(struct serialize_state *state, struct key *data)
{
    for (uint32_t i = 0; i < KEY_SIZE; i++)
        Serialize_read_uint8(state, &data->bytes[i]);
}

void Serialize_reserve_private_key(struct serialize_state *state,
                                   struct private_key const *data)
{
    Serialize_reserve_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_reserve_key(state, &data->coefficients[i]);
}

void Serialize_write_private_key(struct serialize_state *state,
                                 struct private_key const *data)
{
    Serialize_write_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_write_key(state, &data->coefficients[i]);
}

void Serialize_read_private_key(struct serialize_state *state,
                                struct private_key *data)
{
    Serialize_read_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_read_key(state, &data->coefficients[i]);
}

void Serialize_reserve_public_key(struct serialize_state *state,
                                  struct public_key const *data)
{
    Serialize_reserve_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_reserve_key(state, &data->coef_committments[i]);
}

void Serialize_write_public_key(struct serialize_state *state,
                                struct public_key const *data)
{
    Serialize_write_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_write_key(state, &data->coef_committments[i]);
}

void Serialize_read_public_key(struct serialize_state *state,
                               struct public_key *data)
{
    Serialize_read_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_read_key(state, &data->coef_committments[i]);
}

void Serialize_reserve_encrypted_key_share(
    struct serialize_state *state, struct encrypted_key_share const *data)
{
    Serialize_reserve_private_key(state, &data->private_key);
    Serialize_reserve_public_key(state, &data->recipient_public_key);
}

void Serialize_write_encrypted_key_share(struct serialize_state *state,
                                         struct encrypted_key_share const *data)
{
    Serialize_write_private_key(state, &data->private_key);
    Serialize_write_public_key(state, &data->recipient_public_key);
}

void Serialize_read_encrypted_key_share(struct serialize_state *state,
                                        struct encrypted_key_share *data)
{
    Serialize_read_private_key(state, &data->private_key);
    Serialize_read_public_key(state, &data->recipient_public_key);
}

void Serialize_reserve_joint_public_key(struct serialize_state *state,
                                        struct joint_public_key_rep const *data)
{
    Serialize_reserve_uint32(state, &data->num_trustees);
    for (size_t i = 0; i < data->num_trustees; i++)
        Serialize_reserve_public_key(state, &data->public_keys[i]);
}

void Serialize_write_joint_public_key(struct serialize_state *state,
                                      struct joint_public_key_rep const *data)
{
    Serialize_write_uint32(state, &data->num_trustees);
    for (size_t i = 0; i < data->num_trustees; i++)
        Serialize_write_public_key(state, &data->public_keys[i]);
}

void Serialize_read_joint_public_key(struct serialize_state *state,
                                     struct joint_public_key_rep *data)
{
    Serialize_read_uint32(state, &data->num_trustees);
    for (size_t i = 0; i < data->num_trustees; i++)
        Serialize_read_public_key(state, &data->public_keys[i]);
}
