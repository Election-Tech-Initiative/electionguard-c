#ifndef __CRYPTO_REPS_H__
#define __CRYPTO_REPS_H__

#include <gmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bignum.h"
#include "random_source.h"
#include "sha2-openbsd.h"
#include "uint4096.h"
#include <electionguard/crypto.h>
#include <electionguard/max_values.h>
#include <electionguard/rsa.h>

enum Crypto_status
{
    CRYPTO_SUCCESS,
    CRYPTO_INSUFFICIENT_MEMORY,
    CRYPTO_IO_ERROR,
    // it is a bug for the SDK to generate one of these
    CRYPTO_UNKNOWN_ERROR
};

/* A private key, including coefficients */
struct private_key
{
    uint32_t threshold;
    mpz_t coefficients[MAX_TRUSTEES]; //There are threshold of these
};

void Crypto_private_key_init(struct private_key *dst, int threshold);
void Crypto_private_key_free(struct private_key *dst, int threshold);

void Crypto_private_key_copy(struct private_key *dst,
                             struct private_key const *src);

/* Like SHA256Final, but reduce the result mod the right generator. The mpz_t
 * in the output hash will be initialized for you. */
void Crypto_hash_final(struct hash *out, SHA2_CTX *context);
void Crypto_hash_reduce(struct hash *out, raw_hash bytes);

void Crypto_hash_update_bignum_p(SHA2_CTX *context, mpz_t num);
void Crypto_hash_update_bignum_q(SHA2_CTX *context, mpz_t num);

/* A NIZKP of knowledge of the secrets associated with a public key */
struct schnorr_proof
{
    uint32_t threshold;
    // The commitments and challenge_responses are both of length threshold.
    mpz_t commitments[MAX_TRUSTEES];
    struct hash challenge;
    mpz_t challenge_responses[MAX_TRUSTEES];
};

/* A public key, including coefficient commitments */
struct public_key
{
    uint32_t threshold;
    struct schnorr_proof proof;
    mpz_t coef_commitments[MAX_TRUSTEES]; //There are threshold of these
};

/* Check if two public keys are equal. */
bool Crypto_public_key_equal(struct public_key const *,
                             struct public_key const *);

void Crypto_public_key_new(struct public_key *dst, int threshold);
void Crypto_public_key_free(struct public_key *dst, int threshold);

void Crypto_public_key_copy(struct public_key *dst,
                            struct public_key const *src);

void Crypto_schnorr_proof_new(struct schnorr_proof *dst, int threshold);
void Crypto_schnorr_proof_free(struct schnorr_proof *dst, int threshold);
void Crypto_schnorr_proof_copy(struct schnorr_proof *dst,
                               struct schnorr_proof const *src);

bool Crypto_check_keypair_proof(struct public_key key, raw_hash base_hash_code);
/* Generate a random keypair and return the public and private keys */
struct Crypto_gen_keypair_r Crypto_gen_keypair(uint32_t num_coefficients,
                                               raw_hash base_hash_code);

struct Crypto_gen_keypair_r
{
    enum Crypto_status status;
    //@secret The private key must not be leaked from the system.
    struct private_key private_key;
    struct public_key public_key;
};

struct individualPrivateKeyShare
{
    mpz_t share;
};

struct individualPublicKeyShare
{
    struct public_key recipient_public_key;
    rsa_public_key rsa_key;
};

struct encrypted_key_share
{
    mpz_t encrypted;
};
bool Crypto_check_validity_share_against_public_keys(struct encrypted_key_share* share,
                                                     struct public_key *pubKeys,
                                                     rsa_private_key* rsaPrivateKey,
                                                     uint32_t num_trusties, uint32_t my_index);

void Crypto_create_encrypted_key_share(struct encrypted_key_share *dest,
                                       struct public_key *pub_key,
                                       rsa_public_key *rsa_pub_key,
                                       struct private_key *priv_key,
                                       uint32_t index);

void Crypto_encrypted_key_share_init(struct encrypted_key_share *dst);
void Crypto_encrypted_key_share_free(struct encrypted_key_share *dst);

void Crypto_encrypted_key_share_copy(struct encrypted_key_share *dst,
                                     struct encrypted_key_share const *src);

struct joint_public_key_rep
{
    uint32_t num_trustees;
    mpz_t public_key;
};

void Crypto_joint_public_key_init(struct joint_public_key_rep *dst);
void Crypto_joint_public_key_free(struct joint_public_key_rep *dst);

