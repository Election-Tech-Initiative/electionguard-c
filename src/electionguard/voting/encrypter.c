#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/voting/encrypter.h>
#include <electionguard/secure_zero_memory.h>

#include "crypto_reps.h"
#include "random_source.h"
#include "serialize/crypto.h"
#include "serialize/state.h"
#include "serialize/voting.h"
#include "sha2-openbsd.h"
#include "voting/message_reps.h"

// count of ballots encrypted with this encrypter
uint64_t encrypted_ballot_count = 0;

struct Voting_Encrypter_s
{
    struct uid uid;
    struct joint_public_key_rep joint_key;
    uint32_t num_selections;
    struct hash base_hash;
    RandomSource source;
};

enum Voting_Encrypter_status
Voting_Encrypter_RandomSource_status_convert(enum RandomSource_status status)
{
    switch (status)
    {
    case RANDOM_SOURCE_SUCCESS:
        return VOTING_ENCRYPTER_SUCCESS;
    case RANDOM_SOURCE_INSUFFICIENT_MEMORY:
        return VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
    case RANDOM_SOURCE_IO_ERROR:
        return VOTING_ENCRYPTER_IO_ERROR;
    default:
        return VOTING_ENCRYPTER_UNKNOWN_ERROR;
    }
}

enum Voting_Encrypter_status
Voting_Encrypter_serialize_read_status_convert(enum serialize_status status)
{
    switch (status)
    {
    case SERIALIZE_STATE_READING:
        return VOTING_ENCRYPTER_SUCCESS;
    case SERIALIZE_STATE_INSUFFICIENT_MEMORY:
        return VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
    case SERIALIZE_STATE_BUFFER_TOO_SMALL:
        return VOTING_ENCRYPTER_DESERIALIZE_ERROR;
    default:
        return VOTING_ENCRYPTER_UNKNOWN_ERROR;
    }
}

struct Voting_Encrypter_new_r
Voting_Encrypter_new(struct uid uid, struct joint_public_key joint_key,
                     uint32_t num_selections, raw_hash base_hash)
{
    struct Voting_Encrypter_new_r result;
    result.encrypter = NULL;
    result.status = VOTING_ENCRYPTER_SUCCESS;

    // Allocate the Encrypter
    result.encrypter = malloc(sizeof(struct Voting_Encrypter_s));
    if (result.encrypter != NULL)
    {
        secure_zero_memory(result.encrypter, sizeof(struct Voting_Encrypter_s));
    }
    else
    {
        result.status = VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
    }

    // Clone the uid
    uint8_t *uid_buf = NULL;
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        uid_buf = malloc(uid.len);
        if (uid_buf == NULL)
            result.status = VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
        else
            memcpy(uid_buf, uid.bytes, uid.len);

        result.encrypter->uid = (struct uid){
            .len = uid.len,
            .bytes = uid_buf,
        };
    }

    // Clone the joint key
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = joint_key.len,
            .offset = 0,
            .buf = (uint8_t *)joint_key.bytes // discard const-ness and pray
        };
        Crypto_joint_public_key_init(&result.encrypter->joint_key);
        Serialize_read_joint_public_key(&state, &result.encrypter->joint_key);
        result.status =
            Voting_Encrypter_serialize_read_status_convert(state.status);
    }

    // Get a random source
    RandomSource source = NULL;
    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct RandomSource_new_r rs = RandomSource_new();
        result.status = Voting_Encrypter_RandomSource_status_convert(rs.status);
        if (VOTING_ENCRYPTER_SUCCESS == result.status)
        {
            source = rs.source;
            result.encrypter->source = rs.source;
        }
    }

    if (result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        result.encrypter->num_selections = num_selections;
        mpz_init(result.encrypter->base_hash.digest);
        Crypto_hash_reduce(&result.encrypter->base_hash, base_hash);
    }

    if (VOTING_ENCRYPTER_SUCCESS != result.status)
    {
        if (NULL != source)
            RandomSource_free(source);
        if (NULL != uid_buf)
            free(uid_buf);
        if (NULL != result.encrypter)
            free(result.encrypter);
    }

    return result;
}

void Voting_Encrypter_free(Voting_Encrypter encrypter)
{
    free((void *)encrypter->uid.bytes);
    RandomSource_free(encrypter->source);
    Crypto_joint_public_key_free(&encrypter->joint_key);
    free((void *)encrypter);
}

