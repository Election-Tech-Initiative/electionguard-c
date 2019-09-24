#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Important invariants:
// UINT4096_SIZE_BYTES * 8 = UINT4096_SIZE_BITS = 4096
// WORD_SIZE_BITS = WORD_SIZE_BYTES * 8
// WORD_COUNT * WORD_SIZE_BITS = UINT4096_SIZE_BITS
// WORD_T = uintWORD_SIZE_BITS_t
// 2^LOG2_WORD_COUNT = WORD_COUNT
// PRIxUINT4096_WORD_T uses the right format string for a UINT4096_WORD_T
#define UINT4096_SIZE_BITS 4096
#define UINT4096_SIZE_BYTES 512
#define UINT4096_WORD_COUNT 64
#define UINT4096_WORD_SIZE_BYTES 8
#define UINT4096_WORD_SIZE_BITS 64
#define UINT4096_WORD_T uint64_t
#define UINT4096_LOG2_WORD_COUNT 6
#define PRIxUINT4096_WORD_T ("%016" PRIx64)

struct uint4096_s { UINT4096_WORD_T words[UINT4096_WORD_COUNT]; };

// Some discussion of naming here. There are two conflicts to resolve:
//
// 1. The ElectionGuard convention uses upper case, but the base integer types
//    in C are all uint*_t (lower-case).
// 2. The ElectionGuard convention only includes _r and _s as possible
//    suffixes, preferring no suffix at all for abstract types, but the base
//    integer types use a _t suffix to indicate they are types.
//
// There's no solution that will please everybody, I suspect. I choose the
// ElectionGuard convention everywhere except that I keep uint lower-case,
// because the lower-case uint naming convention is so widespread and
// well-known.
typedef struct uint4096_s *uint4096;
typedef struct uint4096_s const *const_uint4096;
typedef struct Modulus4096_s const *const Modulus4096;

// The functions below that return a uint4096 use NULL to signal out-of-memory
// conditions.

// The first byte is the most-significant byte. If there are too many bytes,
// the most-significant ones are dropped.
uint4096 uint4096_zext(const uint8_t *bytes, size_t num_bytes);
uint4096 uint4096_downcast(Modulus4096 modulus);
bool uint4096_eq(const_uint4096 a, const_uint4096 b);
bool uint4096_le(const_uint4096 a, const_uint4096 b);
bool uint4096_lt(const_uint4096 a, const_uint4096 b);
bool uint4096_ge(const_uint4096 a, const_uint4096 b);
bool uint4096_gt(const_uint4096 a, const_uint4096 b);
uint4096 uint4096_multmod(const_uint4096 a, const_uint4096 b, Modulus4096 modulus);
uint4096 uint4096_powmod(const_uint4096 base, const_uint4096 exponent, Modulus4096 modulus);
uint64_t uint4096_logmod(const_uint4096 base, const_uint4096 a, Modulus4096 modulus);
uint4096 uint4096_copy(const_uint4096 src);
void uint4096_free(uint4096 a);

// _o suffix: overwrite an existing, already-allocated uint4096
void uint4096_zext_o(uint4096 out, const uint8_t *bytes, size_t num_bytes);
void uint4096_downcast_o(uint4096 out, Modulus4096 modulus);
void uint4096_multmod_o(uint4096 out, const_uint4096 a, const_uint4096 b, Modulus4096 modulus);
void uint4096_powmod_o(uint4096 out, const_uint4096 base, const_uint4096 exponent, Modulus4096 modulus);
void uint4096_copy_o(uint4096 out, const_uint4096 src);

bool uint4096_fprint(FILE *out, const_uint4096 a);
bool uint4096_fscan(FILE *in, uint4096 out);

// The prime modulus and generator given in the ElectionGuard specification.
extern Modulus4096 Modulus4096_modulus_default;
extern const_uint4096 uint4096_modulus_default;
extern const_uint4096 uint4096_generator_default;
