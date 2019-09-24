#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/crypto.h>

#include "crypto_reps.h"
#include "random_source.h"
#include "serialize/crypto.h"
#include "sha2-openbsd.h"
//#define DEBUG_PRINT =0

void Crypto_hash_final(struct hash *out, SHA2_CTX *context)
{
    uint8_t bytes[HASH_DIGEST_SIZE_BYTES];
    SHA256Final(bytes, context);
    Crypto_hash_reduce(out, bytes);
}

void Crypto_hash_reduce(struct hash *out, raw_hash bytes)
{
    //mpz_init(out->digest);
    mpz_import(out->digest, HASH_DIGEST_SIZE_BYTES / 8, 1, 8, 0, 0, bytes);
    mod_q(out->digest, out->digest);
}

void Crypto_hash_update_bignum(SHA2_CTX *context, mpz_t num)
{
    uint8_t *serialized_buffer = Serialize_reserve_write_bignum(num);
    SHA256Update(context, serialized_buffer, 4096 / 8);
    free(serialized_buffer);
}

bool Crypto_public_key_equal(struct public_key const *key1,
                             struct public_key const *key2)
{
    bool ok = true;

    if (key1->threshold != key2->threshold)
        ok = false;

    for (uint32_t i = 0; i < key1->threshold && ok; i++)
    {
        ok = ok && (0 == mpz_cmp(key1->coef_commitments[i],
                                 key2->coef_commitments[i]));
#ifdef DEBUG_PRINT
        printf("Checking\n");
        print_base16(key1->coef_commitments[i]);
        printf("=");
        print_base16(key2->coef_commitments[i]);
#endif
    }
    return ok;
}

enum Crypto_status
Crypto_RandomSource_status_convert(enum RandomSource_status status)
{
    switch (status)
    {
    case RANDOM_SOURCE_SUCCESS:
        return CRYPTO_SUCCESS;
    case RANDOM_SOURCE_INSUFFICIENT_MEMORY:
        return CRYPTO_INSUFFICIENT_MEMORY;
    case RANDOM_SOURCE_IO_ERROR:
        return CRYPTO_IO_ERROR;
    // should never happen
    default:
        return CRYPTO_UNKNOWN_ERROR;
    }
}

bool Crypto_check_keypair_proof(struct public_key key, raw_hash base_hash_code)
{
    mpz_t gu;
    mpz_t hkc;
    mpz_init(gu);
    mpz_init(hkc);

    //TODO check the hash
    for (int i = 0; i < key.proof.threshold; i++)
    {
        // printf("commitment %d\n", i);
        // print_base16(key.coef_commitments[i]);
        // printf("response\n");
        // print_base16(key.proof.challenge_responses[i]);
        // printf("challenge\n");
        // print_base16(key.proof.challenge.digest);
        pow_mod_p(gu, generator, key.proof.challenge_responses[i]);
        pow_mod_p(hkc, key.coef_commitments[i], key.proof.challenge.digest);
        mul_mod_p(hkc, key.proof.commitments[i], hkc);
        assert(0 == mpz_cmp(gu, hkc));
    }

    mpz_clear(gu);
    mpz_clear(hkc);
}

