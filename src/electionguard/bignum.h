#pragma once

#include <gmp.h>

#include "uint4096.h"

typedef enum bignum_status
{
    BIGNUM_SUCCESS,
    BIGNUM_INSUFFICIENT_MEMORY,
    BIGNUM_IO_ERROR
} bignum_status;

void pow_mod_p(mpz_t res, const mpz_t base, const mpz_t exp);
void mul_mod_p(mpz_t res, const mpz_t a, const mpz_t b);
void div_mod_p(mpz_t res, const mpz_t num, const mpz_t den);
bool log_generator_mod_p(mpz_t result, mpz_t a);

void mod_q(mpz_t res, const mpz_t a);
void add_mod_q(mpz_t res, const mpz_t l, const mpz_t r);
void mul_mod_q(mpz_t res, const mpz_t l, const mpz_t r);
void sub_mod_q(mpz_t res, const mpz_t l, const mpz_t r);
void pow_mod_q(mpz_t res, const mpz_t base, const mpz_t exp);
void div_mod_q(mpz_t res, const mpz_t num, const mpz_t den);


void import_uint4096(mpz_t op, uint4096 v);
void import_uint64_ts(mpz_t op, uint64_t *v, int ct);

bignum_status export_to_256(mpz_t v, uint64_t **out_result);
bignum_status export_to_uint4096(const mpz_t op, uint4096 *out_result);
bignum_status export_to_64_t(const mpz_t v, int ct, uint64_t **out_result);
bignum_status export_to_64_t_pad(const mpz_t v, int ct, uint64_t **out_result);

extern mpz_t p, q, generator, bignum_one;

void print_base16(const mpz_t z);