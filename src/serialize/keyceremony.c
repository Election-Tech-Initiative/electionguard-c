#include "serialize/keyceremony.h"
#include "serialize/builtins.h"
#include "serialize/crypto.h"

void Serialize_reserve_key_generated(struct serialize_state *state,
                                     struct key_generated_rep const *data)
{
    Serialize_reserve_uint32(state, &data->trustee_index);
    Serialize_reserve_public_key(state, &data->public_key);
}

void Serialize_write_key_generated(struct serialize_state *state,
                                   struct key_generated_rep const *data)
{
    Serialize_write_uint32(state, &data->trustee_index);
    Serialize_write_public_key(state, &data->public_key);
}

void Serialize_read_key_generated(struct serialize_state *state,
                                  struct key_generated_rep *data)
{
    Serialize_read_uint32(state, &data->trustee_index);
    Serialize_read_public_key(state, &data->public_key);
}

void Serialize_reserve_all_keys_received(
    struct serialize_state *state, struct all_keys_received_rep const *data)
{
    Serialize_reserve_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_reserve_public_key(state, &data->public_keys[i]);
}

void Serialize_write_all_keys_received(struct serialize_state *state,
                                       struct all_keys_received_rep const *data)
{
    Serialize_write_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_write_public_key(state, &data->public_keys[i]);
}

void Serialize_read_all_keys_received(struct serialize_state *state,
                                      struct all_keys_received_rep *data)
{
    Serialize_read_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_read_public_key(state, &data->public_keys[i]);
}

void Serialize_reserve_shares_generated(struct serialize_state *state,
                                        struct shares_generated_rep const *data)
{
    Serialize_reserve_uint32(state, &data->num_trustees);
    Serialize_reserve_uint32(state, &data->trustee_index);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_reserve_encrypted_key_share(state, &data->shares[i]);
}

void Serialize_write_shares_generated(struct serialize_state *state,
                                      struct shares_generated_rep const *data)
{
    Serialize_write_uint32(state, &data->num_trustees);
    Serialize_write_uint32(state, &data->trustee_index);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_write_encrypted_key_share(state, &data->shares[i]);
}

void Serialize_read_shares_generated(struct serialize_state *state,
                                     struct shares_generated_rep *data)
{
    Serialize_read_uint32(state, &data->num_trustees);
    Serialize_read_uint32(state, &data->trustee_index);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_read_encrypted_key_share(state, &data->shares[i]);
}

void Serialize_reserve_all_shares_received(
    struct serialize_state *state, struct all_shares_received_rep const *data)
{
    Serialize_reserve_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        for (uint32_t j = 0; j < data->num_trustees; j++)
            Serialize_reserve_encrypted_key_share(state, &data->shares[i][j]);
}

void Serialize_write_all_shares_received(
    struct serialize_state *state, struct all_shares_received_rep const *data)
{
    Serialize_write_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        for (uint32_t j = 0; j < data->num_trustees; j++)
            Serialize_write_encrypted_key_share(state, &data->shares[i][j]);
}

void Serialize_read_all_shares_received(struct serialize_state *state,
                                        struct all_shares_received_rep *data)
{
    Serialize_read_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        for (uint32_t j = 0; j < data->num_trustees; j++)
            Serialize_read_encrypted_key_share(state, &data->shares[i][j]);
}

void Serialize_reserve_shares_verified(struct serialize_state *state,
                                       struct shares_verified_rep const *data)
{
    Serialize_reserve_uint32(state, &data->trustee_index);
    Serialize_reserve_bool(state, &data->verified);
}

void Serialize_write_shares_verified(struct serialize_state *state,
                                     struct shares_verified_rep const *data)
{
    Serialize_write_uint32(state, &data->trustee_index);
    Serialize_write_bool(state, &data->verified);
}

void Serialize_read_shares_verified(struct serialize_state *state,
                                    struct shares_verified_rep *data)
{
    Serialize_read_uint32(state, &data->trustee_index);
    Serialize_read_bool(state, &data->verified);
}
