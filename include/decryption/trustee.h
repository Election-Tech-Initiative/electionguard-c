#ifndef __DECRYPTION_TRUSTEE_H__
#define __DECRYPTION_TRUSTEE_H__

#include <stdint.h>
#include <stdio.h>

#include "decryption/messages.h"
#include "trustee_state.h"

typedef struct Decryption_Trustee_s *Decryption_Trustee;

enum Decryption_Trustee_status
{
    DECRYPTION_TRUSTEE_SUCCESS,
    DECRYPTION_TRUSTEE_INSUFFICIENT_MEMORY,
    DECRYPTION_TRUSTEE_INVALID_PARAMS,
    DECRYPTION_TRUSTEE_IO_ERROR,
    DECRYPTION_TRUSTEE_MALFORMED_INPUT,
    DECRYPTION_TRUSTEE_SERIALIZE_ERROR,
    DECRYPTION_TRUSTEE_DESERIALIZE_ERROR,
};

/************************** INITIALIZATION & FREEING ***************************/

// @todo jwaksbaum Do we want this to free the state? My instinct is
// that we would rather have a memory leak than a use-after-free, so
// we probably should not free it and if the caller forgets to free it
// themselves it isn't the end of the world. Also, not freeing is
// consistent with what we do with messages, where we can't free
// because the same message (ie. from a coordinator) may need to be
// consumed multiple times.

/** Create a new trustee. Does not free the trustee state. */
struct Decryption_Trustee_new_r
Decryption_Trustee_new(uint32_t num_trustees, uint32_t threshold,
                       uint32_t num_selections, struct trustee_state state);

struct Decryption_Trustee_new_r
{
    enum Decryption_Trustee_status status;
    Decryption_Trustee decryptor;
};

/** Free a trustee. */
void Decryption_Trustee_free(Decryption_Trustee d);

/********************************* TALLYING **********************************/

/** Parse a voting record, tally it, and store the encrypted tally of all the votes. */
enum Decryption_Trustee_status
Decryption_Trustee_tally_voting_record(Decryption_Trustee d, FILE *in);

/********************************* ANNOUNCING **********************************/

/** Decrypt this trustee's share of the tally. */
struct Decryption_Trustee_compute_share_r
Decryption_Trustee_compute_share(Decryption_Trustee d);

struct Decryption_Trustee_compute_share_r
{
    enum Decryption_Trustee_status status;
    struct decryption_share share;
};

/******************************* COMPENSATING ********************************/

/** Decrypt this trustee's fragment of another trustee's share of the tally.*/
struct Decryption_Trustee_compute_fragments_r
Decryption_Trustee_compute_fragments(Decryption_Trustee d,
                                     struct decryption_fragments_request req);

struct Decryption_Trustee_compute_fragments_r
{
    enum Decryption_Trustee_status status;
    struct decryption_fragments fragments;
};

#endif /* __DECRYPTION_TRUSTEE_H__ */
