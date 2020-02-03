#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/voting/coordinator.h>

#include <log.h>

#include "crypto_reps.h"
#include "serialize/crypto.h"
#include "serialize/voting.h"
#include "sha2-openbsd.h"
#include "voting/ballot_collection.h"
#include "voting/message_reps.h"

// @design mwilhelm This implementation utilizes a hash table to keep
// track of external ballot identifiers (strings) that are already
// registered, cast, or spoiled.  Additionally, to support low-memory
// systems it also allows batch-processing when importing ballots.
// there should only ever be one voting coordinator in memory on a system
// and it should retain it's state through the ballot load and cast cycle

/**
 * The current state of a voting coordinator
 */
struct Voting_Coordinator_s
{
    // The number of selections available on each ballot
    uint32_t num_selections;

    // count of ballots registered
    uint32_t registered_num_ballots;

    // count of ballots in the buffer
    uint32_t buffered_num_ballots;

    // external id buffer
    char *buffered_external_id[MAX_BALLOT_PAYLOAD];

    // selections buffer
    struct encryption_rep *selections[MAX_BALLOT_PAYLOAD];
};

static int Voting_Coordinator_ref_count = 0;

// use a static result as a singleton instance
// since only one coordinator should exist
// per application instance
static struct Voting_Coordinator_new_r voting_coordinator_singleton;

struct Voting_Coordinator_new_r Voting_Coordinator_new(uint32_t num_selections)
{
    voting_coordinator_singleton.status = VOTING_COORDINATOR_SUCCESS;

    // sanity-check if the instance exists and matches this election configuration
    if (voting_coordinator_singleton.coordinator != NULL 
        && voting_coordinator_singleton.coordinator->num_selections == num_selections)
    {
#ifdef DEBUG_PRINT
        printf("\nVoting_Coordinator_new: already exists.  Returning existing instance\n");
#endif
        Voting_Coordinator_ref_count++;
        return voting_coordinator_singleton;
    }

    if (voting_coordinator_singleton.coordinator != NULL 
        && voting_coordinator_singleton.coordinator->num_selections != num_selections)
    {
#ifdef DEBUG_PRINT
        printf("\nVoting_Coordinator_new: already exists for a different number of selections.\n");
#endif
        struct Voting_Coordinator_new_r error_result;
        error_result.status = VOTING_COORDINATOR_ERROR_ALREADY_EXISTS;
        return error_result;
    }

    // Allocate the instance
    voting_coordinator_singleton.coordinator = malloc(sizeof(struct Voting_Coordinator_s));
    if (voting_coordinator_singleton.coordinator == NULL)
    {
        voting_coordinator_singleton.status = VOTING_COORDINATOR_INSUFFICIENT_MEMORY;
    }

    // Initialize the instance
    if (voting_coordinator_singleton.status == VOTING_COORDINATOR_SUCCESS)
    {
        voting_coordinator_singleton.coordinator->num_selections = num_selections;
        voting_coordinator_singleton.coordinator->registered_num_ballots = 0;
        voting_coordinator_singleton.coordinator->buffered_num_ballots = 0;

        Ballot_Collection_new();

        Voting_Coordinator_ref_count++;

#ifdef DEBUG_PRINT
        printf("\nVoting_Coordinator_new: success!\n");
#endif
    }

    return voting_coordinator_singleton;
}

enum Voting_Coordinator_status Voting_Coordinator_clear_buffer(Voting_Coordinator coordinator)
{
    for(uint32_t i = 0; i < coordinator->buffered_num_ballots; i++)
    {
        // clear the slections buffer
        if (coordinator->selections[i] != NULL)
        {
            for(uint32_t j = 0; j < coordinator->num_selections; j++)
            {
                Crypto_encryption_rep_free(&coordinator->selections[i][j]);
            }
        }

        // clear references to buffered external_id's
        // but don't actually free the strings
        if (coordinator->buffered_external_id[i] != NULL)
        {
            coordinator->buffered_external_id[i] = NULL;
        }
    }

