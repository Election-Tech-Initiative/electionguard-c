#ifndef __CRYPTO_REPS_H__
#define __CRYPTO_REPS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "crypto.h"
#include "max_values.h"

enum Crypto_status
{
    CRYPTO_SUCCESS,
    CRYPTO_INSUFFICIENT_MEMORY,
};

/* A single secret or coefficient, ie. a 4096 bit number for now. */
struct key
{
    uint8_t bytes[KEY_SIZE];
};

/* A private key, including coefficients */
struct private_key
{
    uint32_t threshold;
    struct key coefficients[MAX_TRUSTEES];
};

void Crypto_private_key_copy(struct private_key *dst,
                             struct private_key const *src);

/* A public key, including coefficient commitments */
struct public_key
{
    uint32_t threshold;
    struct key coef_committments[MAX_TRUSTEES];
};

/* Check if two public keys are equal. */
bool Crypto_public_key_equal(struct public_key const *,
                             struct public_key const *);

void Crypto_public_key_copy(struct public_key *dst,
                            struct public_key const *src);

/* Generate a random keypair and return the public and private keys */
struct Crypto_gen_keypair_r Crypto_gen_keypair(uint32_t num_coefficients);

struct Crypto_gen_keypair_r
{
    enum Crypto_status status;
    //@secret The private key must not be leaked from the system.
    struct private_key private_key;
    struct public_key public_key;
};

struct encrypted_key_share
{
    // @todo jwaksbaum In the real implementation, the private key will
    // not be stored here, rather a share of it it will be encrypted
    // using the recipient_public_key.
    // @secret The private key must not be leaked from the system.
    struct private_key private_key;
    struct public_key recipient_public_key;
};

void Crypto_encrypted_key_share_copy(struct encrypted_key_share *dst,
                                     struct encrypted_key_share const *src);

struct joint_public_key_rep
{
    uint32_t num_trustees;
    struct public_key public_keys[MAX_TRUSTEES];
};

void Crypto_joint_public_key_copy(struct joint_public_key_rep *dst,
                                  struct joint_public_key_rep const *src);

/* Create a joint public key from a list of public keys. Copies the
   input keys, so does not take ownership. */
void Crypto_generate_joint_public_key(struct joint_public_key_rep *dst,
                                      struct public_key const *public_keys,
                                      uint32_t num_keys);

#endif /* __CRYPTO_REPS_H__ */