struct Crypto_gen_keypair_r Crypto_gen_keypair(uint32_t num_coefficients,
                                               raw_hash base_hash_code)
{
    struct Crypto_gen_keypair_r result;
    result.status = CRYPTO_SUCCESS;

    result.private_key.threshold = num_coefficients;
    result.public_key.threshold = num_coefficients;

    Crypto_private_key_init(&result.private_key, num_coefficients);
    Crypto_public_key_new(&result.public_key, num_coefficients);

    RandomSource source;
    if (CRYPTO_SUCCESS == result.status)
    {
        struct RandomSource_new_r source_r = RandomSource_new();
        source = source_r.source;
        result.status = Crypto_RandomSource_status_convert(source_r.status);
    }

    for (uint32_t i = 0; i < num_coefficients; i++)
    {
        if (CRYPTO_SUCCESS == result.status)
        {
            result.status = Crypto_RandomSource_status_convert(
                RandomSource_uniform_bignum_o(
                    result.private_key.coefficients[i], source));
            if (CRYPTO_SUCCESS != result.status)
            {
                RandomSource_free(source);
            }
        }

        if (CRYPTO_SUCCESS == result.status)
        {
            pow_mod_p(result.public_key.coef_commitments[i], generator,
                      result.private_key.coefficients[i]);
        }
    }

    if (CRYPTO_SUCCESS == result.status)
    {
        result.public_key.proof.threshold = num_coefficients;
        SHA2_CTX context;
        SHA256Init(&context);
        SHA256Update(&context, base_hash_code, HASH_DIGEST_SIZE_BYTES);

        for (uint32_t i = 0; i < num_coefficients; i++)
        {
            Crypto_hash_update_bignum(&context,
                                      result.public_key.coef_commitments[i]);
        }

        for (uint32_t i = 0;
             i < num_coefficients && CRYPTO_SUCCESS == result.status; i++)
        {
            result.status = Crypto_RandomSource_status_convert(
                RandomSource_uniform_bignum_o(
                    result.public_key.proof.challenge_responses[i], source));

            if (CRYPTO_SUCCESS == result.status)
            {
                pow_mod_p(result.public_key.proof.commitments[i], generator,
                          result.public_key.proof.challenge_responses[i]);
                Crypto_hash_update_bignum(
                    &context, result.public_key.proof.challenge_responses[i]);
            }
        }

        if (CRYPTO_SUCCESS == result.status)
        {
            mpz_t product;
            mpz_init(product);
            Crypto_hash_final(&result.public_key.proof.challenge, &context);

            for (uint32_t i = 0; i < num_coefficients; i++)
            {
                mul_mod_q(product, result.public_key.proof.challenge.digest,
                          result.private_key.coefficients[i]);
                add_mod_q(result.public_key.proof.challenge_responses[i],
                          result.public_key.proof.challenge_responses[i],
                          product);
            }

            mpz_clear(product);
        }
    }

    if (CRYPTO_SUCCESS == result.status)
    {
        RandomSource_free(source);
    }

    if (CRYPTO_SUCCESS != result.status)
    {
        Crypto_private_key_free(&result.private_key, num_coefficients);
        Crypto_public_key_free(&result.public_key, num_coefficients);
    }

    return result;
}

//Making threshold an argument to be sure it's provided
void Crypto_private_key_init(struct private_key *dst, int threshold)
{
    for (uint32_t i = 0; i < threshold; i++)
        mpz_init(dst->coefficients[i]);
}

void Crypto_private_key_free(struct private_key *dst, int threshold)
{
    for (uint32_t i = 0; i < threshold; i++)
        mpz_clear(dst->coefficients[i]);
}

void Crypto_private_key_copy(struct private_key *dst,
                             struct private_key const *src)
{
    assert(src->threshold <= MAX_TRUSTEES);
    dst->threshold = src->threshold;
    for (uint32_t i = 0; i < src->threshold; i++)
        mpz_set(dst->coefficients[i], src->coefficients[i]);
}

void Crypto_public_key_free(struct public_key *dst, int threshold)
{
    for (uint32_t i = 0; i < threshold; i++)
        mpz_clear(dst->coef_commitments[i]);
    Crypto_schnorr_proof_new(&dst->proof, threshold);
}

void Crypto_public_key_new(struct public_key *dst, int threshold)
{
    for (uint32_t i = 0; i < threshold; i++)
        mpz_init(dst->coef_commitments[i]);
    Crypto_schnorr_proof_new(&dst->proof, threshold);
}

void Crypto_public_key_copy(struct public_key *dst,
                            struct public_key const *src)
{
    dst->threshold = src->threshold;
    for (uint32_t i = 0; i < src->threshold; i++)
        mpz_set(dst->coef_commitments[i], src->coef_commitments[i]);
    Crypto_schnorr_proof_copy(&dst->proof, &src->proof);
}

void Crypto_schnorr_proof_new(struct schnorr_proof *dst, int threshold)
{
    for (int i = 0; i < threshold; i++)
    {
        mpz_init(dst->commitments[i]);
        mpz_init(dst->challenge_responses[i]);
    }
    mpz_init(dst->challenge.digest);
}

void Crypto_schnorr_proof_free(struct schnorr_proof *dst, int threshold)
{
    for (int i = 0; i < threshold; i++)
    {
        mpz_clear(dst->commitments[i]);
        mpz_clear(dst->challenge_responses[i]);
    }
    mpz_init(dst->challenge.digest);
}

