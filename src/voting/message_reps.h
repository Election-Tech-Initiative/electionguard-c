#ifndef __VOTING_MESSAGE_REPS_H__
#define __VOTING_MESSAGE_REPS_H__

#include <stdbool.h>
#include <stddef.h>

#include "max_values.h"
#include "voting/messages.h"

struct register_ballot_rep
{
    uint64_t id;
    uint32_t num_selections;
    bool selections[MAX_SELECTIONS];
};

struct ballot_tracker_rep
{
    uint64_t id;
};

struct ballot_identifier_rep
{
    uint64_t id;
};

#endif /* __VOTING_MESSAGE_REPS_H__ */
