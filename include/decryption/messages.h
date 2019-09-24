#ifndef __DECRYPTION_MESSAGES_H__
#define __DECRYPTION_MESSAGES_H__

#include <stdint.h>

/* A trustee's share of decrypting the results of an election. */
struct decryption_share
{
    uint64_t len;
    uint8_t const *bytes;
};

/* A request from the coordinator to a trustee, asking it to provide
   fragments to compensate for a missing trustee. */
struct fragments_request
{
    uint64_t len;
    uint8_t const *bytes;
};

/* A response from a trustee to a coordinator providing fragments to
   compensate for a missing trustee. */
struct fragments
{
    uint64_t len;
    uint8_t const *bytes;
};

#endif /* __DECRYPTION_MESSAGES_H__ */