void Crypto_schnorr_proof_copy(struct schnorr_proof *dst,
                               struct schnorr_proof const *src)
{
    dst->threshold = src->threshold;
    for (int i = 0; i < dst->threshold; i++)
    {
        mpz_set(dst->commitments[i], src->commitments[i]);
        mpz_set(dst->challenge_responses[i], src->challenge_responses[i]);
    }
    mpz_set(dst->challenge.digest, src->challenge.digest);
}

void Crypto_encrypted_key_share_init(struct encrypted_key_share *dst,
                                     int threshold)
{
    Crypto_private_key_init(&dst->private_key, threshold);
    Crypto_public_key_new(&dst->recipient_public_key, threshold);
}

void Crypto_encrypted_key_share_free(struct encrypted_key_share *dst,
                                     int threshold)
{
    Crypto_private_key_free(&dst->private_key, threshold);
    Crypto_public_key_free(&dst->recipient_public_key, threshold);
}

void Crypto_encrypted_key_share_copy(struct encrypted_key_share *dst,
                                     struct encrypted_key_share const *src)
{
    Crypto_private_key_copy(&dst->private_key, &src->private_key);
    Crypto_public_key_copy(&dst->recipient_public_key,
                           &src->recipient_public_key);
}

void Crypto_joint_public_key_init(struct joint_public_key_rep *dst)
{
    mpz_init(dst->public_key);
}

void Crypto_joint_public_key_free(struct joint_public_key_rep *dst)
{
    mpz_clear(dst->public_key);
}

void Crypto_joint_public_key_copy(struct joint_public_key_rep *dst,
                                  struct joint_public_key_rep const *src)
{
    dst->num_trustees = src->num_trustees;
    mpz_set(dst->public_key, src->public_key);
}

void Crypto_generate_joint_public_key(struct joint_public_key_rep *dst,
                                      struct public_key const *public_keys,
                                      uint32_t num_keys)
{
    dst->num_trustees = num_keys;
    mpz_set_ui(
        dst->public_key,
        1); //Start with the public key as one, so the first multiplication is identity
    for (uint32_t i = 0; i < num_keys; i++)
    {
        //If there's a problem, double check that mul_mod_p works in place
        mul_mod_p(dst->public_key, dst->public_key,
                  public_keys[i].coef_commitments[0]);
    }
}

void Crypto_cp_proof_commit(struct encryption_rep *commitment_out,
                            struct encryption_rep encryption, mpz_t u)
{
    // commitment a in the documents
    pow_mod_p(commitment_out->nonce_encoding, encryption.nonce_encoding, u);
    // commitment b in the documents
    pow_mod_p(commitment_out->message_encoding, encryption.message_encoding, u);
}

void Crypto_cp_proof_challenge(struct hash *challenge_out,
                               struct encryption_rep encryption,
                               struct encryption_rep commitment,
                               struct hash base_hash)
{
    SHA2_CTX context;

    //Serialize the base hash
    uint8_t *base_serial = Serialize_reserve_write_hash(base_hash);

    //Generate the challenge
    SHA256Init(&context);
    SHA256Update(&context, base_serial, SHA256_DIGEST_LENGTH);
    Crypto_hash_update_bignum(&context, encryption.nonce_encoding);
    Crypto_hash_update_bignum(&context, encryption.message_encoding);
    Crypto_hash_update_bignum(&context, commitment.nonce_encoding);
    Crypto_hash_update_bignum(&context, commitment.message_encoding);
    Crypto_hash_final(challenge_out, &context);
}

void Crypto_generate_decryption_cp_proof(
    struct cp_proof_rep *result, mpz_t secret_key, mpz_t partial_decryption,
    struct encryption_rep aggregate_encryption, struct hash base_hash)
{
    //The random value for the proof, we reuse letters from the spec document
    mpz_t u;
    mpz_init(u);

    //TODO: Save one of these in the trustee
    RandomSource source;
    struct RandomSource_new_r source_r = RandomSource_new();
    source = source_r.source;

    //TODO If we change the exponent prime, this will be wrong
    RandomSource_uniform_bignum_o(u, source);

    // commitment a in the documents
    pow_mod_p(result->commitment.nonce_encoding, generator, u);
    // commitment b in the documents
    pow_mod_p(result->commitment.message_encoding,
              aggregate_encryption.nonce_encoding, u);

