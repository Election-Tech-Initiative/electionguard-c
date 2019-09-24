#ifndef __KEYCEREMONY_MESSAGE_REPS_H__
#define __KEYCEREMONY_MESSAGE_REPS_H__

#include <stdint.h>

#include "crypto_reps.h"

struct key_generated_rep
{
    uint32_t trustee_index;
    struct public_key public_key;
};

struct all_keys_received_rep
{
    uint32_t num_trustees;
    struct public_key public_keys[MAX_TRUSTEES];
};

struct shares_generated_rep
{
    uint32_t trustee_index;
    uint32_t num_trustees;
    //struct encrypted_key_share shares[MAX_TRUSTEES]; TODO this needs to
    //be back for thresholding
};

struct all_shares_received_rep
{
    uint32_t num_trustees;
    //struct encrypted_key_share shares[MAX_TRUSTEES][MAX_TRUSTEES];
};

struct shares_verified_rep
{
    uint32_t trustee_index;
    bool verified;
};

#endif /* __KEYCEREMONY_MESSAGE_REPS_H__ */
