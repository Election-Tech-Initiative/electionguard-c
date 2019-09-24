#pragma once

#include <gmp.h>

#include "uint4096.h"

void pow_mod_p(mpz_t res, const mpz_t base, const mpz_t exp);
void mul_mod_p(mpz_t res, const mpz_t a, const mpz_t b);
void div_mod_p(mpz_t res, const mpz_t num, const mpz_t den);
void log_generator_mod_p(mpz_t result, mpz_t a);

void mod_q(mpz_t res, const mpz_t a);
void add_mod_q(mpz_t res, const mpz_t l, const mpz_t r);
void mul_mod_q(mpz_t res, const mpz_t l, const mpz_t r);
void sub_mod_q(mpz_t res, const mpz_t l, const mpz_t r);

void import_uint4096(mpz_t op, uint4096 v);

uint64_t *export_to_256(mpz_t v);
uint4096 export_to_uint4096(const mpz_t op);

extern mpz_t p, q, generator, bignum_one;

void print_base16(const mpz_t z);