    coordinator->buffered_num_ballots = 0;

    return VOTING_COORDINATOR_SUCCESS;
}

void Voting_Coordinator_free(Voting_Coordinator coordinator)
{
    if (Voting_Coordinator_ref_count > 0)
    {
        Voting_Coordinator_ref_count--;
    }

#ifdef DEBUG_PRINT
    printf("\nVoting_Coordinator_free: active referneces:  %u\n", Voting_Coordinator_ref_count);
#endif

    if (Voting_Coordinator_ref_count > 0)
    {
        return;
    }

    Voting_Coordinator_clear_buffer(coordinator);
    Ballot_Collection_free();

    coordinator->num_selections = 0;
    coordinator->buffered_num_ballots = 0;
    coordinator->registered_num_ballots = 0;

    voting_coordinator_singleton.status = VOTING_COORDINATOR_SUCCESS;
    voting_coordinator_singleton.coordinator = NULL;

    free(coordinator);
}

enum Voting_Coordinator_status
Voting_Coordinator_register_ballot(Voting_Coordinator coordinator,
                                   char *external_identifier,
                                   struct register_ballot_message message,
                                   char **out_ballot_tracker)
{
    // Verify the ballot does not already exist
    struct ballot_state *existing_ballot = NULL;
    if (Ballot_Collection_get_ballot(external_identifier, &existing_ballot
    ) == BALLOT_COLLECTION_SUCCESS)
    {
        free(existing_ballot);
        return VOTING_COORDINATOR_DUPLICATE_BALLOT;
    }

    // Verify we can load another ballot into the ballot state cache
    if (Ballot_Collection_size() >= MAX_BALLOTS)
    {
        return VOTING_COORDINATOR_INVALID_BALLOT_INDEX;
    }

    // Verify we can load another ballot into the selections buffer cache
    if (coordinator->buffered_num_ballots >= MAX_BALLOT_PAYLOAD)
    {
        return VOTING_COORDINATOR_INSUFFICIENT_MEMORY;
    }

    // Deserialize the message
    struct encrypted_ballot_rep message_rep;
    if (!Serialize_deserialize_register_ballot_message(&message, &message_rep))
    {
        return VOTING_COORDINATOR_DESERIALIZE_ERROR;
    }

    // Verify the message content contains the correct number of selections
    if (message_rep.num_selections != coordinator->num_selections)
    {
        return VOTING_COORDINATOR_INVALID_BALLOT;
    }

    // Reconstruct the ballot tracker    
    SHA2_CTX context;
    uint8_t *digest_buffer = malloc(sizeof(uint8_t) * SHA256_DIGEST_LENGTH);
    if (digest_buffer == NULL)
    {
        // handle insufficient memory error
        return VOTING_COORDINATOR_INSUFFICIENT_MEMORY;
    }

    SHA256Init(&context);
    SHA256Update(&context, message.bytes, message.len);
    SHA256Final(digest_buffer, &context);

    struct ballot_tracker tracker = {
        .len = SHA256_DIGEST_LENGTH,
        .bytes = digest_buffer,
    };

    *out_ballot_tracker = display_ballot_tracker(tracker);

    // clear the tracker message
    if (tracker.bytes != NULL)
    {
        free((void *)tracker.bytes);
        tracker.bytes = NULL;
    }

    // Move the ballot into the ballot box state (registered)
    if (Ballot_Collection_register_ballot(
            external_identifier, *out_ballot_tracker, coordinator->registered_num_ballots
        ) != BALLOT_COLLECTION_SUCCESS)
    {
        // note: case alrady handled with Ballot_Collection_get_ballot,
        // however we respect the failure return response from Ballot collection
        // by returning a non-duplicated failure case
        return VOTING_COORDINATOR_IO_ERROR;
    }

    // cache a handle to the external id for lookups
    coordinator->buffered_external_id[coordinator->buffered_num_ballots] = external_identifier;

    // cache the ballot selections in the buffer
    coordinator->selections[coordinator->buffered_num_ballots] = message_rep.selections;
        
    coordinator->registered_num_ballots++;
    coordinator->buffered_num_ballots++;

    return VOTING_COORDINATOR_SUCCESS;
}

