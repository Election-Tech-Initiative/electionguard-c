#ifndef __VOTING_MESSAGES_H__
#define __VOTING_MESSAGES_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * The message that is passed from the encrypter to the ballot box to
 * register a ballot. */
struct register_ballot_message
{
    uint64_t len;
    uint8_t const *bytes;
};

// @todo jwaksbaum We need to decide what kinds of guarantees we
// make/rely on about the uniqueness of ballot trackers and of ballot
// identifiers. We will probably want to program defensively and have
// the coordinator spoil both ballots if there are two lives ballots
// with identical identifiers, even if we try to make sure that never
// happens.

/**
 * Used by a voter to confirm that their ballot was included in the
 * final tally. */
struct ballot_tracker
{
    uint64_t len;
    uint8_t const *bytes;
};

/**
 * Uniquely identifies a ballot in a given polling place for the
 * duration of a ballot's liveness, and is scanned by the ballot box
 * to mark a ballot as cast. */
struct ballot_identifier
{
    uint64_t len;
    uint8_t const *bytes;
};

struct external_identifier_message
{
    uint64_t len;
    uint8_t const *bytes;
};

bool Messages_are_equal(struct register_ballot_message *left_message, struct register_ballot_message *right_message);

#endif /* __VOTING_MESSAGES_H__ */
