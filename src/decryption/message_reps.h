#ifndef __DECRYPTION_MESSAGE_REPS_H__
#define __DECRYPTION_MESSAGE_REPS_H__

#include <stdbool.h>
#include <stdint.h>

#include "max_values.h"

struct decryption_share_rep
{
    uint32_t trustee_index;
    uint64_t num_tallies;
    uint64_t tallies[MAX_SELECTIONS];
};

struct decryption_fragments_request_rep
{
    uint32_t num_trustees;
    bool requested[MAX_TRUSTEES];
};

struct decryption_fragments_rep
{
    uint32_t trustee_index;
    uint32_t num_trustees;
    bool requested[MAX_TRUSTEES];
};

#endif /* __DECRYPTION_MESSAGE_REPS_H__ */
