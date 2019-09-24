// This file was written under the assumption that timing attacks were not
// dangerous. If this changes in the future, I have left the comment
//         // timing
// at points where there are conditions that may depend on sensitive data, so
// you should begin your code audit by reviewing the code near these comments.

#include <stdlib.h>
#include <string.h>
#include "uint4096.h"

// =============================================================================
//
// Type Definitions
//
// =============================================================================

typedef struct uint8192_s { UINT4096_WORD_T words[2*UINT4096_WORD_COUNT]; } *uint8192;
typedef struct uint8192_s const *const_uint8192;

// Algorithms which use one of these assume that
//      reciprocal = floor(2^8192/modulus)
// without checking it. Violating this assumption is safe (you will still get
// the right answer) but slow (the function may not finish before the universe
// does).
typedef struct Modulus4096_s {
    const struct uint4096_s modulus;
    const struct uint8192_s reciprocal;
} const *const Modulus4096;

// =============================================================================
//
// Dynamically-sized operations that work for arbitrary word-counts
//
// =============================================================================

// Returns whether the addition overflowed or not.
bool uintnwords_add_o(size_t n, UINT4096_WORD_T *out, const UINT4096_WORD_T *a, const UINT4096_WORD_T *b) {
    // Simple ripple-carry adder.
    UINT4096_WORD_T carry = 0;
    while(n-- > 0) {
        UINT4096_WORD_T new_carry = 0;
        new_carry |= !!__builtin_add_overflow(a[n], b[n], out+n);
        new_carry |= !!__builtin_add_overflow(out[n], carry, out+n);
        carry = new_carry;
    }
    return carry;
}

// Returns whether the successor operation overflowed or not.
bool uintnwords_succ_o(size_t n, UINT4096_WORD_T *out, const UINT4096_WORD_T *a) {
    bool carry = true;
    // timing
    while(carry && n-- > 0) {
        carry = __builtin_add_overflow(a[n], 1, a+n);
    }
    return carry;
}

void uintnwords_negate_o(size_t n, UINT4096_WORD_T *out, const UINT4096_WORD_T *a) {
    for(size_t i = 0; i < n; i++) {
        out[i] = ~a[i];
    }
    uintnwords_succ_o(n, out, out);
}

int uintnwords_cmp(size_t n, const UINT4096_WORD_T *a, const UINT4096_WORD_T *b) {
    for(size_t i = 0; i < n; i++) {
        // timing
        if(a[i] < b[i]) return -1;
        if(a[i] > b[i]) return 1;
    }
    return 0;
}

bool uintnwords_le(size_t n, const UINT4096_WORD_T *a, const UINT4096_WORD_T *b) {
    return uintnwords_cmp(n, a, b) <= 0;
}

// =============================================================================
//
// Dynamically-sized operations that work on power-of-two word counts only
//
// Define the function f this way:
// f(n) = 2^n * UINT4096_WORD_SIZE_BITS
// Then we will use uintfn as the name of a dynamically-sized uint with f(n)
// bits; e.g. if n=0 then uintfn is a representation of a uint64_t; n=6 gets
// you a uint4096_s.
//
// =============================================================================

const int UINT4096_HALFWORD_SIZE_BYTES = UINT4096_WORD_SIZE_BYTES/2;
const int UINT4096_HALFWORD_SIZE_BITS = UINT4096_WORD_SIZE_BITS/2;
const UINT4096_WORD_T UINT4096_HALFWORD_BOTTOM_MASK = ((UINT4096_WORD_T)1<<(UINT4096_WORD_SIZE_BITS/2))-1;

