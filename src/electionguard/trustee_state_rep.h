#ifndef __TRUSTEE_STATE_REP_H__
#define __TRUSTEE_STATE_REP_H__

#include "crypto_reps.h"
#include "serialize/state.h"

struct trustee_state_rep
{
    uint32_t index;
    struct private_key private_key;
    struct encrypted_key_share my_key_shares
        [MAX_TRUSTEES]; //The shares other trustees have sent to this trustee
    rsa_private_key rsa_private_key;
};

#endif /* __TRUSTEE_STATE_REP_H__ */