    mpz_init(result->challenge.digest);

    SHA2_CTX context;

    //Serialize the base hash
    uint8_t *base_serial = Serialize_reserve_write_hash(base_hash);

    //Generate the challenge
    SHA256Init(&context);
    SHA256Update(&context, base_serial, SHA256_DIGEST_LENGTH);
    Crypto_hash_update_bignum(&context, aggregate_encryption.nonce_encoding);
    Crypto_hash_update_bignum(&context, aggregate_encryption.message_encoding);
    Crypto_hash_update_bignum(&context, result->commitment.nonce_encoding);
    Crypto_hash_update_bignum(&context, result->commitment.message_encoding);
    Crypto_hash_update_bignum(&context, partial_decryption);
    Crypto_hash_final(&result->challenge, &context);

    // CR in the doc
    mul_mod_q(result->response, result->challenge.digest, secret_key);
    add_mod_q(result->response, u, result->response);

    mpz_clear(u);
    RandomSource_free(source);
}

void Crypto_check_decryption_cp_proof(
    struct cp_proof_rep proof, mpz_t public_key, mpz_t partial_decryption,
    struct encryption_rep aggregate_encryption, struct hash base_hash){

    mpz_t gv, av, akc, bbc;
    mpz_init(gv);
    mpz_init(akc);
    mpz_init(av);
    mpz_init(bbc);

    pow_mod_p(gv, generator, proof.response);
    pow_mod_p(akc, public_key, proof.challenge.digest);
    mul_mod_p(akc, proof.commitment.nonce_encoding, akc);

    assert(0 == mpz_cmp(gv, akc));

    //A^v
    pow_mod_p(av, aggregate_encryption.nonce_encoding, proof.response);
    //M^c
    pow_mod_p(bbc, partial_decryption, proof.challenge.digest);
    //b*Beta^c
    mul_mod_p(bbc, proof.commitment.message_encoding, bbc);

    assert(0 == mpz_cmp(av, bbc));

    mpz_clear(gv);
    mpz_clear(akc);
    mpz_clear(av);
    mpz_clear(bbc);
    }

_Bool Crypto_check_aggregate_cp_proof(struct cp_proof_rep proof,
                                      struct encryption_rep encryption,
                                      struct hash base_hash, mpz_t public_key)
{

    //TODO check ranges of values
    SHA2_CTX context;

    struct hash my_C;
    mpz_init(my_C.digest);
    //Serialize the base hash
    uint8_t *base_serial = Serialize_reserve_write_hash(base_hash);

    //Generate the challenge
    SHA256Init(&context);
    SHA256Update(&context, base_serial, SHA256_DIGEST_LENGTH);
    Crypto_hash_update_bignum(&context, encryption.nonce_encoding);
    Crypto_hash_update_bignum(&context, encryption.message_encoding);
    Crypto_hash_update_bignum(&context, proof.commitment.nonce_encoding);
    Crypto_hash_update_bignum(&context, proof.commitment.message_encoding);
    Crypto_hash_final(&my_C, &context);

    assert(0 == mpz_cmp(my_C.digest, proof.challenge.digest));

    mpz_t gv, aac, glc, glckv, bbc;
    mpz_init(gv);
    mpz_init(aac);
    mpz_init(glc);
    mpz_init(glckv);
    mpz_init(bbc);

    pow_mod_p(gv, generator, proof.response);
    pow_mod_p(aac, encryption.nonce_encoding, my_C.digest);
    mul_mod_p(aac, proof.commitment.nonce_encoding, aac);

    assert(0 == mpz_cmp(gv, aac));

    //L is 1 for now, so this is g^LC when this multiplication happens it should be mod q
    pow_mod_p(glc, generator, my_C.digest);
    // K^v
    pow_mod_p(glckv, public_key, proof.response);
    // g^LC * K^v
    mul_mod_p(glckv, glc, glckv);

    //Beta^c
    pow_mod_p(bbc, encryption.message_encoding, my_C.digest);
    //b*Beta^c
    mul_mod_p(bbc, proof.commitment.message_encoding, bbc);

    assert(0 == mpz_cmp(glckv, bbc));