// Contract: a and b are 2^n-word-long numbers, and out has enough space for a
// 2^(n+1)-word-long number.
void uintfn_mult_o(unsigned n, UINT4096_WORD_T *out, const UINT4096_WORD_T *a, const UINT4096_WORD_T *b) {
    if(n == 0) {
        // Textbook multiplication on half-word values, because I believe
        // saving one hardware multiplication at the cost of a conditional
        // branch (for the sign stuff) is probably a loss.
        const UINT4096_WORD_T a_upper_halfword = a[0] >> UINT4096_HALFWORD_SIZE_BITS;
        const UINT4096_WORD_T b_upper_halfword = b[0] >> UINT4096_HALFWORD_SIZE_BITS;
        const UINT4096_WORD_T a_lower_halfword = a[0] & UINT4096_HALFWORD_BOTTOM_MASK;
        const UINT4096_WORD_T b_lower_halfword = b[0] & UINT4096_HALFWORD_BOTTOM_MASK;
        out[0] = a_upper_halfword * b_upper_halfword;
        out[1] = a_lower_halfword * b_lower_halfword;
        const UINT4096_WORD_T out_middle_word_1 = a_upper_halfword * b_lower_halfword;
        const UINT4096_WORD_T out_middle_word_2 = b_upper_halfword * a_lower_halfword;
        uintnwords_add_o(2, out, out, (UINT4096_WORD_T []){out_middle_word_1 >> UINT4096_HALFWORD_SIZE_BITS, out_middle_word_1 << UINT4096_HALFWORD_SIZE_BITS});
        uintnwords_add_o(2, out, out, (UINT4096_WORD_T []){out_middle_word_2 >> UINT4096_HALFWORD_SIZE_BITS, out_middle_word_2 << UINT4096_HALFWORD_SIZE_BITS});
    }

    else {
        // Karatsuba multiplication.
        const size_t SMALL = (size_t)1 << (n-1), MEDIUM = SMALL << 1, LARGE = MEDIUM << 1;
        UINT4096_WORD_T out_tmp[LARGE];
        uintfn_mult_o(n-1, out_tmp, a, b);
        uintfn_mult_o(n-1, out_tmp + MEDIUM, a + SMALL, b + SMALL);

        UINT4096_WORD_T sign = -1, diff1[SMALL], diff2[SMALL], addend[MEDIUM+1], sum[LARGE];
        UINT4096_WORD_T *const middle_of_sum = sum+SMALL-1;

        if(uintnwords_le(SMALL, a, a+SMALL)) {
            sign *= 1;
            uintnwords_negate_o(SMALL, diff1, a);
            uintnwords_add_o(SMALL, diff1, diff1, a+SMALL);
        }
        else {
            sign *= -1;
            uintnwords_negate_o(SMALL, diff1, a+SMALL);
            uintnwords_add_o(SMALL, diff1, diff1, a);
        }

        if(uintnwords_le(SMALL, b, b+SMALL)) {
            sign *= 1;
            uintnwords_negate_o(SMALL, diff2, b);
            uintnwords_add_o(SMALL, diff2, diff2, b+SMALL);
        }
        else {
            sign *= -1;
            uintnwords_negate_o(SMALL, diff2, b+SMALL);
            uintnwords_add_o(SMALL, diff2, diff2, b);
        }

        memset(sum, 0, sizeof(sum));
        uintfn_mult_o(n-1, sum+SMALL, diff1, diff2);
        // timing
        if(-1 == sign) uintnwords_negate_o(MEDIUM+1, middle_of_sum, middle_of_sum);

        addend[0] = 0;
        memmove(addend+1, out_tmp, MEDIUM*UINT4096_WORD_SIZE_BYTES);
        uintnwords_add_o(MEDIUM+1, middle_of_sum, middle_of_sum, addend);
        memmove(addend+1, out_tmp+MEDIUM, MEDIUM*UINT4096_WORD_SIZE_BYTES);
        uintnwords_add_o(MEDIUM+1, middle_of_sum, middle_of_sum, addend);

        uintnwords_add_o(LARGE, out, out_tmp, sum);
    }
}

// =============================================================================
//
// 8192-bit operations
//
// =============================================================================

