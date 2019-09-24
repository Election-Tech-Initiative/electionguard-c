#ifndef __SERIALIZE_KEYCEREMONY_H__
#define __SERIALIZE_KEYCEREMONY_H__

#include "keyceremony/message_reps.h"
#include "serialize/state.h"

void Serialize_reserve_key_generated(struct serialize_state *state,
                                     struct key_generated_rep const *data);

void Serialize_write_key_generated(struct serialize_state *state,
                                   struct key_generated_rep const *data);

void Serialize_read_key_generated(struct serialize_state *state,
                                  struct key_generated_rep *data);

void Serialize_reserve_all_keys_received(
    struct serialize_state *state, struct all_keys_received_rep const *data);

void Serialize_write_all_keys_received(
    struct serialize_state *state, struct all_keys_received_rep const *data);

void Serialize_read_all_keys_received(struct serialize_state *state,
                                      struct all_keys_received_rep *data);
void Serialize_reserve_shares_generated(
    struct serialize_state *state, struct shares_generated_rep const *data);

void Serialize_write_shares_generated(struct serialize_state *state,
                                      struct shares_generated_rep const *data);

void Serialize_read_shares_generated(struct serialize_state *state,
                                     struct shares_generated_rep *data);

void Serialize_reserve_all_shares_received(
    struct serialize_state *state, struct all_shares_received_rep const *data);

void Serialize_write_all_shares_received(
    struct serialize_state *state, struct all_shares_received_rep const *data);

void Serialize_read_all_shares_received(struct serialize_state *state,
                                        struct all_shares_received_rep *data);

void Serialize_reserve_shares_verified(struct serialize_state *state,
                                       struct shares_verified_rep const *data);

void Serialize_write_shares_verified(struct serialize_state *state,
                                     struct shares_verified_rep const *data);

void Serialize_read_shares_verified(struct serialize_state *state,
                                    struct shares_verified_rep *data);

#endif /* __SERIALIZE_KEYCEREMONY_H__ */
