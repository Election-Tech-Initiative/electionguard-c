#include <stdlib.h>
#include <string.h>

#include <electionguard/api/tally_votes.h>

#include "api/base_hash.h"
#include "api/filename.h"
#include "directory.h"

// Initialize
static bool initialize_coordinator(void);
static bool initialize_trustees(uint32_t num_decrypting_trustees, struct trustee_state *trustee_states);

// Tally and Decrypt
static bool tally_ballots(char *in_ballots_filename);
static bool decrypt_tally_shares();
static bool decrypt_tally_decryption_fragments(
    bool *requests_present, struct decryption_fragments_request *requests);
static bool request_missing_trustee_tally_shares(
    bool *request_present, struct decryption_fragments_request *requests);
static bool export_tally_votes(char *export_path, char *filename_prefix,
                               char **output_filename, uint32_t *tally_results_array);

// Global state
static struct api_config api_config;
static Decryption_Coordinator coordinator;
static Decryption_Trustee decryption_trustees[MAX_TRUSTEES];

bool API_TallyVotes(struct api_config config,
                    struct trustee_state *trustee_states,
                    uint32_t num_decrypting_trustees,
                    char *ballots_filename, 
                    char *export_path,
                    char *filename_prefix,
                    char **output_filename,
                    uint32_t *tally_results_array)
{
    bool ok = true;

    // Set global variables

    Crypto_parameters_new();
    api_config = config;
    create_base_hash_code(api_config);

    // set local variables
    bool request_present[MAX_TRUSTEES];
    struct decryption_fragments_request requests[MAX_TRUSTEES];

    // Initialize

    if (ok)
        ok = initialize_coordinator();

    if (ok)
        ok = initialize_trustees(num_decrypting_trustees, trustee_states);

    // Tally and Decrypt Shares

    if (ok)
        ok = tally_ballots(ballots_filename);
        
    if (ok)
        ok = decrypt_tally_shares();

    if (ok)
        ok = request_missing_trustee_tally_shares(request_present, requests);
    
    if (ok)
        ok = decrypt_tally_decryption_fragments(request_present, requests);

    if (ok)
        ok = export_tally_votes(export_path, filename_prefix, output_filename, tally_results_array);

    // Cleanup

    for (uint32_t i = 0; i < api_config.num_trustees; i++)
        if (request_present[i])
        {
            free((void *)requests[i].bytes);
            requests[i].bytes = NULL;
        }

    for (uint32_t i = 0; i < num_decrypting_trustees; i++)
        if (decryption_trustees[i] != NULL)
        {
            Decryption_Trustee_free(decryption_trustees[i]);
            decryption_trustees[i] = NULL;
        }

    if (coordinator != NULL)
    {
        Decryption_Coordinator_free(coordinator);
        coordinator = NULL;
    }

    Crypto_parameters_free();
    
    return ok;
}

void API_TallyVotes_free(char *output_filename)                                       
{
    if (output_filename != NULL)
    {
        free(output_filename);
    }
}                                                            

bool initialize_coordinator(void)
{
    bool ok = true;

    struct Decryption_Coordinator_new_r result =
        Decryption_Coordinator_new(api_config.num_trustees, api_config.threshold);

    if (result.status != DECRYPTION_COORDINATOR_SUCCESS)
        ok = false;
    else
        coordinator = result.coordinator;

    return ok;
}

bool initialize_trustees(uint32_t num_decrypting_trustees, struct trustee_state *trustee_states)
{
    if (num_decrypting_trustees < api_config.threshold)
        return false;

    bool ok = true;

    for (uint32_t i = 0; i < num_decrypting_trustees && ok; i++)
    {
        struct Decryption_Trustee_new_r result =
            Decryption_Trustee_new(api_config.num_trustees, api_config.threshold,
                api_config.num_selections, trustee_states[i], base_hash_code);

        if (result.status != DECRYPTION_TRUSTEE_SUCCESS)
            ok = false;
        else
            decryption_trustees[result.trustee_index] = result.decryptor;
    }

    return ok;
}

bool tally_ballots(char *in_ballots_filename)
{
    bool ok = true;

    FILE *in = fopen(in_ballots_filename, "r");

    for (uint32_t i = 0; i < api_config.num_trustees && ok; i++)
    {
        if (decryption_trustees[i] == NULL)
            continue;

        int seek_status = fseek(in, 0L, SEEK_SET);
        if (seek_status != 0)
            ok = false;

        if (ok)
        {
            enum Decryption_Trustee_status status =
                Decryption_Trustee_tally_voting_record(decryption_trustees[i], in);

            if (status != DECRYPTION_TRUSTEE_SUCCESS)
                ok = false;
        }
    }

    if (in != NULL)
    {
        fclose(in);
        in = NULL;
    }

    return ok;
}