void uint8192_mod_o(uint4096 out, const_uint8192 a, Modulus4096 modulus) {
    UINT4096_WORD_T result[4*UINT4096_WORD_COUNT];
    UINT4096_WORD_T *lores = result + 2*UINT4096_WORD_COUNT;
    UINT4096_WORD_T promoted_modulus[2*UINT4096_WORD_COUNT];
    memset(promoted_modulus, 0, UINT4096_WORD_COUNT*sizeof(UINT4096_WORD_T));
    memcpy(promoted_modulus+UINT4096_WORD_COUNT, modulus->modulus.words, UINT4096_WORD_COUNT*sizeof(UINT4096_WORD_T));

    uintfn_mult_o(UINT4096_LOG2_WORD_COUNT+1, result, a->words, modulus->reciprocal.words);
    uintfn_mult_o(UINT4096_LOG2_WORD_COUNT+1, result, result, promoted_modulus);
    uintnwords_negate_o(2*UINT4096_WORD_COUNT, lores, lores);
    uintnwords_add_o(2*UINT4096_WORD_COUNT, lores, lores, a->words);
    // If I've done my algebra correctly, this loop runs at most once. (If this
    // turns out not to be the case, and you can't restore that property by
    // fixing the algebra, you might want to revisit the decision to allocate
    // and negate the modulus inside the loop.)
    // timing
    while(uintnwords_le(2*UINT4096_WORD_COUNT, promoted_modulus, lores)) {
        UINT4096_WORD_T negated_modulus[2*UINT4096_WORD_COUNT];
        uintnwords_negate_o(2*UINT4096_WORD_COUNT, negated_modulus, promoted_modulus);
        uintnwords_add_o(2*UINT4096_WORD_COUNT, lores, lores, negated_modulus);
    }

    memcpy(out->words, lores + UINT4096_WORD_COUNT, UINT4096_WORD_COUNT*sizeof(UINT4096_WORD_T));
}

// =============================================================================
//
// 4096-bit operations that do not need to dynamically allocate
//
// =============================================================================

void uint4096_zext_o(uint4096 out, const uint8_t *bytes, size_t num_bytes) {
    // ub = upper bound
    const size_t ub = UINT4096_WORD_COUNT*UINT4096_WORD_SIZE_BYTES;
    if(num_bytes > ub) {
        bytes += num_bytes - ub;
        num_bytes = ub;
    }
    memset(out->words, 0, sizeof(out->words));
    for(size_t i = 0; i < num_bytes; i++)
        out->words[UINT4096_WORD_COUNT-1 - i/UINT4096_WORD_SIZE_BYTES] |= (UINT4096_WORD_T)bytes[num_bytes-1 - i] << 8 * (i%UINT4096_WORD_SIZE_BYTES);
}

void uint4096_downcast_o(uint4096 out, Modulus4096 modulus) {
    memmove(out->words, modulus->modulus.words, UINT4096_WORD_COUNT*UINT4096_WORD_SIZE_BYTES);
}

bool uint4096_eq(const_uint4096 a, const_uint4096 b) { return 0 == uintnwords_cmp(UINT4096_WORD_COUNT, a->words, b->words); }
bool uint4096_le(const_uint4096 a, const_uint4096 b) { return 0 >= uintnwords_cmp(UINT4096_WORD_COUNT, a->words, b->words); }
bool uint4096_lt(const_uint4096 a, const_uint4096 b) { return 0 >  uintnwords_cmp(UINT4096_WORD_COUNT, a->words, b->words); }
bool uint4096_ge(const_uint4096 a, const_uint4096 b) { return 0 <= uintnwords_cmp(UINT4096_WORD_COUNT, a->words, b->words); }
bool uint4096_gt(const_uint4096 a, const_uint4096 b) { return 0 <  uintnwords_cmp(UINT4096_WORD_COUNT, a->words, b->words); }

void uint4096_multmod_o(uint4096 out, const_uint4096 a, const_uint4096 b, Modulus4096 modulus) {
    struct uint8192_s product;
    uintfn_mult_o(UINT4096_LOG2_WORD_COUNT, product.words, a->words, b->words);
    uint8192_mod_o(out, &product, modulus);
}

void uint4096_powmod_o(uint4096 out, const_uint4096 base, const_uint4096 exponent, Modulus4096 modulus) {
    // Scan from the left to find the highest set bit.
    int max_bit;
    for(max_bit = 0; max_bit < UINT4096_WORD_COUNT && exponent->words[max_bit] == 0; max_bit++) {}
    UINT4096_WORD_T word = max_bit < UINT4096_WORD_COUNT ? exponent->words[max_bit] : 0;
    max_bit = (UINT4096_WORD_COUNT-1 - max_bit) * UINT4096_WORD_SIZE_BITS; // change from most-significant word index to least-significant bit index
    while(word) { word >>= 1; max_bit++; }

    struct uint4096_s out_tmp;
    struct uint4096_s base_tmp;
    uint4096_zext_o(&out_tmp, (uint8_t[]){1}, 1);
    memcpy(&base_tmp, base, sizeof(struct uint4096_s));

    // Now do some repeated squaring.
    // timing
    for(int i = 0; i < max_bit; i++) {
        if(exponent->words[UINT4096_WORD_COUNT-1 - i/UINT4096_WORD_SIZE_BITS] & (UINT4096_WORD_T)1<<i%UINT4096_WORD_SIZE_BITS)
            uint4096_multmod_o(&out_tmp, &out_tmp, &base_tmp, modulus);
        uint4096_multmod_o(&base_tmp, &base_tmp, &base_tmp, modulus);
    }

    memcpy(out, &out_tmp, sizeof(struct uint4096_s));
}