    mpz_clear(gv);
    mpz_clear(aac);
    mpz_clear(glc);
    mpz_clear(glckv);
    mpz_clear(bbc);
    mpz_clear(my_C.digest);
    return 1;
}

void Crypto_generate_aggregate_cp_proof(struct cp_proof_rep *result,
                                        RandomSource source, mpz_t nonce,
                                        struct encryption_rep encryption,
                                        struct hash base_hash, mpz_t public_key)
{
    //The random value for the proof, we reuse letters from the spec document
    mpz_t u;
    mpz_init(u);

    //TODO If we change the exponent prime, this will be wrong
    RandomSource_uniform_bignum_o(u, source);

    // commitment a in the documents
    pow_mod_p(result->commitment.nonce_encoding, generator, u);
    // commitment b in the documents
    pow_mod_p(result->commitment.message_encoding, public_key, u);

    mpz_init(result->challenge.digest);
    Crypto_cp_proof_challenge(&result->challenge, encryption,
                              result->commitment, base_hash);

    // CR in the doc
    mul_mod_q(result->response, result->challenge.digest, nonce);
    add_mod_q(result->response, u, result->response);

    mpz_clear(u);
}

void Crypto_generate_dis_proof(struct dis_proof_rep *result,
                               RandomSource source, struct hash base_hash,
                               bool selected, mpz_t public_key,
                               struct encryption_rep encryption, mpz_t nonce)
{
    mpz_t fake_challenge;
    mpz_t real_challenge;
    mpz_t fake_response;
    mpz_t real_response;
    mpz_t u;
    mpz_t scratch;

    struct encryption_rep real_commitment;
    struct encryption_rep fake_commitment;

    Crypto_encryption_rep_new(&real_commitment);
    Crypto_encryption_rep_new(&fake_commitment);

    mpz_init(fake_challenge);
    mpz_init(fake_response);
    mpz_init(real_challenge);
    mpz_init(real_response);
    mpz_init(scratch);

    mpz_init(u);

    // Generate the randomness and the fake proof
    RandomSource_uniform_bignum_o(u, source);
    RandomSource_uniform_bignum_o(fake_challenge, source);
    RandomSource_uniform_bignum_o(fake_response, source);

    //Generate the real commitments
    pow_mod_p(real_commitment.nonce_encoding, generator, u);
    pow_mod_p(real_commitment.message_encoding, public_key, u);

    //Generate the fake commitments
    pow_mod_p(scratch, generator, fake_response);
    pow_mod_p(fake_commitment.nonce_encoding, encryption.nonce_encoding,
              fake_challenge);
    div_mod_p(fake_commitment.nonce_encoding, scratch,
              fake_commitment.nonce_encoding);

    pow_mod_p(scratch, public_key, fake_response);
    if (!selected)
    {
        //using message encoding temporarily
        pow_mod_p(fake_commitment.message_encoding, generator, fake_challenge);
        mul_mod_p(scratch, scratch, fake_commitment.message_encoding);
    }

    pow_mod_p(fake_commitment.message_encoding, encryption.message_encoding,
              fake_challenge);
    div_mod_p(fake_commitment.message_encoding, scratch,
              fake_commitment.message_encoding);

    //Generate the main challenge
    SHA2_CTX context;

    //Serialize the base hash
    uint8_t *base_serial = Serialize_reserve_write_hash(base_hash);

    SHA256Init(&context);
    SHA256Update(&context, base_serial, SHA256_DIGEST_LENGTH);
    Crypto_hash_update_bignum(&context, encryption.nonce_encoding);
    Crypto_hash_update_bignum(&context, encryption.message_encoding);

    if (selected)
    {
        Crypto_hash_update_bignum(&context, fake_commitment.nonce_encoding);
        Crypto_hash_update_bignum(&context, fake_commitment.message_encoding);
        Crypto_hash_update_bignum(&context, real_commitment.nonce_encoding);
        Crypto_hash_update_bignum(&context, real_commitment.message_encoding);
    }
    else
    {
        Crypto_hash_update_bignum(&context, real_commitment.nonce_encoding);
        Crypto_hash_update_bignum(&context, real_commitment.message_encoding);
        Crypto_hash_update_bignum(&context, fake_commitment.nonce_encoding);
        Crypto_hash_update_bignum(&context, fake_commitment.message_encoding);
    }
    Crypto_hash_final(&result->challenge, &context);