void Crypto_joint_public_key_copy(struct joint_public_key_rep *dst,
                                  struct joint_public_key_rep const *src);

/* Create a joint public key from a list of public keys. Copies the
   input keys, so does not take ownership. */
void Crypto_generate_joint_public_key(struct joint_public_key_rep *dst,
                                      struct public_key const *public_keys,
                                      uint32_t num_keys);

struct encryption_rep
{
    mpz_t nonce_encoding;
    mpz_t message_encoding;
};

void Crypto_encrypt(struct encryption_rep *out, mpz_t out_nonce,
                    RandomSource source, const struct joint_public_key_rep *key,
                    mpz_t message);
void Crypto_encryption_homomorphic_zero(struct encryption_rep *out);
void Crypto_encryption_homomorphic_add(struct encryption_rep *out,
                                       const struct encryption_rep *a,
                                       const struct encryption_rep *b);

bool Crypto_encryption_fprint(FILE *out, const struct encryption_rep *rep);

struct cp_proof_rep
{
    struct encryption_rep commitment;
    struct hash challenge;
    mpz_t response;
};

struct dis_proof_rep
{
    //TODO if we use arrays for commitments and challenges we can clean
    //up quite a bit of code in the proof work
    struct encryption_rep commitment0;
    struct encryption_rep commitment1;
    struct hash challenge;
    mpz_t challenge0;
    mpz_t challenge1;
    mpz_t response0;
    mpz_t response1;
};

void Crypto_dis_proof_new(struct dis_proof_rep *dst);
void Crypto_dis_proof_free(struct dis_proof_rep *dst);

bool Crypto_check_aggregate_cp_proof(struct cp_proof_rep proof,
                                      struct encryption_rep encryption,
                                      struct hash base_hash, mpz_t public_key,
                                      uint32_t l_int);

void Crypto_generate_aggregate_cp_proof(struct cp_proof_rep *result,
                                        RandomSource source, mpz_t nonce,
                                        struct encryption_rep encryption,
                                        struct hash base_hash,
                                        mpz_t public_key);

void Crypto_generate_decryption_cp_proof(
    struct cp_proof_rep *result, mpz_t secret_key, mpz_t partial_decryption,
    struct encryption_rep aggregate_encryption, struct hash base_hash);

void Crypto_generate_dis_proof(struct dis_proof_rep *result,
                               RandomSource source, struct hash base_hash,
                               bool selected, mpz_t public_key,
                               struct encryption_rep encryption, mpz_t nonce);

bool Crypto_check_decryption_cp_proof(
    struct cp_proof_rep proof, mpz_t public_key, mpz_t partial_decryption,
    struct encryption_rep aggregate_encryption, struct hash base_hash);

bool Crypto_check_dis_proof(struct dis_proof_rep proof,
                            struct encryption_rep encryption,
                            struct hash base_hash, mpz_t public_key);

void Crypto_cp_proof_new(struct cp_proof_rep *dst);
void Crypto_cp_proof_free(struct cp_proof_rep *dst);

void Crypto_rsa_private_key_new( rsa_private_key *dst);
void Crypto_rsa_private_key_free( rsa_private_key *dst);
void Crypto_rsa_private_key_copy(rsa_private_key *dst, rsa_private_key *src);

void Crypto_rsa_public_key_new(rsa_public_key *dst);
void Crypto_rsa_public_key_free(rsa_public_key *dst);
void Crypto_rsa_public_key_copy(rsa_public_key *dst, rsa_public_key *src);


struct encrypted_ballot_rep
{
    uint64_t id;
    uint32_t num_selections;
    struct encryption_rep *selections;
    struct dis_proof_rep *dis_proof;
    struct cp_proof_rep cp_proof;
};

struct Crypto_encrypted_ballot_new_r
{
    enum Crypto_status status;
    struct encrypted_ballot_rep result;
};

int mpz_t_fscan(FILE *in, mpz_t out);

void Crypto_encryption_rep_new(struct encryption_rep *dst);
void Crypto_encryption_rep_free(struct encryption_rep *dst);
void Crypto_encryption_rep_copy(struct encryption_rep *dst,
                                struct encryption_rep *src);

struct Crypto_encrypted_ballot_new_r
Crypto_encrypted_ballot_new(uint32_t num_selections, uint64_t ballot_id);
void Crypto_encrypted_ballot_free(struct encrypted_ballot_rep *ballot);

#endif /* __CRYPTO_REPS_H__ */