uint64_t uint4096_logmod(const_uint4096 base, const_uint4096 a, Modulus4096 modulus) {
    uint64_t exponent = 0;
    struct uint4096_s powmod;
    uint4096_zext_o(&powmod, (uint8_t[]){1}, 1);
    while(!uint4096_eq(&powmod, a)) {
        exponent++;
        uint4096_multmod_o(&powmod, &powmod, base, modulus);
    }
    return exponent;
}

void uint4096_copy_o(uint4096 out, const_uint4096 src) {
    memmove(out, src, sizeof(*src));
}

// Currently just a wrapper around free; exists for abstraction boundary's sake.
void uint4096_free(uint4096 a) { free(a); }

bool uint4096_fprint(FILE *out, const_uint4096 a) {
    if(2 != fprintf(out, "0x")) return false;
    for(size_t i = 0; i < UINT4096_WORD_COUNT; i++) {
        if(2*UINT4096_WORD_SIZE_BYTES != fprintf(out, PRIxUINT4096_WORD_T, a->words[i]))
            return false;
    }
    return true;
}

bool uint4096_fscan(FILE *in, uint4096 out) {
    int num_read = fscanf(in, "0x");
    if(0 != num_read) return false;
    for(size_t i = 0; i < UINT4096_WORD_COUNT; i++) {
        num_read = fscanf(in, PRIxUINT4096_WORD_T, &out->words[i]);
        if(1 != num_read) return false;
    }
    return true;
}

// =============================================================================
//
// Dynamically-allocating wrappers around the core 4096-bit operations
//
// =============================================================================

uint4096 uint4096_zext(const uint8_t *bytes, size_t num_bytes) {
    uint4096 out = malloc(sizeof(struct uint4096_s));
    if(out != NULL) uint4096_zext_o(out, bytes, num_bytes);
    return out;
}

uint4096 uint4096_downcast(Modulus4096 modulus) {
    uint4096 out = malloc(sizeof(struct uint4096_s));
    if(out != NULL) uint4096_downcast_o(out, modulus);
    return out;
}

uint4096 uint4096_multmod(const_uint4096 a, const_uint4096 b, Modulus4096 modulus) {
    uint4096 out = malloc(sizeof(struct uint4096_s));
    if(out != NULL) uint4096_multmod_o(out, a, b, modulus);
    return out;
}

uint4096 uint4096_powmod(const_uint4096 base, const_uint4096 exponent, Modulus4096 modulus) {
    uint4096 out = malloc(sizeof(struct uint4096_s));
    if(out != NULL) uint4096_powmod_o(out, base, exponent, modulus);
    return out;
}

uint4096 uint4096_copy(const_uint4096 src) {
    uint4096 out = malloc(sizeof(struct uint4096_s));
    if(out != NULL) uint4096_copy_o(out, src);
    return out;
}

// =============================================================================
//
// Static data
//
// =============================================================================

