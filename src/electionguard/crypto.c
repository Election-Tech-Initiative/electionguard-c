#include <stdlib.h>
#include <string.h>

#include <electionguard/crypto.h>

#include "random_source.h"
#include "crypto_reps.h"

bool Crypto_public_key_equal(struct public_key const *key1,
                             struct public_key const *key2)
{
    bool ok = true;

    if (key1->threshold != key2->threshold)
        ok = false;

    for (uint32_t i = 0; i < key1->threshold && ok; i++)
        ok = ok && uint4096_eq(&key1->coef_commitments[i],
                               &key2->coef_commitments[i]);

    return ok;
}

enum Crypto_status Crypto_RandomSource_status_convert(enum RandomSource_status status) {
    switch(status) {
        case RANDOM_SOURCE_SUCCESS: return CRYPTO_SUCCESS;
        case RANDOM_SOURCE_INSUFFICIENT_MEMORY: return CRYPTO_INSUFFICIENT_MEMORY;
        case RANDOM_SOURCE_IO_ERROR: return CRYPTO_IO_ERROR;
        // should never happen
        default: return CRYPTO_UNKNOWN_ERROR;
    }
}

struct Crypto_gen_keypair_r Crypto_gen_keypair(uint32_t num_coefficients)
{
    struct Crypto_gen_keypair_r result;
    result.status = CRYPTO_SUCCESS;

    result.private_key.threshold = num_coefficients;
    result.public_key.threshold = num_coefficients;

    RandomSource source;
    if(CRYPTO_SUCCESS == result.status) {
        struct RandomSource_new_r source_r = RandomSource_new();
        source = source_r.source;
        result.status = Crypto_RandomSource_status_convert(source_r.status);
    }

    // Set all bytes to a non-zero value to help catch bugs. Before I
    // did this, I had a couple of bugs related to actually copying
    // the keys around, but because everything was zeroes nothing
    // broke.
    for (uint32_t i = 0; i < num_coefficients; i++)
    {
        if(CRYPTO_SUCCESS == result.status) {
            result.status = Crypto_RandomSource_status_convert(RandomSource_uniform_o(source, &result.private_key.coefficients[i]));
            if(CRYPTO_SUCCESS != result.status) {
                RandomSource_free(source);
            }
        }

        if(CRYPTO_SUCCESS == result.status) {
            uint4096_powmod_o
                ( &result.public_key.coef_commitments[i]
                , uint4096_generator_default
                , &result.private_key.coefficients[i]
                , Modulus4096_modulus_default
                );
        }
    }

    if(CRYPTO_SUCCESS == result.status) {
        RandomSource_free(source);
    }

    return result;
}

void Crypto_private_key_copy(struct private_key *dst,
                             struct private_key const *src)
{
    dst->threshold = src->threshold;
    for (uint32_t i = 0; i < src->threshold; i++)
        uint4096_copy_o(&dst->coefficients[i], &src->coefficients[i]);
}

void Crypto_public_key_copy(struct public_key *dst,
                            struct public_key const *src)
{
    dst->threshold = src->threshold;
    for (uint32_t i = 0; i < src->threshold; i++)
        uint4096_copy_o(&dst->coef_commitments[i], &src->coef_commitments[i]);
}

void Crypto_encrypted_key_share_copy(struct encrypted_key_share *dst,
                                     struct encrypted_key_share const *src)
{
    Crypto_private_key_copy(&dst->private_key, &src->private_key);
    Crypto_public_key_copy(&dst->recipient_public_key,
                           &src->recipient_public_key);
}

void Crypto_joint_public_key_copy(struct joint_public_key_rep *dst,
                                  struct joint_public_key_rep const *src)
{
    dst->num_trustees = src->num_trustees;
    uint4096_copy_o(&dst->public_key, &src->public_key);
}

void Crypto_generate_joint_public_key(struct joint_public_key_rep *dst,
                                      struct public_key const *public_keys,
                                      uint32_t num_keys)
{
    dst->num_trustees = num_keys;
    uint4096_zext_o(&dst->public_key, (uint8_t[]){1}, 1);
    for (uint32_t i = 0; i < num_keys; i++)
        uint4096_multmod_o(&dst->public_key, &dst->public_key, &public_keys[i].coef_commitments[0], Modulus4096_modulus_default);
}

void Crypto_encrypt(struct encryption_rep *out, RandomSource source, const struct joint_public_key_rep *key, const_uint4096 message) {
    struct uint4096_s r;
    RandomSource_uniform_o(source, &r);
    uint4096_powmod_o(&out->nonce_encoding, uint4096_generator_default, &r, Modulus4096_modulus_default);
    uint4096_powmod_o(&out->message_encoding, &key->public_key, &r, Modulus4096_modulus_default);
    uint4096_multmod_o(&out->message_encoding, &out->message_encoding, message, Modulus4096_modulus_default);
}

void Crypto_encryption_homomorphic_zero(struct encryption_rep *out) {
    uint4096_zext_o(&out->nonce_encoding, (uint8_t[]){1}, 1);
    uint4096_zext_o(&out->message_encoding, (uint8_t[]){1}, 1);
}

void Crypto_encryption_homomorphic_add(struct encryption_rep *out, const struct encryption_rep *a, const struct encryption_rep *b) {
    uint4096_multmod_o(&out->nonce_encoding, &a->nonce_encoding, &b->nonce_encoding, Modulus4096_modulus_default);
    uint4096_multmod_o(&out->message_encoding, &a->message_encoding, &b->message_encoding, Modulus4096_modulus_default);
}

bool Crypto_encryption_fprint(FILE *out, const struct encryption_rep *rep) {
    bool ok = true;

    if(ok) ok = fprintf(out, "(") == 1;
    if(ok) ok = uint4096_fprint(out, &rep->nonce_encoding);
    if(ok) ok = fprintf(out, ",") == 1;
    if(ok) ok = uint4096_fprint(out, &rep->message_encoding);
    if(ok) ok = fprintf(out, ")") == 1;

    return ok;
}

struct Crypto_encrypted_ballot_new_r Crypto_encrypted_ballot_new(uint32_t num_selections, uint64_t id) {
    struct Crypto_encrypted_ballot_new_r result;
    result.result.id = id;
    result.result.num_selections = num_selections;
    result.result.selections = malloc(num_selections * sizeof(*result.result.selections));
    if(NULL == result.result.selections)
        result.status = CRYPTO_INSUFFICIENT_MEMORY;
    return result;
}

void Crypto_encrypted_ballot_free(struct encrypted_ballot_rep *ballot) {
    free(ballot->selections);
}
