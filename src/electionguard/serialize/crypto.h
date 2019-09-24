#ifndef __SERIALIZE_CRYPTO_H__
#define __SERIALIZE_CRYPTO_H__

#include <electionguard/crypto.h>

#include "crypto_reps.h"
#include "serialize/state.h"

void Serialize_reserve_uint4096(struct serialize_state *state, const_uint4096 data);

void Serialize_write_uint4096(struct serialize_state *state, const mpz_t data);

void Serialize_read_uint4096(struct serialize_state *state, mpz_t data);

void Serialize_reserve_hash(struct serialize_state *state);
void Serialize_write_hash(struct serialize_state *state, struct hash data);

uint8_t* Serialize_reserve_write_hash(struct hash in);

uint8_t *Serialize_reserve_write_bignum(mpz_t in);


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

void Serialize_reserve_schnorr_proof(struct serialize_state *state,
                                     struct schnorr_proof const *data);

void Serialize_write_schnorr_proof(struct serialize_state *state,
                                   struct schnorr_proof const *data);

void Serialize_read_schnorr_proof(struct serialize_state *state,
                                   struct schnorr_proof *data);


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

void Serialize_reserve_encryption(struct serialize_state *state,
                                  struct encryption_rep const *data);

void Serialize_write_encryption(struct serialize_state *state,
                                struct encryption_rep const *data);

void Serialize_read_encryption(struct serialize_state *state,
                               struct encryption_rep *data);

void Serialize_reserve_encrypted_ballot(struct serialize_state *state,
                                        struct encrypted_ballot_rep const *data);

void Serialize_write_encrypted_ballot(struct serialize_state *state,
                                      struct encrypted_ballot_rep const *data);

void Serialize_read_encrypted_ballot(struct serialize_state *state,
                                     struct encrypted_ballot_rep *data);

#endif /* __CRYPTO_SERIALIZE_H__ */