struct Modulus4096_s p = {
    .modulus = {
        .words = {
            0xFFFFFFFFFFFFFFFF, 0xC90FDAA22168C234, 0xC4C6628B80DC1CD1, 0x29024E088A67CC74,
            0x020BBEA63B139B22, 0x514A08798E3404DD, 0xEF9519B3CD3A431B, 0x302B0A6DF25F1437,
            0x4FE1356D6D51C245, 0xE485B576625E7EC6, 0xF44C42E9A637ED6B, 0x0BFF5CB6F406B7ED,
            0xEE386BFB5A899FA5, 0xAE9F24117C4B1FE6, 0x49286651ECE45B3D, 0xC2007CB8A163BF05,
            0x98DA48361C55D39A, 0x69163FA8FD24CF5F, 0x83655D23DCA3AD96, 0x1C62F356208552BB,
            0x9ED529077096966D, 0x670C354E4ABC9804, 0xF1746C08CA18217C, 0x32905E462E36CE3B,
            0xE39E772C180E8603, 0x9B2783A2EC07A28F, 0xB5C55DF06F4C52C9, 0xDE2BCBF695581718,
            0x3995497CEA956AE5, 0x15D2261898FA0510, 0x15728E5A8AAAC42D, 0xAD33170D04507A33,
            0xA85521ABDF1CBA64, 0xECFB850458DBEF0A, 0x8AEA71575D060C7D, 0xB3970F85A6E1E4C7,
            0xABF5AE8CDB0933D7, 0x1E8C94E04A25619D, 0xCEE3D2261AD2EE6B, 0xF12FFA06D98A0864,
            0xD87602733EC86A64, 0x521F2B18177B200C, 0xBBE117577A615D6C, 0x770988C0BAD946E2,
            0x08E24FA074E5AB31, 0x43DB5BFCE0FD108E, 0x4B82D120A9210801, 0x1A723C12A787E6D7,
            0x88719A10BDBA5B26, 0x99C327186AF4E23C, 0x1A946834B6150BDA, 0x2583E9CA2AD44CE8,
            0xDBBBC2DB04DE8EF9, 0x2E8EFC141FBECAA6, 0x287C59474E6BC05D, 0x99B2964FA090C3A2,
            0x233BA186515BE7ED, 0x1F612970CEE2D7AF, 0xB81BDD762170481C, 0xD0069127D5B05AA9,
            0x93B4EA988D8FDDC1, 0x86FFB7DC90A6C08F, 0x4DF435C934063199, 0xFFFFFFFFFFFFFFFF
        }
    },
    .reciprocal = {
        .words = {
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000001,
            0x0000000000000000, 0x36F0255DDE973DCB, 0x4703CE7E2E815197, 0xA6DB0F588448B611,
            0x64CFCAC5F1872E51, 0xB1F9FBB5BF16FBE7, 0x9689FC0903A801E3, 0xD4802FB8D329550D,
            0xC8C9D3D922EECE9A, 0x5475DB33DB7B83BB, 0x5C0E13D168049BBC, 0x86C5817647B088D1,
            0x7AA5CC40E0203558, 0x8EDB2DE189934137, 0x19FC258D79BC217A, 0xC4B8739CBEA038AA,
            0xA88D0D2F78A77A8A, 0x6FC7FAA8B2BDCA9B, 0xE7502D2F5F6A7B65, 0xF5E4F07AB8B286E4,
            0x1115F024A6E976BD, 0x2BCE3E5190B891AB, 0xBF2331E9C94DE91F, 0xBE8574370494A354,
            0xEAC9BE0B31EB3185, 0x40E4069D556E9DD0, 0x9D5D89D7DE4A75C8, 0x8BB49316C106E4E0,
            0x14B636E60FEBC292, 0xE6249105F5B195FE, 0x906EEF7D26CAF052, 0x9A3E0BC10E100CE0,
            0xA899C59999BF877D, 0xBA72C59BF5CCF326, 0x2EB59041E144783A, 0xEE4CD860EE0B6450,
            0x6DAB2569611BADDB, 0x6B78E82043041716, 0xDEC14CC95569811E, 0x498FDEC9D54BD071,
            0x1EC97A0B25201C17, 0x763900498B0F0308, 0x746D18CEEDB565FF, 0x29964AFA53E3C1B9,
            0x67ED5909172FB4D7, 0xF345A315C4768765, 0x5294F5E127281391, 0x0C258E7E91556CE5,
            0xC145E09DAE0A5C7E, 0xEDE843F90F5DA834, 0x399A4ECB4B05F36F, 0xE586ABCAEF7FA12E,
            0x18B7F0A5564A1616, 0x5D4680FE70FC2A3F, 0xEDFD7374A1D9CA9A, 0xEFBDDF3F3060E354,
            0x8E1CD71EA518F66B, 0x9725EFEC54AEDB55, 0xAE1CF670C7E28D82, 0x07F6D09E269060E5,
            0x591A0721ADFEA421, 0x8E27DA72BF177212, 0x072053629D931252, 0xC14AB0DDCC03AA20
        }
    }
};
Modulus4096 Modulus4096_modulus_default = &p;
const_uint4096 uint4096_modulus_default = &p.modulus;

struct uint4096_s g = {
    .words = {
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
        0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000002
    }
};
const_uint4096 uint4096_generator_default = &g;
