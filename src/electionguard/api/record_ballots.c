#include <stdlib.h>

#include <electionguard/api/record_ballots.h>

#include <log.h>

#include "api/base_hash.h"
#include "directory.h"
#include "api/filename.h"
#include "serialize/voting.h"

static bool initialize_coordinator(uint32_t num_selections);
static bool get_serialized_ballot_identifier(int64_t ballot_id, struct ballot_identifier *ballot_identifier);
static bool export_ballots(char *export_path, char *filename_prefix, char **output_filename);

// Global state
static Voting_Coordinator _record_coordinator = NULL;

bool API_RecordBallots(uint32_t num_selections,
                       uint32_t num_cast_ballots,
                       uint32_t num_spoil_ballots,
                       uint32_t total_num_ballots,
                       char **cast_ids,
                       char **spoil_ids,
                       char **external_identifiers,
                       struct register_ballot_message *encrypted_ballots,
                       char *export_path,
                       char *filename_prefix,
                       char **output_filename,
                       char **casted_tracker_strings,
                       char **spoiled_tracker_strings)
{
    bool ok = true;

    // Set global variables
    Crypto_parameters_new();
    
    // Initialize Voting Coordinator
    if (ok && _record_coordinator == NULL)
    {
        ok = initialize_coordinator(num_selections);
    }

    // Register Ballots
    for (uint32_t i = 0; i < total_num_ballots && ok; i++)
    {
        DEBUG_PRINT(("\nAPI_RecordBallots: register_ballot: id: %s .len = %lu\n", external_identifiers[i], encrypted_ballots[i].len));
        
        char *registered_tracker;
        enum Voting_Coordinator_status status =
            Voting_Coordinator_register_ballot(
                _record_coordinator, 
                external_identifiers[i], 
                encrypted_ballots[i], 
                &registered_tracker
        );

        if (status != VOTING_COORDINATOR_SUCCESS)
        {
            DEBUG_PRINT(("API_RecordBallots: register_ballot: failed!\n"));
            ok = false;
        }
        else 
        {
            DEBUG_PRINT(("API_RecordBallots: register_ballot: success! tracker: %.20s...\n", registered_tracker));
        }
    }

    // Record Cast Ballots
    for (uint32_t i = 0; i < num_cast_ballots && ok; i++)
    {   
        DEBUG_PRINT(("\nAPI_RecordBallots: cast_ballot : id: %s\n", cast_ids[i]));

        char *cast_tracker;
        enum Voting_Coordinator_status status =
                Voting_Coordinator_cast_ballot(_record_coordinator, cast_ids[i], &cast_tracker);

        if (status != VOTING_COORDINATOR_SUCCESS)
        {
            DEBUG_PRINT(("API_RecordBallots: cast_ballot : failed!\n"));
            casted_tracker_strings[i] = NULL;
            ok = false;
        }
        else 
        {
            DEBUG_PRINT(("API_RecordBallots: cast_ballot : sucess! tracker: %.20s...\n", cast_tracker));
        }

        casted_tracker_strings[i] = cast_tracker;
    }
    
    // Record Spoiled Ballots
    for (uint32_t i = 0; i < num_spoil_ballots && ok; i++)
    {   
        DEBUG_PRINT(("\nAPI_RecordBallots: spoil_ballot: id: %s\n", spoil_ids[i]));

        char *spoiled_tracker;
        enum Voting_Coordinator_status status =
            Voting_Coordinator_spoil_ballot(_record_coordinator, spoil_ids[i], &spoiled_tracker);

        if (status != VOTING_COORDINATOR_SUCCESS)
        {
            DEBUG_PRINT(("API_RecordBallots: spoil_ballot : failed!\n"));
            spoiled_tracker_strings[i] = NULL;
            ok = false;
        }
        else 
        {
            DEBUG_PRINT(("API_RecordBallots: spoil_ballot: success! tracker: %.20s...\n", spoiled_tracker));
        }

        spoiled_tracker_strings[i] = spoiled_tracker;
    }

    // Export
    
    if (ok)
    {
        ok = export_ballots(export_path, filename_prefix, output_filename);
    }

    // Clean up

    Voting_Coordinator_clear_buffer(_record_coordinator);
    
    // Unlike other API Methods, we do not call
    // Voting_Coordinator_free, because that component
    // is used both when loading ballots and
    // during record/cast/spoil

    Crypto_parameters_free();

    return ok;
}

void API_RecordBallots_free(char *output_filename,
                            uint32_t num_cast_ballots,
                            uint32_t num_spoil_ballots,
                            char **casted_tracker_strings,
                            char **spoiled_tracker_strings)
{
    if (output_filename != NULL)
    {
        free(output_filename);
        output_filename = NULL;
    }
    
    for (uint32_t i = 0; i < num_cast_ballots; i++)
    {
        if (casted_tracker_strings[i] != NULL)
        {
            free(casted_tracker_strings[i]);
        }
    }

    for (uint32_t i = 0; i < num_spoil_ballots; i++)
    {
        if (spoiled_tracker_strings[i] != NULL)
            free(spoiled_tracker_strings[i]);
    }

    if (_record_coordinator != NULL)
    {
        Voting_Coordinator_free(_record_coordinator);
        _record_coordinator = NULL;
    }
}

bool initialize_coordinator(uint32_t num_selections)
{
    bool ok = true;

    struct Voting_Coordinator_new_r result =
        Voting_Coordinator_new(num_selections);

    if (result.status != VOTING_COORDINATOR_SUCCESS)
        ok = false;
    else
        _record_coordinator = result.coordinator;

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
        *ballot_identifier = (struct ballot_identifier) {
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
    *output_filename = malloc(FILENAME_MAX + 1);
    if (output_filename == NULL)
    {
        ok = false;
        return ok;
    }

    if (ok)
    {
        ok = generate_filename(export_path, filename_prefix, default_prefix, *output_filename);   
        DEBUG_PRINT(("\nAPI_RecordBallots: generated unique filename for export at \"%s\"\n", *output_filename));
    }

    if (ok && !Directory_exists(export_path)) 
    {
        ok = create_directory(export_path);
    }

    if (ok)
    {
        FILE *out = fopen(*output_filename, "w+");
        if (out == NULL)
        {
            INFO_PRINT(("API_RecordBallots: error accessing file\n"));
            return false;
        }
        
        enum Voting_Coordinator_status status =
            Voting_Coordinator_export_buffered_ballots(_record_coordinator, out);
        
        if (status != VOTING_COORDINATOR_SUCCESS)
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
        free(output_filename);
        output_filename = NULL;
        DEBUG_PRINT(("API_RecordBallots: error exporting to: %s\n", *output_filename));
    }

    return ok;
}
