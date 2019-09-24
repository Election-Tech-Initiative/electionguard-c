#include <stdlib.h>
#include <string.h>

#include "crypto.h"
#include "crypto_reps.h"

bool Crypto_public_key_equal(struct public_key const *key1,
                             struct public_key const *key2)
{
    bool ok = true;

    if (key1->threshold != key2->threshold)
        ok = false;

    for (uint32_t i = 0; i < key1->threshold && ok; i++)
        for (uint32_t j = 0; j < KEY_SIZE && ok; j++)
            if (key1->coef_committments[i].bytes[j] !=
                key2->coef_committments[i].bytes[j])
                ok = false;

    return ok;
}

struct Crypto_gen_keypair_r Crypto_gen_keypair(uint32_t num_coefficients)
{
    struct Crypto_gen_keypair_r result;
    result.status = CRYPTO_SUCCESS;

    result.private_key.threshold = num_coefficients;
    result.public_key.threshold = num_coefficients;

    // Set all bytes to a non-zero value to help catch bugs. Before I
    // did this, I had a couple of bugs related to actually copying
    // the keys around, but because everything was zeroes nothing
    // broke.
    for (uint32_t i = 0; i < num_coefficients; i++)
    {
        for (uint32_t j = 0; j < KEY_SIZE; j++)
        {
            result.private_key.coefficients[i].bytes[j] = rand();
            result.public_key.coef_committments[i].bytes[j] = rand();
        }
    }

    return result;
}

void Crypto_private_key_copy(struct private_key *dst,
                             struct private_key const *src)
{
    dst->threshold = src->threshold;
    for (uint32_t i = 0; i < src->threshold; i++)
        memcpy(&dst->coefficients[i].bytes, &src->coefficients[i].bytes,
               KEY_SIZE * sizeof(uint8_t));
}

void Crypto_public_key_copy(struct public_key *dst,
                            struct public_key const *src)
{
    dst->threshold = src->threshold;
    for (uint32_t i = 0; i < src->threshold; i++)
        memcpy(&dst->coef_committments[i].bytes,
               &src->coef_committments[i].bytes, KEY_SIZE * sizeof(uint8_t));
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
    for (uint32_t i = 0; i < src->num_trustees; i++)
        Crypto_public_key_copy(&dst->public_keys[i], &src->public_keys[i]);
}

void Crypto_generate_joint_public_key(struct joint_public_key_rep *dst,
                                      struct public_key const *public_keys,
                                      uint32_t num_keys)
{
    dst->num_trustees = num_keys;
    for (uint32_t i = 0; i < num_keys; i++)
        Crypto_public_key_copy(&dst->public_keys[i], &public_keys[i]);
}
