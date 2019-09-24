#include <gmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "uint4096.h"

uint64_t p_array[64] = {
    0xFFFFFFFFFFFFFFFF, 0xC90FDAA22168C234, 0xC4C6628B80DC1CD1,
    0x29024E088A67CC74, 0x020BBEA63B139B22, 0x514A08798E3404DD,
    0xEF9519B3CD3A431B, 0x302B0A6DF25F1437, 0x4FE1356D6D51C245,
    0xE485B576625E7EC6, 0xF44C42E9A637ED6B, 0x0BFF5CB6F406B7ED,
    0xEE386BFB5A899FA5, 0xAE9F24117C4B1FE6, 0x49286651ECE45B3D,
    0xC2007CB8A163BF05, 0x98DA48361C55D39A, 0x69163FA8FD24CF5F,
    0x83655D23DCA3AD96, 0x1C62F356208552BB, 0x9ED529077096966D,
    0x670C354E4ABC9804, 0xF1746C08CA18217C, 0x32905E462E36CE3B,
    0xE39E772C180E8603, 0x9B2783A2EC07A28F, 0xB5C55DF06F4C52C9,
    0xDE2BCBF695581718, 0x3995497CEA956AE5, 0x15D2261898FA0510,
    0x15728E5A8AAAC42D, 0xAD33170D04507A33, 0xA85521ABDF1CBA64,
    0xECFB850458DBEF0A, 0x8AEA71575D060C7D, 0xB3970F85A6E1E4C7,
    0xABF5AE8CDB0933D7, 0x1E8C94E04A25619D, 0xCEE3D2261AD2EE6B,
    0xF12FFA06D98A0864, 0xD87602733EC86A64, 0x521F2B18177B200C,
    0xBBE117577A615D6C, 0x770988C0BAD946E2, 0x08E24FA074E5AB31,
    0x43DB5BFCE0FD108E, 0x4B82D120A9210801, 0x1A723C12A787E6D7,
    0x88719A10BDBA5B26, 0x99C327186AF4E23C, 0x1A946834B6150BDA,
    0x2583E9CA2AD44CE8, 0xDBBBC2DB04DE8EF9, 0x2E8EFC141FBECAA6,
    0x287C59474E6BC05D, 0x99B2964FA090C3A2, 0x233BA186515BE7ED,
    0x1F612970CEE2D7AF, 0xB81BDD762170481C, 0xD0069127D5B05AA9,
    0x93B4EA988D8FDDC1, 0x86FFB7DC90A6C08F, 0x4DF435C934063199,
    0xFFFFFFFFFFFFFFFF};

mpz_t p;
mpz_t q;
mpz_t generator;
mpz_t bignum_one;

void Crypto_parameters_new(){
    mpz_init(p);
    mpz_init(q);
    mpz_init(generator);
    mpz_init(bignum_one);

    mpz_set_ui(generator, 2);
    mpz_set_ui(bignum_one, 1);
    mpz_import(p, 64, 1, 8, 0, 0, p_array);
    // In the v0.8 spec this is much smaller -- a 256-bit number instead.
    mpz_sub(q, p, bignum_one);
}

void Crypto_parameters_free(){
    mpz_clear(p);
    mpz_clear(generator);
}

void print_base16(const mpz_t z){
    char *resStr = mpz_get_str(NULL, 16, z);
    printf("%.20s...\n", resStr);
    //printf("%s\n", resStr);
    free(resStr);
}

void pow_mod_p(mpz_t res, const mpz_t base, const mpz_t exp)
{
    mpz_powm(res, base, exp, p);

    #ifdef DEBUG_PRINT
    printf("Performing operation powmod (base^exp)%%p");
    printf("\nbase = ");
    print_base16(base);
    printf("\nexp = ");
    print_base16(exp);
    printf("\np = ");
    print_base16(p);
    printf("\nresult = ");
    print_base16(res);
    printf("\n");
    #endif

}

void mul_mod_p(mpz_t res, const mpz_t a, const mpz_t b){
    mpz_mul(res, a, b);
    mpz_mod(res, res, p);
}

void log_generator_mod_p(mpz_t result, mpz_t a) {
    mpz_set_ui(result, 0);
    mpz_t powmod;
    mpz_init(powmod);
    mpz_set_ui(powmod, 1);
    while(!(0 == mpz_cmp(powmod, a))) {
        mpz_add_ui(result, result,1);
        mul_mod_p(powmod, powmod, generator);
        #ifdef DEBUG_PRINT
        print_base16(powmod);
        #endif
    }
}

void mod_q(mpz_t res, const mpz_t a) {
    mpz_mod(res, a, q);
}

void add_mod_q(mpz_t res, const mpz_t l, const mpz_t r){
    mpz_add(res, l, r);
    mod_q(res, res);
}

void sub_mod_q(mpz_t res, const mpz_t l, const mpz_t r){
    mpz_sub(res, l, r);
    mod_q(res, res);
}

void mul_mod_q(mpz_t res, const mpz_t l, const mpz_t r){
    mpz_mul(res, l, r);
    mod_q(res, res);
}


void div_mod_p(mpz_t res, const mpz_t num, const mpz_t den){
    mpz_t inverse;
    mpz_init(inverse);

    mpz_invert(inverse, den, p);
    mul_mod_p(res, num, inverse);

    mpz_clear(inverse);
}

uint64_t *export_to_256(mpz_t v){
    uint64_t* result = malloc(sizeof(uint64_t) * 4);
    size_t written;
    // print_base16(v);
    mpz_export(result, &written, 1, 8, 0, 0, v);
    assert(written==4);
    return result;
}

uint4096 export_to_uint4096(mpz_t v)
{
    uint4096 result = malloc(sizeof(struct uint4096_s));

    size_t written;
    mpz_export(&result->words, &written, 1, 8, 0, 0, v);
    assert(written==64);

    #ifdef DEBUG_PRINT

    printf("Exporting %zu bits", outsize);

    printf("\nExported to bits\n");
    print_base16(v);
    printf("\n");
    #endif

    return result;
}

void import_uint4096(mpz_t op, uint4096 v)
{
    mpz_import(op, 64, 1, 8, 0, 0, v->words);
    #ifdef DEBUG_PRINT
    printf("\nImported from bits\n");
    print_base16(op);
    printf("\n");
    #endif
}

