#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <electionguard/voting/tracker.h>

#include "voting/nouns.h"

const char chars[16] = "2346789BCDFGHJKM";

char const *display_noun(uint8_t a, uint8_t b) 
{
    return get_noun((uint16_t)a*16 + (b>>4));
}

size_t chunk_len(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
{
    // c and d aren't used; they're included in the argument list for consistency with display_chunk
    (void)c; (void)d;

    return strlen(display_noun(a, b)) + 5/* hex digits */ + 2/* spaces */;
}

size_t display_chunk(char *out, size_t out_len, uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
{
    char const *noun = display_noun(a, b);
    const size_t noun_len = strlen(noun);
    const size_t space_used = noun_len + 5/* hex digits */ + 2/* spaces */;
    assert(space_used <= out_len);

    memcpy(out, noun, noun_len);
    out[noun_len+0] = ' ';
    out[noun_len+1] = chars[(b & 0x0f) >> 0];
    out[noun_len+2] = chars[(c & 0xf0) >> 4];
    out[noun_len+3] = chars[(c & 0x0f) >> 0];
    out[noun_len+4] = chars[(d & 0xf0) >> 4];
    out[noun_len+5] = chars[(d & 0x0f) >> 0];
    out[noun_len+6] = ' ';
    return space_used;
}

char *display_ballot_tracker(struct ballot_tracker tracker) 
{
    // First compute how much space we need to allocate.
    size_t len = 0, i;
    for(i = 0; i+3 < tracker.len; i += 4)
    {
        len += chunk_len(
            tracker.bytes[i], 
            tracker.bytes[i+1], 
            tracker.bytes[i+2], 
            tracker.bytes[i+3]
        );
    }

    if(i < tracker.len) 
    {
        const size_t remaining = tracker.len - i;
        len += chunk_len(
            remaining > 0 ? tracker.bytes[i+0] : 0,
            remaining > 1 ? tracker.bytes[i+1] : 0,
            remaining > 2 ? tracker.bytes[i+2] : 0,
            remaining > 3 ? tracker.bytes[i+3] : 0);
            // remaining > 3 will always be false; but this way is nice and consistent
    }
    // Do not need to add one for the null terminator, since we'll overwrite the final space with it.

    char *result = malloc(len);
    assert(result != NULL);

    // Now fill the space.
    size_t cur_idx = 0;
    for(i = 0; i+3 < tracker.len; i += 4)
    {
        cur_idx += display_chunk(
            result+cur_idx, 
            len-cur_idx, 
            tracker.bytes[i], 
            tracker.bytes[i+1], 
            tracker.bytes[i+2], 
            tracker.bytes[i+3]
        );
    }

    if(i < tracker.len) 
    {
        const size_t remaining = tracker.len - i;
        cur_idx += display_chunk(
            result+cur_idx, 
            len-cur_idx,
            remaining > 0 ? tracker.bytes[i+0] : 0,
            remaining > 1 ? tracker.bytes[i+1] : 0,
            remaining > 2 ? tracker.bytes[i+2] : 0,
            remaining > 3 ? tracker.bytes[i+3] : 0
        );
    }

    // How paranoid can we be? Let's find out.
    assert(cur_idx == len);
    
    // Null terminator.
    result[cur_idx-1] = '\0';

    return result;
}
