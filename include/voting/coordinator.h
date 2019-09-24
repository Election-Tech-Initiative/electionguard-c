#ifndef __VOTING_COORDINATOR_H__
#define __VOTING_COORDINATOR_H__

#include <stdio.h>

#include "voting/messages.h"

// @todo jwaksbaum What sort of assurances do we make about the
// machine being shut off? How does it persist votes?

typedef struct Voting_Coordinator_s *Voting_Coordinator;

enum Voting_Coordinator_status
{
    VOTING_COORDINATOR_SUCCESS,
    VOTING_COORDINATOR_INSUFFICIENT_MEMORY,
    VOTING_COORDINATOR_INVALID_BALLOT_ID,
    VOTING_COORDINATOR_INVALID_BALLOT,
    VOTING_COORDINATOR_UNREGISTERED_BALLOT,
    VOTING_COORDINATOR_DUPLICATE_BALLOT,
    VOTING_COORDINATOR_TIMED_OUT_BALLOT,
    VOTING_COORDINATOR_IO_ERROR,
    VOTING_COORDINATOR_SERIALIZE_ERROR,
    VOTING_COORDINATOR_DESERIALIZE_ERROR,
};

/************************** INITIALIZATION & FREEING ***************************/

// @todo jwaksbaum Do we want to take in information about the ballot
// formats so that we can validate that the ballots we receive are
// well-formed?

/* Create a new voting coordinator. */
struct Voting_Coordinator_new_r Voting_Coordinator_new(uint32_t num_selections);

struct Voting_Coordinator_new_r
{
    enum Voting_Coordinator_status status;
    Voting_Coordinator coordinator;
};

/* Free a ballot box. */
void Voting_Coordinator_free(Voting_Coordinator coordinator);

/****************** REGISTERING, CASTING & SPOILING BALLOTS *******************/

/* Register a ballot with the coordinator so that it may be cast or
   spoiled. */
enum Voting_Coordinator_status
Voting_Coordinator_register_ballot(Voting_Coordinator coordinator,
                                   struct register_ballot_message message);

/* Mark the ballot specified by ballot_id as cast. */
enum Voting_Coordinator_status
Voting_Coordinator_cast_ballot(Voting_Coordinator coordinator,
                               struct ballot_identifier ballot_id);

/* Mark the ballot specified by ballot_id as spoiled. */
enum Voting_Coordinator_status
Voting_Coordinator_spoil_ballot(Voting_Coordinator coordinator,
                                struct ballot_identifier ballot_id);

/***************************** EXPORTING BALLOTS ******************************/

// @todo jwaksbaum Do we want to return the number of bytes written?

// @todo jwaksbaum What format is it writing in?

/* Write all of the cast and spoiled ballots to out. */
enum Voting_Coordinator_status
Voting_Coordinator_export_ballots(Voting_Coordinator coordinator, FILE *out);

#endif /* __VOTING_COORDINATOR_H__ */
