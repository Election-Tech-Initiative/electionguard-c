#ifndef __TRUSTEE_STATE_H__
#define __TRUSTEE_STATE_H__

#include <stddef.h>
#include <stdint.h>

/* The state that needs to be persisted over the course of an election
   and that will be necessary for the trustee to decrypt its share of
   the election results. This state can be serialized and stored, and
   then deserialized and used to construct a Decryption_Trustee. */
struct trustee_state
{
    uint64_t len;
    uint8_t const *bytes;
};

#endif /* __TRUSTEE_STATE_H__ */
