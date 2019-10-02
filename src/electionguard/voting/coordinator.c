#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/voting/coordinator.h>

#include "serialize/crypto.h"
#include "serialize/voting.h"
#include "voting/message_reps.h"
#include "crypto_reps.h"
#include "voting/num_ballots.h"

// @design jwaksbaum This implementation relies on the fact that the
// Encrypters are generating uids from a shared counter, so we can
// just store the ballots at the index of their uid and they will all
// be contiguous. In a real implementation you would probably need a
// hash table.

struct Voting_Coordinator_s
{
    // The number of selections on each ballot
    uint32_t num_selections;
    struct encryption_rep *selections[MAX_BALLOTS];
    bool registered[MAX_BALLOTS];
    bool cast[MAX_BALLOTS];
    bool spoiled[MAX_BALLOTS];
};

struct Voting_Coordinator_new_r Voting_Coordinator_new(uint32_t num_selections)
{
    struct Voting_Coordinator_new_r result;
    result.status = VOTING_COORDINATOR_SUCCESS;

    // Allocate the ballot box
    result.coordinator = malloc(sizeof(struct Voting_Coordinator_s));
    if (result.coordinator == NULL)
        result.status = VOTING_COORDINATOR_INSUFFICIENT_MEMORY;

    // Initialize the ballot box
    if (result.status == VOTING_COORDINATOR_SUCCESS)
    {
        result.coordinator->num_selections = num_selections;
        memset(result.coordinator->registered, 0,
               sizeof(result.coordinator->registered));
    }

    return result;
}

void Voting_Coordinator_free(Voting_Coordinator ballot_box)
{
    for(size_t i = 0; i < MAX_BALLOTS; i++)
        if(ballot_box->registered[i]){
            for(uint32_t j=0; j<ballot_box->num_selections; j++){
                Crypto_encryption_rep_free(&ballot_box->selections[i][j]);
            }
        }
    free(ballot_box);
}

enum Voting_Coordinator_status
Voting_Coordinator_register_ballot(Voting_Coordinator c,
                                   struct register_ballot_message message)
{
    enum Voting_Coordinator_status status = VOTING_COORDINATOR_SUCCESS;

    struct encrypted_ballot_rep message_rep;

    // Deserialize the message
    if (status == VOTING_COORDINATOR_SUCCESS)
    {
        struct serialize_state state = {
            .status = SERIALIZE_STATE_READING,
            .len = message.len,
            .offset = 0,
            .buf = (uint8_t *)message.bytes,
        };

        Serialize_read_encrypted_ballot(&state, &message_rep);

        if (state.status != SERIALIZE_STATE_READING)
            status = VOTING_COORDINATOR_DESERIALIZE_ERROR;
    }

    // Ensure we have space for another ballot
    if (status == VOTING_COORDINATOR_SUCCESS)
    {
        if (message_rep.id >= MAX_BALLOTS)
            status = VOTING_COORDINATOR_INVALID_BALLOT_ID;
        else if (c->registered[message_rep.id])
            status = VOTING_COORDINATOR_DUPLICATE_BALLOT;
        else if (message_rep.num_selections != c->num_selections)
            status = VOTING_COORDINATOR_INVALID_BALLOT;
    }

    if (status == VOTING_COORDINATOR_SUCCESS)
    {
        // Move the ballot into the ballot box state
        c->selections[message_rep.id] = message_rep.selections;

        // Mark it as registered but unspoiled and uncast
        c->registered[message_rep.id] = true;
        c->cast[message_rep.id] = false;
        c->spoiled[message_rep.id] = false;
    }

    return status;
}

