#ifndef __VOTING_MESSAGE_REPS_H__
#define __VOTING_MESSAGE_REPS_H__

#include <stdbool.h>
#include <stddef.h>

#include <electionguard/max_values.h>
#include <electionguard/voting/messages.h>

struct ballot_tracker_rep
{
    uint64_t id;
};

struct ballot_identifier_rep
{
    uint64_t id;
};

#endif /* __VOTING_MESSAGE_REPS_H__ */
