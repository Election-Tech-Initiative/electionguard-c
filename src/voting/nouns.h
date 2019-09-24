// This file is included in src, rather than include, because it is intended to
// be internal-only, not part of the public API.
#pragma once
#include <stdint.h>

// Given a number, return the corresponding (null-terminated) word from the
// dictionary. You should copy its contents. There are 2^12 words in the
// dictionary; numbers bigger than this will wrap around.
char const *get_noun(uint16_t index);
