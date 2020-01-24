#include <stdlib.h>

#include <electionguard/api/encrypt_ballot.h>

#include "api/base_hash.h"
#include "directory.h"
#include "api/filename.h"
#include "serialize/voting.h"

static bool initialize_encrypter(struct joint_public_key joint_key);
static bool export_ballot(char *export_path, char *filename_prefix, char **output_filename, char *identifier,
                       struct register_ballot_message *encrypted_ballot_message);

// Global state
static struct api_config api_config;
static Voting_Encrypter encrypter;

bool API_EncryptBallot(uint8_t *selections_byte_array,
                       uint32_t expected_num_selected,
                       struct api_config config,
                       char *external_identifier,
                       struct register_ballot_message *encrypted_ballot_message,
                       char *export_path,
                       char *filename_prefix,
                       char **output_filename,
                       char **tracker_string)
{
    bool ok = true;
    
    // Set global variables

    Crypto_parameters_new();
    api_config = config;
    create_base_hash_code(api_config);

    // Convert selections byte array to boolean array
    // And validate ballot selections before continuing

    bool selections[config.num_selections];
    for(uint32_t i = 0; i < config.num_selections; i++) {
        selections[i] = selections_byte_array[i] == 1;
    }

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
        result = Voting_Encrypter_encrypt_ballot(
            encrypter, 
            external_identifier, 
            selections, 
            expected_num_selected
        );

        if (result.status != VOTING_ENCRYPTER_SUCCESS)
            ok = false;
        else
        {
            *encrypted_ballot_message = result.message;

            // Deserialize the id to get its representation
            struct ballot_identifier_rep id_rep;
            struct serialize_state state = {
                .status = SERIALIZE_STATE_READING,
                .len = result.id.len,
                .offset = 0,
                .buf = (uint8_t *)result.id.bytes,
            };
            Serialize_read_ballot_identifier(&state, &id_rep);
            
            // Convert tracker to string represntation
            *tracker_string = display_ballot_tracker(result.tracker);         
        }
    }

    // Export to file system
    
    if (ok)
        ok = export_ballot(
            export_path, 
            filename_prefix, 
            output_filename, 
            external_identifier, 
            encrypted_ballot_message
        );

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
    {
        free(tracker_string);
        tracker_string = NULL;
    }
}

bool API_EncryptBallot_soft_delete_file(char *export_path, char *filename)
{
    bool ok = true;
    char *default_prefix = "electionguard_encrypted_ballots-";
    char *existing_filename = malloc(FILENAME_MAX + 1);
    if (existing_filename == NULL)
    {
        ok = false;
        return ok;
    }

    char *soft_delete_filename = malloc(FILENAME_MAX + 1);
    if (soft_delete_filename == NULL)
    {
        ok = false;
        return ok;
    }

    ok = generate_filename(export_path, filename, default_prefix, existing_filename);

    ok = generate_unique_filename(export_path, filename, default_prefix, soft_delete_filename);

    uint32_t result = rename(existing_filename, soft_delete_filename);

#ifdef DEBUG_PRINT 	
   if(result != 0) 
   {
      printf("API_EncryptBallot_soft_delete_file: unable to rename the file\n\n");
   }
#endif

    free(existing_filename);
    free(soft_delete_filename);

    return ok;
}

bool export_ballot(char *export_path, char *filename, char **output_filename, 
                    char *identifier,
                       struct register_ballot_message *encrypted_ballot_message)
{
    bool ok = true;
    char *default_prefix = "electionguard_encrypted_ballots-";
    *output_filename = malloc(FILENAME_MAX + 1);
    if (output_filename == NULL)
    {
        ok = false;
        return ok;
    }
    
    ok = generate_filename(export_path, filename, default_prefix, *output_filename);
#ifdef DEBUG_PRINT 
    printf("API_EncryptBallots: generated filename for export at \"%s\"\n", *output_filename); 
#endif

    if (ok && !Directory_exists(export_path))
    {
        ok = create_directory(export_path);
    } 

    if (ok)
    {
        FILE *out = fopen(*output_filename, "a+");
        if (out == NULL)
        {
            printf("API_EncryptBallots: error accessing file\n");
            return false;
        }

        int seek_status = fseek(out, 0, SEEK_END);
        if (seek_status != 0)
        {
            printf("API_EncryptBallots: error seeking file\n");
            return false;
        }
        
        enum Voting_Encrypter_status status =
            Voting_Encrypter_write_ballot(out, identifier, encrypted_ballot_message);
        
        if (status != VOTING_ENCRYPTER_SUCCESS)
        {
            ok = false;
        }

        if (out != NULL)
        {
            fclose(out);
            out = NULL;
        }
    }

    if (!ok)
    {

#ifdef DEBUG_PRINT 
        printf("API_EncryptBallots: error exporting to: %s\n", *output_filename);
#endif

    }

    return ok;
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
