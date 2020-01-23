#ifndef __VOTING_MESSAGE_REPS_H__
#define __VOTING_MESSAGE_REPS_H__

#include <stdbool.h>
#include <stddef.h>

struct ballot_tracker_rep
{
    uint64_t id;
};

struct ballot_identifier_rep
{
    uint64_t id;
};

struct external_identifer_rep
{
    char *id;
};

#endif /* __VOTING_MESSAGE_REPS_H__ */