static enum Voting_Coordinator_status
Voting_Coordinator_assert_registered(Voting_Coordinator coordinator,
                                     struct ballot_identifier ballot_id,
                                     uint64_t *i)
{
    enum Voting_Coordinator_status result = VOTING_COORDINATOR_SUCCESS;

    // Check that the bytes look like what we think a ballot
    // identifier should be
    if (ballot_id.len != sizeof(uint64_t))
        result = VOTING_COORDINATOR_INVALID_BALLOT_ID;

    // "Deserialize" the ballot identifier
    if (result == VOTING_COORDINATOR_SUCCESS)
        memcpy(i, ballot_id.bytes, ballot_id.len);

    // Verify that the ballot identifier is a valid index
    if (result == VOTING_COORDINATOR_SUCCESS)
        if (*i >= Voting_num_ballots)
            result = VOTING_COORDINATOR_INVALID_BALLOT_ID;

    // Verify that the ballot is registered
    if (result == VOTING_COORDINATOR_SUCCESS)
        if (!coordinator->registered[*i])
            result = VOTING_COORDINATOR_UNREGISTERED_BALLOT;

    // Check that the ballot isn't already cast or spoiled
    if (result == VOTING_COORDINATOR_SUCCESS)
        //@ assert ballot_box->ballot_ids[i] == ballot->ballot_id;
        if (coordinator->cast[*i] || coordinator->spoiled[*i])
            result = VOTING_COORDINATOR_DUPLICATE_BALLOT;

    return result;
}

enum Voting_Coordinator_status
Voting_Coordinator_cast_ballot(Voting_Coordinator coordinator,
                               struct ballot_identifier ballot_id)
{
    uint64_t i;

    enum Voting_Coordinator_status result =
        Voting_Coordinator_assert_registered(coordinator, ballot_id, &i);

    // Mark the ballot as cast
    if (result == VOTING_COORDINATOR_SUCCESS)
        coordinator->cast[i] = true;

    return result;
}

enum Voting_Coordinator_status
Voting_Coordinator_spoil_ballot(Voting_Coordinator coordinator,
                                struct ballot_identifier ballot_id)
{
    uint64_t i;

    enum Voting_Coordinator_status result =
        Voting_Coordinator_assert_registered(coordinator, ballot_id, &i);

    // Mark the ballot as cast
    if (result == VOTING_COORDINATOR_SUCCESS)
        coordinator->spoiled[i] = true;

    return result;
}

/* Write a single ballot to out, using the format
   <cast> TAB <id> TAB <selection1> ... \n
 */
static enum Voting_Coordinator_status
Voting_Coordinator_write_ballot(FILE *out, uint64_t ballot_id, bool cast,
                                uint32_t num_selections, struct encryption_rep *selections)
{
    enum Voting_Coordinator_status status = VOTING_COORDINATOR_SUCCESS;

    // Write the fixed-length part of the line, ie. everything but the
    // selections
    {
        const char *header_fmt = "%d\t%" PRIu64;
        int io_status = fprintf(out, header_fmt, cast, ballot_id);
        if (io_status < 0)
            status = VOTING_COORDINATOR_IO_ERROR;
    }

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

enum Voting_Coordinator_status
Voting_Coordinator_export_ballots(Voting_Coordinator c, FILE *out)
{
    enum Voting_Coordinator_status status = VOTING_COORDINATOR_SUCCESS;

    // Write the first line containing the number of ballots
    {
        int io_status = fprintf(out, "%" PRIu64 "\n", Voting_num_ballots);
        if (io_status < 0)
            status = VOTING_COORDINATOR_IO_ERROR;
    }

    // Write the second line containing the number of selections per ballot
    if (status == VOTING_COORDINATOR_SUCCESS)
    {
        int io_status = fprintf(out, "%" PRIu32 "\n", c->num_selections);
        if (io_status < 0)
            status = VOTING_COORDINATOR_IO_ERROR;
    }

    // Write each ballot
    for (uint64_t i = 0;
         i < Voting_num_ballots && status == VOTING_COORDINATOR_SUCCESS; i++)
        status = Voting_Coordinator_write_ballot(
            out, i, c->cast[i], c->num_selections, c->selections[i]);

    return status;
}