    sub_mod_q(real_challenge, result->challenge.digest, fake_challenge);

    mul_mod_q(real_response, real_challenge, nonce);
    add_mod_q(real_response, u, real_response);

    if (selected)
    {
        Crypto_encryption_rep_copy(&result->commitment0, &fake_commitment);
        Crypto_encryption_rep_copy(&result->commitment1, &real_commitment);
        mpz_set(result->challenge0, fake_challenge);
        mpz_set(result->challenge1, real_challenge);
        mpz_set(result->response0, fake_response);
        mpz_set(result->response1, real_response);
    }
    else
    {
        Crypto_encryption_rep_copy(&result->commitment0, &real_commitment);
        Crypto_encryption_rep_copy(&result->commitment1, &fake_commitment);
        mpz_set(result->challenge0, real_challenge);
        mpz_set(result->challenge1, fake_challenge);
        mpz_set(result->response0, real_response);
        mpz_set(result->response1, fake_response);
    }

    Crypto_encryption_rep_free(&real_commitment);
    Crypto_encryption_rep_free(&fake_commitment);

    mpz_clear(fake_challenge);
    mpz_clear(fake_response);
    mpz_clear(u);
    mpz_clear(real_challenge);
    mpz_clear(real_response);
    mpz_clear(scratch);

    //TODO
}

bool Crypto_check_dis_proof(struct dis_proof_rep proof,
                            struct encryption_rep encryption,
                            struct hash base_hash, mpz_t public_key)
{

    mpz_t my_challenge;
    mpz_init(my_challenge);

    add_mod_q(my_challenge, proof.challenge0, proof.challenge1);
    //Check c = c0 + c1 mod q
    assert(0 == mpz_cmp(proof.challenge.digest, my_challenge));

    //TODO we can probably share some code with the disjunctive and decryption proofs here
    mpz_t gv, aac, glc, glckv, bbc;
    mpz_init(gv);
    mpz_init(aac);
    mpz_init(glc);
    mpz_init(glckv);
    mpz_init(bbc);

    pow_mod_p(gv, generator, proof.response0);
    pow_mod_p(aac, encryption.nonce_encoding, proof.challenge0);
    mul_mod_p(aac, proof.commitment0.nonce_encoding, aac);

    assert(0 == mpz_cmp(gv, aac));

    pow_mod_p(gv, generator, proof.response1);
    pow_mod_p(aac, encryption.nonce_encoding, proof.challenge1);
    mul_mod_p(aac, proof.commitment1.nonce_encoding, aac);

    assert(0 == mpz_cmp(gv, aac));

    // K^v
    pow_mod_p(glckv, public_key, proof.response0);

    //Beta^c
    pow_mod_p(bbc, encryption.message_encoding, proof.challenge0);
    //b*Beta^c
    mul_mod_p(bbc, proof.commitment0.message_encoding, bbc);

    assert(0 == mpz_cmp(glckv, bbc));

    //g^c1
    pow_mod_p(glc, generator, proof.challenge1);
    // K^v
    pow_mod_p(glckv, public_key, proof.response1);
    mul_mod_p(glckv, glc, glckv);

    //Beta^c
    pow_mod_p(bbc, encryption.message_encoding, proof.challenge1);
    //b*Beta^c
    mul_mod_p(bbc, proof.commitment1.message_encoding, bbc);

    assert(0 == mpz_cmp(glckv, bbc));

    mpz_clear(gv);
    mpz_clear(aac);
    mpz_clear(glc);
    mpz_clear(glckv);
    mpz_clear(bbc);

    mpz_clear(my_challenge);
}

//Encrypt a message mapped onto the group (e.g. g^message % p)
void Crypto_encrypt(struct encryption_rep *out, mpz_t out_nonce,
                    RandomSource source, const struct joint_public_key_rep *key,
                    mpz_t message)
{

    RandomSource_uniform_bignum_o(out_nonce, source);

    pow_mod_p(out->nonce_encoding, generator, out_nonce);
    pow_mod_p(out->message_encoding, key->public_key, out_nonce);
    mul_mod_p(out->message_encoding, out->message_encoding, message);
}

void Crypto_encryption_rep_new(struct encryption_rep *dst)
{
    mpz_init(dst->nonce_encoding);
    mpz_init(dst->message_encoding);
}