static enum Voting_Coordinator_status
Voting_Coordinator_assert_registered(Voting_Coordinator coordinator,
                                     char *external_identifier)
{
    struct ballot_state *existing_ballot = NULL;
    if (Ballot_Collection_get_ballot(external_identifier, &existing_ballot) != BALLOT_COLLECTION_SUCCESS)
    {
        return VOTING_COORDINATOR_UNREGISTERED_BALLOT;
    }

    if (existing_ballot->cast || existing_ballot->spoiled)
    {
        return VOTING_COORDINATOR_DUPLICATE_BALLOT;
    }

    return VOTING_COORDINATOR_SUCCESS;
}

enum Voting_Coordinator_status
Voting_Coordinator_cast_ballot(Voting_Coordinator coordinator,
                               char *external_identifier, char **out_tracker)
{
    enum Ballot_Collection_result result = Ballot_Collection_mark_cast(
        external_identifier, out_tracker);
    if (result == BALLOT_COLLECTION_SUCCESS)
    {
        return VOTING_COORDINATOR_SUCCESS;
    }

    // TODO: map Ballot Colection enums to Voting Coordinator Enums
    return VOTING_COORDINATOR_INVALID_BALLOT;
}

enum Voting_Coordinator_status
Voting_Coordinator_spoil_ballot(Voting_Coordinator coordinator,
                               char *external_identifier, char **out_tracker)
{
    enum Ballot_Collection_result result = Ballot_Collection_mark_spoiled(
        external_identifier, out_tracker);
    if (result == BALLOT_COLLECTION_SUCCESS)
    {
        return VOTING_COORDINATOR_SUCCESS;
    }

    // TODO: map Ballot Colection enums to Voting Coordinator Enums
    return VOTING_COORDINATOR_INVALID_BALLOT;
}

char *Voting_Coordinator_get_tracker(Voting_Coordinator coordinator,
                                     char *external_identifier)
{
    struct ballot_state *existing_ballot = NULL;
    if (Ballot_Collection_get_ballot(external_identifier, &existing_ballot
    ) != BALLOT_COLLECTION_SUCCESS)
    {
        return NULL;
    }

    char *result = existing_ballot->tracker;
    return result;
}

/* Write a single ballot to the out file to be imported for decryption, using the format
   <cast> TAB <id> TAB <selection1> ... \n
 */
static enum Voting_Coordinator_status
Voting_Coordinator_write_ballot(FILE *out, uint32_t registered_ballot_index, bool cast,
                                uint32_t num_selections, struct encryption_rep *selections)
{
    enum Voting_Coordinator_status status = VOTING_COORDINATOR_SUCCESS;

    // Write the fixed-length part of the line, 
    // ie. everything but the selections
    const char *header_fmt = "%d\t%" PRIu32;
    int io_status = fprintf(out, header_fmt, cast, registered_ballot_index);
    if (io_status < 0)
        status = VOTING_COORDINATOR_IO_ERROR;
    

    // Write the selections
    for (uint32_t i = 0;
         i < num_selections && status == VOTING_COORDINATOR_SUCCESS; i++) {
        if(fprintf(out, "\t") < 1)
            status = VOTING_COORDINATOR_IO_ERROR;

        if(VOTING_COORDINATOR_SUCCESS == status) {
            if(!Crypto_encryption_fprint(out, &selections[i])) {
                status = VOTING_COORDINATOR_IO_ERROR;
            }
        }
    }

    // Write the newline
    if (status == VOTING_COORDINATOR_SUCCESS)
    {
        int put_status = fputc('\n', out);
        if (put_status == EOF)
            status = VOTING_COORDINATOR_IO_ERROR;
    }

    return status;
}

