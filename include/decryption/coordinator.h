#ifndef __DECRYPTION_COORDINATOR_H__
#define __DECRYPTION_COORDINATOR_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "decryption/messages.h"
#include "max_values.h"

// @design jwaksbaum Should we have separate entities to decrypt
// totals and spoiled ballots? The types are different, but maybe we
// could provide some sort of generic interface? If not, we could
// provide multiple methods on a single entity, or separate entities.
// The methods here are sort of named with this in mind, often
// specifying tally, for this reason.

typedef struct Decryption_Coordinator_s *Decryption_Coordinator;

enum Decryption_Coordinator_status
{
    DECRYPTION_COORDINATOR_SUCCESS,
    DECRYPTION_COORDINATOR_INSUFFICIENT_MEMORY,
    DECRYPTION_COORDINATOR_INVALID_PARAMS,
    DECRYPTION_COORDINATOR_INVALID_TRUSTEE_INDEX,
    DECRYPTION_COORDINATOR_DUPLICATE_TRUSTEE_INDEX,
    DECRYPTION_COORDINATOR_MISSING_TRUSTEES,
    DECRYPTION_COORDINATOR_IO_ERROR,
    DECRYPTION_COORDINATOR_SERIALIZE_ERROR,
    DECRYPTION_COORDINATOR_DESERIALIZE_ERROR,
};

/************************** INITIALIZATION & FREEING ***************************/

/**
 * Create a new decryption coordinator.
 */
struct Decryption_Coordinator_new_r
Decryption_Coordinator_new(uint32_t num_trustees, uint32_t threshold);

struct Decryption_Coordinator_new_r
{
    enum Decryption_Coordinator_status status;
    Decryption_Coordinator coordinator;
};

/**
 * Free a decryption coordinator.
 */
void Decryption_Coordinator_free(Decryption_Coordinator c);

/********************************* ANNOUNCING **********************************/

/* Receive a trustee's share of a decrypted tally */
enum Decryption_Coordinator_status
Decryption_Coordinator_receive_share(Decryption_Coordinator c,
                                     struct decryption_share share);

/**
 * Check that at least the threshold number of trustees have sent
 * their shares. Return a list of share fragments to be provided by
 * each trustee who has sent a share: for each trustee index i from 0
 * to num_trustees (exclusive), if request_present[i] is true, the
 * caller must pass requests[i] to the appropriate trustee, and must
 * free requests[i].bytes. If status is not
 * DECRYPTION_COORDINATOR_SUCCESS, the client is not responsible for
 * freeing anything.
 */
struct Decryption_Coordinator_all_shares_received_r
Decryption_Coordinator_all_shares_received(Decryption_Coordinator c);

struct Decryption_Coordinator_all_shares_received_r
{
    enum Decryption_Coordinator_status status;
    uint32_t num_trustees;
    bool request_present[MAX_TRUSTEES];
    struct decryption_fragments_request requests[MAX_TRUSTEES];
};

/******************************* COMPENSATING ********************************/

/**
 * Receive the requested set of share fragments from a trustee.
*/
enum Decryption_Coordinator_status
Decryption_Coordinator_receive_fragments(Decryption_Coordinator c,
                                         struct decryption_fragments fragments);

// @todo jwaksbaum Do we want to return the number of bytes written?

/**
 * Check that all trustees have sent their share fragments of the
 * missing trustees' shares. Write the decrypted total to out.
*/
enum Decryption_Coordinator_status
Decryption_Coordinator_all_fragments_received(Decryption_Coordinator c,
                                              FILE *out);

#endif
