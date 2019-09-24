#ifndef __CRYPTO_REPS_H__
#define __CRYPTO_REPS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <electionguard/crypto.h>
#include <electionguard/max_values.h>
#include "random_source.h"
#include "uint4096.h"

enum Crypto_status
{
    CRYPTO_SUCCESS,
    CRYPTO_INSUFFICIENT_MEMORY,
    CRYPTO_IO_ERROR,
    // it is a bug for the SDK to generate one of these
    CRYPTO_UNKNOWN_ERROR
};

/* A private key, including coefficients */
struct private_key
{
    uint32_t threshold;
    struct uint4096_s coefficients[MAX_TRUSTEES];
};

void Crypto_private_key_copy(struct private_key *dst,
                             struct private_key const *src);

/* A public key, including coefficient commitments */
struct public_key
{
    uint32_t threshold;
    struct uint4096_s coef_commitments[MAX_TRUSTEES];
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
    struct uint4096_s public_key;
};

void Crypto_joint_public_key_copy(struct joint_public_key_rep *dst,
                                  struct joint_public_key_rep const *src);

/* Create a joint public key from a list of public keys. Copies the
   input keys, so does not take ownership. */
void Crypto_generate_joint_public_key(struct joint_public_key_rep *dst,
                                      struct public_key const *public_keys,
                                      uint32_t num_keys);

struct encryption_rep
{
    struct uint4096_s nonce_encoding;
    struct uint4096_s message_encoding;
};

void Crypto_encrypt(struct encryption_rep *out, RandomSource source, const struct joint_public_key_rep *key, const_uint4096 message);
void Crypto_encryption_homomorphic_zero(struct encryption_rep *out);
void Crypto_encryption_homomorphic_add(struct encryption_rep *out, const struct encryption_rep *a, const struct encryption_rep *b);

bool Crypto_encryption_fprint(FILE *out, const struct encryption_rep *rep);

struct encrypted_ballot_rep
{
    uint64_t id;
    uint32_t num_selections;
    struct encryption_rep *selections;
};

struct Crypto_encrypted_ballot_new_r {
    enum Crypto_status status;
    struct encrypted_ballot_rep result;
};

struct Crypto_encrypted_ballot_new_r Crypto_encrypted_ballot_new(uint32_t num_selections, uint64_t id);
void Crypto_encrypted_ballot_free(struct encrypted_ballot_rep *ballot);

#endif /* __CRYPTO_REPS_H__ */