static enum Voting_Coordinator_status
Voting_Coordinator_write_ballots_file_header(Voting_Coordinator coordinator, FILE *out)
{
    enum Voting_Coordinator_status status = VOTING_COORDINATOR_SUCCESS;

    // Write the first line containing the number of ballots
    {
        int io_status = fprintf(out, "%" PRIu32 "\n", coordinator->registered_num_ballots);
        if (io_status < 0)
            status = VOTING_COORDINATOR_IO_ERROR;
    }

    // Write the second line containing the number of selections per ballot
    if (status == VOTING_COORDINATOR_SUCCESS)
    {
        int io_status = fprintf(out, "%" PRIu32 "\n", coordinator->num_selections);
        if (io_status < 0)
            status = VOTING_COORDINATOR_IO_ERROR;
    }

    return status;
}

enum Voting_Coordinator_status
Voting_Coordinator_export_buffered_ballots(Voting_Coordinator coordinator, FILE *out)
{
    enum Voting_Coordinator_status status = VOTING_COORDINATOR_SUCCESS;

    // ensure the file cursor is at the beginning
    int seek_status = fseek(out, 0L, SEEK_SET);

    // write the header
    status = Voting_Coordinator_write_ballots_file_header(coordinator, out);
    if (status != VOTING_COORDINATOR_SUCCESS) 
    {
        return status;
    }

    // flush the write buffer before reading the file
    fflush(out);

    // seek to the end of the file
    int character, number_of_ballots_written = 0; 
    while ((character = fgetc(out)) != EOF) 
    {
        if (character == '\n')
        {
            number_of_ballots_written++;
        }
    }

    uint32_t registered_ballot_index = number_of_ballots_written;

#ifdef DEBUG_PRINT 
    printf("\nVoting_Coordinator_export: writing out %u ballots\n\n", coordinator->buffered_num_ballots);
#endif

    // Write each ballot
    for (uint32_t i = 0;
         i < coordinator->buffered_num_ballots && status == VOTING_COORDINATOR_SUCCESS; 
         i++)
    {
        struct ballot_state *ballot_state = NULL;
        if (Ballot_Collection_get_ballot(
            coordinator->buffered_external_id[i], &ballot_state
        ) != BALLOT_COLLECTION_SUCCESS)
        {
            printf("\n could not find in cache: %s\n", coordinator->buffered_external_id[i]);
            return status = VOTING_COORDINATOR_INVALID_BALLOT_ID;
        }

#ifdef DEBUG_PRINT 
        printf("Voting_Coordinator_export: id: %s registered: %d cast: %d spoiled: %d\n", 
            ballot_state->external_identifier, ballot_state->registered, ballot_state->cast, ballot_state->spoiled);
#endif

        status = Voting_Coordinator_write_ballot(
            out, 
            registered_ballot_index, 
            ballot_state->cast,
            coordinator->num_selections, 
            coordinator->selections[i]
        );

        registered_ballot_index++;
    }

    // clear the selections buffer
    status = Voting_Coordinator_clear_buffer(coordinator);

    return status;
}

static enum Voting_Coordinator_status
Voting_Coordinator_read_ballot(FILE *in,
                               uint32_t num_selections,
                               char *out_external_identifier,
                               struct encryption_rep *out_selections)
{
    enum Voting_Coordinator_status status = VOTING_COORDINATOR_SUCCESS;

    // get the external ballot Id
    {
        int num_read = fscanf(in, "%s", out_external_identifier);
        if (num_read == EOF)
        {
            DEBUG_PRINT(("\nVoting_Coordinator_read_ballot: VOTING_COORDINATOR_END_OF_FILE\n"));
            status = VOTING_COORDINATOR_END_OF_FILE;
        }
        else if (num_read != 1)
        {
            status = VOTING_COORDINATOR_IO_ERROR;
        }
    }
    
