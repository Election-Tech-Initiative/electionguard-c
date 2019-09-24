#include "serialize/crypto.h"
#include "serialize/builtins.h"
#include <assert.h>
#include <stdlib.h>

void Serialize_reserve_uint4096(struct serialize_state *state,
                                const_uint4096 data)
{
    for (uint32_t i = 0; i < UINT4096_WORD_COUNT; i++)
        Serialize_reserve_uint64(state, NULL);
}

void Serialize_write_uint4096(struct serialize_state *state, const mpz_t data)
{
    uint4096 tmp = export_to_uint4096(data);
    for (uint32_t i = 0; i < UINT4096_WORD_COUNT; i++)
    {
        Serialize_write_uint64(state, &tmp->words[i]);
    }
    free(tmp);
}

void Serialize_reserve_hash(struct serialize_state *state)
{
    //Divide by 8 because were going to do this in 64s
    for (uint32_t i = 0; i < SHA256_DIGEST_LENGTH / 8; i++)
        Serialize_reserve_uint64(state, NULL);
}

void Serialize_write_hash(struct serialize_state *state, struct hash data)
{
    uint64_t *tmp = export_to_256(data.digest);
    for (uint32_t i = 0; i < SHA256_DIGEST_LENGTH / 8; i++)
    {
        Serialize_write_uint64(state, &tmp[i]);
    }
    free(tmp);
}

void Serialize_read_hash(struct serialize_state *state, struct hash *data)
{
    uint8_t *tmp = malloc(sizeof(uint8_t) * 32);
    for (uint32_t i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        Serialize_read_uint8(state, &tmp[i]);
    }
    Crypto_hash_reduce(data, tmp);
    free(tmp);
}

uint8_t *Serialize_reserve_write_hash(struct hash in)
{
    struct serialize_state state = {.status = SERIALIZE_STATE_RESERVING,
                                    .len = 0,
                                    .offset = 0,
                                    .buf = NULL};

    Serialize_reserve_hash(&state);
    Serialize_allocate(&state);
    Serialize_write_hash(&state, in);

    assert(state.len == 32);
    return state.buf;
}

uint8_t *Serialize_reserve_write_bignum(mpz_t in)
{
    struct serialize_state state = {.status = SERIALIZE_STATE_RESERVING,
                                    .len = 0,
                                    .offset = 0,
                                    .buf = NULL};

    Serialize_reserve_uint4096(&state, NULL);
    Serialize_allocate(&state);
    Serialize_write_uint4096(&state, in);

    assert(state.len == 512);
    return state.buf;
}

void Serialize_read_uint4096(struct serialize_state *state, mpz_t data)
{
    uint4096 tmp = malloc(sizeof(struct uint4096_s));
    for (uint32_t i = 0; i < UINT4096_WORD_COUNT; i++)
    {
        Serialize_read_uint64(state, &tmp->words[i]);
    }
    import_uint4096(data, tmp);
    free(tmp);
    // TODO: check that the value is in the right range??
}

void Serialize_reserve_private_key(struct serialize_state *state,
                                   struct private_key const *data)
{
    Serialize_reserve_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_reserve_uint4096(state, NULL);
}

void Serialize_write_private_key(struct serialize_state *state,
                                 struct private_key const *data)
{
    Serialize_write_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
    {
        Serialize_write_uint4096(state, data->coefficients[i]);
    }
}

void Serialize_read_private_key(struct serialize_state *state,
                                struct private_key *data)
{
    Serialize_read_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
    {
        Serialize_read_uint4096(state, data->coefficients[i]);
    }
}

void Serialize_reserve_schnorr_proof(struct serialize_state *state,
                                     struct schnorr_proof const *data)
{
    Serialize_reserve_uint32(state, NULL); //threshold

    for (int i = 0; i < data->threshold; i++)
    {
        Serialize_reserve_uint4096(state, NULL); //commitments
    }
    Serialize_reserve_hash(state); //challenge
    for (int i = 0; i < data->threshold; i++)
    {
        Serialize_reserve_uint4096(state, NULL); //challenge_responses
    }
}

void Serialize_write_schnorr_proof(struct serialize_state *state,
                                   struct schnorr_proof const *data)
{
    Serialize_write_uint32(state, &data->threshold);

