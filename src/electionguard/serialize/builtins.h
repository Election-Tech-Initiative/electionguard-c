#ifndef __BUILTINS_H__
#define __BUILTINS_H__ 

#include <stdbool.h>

#include "serialize/state.h"

void Serialize_reserve_bool(struct serialize_state *state, bool const *data);

void Serialize_write_bool(struct serialize_state *state, bool const *data);

void Serialize_read_bool(struct serialize_state *state, bool *data);

void Serialize_reserve_uint8(struct serialize_state *state,
                             uint8_t const *data);

void Serialize_write_uint8(struct serialize_state *state, uint8_t const *data);

void Serialize_read_uint8(struct serialize_state *state, uint8_t *data);

void Serialize_reserve_uint32(struct serialize_state *state,
                              uint32_t const *data);

void Serialize_write_uint32(struct serialize_state *state,
                            uint32_t const *data);

void Serialize_read_uint32(struct serialize_state *state, uint32_t *data);

void Serialize_reserve_uint64(struct serialize_state *state,
                              uint64_t const *data);

void Serialize_write_uint64(struct serialize_state *state,
                            uint64_t const *data);

void Serialize_read_uint64(struct serialize_state *state, uint64_t *data);

#endif /* __BUILTINS_H__ */