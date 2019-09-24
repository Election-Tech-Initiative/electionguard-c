#ifndef __SERIALIZE_TRUSTEE_STATE_H__
#define __SERIALIZE_TRUSTEE_STATE_H__

#include "serialize/state.h"
#include "trustee_state_rep.h"

void Serialize_reserve_trustee_state(struct serialize_state *state,
                                     struct trustee_state_rep const *data);

void Serialize_write_trustee_state(struct serialize_state *state,
                                   struct trustee_state_rep const *data);

void Serialize_read_trustee_state(struct serialize_state *state,
                                  struct trustee_state_rep *data);

#endif /* __SERIALIZE_TRUSTEE_STATE_H__ */
