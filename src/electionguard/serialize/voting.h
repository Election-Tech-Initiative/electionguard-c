#ifndef __SERIALIZE_VOTING_H__
#define __SERIALIZE_VOTING_H__

#include "serialize/state.h"
#include "voting/message_reps.h"

void Serialize_reserve_register_ballot(struct serialize_state *state,
                                       struct register_ballot_rep const *data);

void Serialize_write_register_ballot(struct serialize_state *state,
                                     struct register_ballot_rep const *data);

void Serialize_read_register_ballot(struct serialize_state *state,
                                    struct register_ballot_rep *data);

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

#endif /* __SERIALIZE_VOTING_H__ */
