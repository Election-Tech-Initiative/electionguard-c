#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <stddef.h>

#include "max_values.h"

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

#endif /* __CRYPTO__H__ */
