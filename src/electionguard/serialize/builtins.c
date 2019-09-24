#include "serialize/builtins.h"

// @design jwaksbaum Even when it is unnecessary for these functions
// to take the data as an argument when reserving space, it makes
// things more consistent. And who knows, you can imagine a format
// where the number of bits depends on the actual value.

static uint8_t read_nth_byte(uint64_t data, uint8_t n)
{
    uint64_t const shift = n * 8;
    uint64_t mask = 0xFF;
    mask <<= shift;
    data &= mask;
    data >>= shift;
    return data;
}

static uint64_t write_nth_byte(uint64_t data, uint8_t n, uint8_t byte)
{
    uint64_t const shift = n * 8;
    uint64_t mask = byte;
    mask <<= shift;
    return data | mask;
}

void Serialize_reserve_bool(struct serialize_state *state, bool const *data)
{
    (void)data;
    if (state->status == SERIALIZE_STATE_RESERVING)
        state->len += sizeof(bool);
}

void Serialize_write_bool(struct serialize_state *state, bool const *data)
{
    if (state->status == SERIALIZE_STATE_WRITING)
    {
        if (data)
            state->buf[state->offset++] = 1;
        else
            state->buf[state->offset++] = 0;
    }
}

void Serialize_read_bool(struct serialize_state *state, bool *data)
{
    if (state->status == SERIALIZE_STATE_READING)
        *data = state->buf[state->offset++];
}

void Serialize_reserve_uint8(struct serialize_state *state, uint8_t const *data)
{
    (void)data;
    if (state->status == SERIALIZE_STATE_RESERVING)
        state->len += sizeof(uint8_t);
}

void Serialize_write_uint8(struct serialize_state *state, uint8_t const *data)
{
    if (state->status == SERIALIZE_STATE_WRITING)
        state->buf[state->offset++] = *data;
}

void Serialize_read_uint8(struct serialize_state *state, uint8_t *data)
{
    if (state->status == SERIALIZE_STATE_READING)
        *data = state->buf[state->offset++];
}

void Serialize_reserve_uint32(struct serialize_state *state,
                              uint32_t const *data)
{
    (void)data;
    if (state->status == SERIALIZE_STATE_RESERVING)
        state->len += sizeof(uint32_t);
}

void Serialize_write_uint32(struct serialize_state *state, uint32_t const *data)
{
    static const size_t num_bytes = sizeof(uint32_t) / sizeof(uint8_t);

    if (state->status == SERIALIZE_STATE_WRITING)
    {
        for (size_t i = 0; i < num_bytes; i++)
            state->buf[state->offset + i] = read_nth_byte(*data, i);

        state->offset += num_bytes;
    }
}

void Serialize_read_uint32(struct serialize_state *state, uint32_t *data)
{
    static const size_t num_bytes = sizeof(uint32_t) / sizeof(uint8_t);

    *data = 0;

    if (state->status == SERIALIZE_STATE_READING)
    {
        for (size_t i = 0; i < num_bytes; i++)
            *data = write_nth_byte(*data, i, state->buf[state->offset + i]);

        state->offset += num_bytes;
    }
}

void Serialize_reserve_uint64(struct serialize_state *state,
                              uint64_t const *data)
{
    (void)data;
    if (state->status == SERIALIZE_STATE_RESERVING)
        state->len += sizeof(uint64_t);
}

void Serialize_write_uint64(struct serialize_state *state, uint64_t const *data)
{
    static const size_t num_bytes = sizeof(uint64_t) / sizeof(uint8_t);

    if (state->status == SERIALIZE_STATE_WRITING)
    {
        for (size_t i = 0; i < num_bytes; i++)
            state->buf[state->offset + i] = read_nth_byte(*data, i);

        state->offset += num_bytes;
    }
}

void Serialize_read_uint64(struct serialize_state *state, uint64_t *data)
{
    static const size_t num_bytes = sizeof(uint64_t) / sizeof(uint8_t);

    *data = 0;

    if (state->status == SERIALIZE_STATE_READING)
    {
        for (size_t i = 0; i < num_bytes; i++)
            *data = write_nth_byte(*data, i, state->buf[state->offset + i]);

        state->offset += num_bytes;
    }
}
