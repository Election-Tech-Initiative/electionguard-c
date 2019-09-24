#ifndef __SERIALIZE_DECRYPTION_H__
#define __SERIALIZE_DECRYPTION_H__

#include "decryption/message_reps.h"
#include "serialize/state.h"

void Serialize_reserve_decryption_share(
    struct serialize_state *state, struct decryption_share_rep const *data);

void Serialize_write_decryption_share(struct serialize_state *state,
                                      struct decryption_share_rep const *data);

void Serialize_read_decryption_share(struct serialize_state *state,
                                     struct decryption_share_rep *data);

void Serialize_reserve_decryption_fragments_request(
    struct serialize_state *state,
    struct decryption_fragments_request_rep const *data);

void Serialize_write_decryption_fragments_request(
    struct serialize_state *state,
    struct decryption_fragments_request_rep const *data);

void Serialize_read_decryption_fragments_request(
    struct serialize_state *state,
    struct decryption_fragments_request_rep *data);

void Serialize_reserve_decryption_fragments(
    struct serialize_state *state, struct decryption_fragments_rep const *data);

void Serialize_write_decryption_fragments(
    struct serialize_state *state, struct decryption_fragments_rep const *data);

void Serialize_read_decryption_fragments(struct serialize_state *state,
                                         struct decryption_fragments_rep *data);

#endif /* __SERIALIZE_DECRYPTION_H__ */
