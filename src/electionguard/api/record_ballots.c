#include <stdlib.h>

#include <electionguard/api/record_ballots.h>

#include "api/base_hash.h"
#include "api/filename.h"
#include "serialize/voting.h"
#include "voting/num_ballots.h"

static bool initialize_coordinator(uint32_t num_selections);
static bool get_serialized_ballot_identifier(int64_t ballot_id, struct ballot_identifier *ballot_identifier);
static bool export_ballots(char *export_path, char *filename_prefix, char **output_filename);

// Global state
static Voting_Coordinator coordinator;

bool API_RecordBallots(uint32_t num_selections,
                       uint32_t num_cast_ballots,
                       uint32_t num_spoil_ballots,
                       uint64_t total_num_ballots,
                       uint64_t *cast_ids,
                       uint64_t *spoil_ids,
                       struct register_ballot_message *encrypted_ballots,
                       char *export_path,
                       char *filename_prefix,
                       char **output_filename)
{
    bool ok = true;
    
    // Set global variables

    Crypto_parameters_new();
    Voting_num_ballots = total_num_ballots;

    // Initialize Voting Coordinator

    if (ok)
        ok = initialize_coordinator(num_selections);

    // Register Ballots

    for (uint64_t i = 0; i < total_num_ballots && ok; i++)
    {
#ifdef DEBUG_PRINT
        printf("API_RecordBallots :: Voting_Coordinator_register_ballot :: i = %lu :: encrypted_ballots[i].len = %lu\n", i, encrypted_ballots[i].len);
#endif
        enum Voting_Coordinator_status status =
            Voting_Coordinator_register_ballot(coordinator, encrypted_ballots[i]);

        if (status != VOTING_COORDINATOR_SUCCESS)
            ok = false;
    }

    // Record Casted Ballots

    for (uint32_t i = 0; i < num_cast_ballots && ok; i++)
    {   
        struct ballot_identifier ballot_identifier;

        ok = get_serialized_ballot_identifier(cast_ids[i], &ballot_identifier);

        if (ok)
        {
#ifdef DEBUG_PRINT
        printf("API_RecordBallots :: Voting_Coordinator_cast_ballot :: i = %u :: cast_ids[i] = %lu\n", i, cast_ids[i]);
#endif
            enum Voting_Coordinator_status status =
                Voting_Coordinator_cast_ballot(coordinator, ballot_identifier);

            if (status != VOTING_COORDINATOR_SUCCESS)
                ok = false;
        }

        // Free current ballot identifier
        
        if (ballot_identifier.bytes != NULL)
        {
            free((void *)ballot_identifier.bytes);
            ballot_identifier.bytes = NULL;
        }
    }
    
    // Record Spoiled Ballots

    for (uint32_t i = 0; i < num_spoil_ballots && ok; i++)
    {   
        struct ballot_identifier ballot_identifier;

        ok = get_serialized_ballot_identifier(spoil_ids[i], &ballot_identifier);

        if (ok)
        {
#ifdef DEBUG_PRINT 
            printf("API_RecordBallots :: Voting_Coordinator_spoil_ballot :: i = %u :: spoil_ids[i] = %lu\n", i, spoil_ids[i]);
#endif
            enum Voting_Coordinator_status status =
                Voting_Coordinator_spoil_ballot(coordinator, ballot_identifier);

            if (status != VOTING_COORDINATOR_SUCCESS)
                ok = false;
        }

        // Free current ballot identifier
        
        if (ballot_identifier.bytes != NULL)
        {
            free((void *)ballot_identifier.bytes);
            ballot_identifier.bytes = NULL;
        }
    }

    // Export
    
    if (ok)
        ok = export_ballots(export_path, filename_prefix, output_filename);

    // Clean up

    if (coordinator != NULL)
    {
        Voting_Coordinator_free(coordinator);
        coordinator = NULL;
    }

    Crypto_parameters_free();

    return ok;
}

void API_RecordBallots_free(char *output_filename)
{
    if (output_filename != NULL)
        free(output_filename);
}

bool initialize_coordinator(uint32_t num_selections)
{
    bool ok = true;

    struct Voting_Coordinator_new_r result =
        Voting_Coordinator_new(num_selections);

    if (result.status != VOTING_COORDINATOR_SUCCESS)
        ok = false;
    else
        coordinator = result.coordinator;

    return ok;
}

bool get_serialized_ballot_identifier(int64_t ballot_id, struct ballot_identifier *ballot_identifier)
{
    bool ok = true;

    struct ballot_identifier_rep rep = {.id = ballot_id};

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
        ok = false;
    else
    {
        *ballot_identifier = (struct ballot_identifier){
            .len = state.len,
            .bytes = state.buf,
        };
    }

    return ok;
}

bool export_ballots(char *export_path, char *filename_prefix, char **output_filename)
{
    bool ok = true;
    char *default_prefix = "electionguard_ballots-";
    *output_filename = malloc(FILENAME_MAX * sizeof(char));
    ok = generate_unique_filename(export_path, filename_prefix, default_prefix, *output_filename);   
#ifdef DEBUG_PRINT 
    printf("API_RecordBallots :: generated unique filename for export at \"%s\"\n", *output_filename);
#endif

    if (ok)
    {
        FILE *out = fopen(*output_filename, "w+");
        
        enum Voting_Coordinator_status status =
            Voting_Coordinator_export_ballots(coordinator, out);
        
        if (status != VOTING_COORDINATOR_SUCCESS)
            ok = false;

        if (out != NULL)
        {
            fclose(out);
            out = NULL;
        }
    }

    return ok;
}
