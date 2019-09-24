#include <stdlib.h>

#include "serialize/state.h"

void Serialize_allocate(struct serialize_state *state)
{
    if (state->status == SERIALIZE_STATE_RESERVING && state->len > 0)
    {
        state->buf = malloc(state->len);
        if (state->buf == NULL)
            state->status = SERIALIZE_STATE_INSUFFICIENT_MEMORY;
        else
        {
            state->status = SERIALIZE_STATE_WRITING;
            state->offset = 0;
        }
    }
}
