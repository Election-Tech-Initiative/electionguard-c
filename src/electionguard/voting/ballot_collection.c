#include <stdlib.h>
#include <stdio.h>

#include "voting/ballot_collection.h"

struct ballot_state *ballot_box = NULL;

static enum Ballot_Collection_result Ballot_Collection_update(
    char *external_identifier, bool cast, bool spoiled, char **out_tracker);
static enum Ballot_Collection_result Ballot_Collection_assert_can_mutate_state(
    struct ballot_state *existing_ballot);

enum Ballot_Collection_result Ballot_Collection_new()
{
    // nothing to do, just following the component initialization pattern
    return BALLOT_COLLECTION_SUCCESS;
}

enum Ballot_Collection_result Ballot_Collection_free()
{
    enum Ballot_Collection_result delete_result = Ballot_Collection_remove_all();
    if (delete_result != BALLOT_COLLECTION_SUCCESS)
    {
        return delete_result;
    }

    ballot_box = NULL;
    return BALLOT_COLLECTION_SUCCESS;
}

uint32_t Ballot_Collection_size()
{
    return HASH_COUNT(ballot_box);
}

enum Ballot_Collection_result Ballot_Collection_register_ballot(char *external_identifier, char *tracker, uint32_t registered_index)
{
    struct ballot_state *existing_ballot, *new_ballot = NULL;
    if (Ballot_Collection_get_ballot(external_identifier, &existing_ballot) == BALLOT_COLLECTION_SUCCESS)
    {
        return BALLOT_COLLECTION_ERROR_ALREADY_REGISTERED;
    }

    new_ballot = (struct ballot_state *)malloc(sizeof *new_ballot);
    if (new_ballot == NULL)
    {
        return BALLOT_COLLECTION_ERROR_INSUFFICIENT_MEMORY;
    }

    new_ballot->external_identifier = external_identifier;
    new_ballot->registered_index = registered_index;
    new_ballot->registered = true;
    new_ballot->cast = false;
    new_ballot->spoiled = false;
    new_ballot->tracker = tracker;

    HASH_ADD_KEYPTR(
        hh, 
        ballot_box, 
        new_ballot->external_identifier, 
        strlen(new_ballot->external_identifier), 
        new_ballot
    );

    return BALLOT_COLLECTION_SUCCESS;
}

enum Ballot_Collection_result Ballot_Collection_mark_cast(char *external_identifier, char **out_tracker)
{
    return Ballot_Collection_update(external_identifier, true, false, out_tracker);
}

enum Ballot_Collection_result Ballot_Collection_mark_spoiled(char *external_identifier, char **out_tracker)
{
    return Ballot_Collection_update(external_identifier, false, true, out_tracker);
}

enum Ballot_Collection_result Ballot_Collection_get_ballot(char *external_identifier, struct ballot_state **ballot)
{
    struct ballot_state *existing_ballot = NULL;
    HASH_FIND_STR(ballot_box, external_identifier, existing_ballot);

    if (existing_ballot != NULL)
    {
        *ballot = existing_ballot;
        existing_ballot = NULL;

        return BALLOT_COLLECTION_SUCCESS;
    } 
    else 
    {
        return BALLOT_COLLECTION_ERROR_NOT_FOUND;
    }
}

enum Ballot_Collection_result Ballot_Collection_remove_ballot(char *external_identifier)
{
    struct ballot_state *existing_ballot;
    if (Ballot_Collection_get_ballot(external_identifier, &existing_ballot) == BALLOT_COLLECTION_ERROR_NOT_FOUND)
    {
        return BALLOT_COLLECTION_ERROR_NOT_FOUND;
    }

    if (existing_ballot == NULL)
    {
        return BALLOT_COLLECTION_ERROR_UNKNOWN;
    }

    HASH_DEL(
        ballot_box, 
        existing_ballot
    );

    free(existing_ballot);
    existing_ballot = NULL;
    return BALLOT_COLLECTION_SUCCESS;
}

enum Ballot_Collection_result Ballot_Collection_remove_all()
{
    struct ballot_state *existing_ballot, *tmp_ballot;

    HASH_ITER(hh, ballot_box, existing_ballot, tmp_ballot)
    {
        HASH_DEL(ballot_box, existing_ballot);
        free(existing_ballot);
    }

    return BALLOT_COLLECTION_SUCCESS;
}

enum Ballot_Collection_result Ballot_Collection_update(
    char *external_identifier, bool cast, bool spoiled, char **out_tracker)
{
    if (cast == spoiled)
    {
        return BALLOT_COLLECTION_ERROR_INVALID_ARGUMENT;
    }

    struct ballot_state *new_ballot_state, *existing_ballot, *tmp = NULL;
    if (Ballot_Collection_get_ballot(external_identifier, &existing_ballot) == BALLOT_COLLECTION_ERROR_NOT_FOUND)
    {
        return BALLOT_COLLECTION_ERROR_NOT_FOUND;
    }

    if (existing_ballot == NULL)
    {
        return BALLOT_COLLECTION_ERROR_UNKNOWN;
    }

    // verify that the existing_ballot is registered and not already cast or spoiled
    enum Ballot_Collection_result assert_can_mutuate = Ballot_Collection_assert_can_mutate_state(
        existing_ballot);
    if (assert_can_mutuate != BALLOT_COLLECTION_SUCCESS)
    {
        return assert_can_mutuate;
    }

    // copy the existing state to the new state
    new_ballot_state = (struct ballot_state *)malloc(sizeof *new_ballot_state);
    if (new_ballot_state == NULL)
    {
        return BALLOT_COLLECTION_ERROR_INSUFFICIENT_MEMORY;
    }

    memcpy(new_ballot_state, existing_ballot, sizeof(ballot_state));
    new_ballot_state->cast = cast;
    new_ballot_state->spoiled = spoiled;

    // replace the existing state with the new state
    HASH_DEL(
        ballot_box, 
        existing_ballot
    );

    free(existing_ballot);
    existing_ballot = NULL;

    HASH_ADD_KEYPTR(
        hh, 
        ballot_box, 
        new_ballot_state->external_identifier, 
        strlen(new_ballot_state->external_identifier), 
        new_ballot_state
    );

    // assign out parameter
    *out_tracker = new_ballot_state->tracker;

    return BALLOT_COLLECTION_SUCCESS;
}

enum Ballot_Collection_result Ballot_Collection_assert_can_mutate_state(
    struct ballot_state *existing_ballot)
{
    if (existing_ballot == NULL)
    {
        return BALLOT_COLLECTION_ERROR_UNKNOWN;
    }

    if (existing_ballot->registered == false) 
    {
        return BALLOT_COLLECTION_ERROR_UNKNOWN;
    }

    if (existing_ballot->cast == true) 
    {
        return BALLOT_COLLECTION_ERROR_ALREADY_CAST;
    }

    if (existing_ballot->spoiled == true) 
    {
        return BALLOT_COLLECTION_ERROR_ALREADY_SPOILED;
    }

    return BALLOT_COLLECTION_SUCCESS;
}