bool decrypt_tally_shares()
{
    bool ok = true;

    for (uint32_t i = 0; i < api_config.num_trustees && ok; i++)
    {
        if (decryption_trustees[i] == NULL)
            continue;

        struct decryption_share share = {.bytes = NULL};

        struct Decryption_Trustee_compute_share_r result =
            Decryption_Trustee_compute_share(decryption_trustees[i]);

        if (result.status != DECRYPTION_TRUSTEE_SUCCESS)
            ok = false;
        else
            share = result.share;

        if (ok)
        {
            enum Decryption_Coordinator_status status =
                Decryption_Coordinator_receive_share(coordinator, share);
            if (status != DECRYPTION_COORDINATOR_SUCCESS)
                ok = false;
        }

        if (share.bytes != NULL)
        {
            free((void *)share.bytes);
            share.bytes = NULL;
        }
    }

    return ok;
}

bool request_missing_trustee_tally_shares(
    bool *request_present, struct decryption_fragments_request *requests)
{
    bool ok = true;

    for (uint32_t i = 0; i < api_config.num_trustees && ok; i++)
    {
        requests[i] = (struct decryption_fragments_request){.bytes = NULL};

        if (ok)
        {
            struct Decryption_Coordinator_all_shares_received_r result =
                Decryption_Coordinator_all_shares_received(coordinator);

            if (result.status != DECRYPTION_COORDINATOR_SUCCESS)
                ok = false;
            else
                for (uint32_t i = 0; i < result.num_trustees; i++)
                    requests[i] = result.requests[i],
                    request_present[i] = result.request_present[i];
        }
    }

    return ok;
}

bool decrypt_tally_decryption_fragments(
    bool *requests_present, struct decryption_fragments_request *requests)
{
    bool ok = true;

    for (uint32_t i = 0; i < api_config.num_trustees && ok; i++)
    {
        // a request exists at this index for the announced trustee
        // to decrypt fragments for a missing trustee
        if (requests_present[i])
        {

            // if the decryption trustee at the index is null, then they have not announced
            if (decryption_trustees[i] == NULL)
                ok = false;

            struct decryption_fragments decryption_fragments = {.bytes = NULL};
            {
                struct Decryption_Trustee_compute_fragments_r result =
                    Decryption_Trustee_compute_fragments(decryption_trustees[i],
                                                         requests[i]);

                if (result.status != DECRYPTION_TRUSTEE_SUCCESS)
                    ok = false;
                else
                    decryption_fragments = result.fragments;
            }

            if (ok)
            {
                enum Decryption_Coordinator_status status =
                    Decryption_Coordinator_receive_fragments(
                        coordinator, decryption_fragments);

                if (status != DECRYPTION_COORDINATOR_SUCCESS)
                    ok = false;
            }

            if (decryption_fragments.bytes != NULL)
            {
                free((void *)decryption_fragments.bytes);
                decryption_fragments.bytes = NULL;
            }
        }
    }

    return ok;
}

bool export_tally_votes(char *export_path, char *filename_prefix,
                        char **output_filename, uint32_t *tally_results_array)
{
    bool ok = true;
    char *default_prefix = "electionguard_tally-";
    *output_filename = malloc(FILENAME_MAX * + 1);
    ok = generate_unique_filename(export_path, filename_prefix, default_prefix, *output_filename);   
#ifdef DEBUG_PRINT 
    printf("API_TALLYVOTES :: generated unique filename for export at \"%s\"\n", *output_filename);
#endif

    if (ok && !Directory_exists(export_path)) 
    {
        ok = create_directory(export_path);
    }

    if (ok)
    {
        FILE *out = fopen(*output_filename, "w+");
        if (out == NULL)
        {
            printf("API_TallyVotes: error accessing file\n");
            return false;
        }

        enum Decryption_Coordinator_status status =
            Decryption_Coordinator_all_fragments_received(coordinator, out, tally_results_array);

        if (status != DECRYPTION_COORDINATOR_SUCCESS)
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
        printf("API_TallyVotes: error exporting to: %s\n", *output_filename);
#endif

    }

    return ok;
}