enum Voting_Encrypter_status
Voting_Encrypter_Crypto_status_convert(enum Crypto_status status)
{
    switch (status)
    {
    case CRYPTO_SUCCESS:
        return VOTING_ENCRYPTER_SUCCESS;
    case CRYPTO_INSUFFICIENT_MEMORY:
        return VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
    case CRYPTO_IO_ERROR:
        return VOTING_ENCRYPTER_IO_ERROR;
    default:
        return VOTING_ENCRYPTER_UNKNOWN_ERROR;
    }
}

bool Validate_selections(bool const *selections, uint32_t num_selections, uint32_t expected_num_selected)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < num_selections; i++)
    {
        if (selections[i])
            count += 1;
    }
    return count == expected_num_selected ? true : false;
}

struct Voting_Encrypter_encrypt_ballot_r
Voting_Encrypter_encrypt_ballot(Voting_Encrypter encrypter,
                                char *external_identifier,
                                bool const *selections,
                                uint32_t expected_num_selected)
{
    // TODO: associate the external_identifier with the internal one, possibly via hash
    
    uint64_t internal_ballot_id = encrypted_ballot_count;
    struct Voting_Encrypter_encrypt_ballot_r ballot_result;
    ballot_result.status = VOTING_ENCRYPTER_SUCCESS;

    // validate selection
    if (!Validate_selections(selections, encrypter->num_selections, expected_num_selected))
    {
        ballot_result.status = VOTING_ENCRYPTER_SELECTION_ERROR;
    }

    // TODO: refactor this since this block exists in record_ballots.c
    // Construct the ballot id
    if (ballot_result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct ballot_identifier_rep rep = {
            .id = internal_ballot_id
        };

        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL,
        };

        Serialize_reserve_ballot_identifier(&state, &rep);
        Serialize_allocate(&state);
        Serialize_write_ballot_identifier(&state, &rep);

        if (state.status != SERIALIZE_STATE_WRITING)
        {
            ballot_result.status = VOTING_ENCRYPTER_SERIALIZE_ERROR;
        }
        else
        {
            ballot_result.id = (struct ballot_identifier)
            {
                .len = state.len,
                .bytes = state.buf,
            };
        }
    }

    // Construct the message
    // TODO: refactor this out as it is similar to the one in coordinator.c
    struct encrypted_ballot_rep encrypted_ballot;
    if (ballot_result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        // TODO: verify that encrypted_ballot_count can be duplicated in a single file
        // but not in a single instance
        struct Crypto_encrypted_ballot_new_r temp_result =
            Crypto_encrypted_ballot_new(encrypter->num_selections,
                                        internal_ballot_id);
        encrypted_ballot = temp_result.result;

        // TODO: use this pattern for swith/case conversions?
        ballot_result.status = Voting_Encrypter_Crypto_status_convert(temp_result.status);
    }

    if (ballot_result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct encryption_rep tally;
        Crypto_encryption_rep_new(&tally);
        Crypto_encryption_homomorphic_zero(&tally);

        mpz_t nonce, aggregate_nonce;
        mpz_init(nonce);
        mpz_init(aggregate_nonce);

        // encrypt the ballot
        for (uint32_t i = 0; i < encrypter->num_selections; i++)
        {
            Crypto_encrypt(
                &encrypted_ballot.selections[i], 
                nonce, 
                encrypter->source,
                &encrypter->joint_key,
                selections[i] 
                    ? generator /*g^1*/ 
                    : bignum_one /*g^0*/
            );

            Crypto_encryption_homomorphic_add(
                &tally, &tally, &encrypted_ballot.selections[i]);
                
            if (i == 0)
            {
                mpz_set(aggregate_nonce, nonce);
            }
            else
            {
                add_mod_q(aggregate_nonce, aggregate_nonce, nonce);
            }

            Crypto_generate_dis_proof(&encrypted_ballot.dis_proof[i],
                                      encrypter->source,
                                      encrypter->base_hash,
                                      selections[i],
                                      encrypter->joint_key.public_key,
                                      encrypted_ballot.selections[i], 
                                      nonce);
            if (!Crypto_check_dis_proof(encrypted_ballot.dis_proof[i],
                                        encrypted_ballot.selections[i],
                                        encrypter->base_hash,
                                        encrypter->joint_key.public_key))
            {
                ballot_result.status = VOTING_ENCRYPTER_UNKNOWN_ERROR;
            }
        }

        Crypto_generate_aggregate_cp_proof(
            &encrypted_ballot.cp_proof, 
            encrypter->source, aggregate_nonce,
            tally, encrypter->base_hash,
            encrypter->joint_key.public_key
        );

        if (!Crypto_check_aggregate_cp_proof(encrypted_ballot.cp_proof, tally,
                                             encrypter->base_hash,
                                             encrypter->joint_key.public_key,
                                             expected_num_selected))
        {
            ballot_result.status = VOTING_ENCRYPTER_UNKNOWN_ERROR;
        }

        mpz_clear(nonce);
        mpz_clear(aggregate_nonce);
        Crypto_encryption_rep_free(&tally);

        struct serialize_state state = {
            .status = SERIALIZE_STATE_RESERVING,
            .len = 0,
            .offset = 0,
            .buf = NULL
        };

        Serialize_reserve_encrypted_ballot(&state, &encrypted_ballot);
        Serialize_allocate(&state);
        Serialize_write_encrypted_ballot(&state, &encrypted_ballot);

        if (state.status != SERIALIZE_STATE_WRITING)
            ballot_result.status = VOTING_ENCRYPTER_SERIALIZE_ERROR;
        else
        {
            ballot_result.message = (struct register_ballot_message)
            {
                .len = state.len,
                .bytes = state.buf,
            };
        }
    }
    else 
    {
        printf("Voting_Encrypter_encrypt_ballot: ERROR! encrypting ballot");
    }

    // clear proofs and encrypted ballot
    Crypto_encrypted_ballot_free(&encrypted_ballot);

    // TODO: clear temp_result

    // Construct the ballot tracker
    if (ballot_result.status == VOTING_ENCRYPTER_SUCCESS)
    {
        SHA2_CTX context;
        uint8_t *digest_buffer = malloc(sizeof(uint8_t) * SHA256_DIGEST_LENGTH);

        if (digest_buffer == NULL)
        {
            // handle insufficient memory error
            balotR.status = VOTING_ENCRYPTER_INSUFFICIENT_MEMORY;
            return balotR;
        }

        SHA256Init(&context);
        SHA256Update(&context, ballot_result.message.bytes, ballot_result.message.len);
        SHA256Final(digest_buffer, &context);

        ballot_result.tracker = (struct ballot_tracker)
        {
            .len = SHA256_DIGEST_LENGTH,
            .bytes = digest_buffer,
        };
    }

    if (ballot_result.status == VOTING_ENCRYPTER_SUCCESS)
        encrypted_ballot_count++;

    return ballot_result;
}

