#ifndef __KEYCEREMONY_MESSAGES_H__
#define __KEYCEREMONY_MESSAGES_H__

#include <stdint.h>

/* The message that is produced by a trustee after generating their
   public/private keypair, and which must be passed to the
   coordinator. */
struct key_generated_message
{
    uint64_t len;
    uint8_t const *bytes;
};

/* The message that is produced by a coordinator after confirming
   that is has received all of the trustees' public keys, and which
   must be passed back to each trustee. */
struct all_keys_received_message
{
    uint64_t len;
    uint8_t const *bytes;
};

/* The message that is produced by a trustee after computing and
   encrypting the shares of its private key for the other trustees. */
struct shares_generated_message
{
    uint64_t len;
    uint8_t const *bytes;
};

/* The message that is produced by a coordinator after confirming that
   is has received all of the trustees' encrypted private key shares,
   and which must be passed back to each trustee. */
struct all_shares_received_message
{
    uint64_t len;
    uint8_t const *bytes;
};

/* The message that is produced by a trustee after verifying the
   private shares of the other trustees. */
struct shares_verified_message
{
    uint64_t len;
    uint8_t const *bytes;
};

#endif /* __KEYCEREMONY_MESSAGES_H__ */
