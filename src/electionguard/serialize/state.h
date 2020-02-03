#ifndef __SERIALIZE_STATE_H__
#define __SERIALIZE_STATE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum serialize_status
{
    SERIALIZE_STATE_RESERVING,
    SERIALIZE_STATE_WRITING,
    SERIALIZE_STATE_READING,
    SERIALIZE_STATE_INSUFFICIENT_MEMORY,
    SERIALIZE_STATE_BUFFER_TOO_SMALL,
    SERIALIZE_STATE_IO_ERROR
};

struct serialize_state
{
    enum serialize_status status;
    size_t len;
    size_t offset;
    uint8_t *buf;
};

void Serialize_allocate(struct serialize_state *state);

#endif /* __SERIALIZE_STATE_H__ */
