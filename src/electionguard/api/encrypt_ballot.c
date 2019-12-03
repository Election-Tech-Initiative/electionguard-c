#include <stdlib.h>

#include <electionguard/api/encrypt_ballot.h>

#include "api/base_hash.h"
#include "serialize/voting.h"
#include "voting/num_ballots.h"

static bool initialize_encrypter(struct joint_public_key joint_key);

// Global state
static struct api_config api_config;
static Voting_Encrypter encrypter;

bool API_EncryptBallot(uint8_t *selections_byte_array,
                       struct api_config config,
                       uint64_t *current_num_ballots,
                       uint64_t *identifier,
                       struct register_ballot_message *encrypted_ballot_message,
                       char **tracker_string)
{
    bool ok = true;
    
    // Set global variables

    Crypto_parameters_new();
    api_config = config;
    create_base_hash_code(api_config);
    Voting_num_ballots = *current_num_ballots;

    // Convert selections byte array to boolean array
    // And validate ballot selections before continuing

    bool selections[config.num_selections];
    for(uint32_t i = 0; i < config.num_selections; i++) {
        selections[i] = selections_byte_array[i] == 1;
    }

    if (!Validate_selections(selections, config.num_selections))
        ok = false;

    // Initialize Encrypter

    if (ok)
        ok = initialize_encrypter(api_config.joint_key);

    // Encrypt ballot
    
    struct Voting_Encrypter_encrypt_ballot_r result = {
        .id = {.bytes = NULL},
        .tracker = {.bytes = NULL},
    };
    if (ok)
    {
        result = Voting_Encrypter_encrypt_ballot(encrypter, selections);

        if (result.status != VOTING_ENCRYPTER_SUCCESS)
            ok = false;
        else
        {
            *encrypted_ballot_message = result.message;

            // Deserialize the id to get its ulong representation
            struct ballot_identifier_rep id_rep;
            struct serialize_state state = {
                .status = SERIALIZE_STATE_READING,
                .len = result.id.len,
                .offset = 0,
                .buf = (uint8_t *)result.id.bytes,
            };
            Serialize_read_ballot_identifier(&state, &id_rep);
            *identifier = id_rep.id;
            
            // Convert tracker to string represntation
            *tracker_string = display_ballot_tracker(result.tracker);

            // Voting_Encrypter_encrypt_ballot will increment the global Voting_num_ballots.
            // Update the *current_num_ballots param to have new value tracked by caller
            *current_num_ballots = Voting_num_ballots;            
        }
    }

    // Clean up

    if (result.id.bytes != NULL)
    {
        free((void *)result.id.bytes);
        result.id.bytes = NULL;
    }
    
    if (result.tracker.bytes != NULL)
    {
        free((void *)result.tracker.bytes);
        result.tracker.bytes = NULL;
    }

    if (encrypter != NULL)
    {
        Voting_Encrypter_free(encrypter);
        encrypter = NULL;
    }

    Crypto_parameters_free();

    return ok;
}

void API_EncryptBallot_free(struct register_ballot_message message,
                            char *tracker_string)
{
    if (message.bytes != NULL)
    {
        free((void *)message.bytes);
        message.bytes = NULL;
    }

    if (tracker_string != NULL)
        free(tracker_string);
}

bool initialize_encrypter(struct joint_public_key joint_key)
{
    bool ok = true;

    uint8_t id_buf[1] = { 0 };
    struct uid uid = {
        .len = 1,
        .bytes = id_buf,
    };

    struct Voting_Encrypter_new_r result = Voting_Encrypter_new(
        uid, joint_key, api_config.num_selections, base_hash_code);

    if (result.status != VOTING_ENCRYPTER_SUCCESS)
        ok = false;
    else
        encrypter = result.encrypter;

    return ok;
}
