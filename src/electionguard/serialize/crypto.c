#include <stdlib.h>
#include "serialize/crypto.h"
#include "serialize/builtins.h"

void Serialize_reserve_uint4096(struct serialize_state *state, const_uint4096 data)
{
    for (uint32_t i = 0; i < UINT4096_WORD_COUNT; i++)
        Serialize_reserve_uint64(state, &data->words[i]);
}

void Serialize_write_uint4096(struct serialize_state *state, const_uint4096 data)
{
    for (uint32_t i = 0; i < UINT4096_WORD_COUNT; i++)
        Serialize_write_uint64(state, &data->words[i]);
}

void Serialize_read_uint4096(struct serialize_state *state, uint4096 data)
{
    for (uint32_t i = 0; i < UINT4096_WORD_COUNT; i++)
        Serialize_read_uint64(state, &data->words[i]);
    // TODO: check that the key is in the right range??
}

void Serialize_reserve_private_key(struct serialize_state *state,
                                   struct private_key const *data)
{
    Serialize_reserve_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_reserve_uint4096(state, &data->coefficients[i]);
}

void Serialize_write_private_key(struct serialize_state *state,
                                 struct private_key const *data)
{
    Serialize_write_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_write_uint4096(state, &data->coefficients[i]);
}

void Serialize_read_private_key(struct serialize_state *state,
                                struct private_key *data)
{
    Serialize_read_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_read_uint4096(state, &data->coefficients[i]);
}

void Serialize_reserve_public_key(struct serialize_state *state,
                                  struct public_key const *data)
{
    Serialize_reserve_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_reserve_uint4096(state, &data->coef_commitments[i]);
}

void Serialize_write_public_key(struct serialize_state *state,
                                struct public_key const *data)
{
    Serialize_write_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_write_uint4096(state, &data->coef_commitments[i]);
}

void Serialize_read_public_key(struct serialize_state *state,
                               struct public_key *data)
{
    Serialize_read_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_read_uint4096(state, &data->coef_commitments[i]);
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
    Serialize_reserve_uint4096(state, &data->public_key);
}

void Serialize_write_joint_public_key(struct serialize_state *state,
                                      struct joint_public_key_rep const *data)
{
    Serialize_write_uint32(state, &data->num_trustees);
    Serialize_write_uint4096(state, &data->public_key);
}

void Serialize_read_joint_public_key(struct serialize_state *state,
                                     struct joint_public_key_rep *data)
{
    Serialize_read_uint32(state, &data->num_trustees);
    Serialize_read_uint4096(state, &data->public_key);
}

void Serialize_reserve_encryption(struct serialize_state *state,
                                  struct encryption_rep const *data)
{
    Serialize_reserve_uint4096(state, &data->nonce_encoding);
    Serialize_reserve_uint4096(state, &data->message_encoding);
}

void Serialize_write_encryption(struct serialize_state *state,
                                struct encryption_rep const *data)
{
    Serialize_write_uint4096(state, &data->nonce_encoding);
    Serialize_write_uint4096(state, &data->message_encoding);
}

void Serialize_read_encryption(struct serialize_state *state,
                               struct encryption_rep *data)
{
    Serialize_read_uint4096(state, &data->nonce_encoding);
    Serialize_read_uint4096(state, &data->message_encoding);
}

void Serialize_reserve_encrypted_ballot(struct serialize_state *state,
                                        struct encrypted_ballot_rep const *data)
{
    Serialize_reserve_uint64(state, &data->id);
    Serialize_reserve_uint32(state, &data->num_selections);
    for(uint32_t i = 0; i < data->num_selections; i++) {
        Serialize_reserve_encryption(state, &data->selections[i]);
    }
}

void Serialize_write_encrypted_ballot(struct serialize_state *state,
                                      struct encrypted_ballot_rep const *data)
{
    Serialize_write_uint64(state, &data->id);
    Serialize_write_uint32(state, &data->num_selections);
    for(uint32_t i = 0; i < data->num_selections; i++) {
        Serialize_write_encryption(state, &data->selections[i]);
    }
}

void Serialize_read_encrypted_ballot(struct serialize_state *state,
                                     struct encrypted_ballot_rep *data)
{
    Serialize_read_uint64(state, &data->id);
    Serialize_read_uint32(state, &data->num_selections);
    data->selections = malloc(data->num_selections * sizeof(*data->selections));
    if(NULL == data->selections) {
        state->status = SERIALIZE_STATE_INSUFFICIENT_MEMORY;
    }

    for(uint32_t i = 0; i < data->num_selections && SERIALIZE_STATE_READING == state->status; i++) {
        Serialize_read_encryption(state, &data->selections[i]);
    }
}
