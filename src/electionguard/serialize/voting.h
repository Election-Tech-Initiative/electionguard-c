#ifndef __SERIALIZE_VOTING_H__
#define __SERIALIZE_VOTING_H__

#include "crypto_reps.h"
#include "serialize/state.h"
#include "voting/message_reps.h"

#include <electionguard/voting/messages.h>

void Serialize_reserve_ballot_tracker(struct serialize_state *state,
                                      struct ballot_tracker_rep const *data);

void Serialize_write_ballot_tracker(struct serialize_state *state,
                                    struct ballot_tracker_rep const *data);

void Serialize_read_ballot_tracker(struct serialize_state *state,
                                   struct ballot_tracker_rep *data);

void Serialize_reserve_ballot_identifier(
    struct serialize_state *state, struct ballot_identifier_rep const *data);

void Serialize_write_ballot_identifier(
    struct serialize_state *state, struct ballot_identifier_rep const *data);

void Serialize_read_ballot_identifier(struct serialize_state *state,
                                      struct ballot_identifier_rep *data);

bool Serialize_deserialize_ballot_identifier_message(struct ballot_identifier *ballot_identifier_message, 
                                                    struct ballot_identifier_rep *out_ballot_identifier_rep);

bool Serialize_deserialize_register_ballot_message(struct register_ballot_message *ballot_message, 
                                                    struct encrypted_ballot_rep *out_ballot_rep);

#endif /* __SERIALIZE_VOTING_H__ */