    for (uint32_t i = 0;
         i < num_selections && status == VOTING_COORDINATOR_SUCCESS; i++)
    {
        // read the nonce encoding
        int num_read = fscanf(in, "\t(");
        if (0 != num_read)
        {
            status = VOTING_COORDINATOR_IO_ERROR;
        }

        if (status == VOTING_COORDINATOR_SUCCESS)
        {
            if (!mpz_t_fscan(in, out_selections[i].nonce_encoding))
            {
                DEBUG_PRINT(("\nVoting_Coordinator_read_ballot: mpz_t_fscan nonce_encoding VOTING_COORDINATOR_IO_ERROR\n"));
                status = VOTING_COORDINATOR_IO_ERROR;
            }
        }

        // move the cursor to the separator
        if (status == VOTING_COORDINATOR_SUCCESS)
        {
            num_read = fscanf(in, ",");
            if (0 != num_read)
            {
                status = VOTING_COORDINATOR_IO_ERROR;
            }
        }

        // read the message encoding
        if (status == VOTING_COORDINATOR_SUCCESS)
        {
            if (!mpz_t_fscan(in, out_selections[i].message_encoding))
            {
                DEBUG_PRINT(("\nVoting_Coordinator_read_ballot: mpz_t_fscan message_encoding VOTING_COORDINATOR_IO_ERROR\n"));
                status = VOTING_COORDINATOR_IO_ERROR;
            }
        }

        if (status == VOTING_COORDINATOR_SUCCESS)
        {
            num_read = fscanf(in, ")");
            if (0 != num_read)
            {
                status = VOTING_COORDINATOR_IO_ERROR;
            }
        }
    }

    return status;
}

enum Voting_Coordinator_status
Voting_Coordinator_import_encrypted_ballots(Voting_Coordinator coordinator, 
                                            uint64_t start_index, 
                                            uint64_t count,
                                            uint32_t num_selections,
                                            FILE *in,
                                            char **out_external_identifiers,
                                            struct register_ballot_message *out_messages)
{
    enum Voting_Coordinator_status status = VOTING_COORDINATOR_SUCCESS;

    // start at the index
    if (in == NULL || fseek(in, start_index, SEEK_SET) != 0) {
        status = VOTING_COORDINATOR_IO_ERROR;
        return status;
    }

    DEBUG_PRINT(("Voting_Coordinator_import_encrypted_ballots: attempting to import: %ld\n", count));

    int scanResult = 0;
    enum Voting_Coordinator_status load_status = VOTING_COORDINATOR_SUCCESS;

    for (uint32_t i = 0; i < count && load_status == VOTING_COORDINATOR_SUCCESS; i++) {

        out_external_identifiers[i] = malloc(MAX_EXTERNAL_ID_LENGTH*sizeof(char));
        if (out_external_identifiers[i] == NULL)
        {
            load_status = VOTING_COORDINATOR_INSUFFICIENT_MEMORY;
        }
        
        // create an encryption representation
        struct encryption_rep selections[num_selections];
        for (int j = 0; j < num_selections; j++)
        {
            Crypto_encryption_rep_new(&selections[j]);
        }

        // load the data from the row in the file
        load_status = Voting_Coordinator_read_ballot(
            in, 
            num_selections, 
            out_external_identifiers[i], 
            selections);

        if (load_status != VOTING_COORDINATOR_SUCCESS)
        {
            break;
        }

        DEBUG_PRINT(("Voting_Coordinator_import_encrypted_ballots: imported: %s\n", out_external_identifiers[i]));

        // reconstruct the original register_ballot_message
        struct encrypted_ballot_rep encrypted_ballot;
        struct Crypto_encrypted_ballot_new_r result =
            Crypto_encrypted_ballot_new(num_selections, i);
        encrypted_ballot = result.result;

        // TODO: check/convert status from result

        encrypted_ballot.selections = selections;

        // serialize the encrypted ballot
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
            status = VOTING_COORDINATOR_SERIALIZE_ERROR;
        else
        {
            // add the serialization to message_out
            struct register_ballot_message scanned_message = (struct register_ballot_message)
            {
                .len = state.len,
                .bytes = state.buf,
            };

            out_messages[start_index + scanResult] = scanned_message;
        }

        // clean up
        // TODO: free encryption rep?

        // TODO: write scan result count to out param
        // so consumers know how many were loaded
        scanResult++;
    }

    return status;

}
