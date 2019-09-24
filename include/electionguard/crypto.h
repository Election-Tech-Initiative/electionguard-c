#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <stddef.h>
#include <gmp.h>

#include <electionguard/max_values.h>

/** Size of a key in bytes. */
enum KEY_SIZE_e
{
    KEY_SIZE = 4096 / 8,
};

/** Maximum size of a private key in bytes. */
enum MAX_PRIVATE_KEY_SIZE_e
{
    MAX_PRIVATE_KEY_SIZE = MAX_TRUSTEES * KEY_SIZE,
};

/** Maximum size of a public key in bytes. */
enum MAX_PUBLIC_KEY_SIZE_e
{
    MAX_PUBLIC_KEY_SIZE = MAX_TRUSTEES * KEY_SIZE,
};

/**
 * A public key that will be used to encrypt ballots, such that any
 * group of `k` of the original trustees can decrypt a message
 * encrypted with this key. */
struct joint_public_key
{
    uint64_t len;
    uint8_t const *bytes;
};

enum HASH_DIGEST_SIZE_BYTES_e
{
    HASH_DIGEST_SIZE_BYTES = 32,
};

/* This typedef and struct are used to differentiate between raw hashes and
 * hashes reduced mod the generator. We use uint8_t[]'s for raw hashes, and
 * struct hash's for things reduced mod the generator. */
typedef uint8_t raw_hash[HASH_DIGEST_SIZE_BYTES];
struct hash {
    mpz_t digest;
};

/** You must call this before any of the other SDK functions. */
void Crypto_parameters_new();

/** After calling this, you should not call any other SDK functions (until you
 * re-initialize the parameters, at least). */
void Crypto_parameters_free();

#endif /* __CRYPTO__H__ */
