#ifndef __BALLOT_COLLECTION_H__
#define __BALLOT_COLLECTION_H__

#include <inttypes.h>
#include <stdbool.h>

#include "uthash.h"

/**
 * Representation of a ballot in a ballot box
 */
typedef struct ballot_state {
    char *external_identifier;
    uint32_t registered_index;
    bool registered;
    bool cast;
    bool spoiled;
    char *tracker;
    UT_hash_handle hh;
} ballot_state;

enum Ballot_Collection_result
{
    BALLOT_COLLECTION_SUCCESS,
    BALLOT_COLLECTION_ERROR_ALREADY_REGISTERED,
    BALLOT_COLLECTION_ERROR_ALREADY_CAST,
    BALLOT_COLLECTION_ERROR_ALREADY_SPOILED,
    BALLOT_COLLECTION_ERROR_INSUFFICIENT_MEMORY,
    BALLOT_COLLECTION_ERROR_INVALID_ARGUMENT,
    BALLOT_COLLECTION_ERROR_NOT_FOUND,
    BALLOT_COLLECTION_ERROR_UNKNOWN
};

enum Ballot_Collection_result Ballot_Collection_new();

enum Ballot_Collection_result Ballot_Collection_free();

uint32_t Ballot_Collection_size();

enum Ballot_Collection_result Ballot_Collection_register_ballot(char *external_identifier, char *tracker, uint32_t registered_index);

enum Ballot_Collection_result Ballot_Collection_mark_cast(char *external_identifier, char **out_tracker);

enum Ballot_Collection_result Ballot_Collection_mark_spoiled(char *external_identifier, char **out_tracker);

enum Ballot_Collection_result Ballot_Collection_get_ballot(char *external_identifier, struct ballot_state **ballot);

enum Ballot_Collection_result Ballot_Collection_remove_ballot(char *external_identifier);

enum Ballot_Collection_result Ballot_Collection_remove_all();


#endif /* __BALLOT_COLLECTION_H__ */