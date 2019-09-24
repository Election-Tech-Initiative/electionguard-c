#ifndef __SERIALIZE_CRYPTO_H__
#define __SERIALIZE_CRYPTO_H__

#include "crypto.h"
#include "crypto_reps.h"
#include "serialize/state.h"

void Serialize_reserve_key(struct serialize_state *state,
                           struct key const *data);

void Serialize_write_key(struct serialize_state *state, struct key const *data);

void Serialize_read_key(struct serialize_state *state, struct key *data);

void Serialize_reserve_private_key(struct serialize_state *state,
                                   struct private_key const *data);

void Serialize_write_private_key(struct serialize_state *state,
                                 struct private_key const *data);

void Serialize_read_private_key(struct serialize_state *state,
                                struct private_key *data);

void Serialize_reserve_public_key(struct serialize_state *state,
                                  struct public_key const *data);

void Serialize_write_public_key(struct serialize_state *state,
                                struct public_key const *data);

void Serialize_read_public_key(struct serialize_state *state,
                               struct public_key *data);

void Serialize_reserve_encrypted_key_share(
    struct serialize_state *state, struct encrypted_key_share const *data);

void Serialize_write_encrypted_key_share(
    struct serialize_state *state, struct encrypted_key_share const *data);

void Serialize_read_encrypted_key_share(struct serialize_state *state,
                                        struct encrypted_key_share *data);

void Serialize_reserve_joint_public_key(
    struct serialize_state *state, struct joint_public_key_rep const *data);

void Serialize_write_joint_public_key(struct serialize_state *state,
                                      struct joint_public_key_rep const *data);

void Serialize_read_joint_public_key(struct serialize_state *state,
                                     struct joint_public_key_rep *data);

#endif /* __CRYPTO_SERIALIZE_H__ */