void Crypto_encryption_rep_free(struct encryption_rep *dst)
{
    mpz_clear(dst->nonce_encoding);
    mpz_clear(dst->message_encoding);
}

void Crypto_encryption_rep_copy(struct encryption_rep *dst,
                                struct encryption_rep *src)
{
    mpz_set(dst->nonce_encoding, src->nonce_encoding);
    mpz_set(dst->message_encoding, src->message_encoding);
}

void Crypto_dis_proof_new(struct dis_proof_rep *dst)
{
    Crypto_encryption_rep_new(&dst->commitment0);
    Crypto_encryption_rep_new(&dst->commitment1);
    mpz_init(dst->challenge.digest);
    mpz_init(dst->challenge0);
    mpz_init(dst->challenge1);
    mpz_init(dst->response0);
    mpz_init(dst->response1);
}

void Crypto_dis_proof_free(struct dis_proof_rep *dst)
{
    Crypto_encryption_rep_free(&dst->commitment0);
    Crypto_encryption_rep_free(&dst->commitment1);
    mpz_clear(dst->challenge.digest);
    mpz_clear(dst->challenge0);
    mpz_clear(dst->challenge1);
    mpz_clear(dst->response0);
    mpz_clear(dst->response1);
}

void Crypto_cp_proof_new(struct cp_proof_rep *dst)
{
    Crypto_encryption_rep_new(&dst->commitment);
    mpz_init(dst->response);
}

void Crypto_cp_proof_free(struct cp_proof_rep *dst)
{
    Crypto_encryption_rep_free(&dst->commitment);
    mpz_clear(dst->response);
}

void Crypto_encryption_homomorphic_zero(struct encryption_rep *out)
{
    mpz_set_ui(out->nonce_encoding, 1);
    mpz_set_ui(out->message_encoding, 1);
}

void Crypto_encryption_homomorphic_add(struct encryption_rep *out,
                                       const struct encryption_rep *a,
                                       const struct encryption_rep *b)
{
    mul_mod_p(out->nonce_encoding, a->nonce_encoding, b->nonce_encoding);
    mul_mod_p(out->message_encoding, a->message_encoding, b->message_encoding);
}

//Read a uint4096 as a mpz_t
int mpz_t_fscan(FILE *in, mpz_t out)
{
    uint4096 tmp = malloc(sizeof(struct uint4096_s));
    int res = uint4096_fscan(in, tmp);
    import_uint4096(out, tmp);
    free(tmp);
    return res;
}

int mpz_t_fprint(FILE *out, const mpz_t z)
{
    uint4096 tmp = export_to_uint4096(z);
    int ret = uint4096_fprint(out, tmp);
    free(tmp);
    return ret;
}

bool Crypto_encryption_fprint(FILE *out, const struct encryption_rep *rep)
{
    bool ok = true;

    if (ok)
        ok = fprintf(out, "(") == 1;
    if (ok)
        ok = mpz_t_fprint(out, rep->nonce_encoding);
    if (ok)
        ok = fprintf(out, ",") == 1;
    if (ok)
        ok = mpz_t_fprint(out, rep->message_encoding);
    if (ok)
        ok = fprintf(out, ")") == 1;

    return ok;
}

struct Crypto_encrypted_ballot_new_r
Crypto_encrypted_ballot_new(uint32_t num_selections, uint64_t id)
{
    struct Crypto_encrypted_ballot_new_r result;
    result.result.id = id;
    result.result.num_selections = num_selections;
    result.result.selections =
        malloc(num_selections * sizeof(*result.result.selections));
    result.result.dis_proof =
        malloc(num_selections * sizeof(*result.result.dis_proof));

    for (int i = 0; i < num_selections; i++)
    {
        Crypto_encryption_rep_new(&result.result.selections[i]);
        Crypto_dis_proof_new(&result.result.dis_proof[i]);
    }

    Crypto_cp_proof_new(&result.result.cp_proof);
    result.status = CRYPTO_SUCCESS;
    return result;
}

void Crypto_encrypted_ballot_free(struct encrypted_ballot_rep *ballot)
{
    for (int i = 0; i < ballot->num_selections; i++)
    {
        Crypto_encryption_rep_free(&ballot->selections[i]);
    }
    free(ballot->selections);
    Crypto_cp_proof_free(&ballot->cp_proof);
}