enum Voting_Encrypter_status
Voting_Encrypter_write_ballot(FILE *out, char *external_identifier,
                                struct register_ballot_message *encrypted_ballot_message)
{
    enum Voting_Encrypter_status status = VOTING_ENCRYPTER_SUCCESS;

    // TODO: dedupe this code. it is here, and in a few other places
    struct encrypted_ballot_rep message_rep;

    // Deserialize the message
    if (status == VOTING_ENCRYPTER_SUCCESS)
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = encrypted_ballot_message->len,
            .offset = 0,
            .buf = (uint8_t *)encrypted_ballot_message->bytes,
        };

        Serialize_read_encrypted_ballot(&state, &message_rep);

        if (state.status != SERIALIZE_STATE_READING)
            status = VOTING_ENCRYPTER_DESERIALIZE_ERROR;
    }

    // Write the fixed length part of the line
    {
        const char *header_fmt = "%s\t";
        int io_status = fprintf(out, header_fmt, external_identifier);
        if (io_status < 0)
            status = VOTING_ENCRYPTER_IO_ERROR;
    }

    // Write the selections
    for (uint32_t i = 0;
         i < message_rep.num_selections && status == VOTING_ENCRYPTER_SUCCESS; i++) {
        if(fprintf(out, "\t") < 1)
            status = VOTING_ENCRYPTER_IO_ERROR;

        if(VOTING_ENCRYPTER_SUCCESS == status) {
            if(!Crypto_encryption_fprint(out, &message_rep.selections[i])) {
                status = VOTING_ENCRYPTER_IO_ERROR;
            }
        }
    }

    // Write the newline
    if (status == VOTING_ENCRYPTER_SUCCESS)
    {
        int put_status = fputc('\n', out);
        if (put_status == EOF)
            status = VOTING_ENCRYPTER_IO_ERROR;
    }

    // free the ballot rep
    for (uint32_t i = 0; i < message_rep.num_selections; i++)
    {
        Crypto_encryption_rep_free(&message_rep.selections[i]);
    }
    free(message_rep.selections);

    return status;
}