    for (int i = 0; i < data->threshold; i++)
    {
        Serialize_write_uint4096(state, data->commitments[i]);
    }
    Serialize_write_hash(state, data->challenge);
    for (int i = 0; i < data->threshold; i++)
    {
        Serialize_write_uint4096(
            state, data->challenge_responses[i]); //challenge_responses
    }
}

void Serialize_read_schnorr_proof(struct serialize_state *state,
                                  struct schnorr_proof *data)
{
    Serialize_read_uint32(state, &data->threshold);

    for (int i = 0; i < data->threshold; i++)
    {
        Serialize_read_uint4096(state, data->commitments[i]);
    }
    Serialize_read_hash(state, &data->challenge);
    for (int i = 0; i < data->threshold; i++)
    {
        Serialize_read_uint4096(
            state, data->challenge_responses[i]); //challenge_responses
    }
}

void Serialize_reserve_public_key(struct serialize_state *state,
                                  struct public_key const *data)
{
    Serialize_reserve_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
        Serialize_reserve_uint4096(state, NULL);
    Serialize_reserve_schnorr_proof(state, &data->proof);
}

void Serialize_write_public_key(struct serialize_state *state,
                                struct public_key const *data)
{
    Serialize_write_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
    {
        Serialize_write_uint4096(state, data->coef_commitments[i]);
    }
    Serialize_write_schnorr_proof(state, &data->proof);
}

void Serialize_read_public_key(struct serialize_state *state,
                               struct public_key *data)
{
    Serialize_read_uint32(state, &data->threshold);
    for (uint32_t i = 0; i < data->threshold; i++)
    {
        Serialize_read_uint4096(state, data->coef_commitments[i]);
    }
    Serialize_read_schnorr_proof(state, &data->proof);
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
    Serialize_reserve_uint4096(state, NULL);
}

void Serialize_write_joint_public_key(struct serialize_state *state,
                                      struct joint_public_key_rep const *data)
{
    Serialize_write_uint32(state, &data->num_trustees);
    Serialize_write_uint4096(state, data->public_key);
}

void Serialize_read_joint_public_key(struct serialize_state *state,
                                     struct joint_public_key_rep *data)
{
    Serialize_read_uint32(state, &data->num_trustees);
    Serialize_read_uint4096(state, data->public_key);
}

void Serialize_reserve_encryption(struct serialize_state *state,
                                  struct encryption_rep const *data)
{
    Serialize_reserve_uint4096(state, NULL);
    Serialize_reserve_uint4096(state, NULL);
}

void Serialize_write_encryption(struct serialize_state *state,
                                struct encryption_rep const *data)
{
    Serialize_write_uint4096(state, data->nonce_encoding);
    Serialize_write_uint4096(state, data->message_encoding);
}

void Serialize_read_encryption(struct serialize_state *state,
                               struct encryption_rep *data)
{
    Serialize_read_uint4096(state, data->nonce_encoding);
    Serialize_read_uint4096(state, data->message_encoding);
}

void Serialize_reserve_encrypted_ballot(struct serialize_state *state,
                                        struct encrypted_ballot_rep const *data)
{
    Serialize_reserve_uint64(state, &data->id);
    Serialize_reserve_uint32(state, &data->num_selections);
    for (uint32_t i = 0; i < data->num_selections; i++)
    {
        Serialize_reserve_encryption(state, &data->selections[i]);
    }
}

void Serialize_write_encrypted_ballot(struct serialize_state *state,
                                      struct encrypted_ballot_rep const *data)
{
    Serialize_write_uint64(state, &data->id);
    Serialize_write_uint32(state, &data->num_selections);
    for (uint32_t i = 0; i < data->num_selections; i++)
    {
        Serialize_write_encryption(state, &data->selections[i]);
    }
}

void Serialize_read_encrypted_ballot(struct serialize_state *state,
                                     struct encrypted_ballot_rep *data)
{
    Serialize_read_uint64(state, &data->id);
    Serialize_read_uint32(state, &data->num_selections);
    data->selections = malloc(data->num_selections * sizeof(*data->selections));
    if (NULL == data->selections)
    {
        state->status = SERIALIZE_STATE_INSUFFICIENT_MEMORY;
    }

    for (uint32_t i = 0;
         i < data->num_selections && SERIALIZE_STATE_READING == state->status;
         i++)
    {
        Crypto_encryption_rep_new(&data->selections[i]);
        Serialize_read_encryption(state, &data->selections[i]);
    }
}
