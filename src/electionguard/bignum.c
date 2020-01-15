#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uint4096.h"

uint64_t old_p_array[64] = {
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

uint64_t p_array[64] = {
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFba,
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFE0175E30B1B0E79,
    0x1DB502994F24DFB1};

uint64_t g_array[64] = {
    0x9B61C275E06F3E38, 0x372F9A9ADE0CDC4C, 0x82F4CE5337B3EF0E,
    0xD28BEDBC01342EB8, 0x9977C8116D741270, 0xD45B0EBE12D96C5A,
    0xEE997FEFDEA18569, 0x018AFE1284E702BB, 0x9B8C78E03E697F37,
    0x8D25BCBCB94FEFD1, 0x2B7F97047F634232, 0x68881C3B96B389E1,
    0x34CB3162CB73ED80, 0x52F7946C7E72907F, 0xD8B96862D443B5C2,
    0x6F7B0E3FDC9F035C, 0xBF0F5AAB670B7901, 0x1A8BCDEBCF421CC9,
    0xCBBE12C788E50328, 0x041EB59D81079497, 0xB667B96049DA04C7,
    0x9D60F527B1C02F7E, 0xCBA66849179CB5CF, 0xBE7C990CD888B69C,
    0x44171E4F54C21A8C, 0xFE9D821F195F7553, 0xB73A705707263EAE,
    0xA3B7AFA7DED79ACF, 0x5A64F3BFB939B815, 0xC52085F40714F4C6,
    0x460B0B0C3598E317, 0x46A06C2A3457676C, 0xB345C8A390EBB942,
    0x8CEECEFA6FCB1C27, 0xA9E527A6C55B8D6B, 0x2B1868D6EC719E18,
    0x9A799605C540F864, 0x1F135D5DC7FB62D5, 0x8E0DE0B6AE3AB90E,
    0x91FB996505D7D928, 0x3DA833FF0CB6CC8C, 0xA7BAFA0E90BB1ADB,
    0x81545A801F0016DC, 0x7088A4DF2CFB7D6D, 0xD876A2A5807BDAA4,
    0x000DAFA2DFB6FBB0, 0xED9D775589156DDB, 0xFC24FF2203FFF9C5,
    0xCF7C85C68F66DE94, 0xC98331F50FEF59CF, 0x8E7CE9D95FA008F7,
    0xC1672D269C163751, 0x012826C4C8F5B5F4, 0xC11EDB62550F3CF9,
    0x3D86F3CC6E22B0E7, 0x69AC659157F40383, 0xB5DF9DB9F8414F6C,
    0xB5FA7D17BDDD3BC9, 0x0DC7BDC39BAF3BE6, 0x02A99E2A37CE3A5C,
    0x098A8C1EFD3CD28A, 0x6B79306CA2C20C55, 0x174218A3935F697E,
    0x813628D2D861BE54};

uint64_t q_array[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
                       0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF43};

mpz_t p;
mpz_t q;
mpz_t generator;
mpz_t bignum_one;

void Crypto_parameters_new()
{
    mpz_init(p);
    mpz_init(q);
    mpz_init(generator);
    mpz_init(bignum_one);

    mpz_import(generator, 64, 1, 8, 0, 0, g_array);
    mpz_set_ui(bignum_one, 1);
    mpz_import(p, 64, 1, 8, 0, 0, p_array);
    // In the v0.8 spec this is much smaller -- a 256-bit number instead.
    mpz_import(q, 4, 1, 8, 0, 0, q_array);
}

void Crypto_parameters_free()
{
    mpz_clear(p);
    mpz_clear(generator);
}

void print_base16(const mpz_t z)
{
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

void pow_mod_q(mpz_t res, const mpz_t base, const mpz_t exp)
{
    mpz_powm(res, base, exp, q);

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

void mul_mod_p(mpz_t res, const mpz_t a, const mpz_t b)
{
    mpz_mul(res, a, b);
    mpz_mod(res, res, p);
}

//This function can only decrypt numbers below 5,000,000.
//if provided a larger number it will return false and a nonsense result
//if this is too slow for larger elections it can be replaced with a precomputed
//lookup table to achieve log n performance relative to the value being decrypted
bool log_generator_mod_p(mpz_t result, mpz_t a)
{
    mpz_t max_decryption;
    mpz_init(max_decryption);
    mpz_set_ui(max_decryption, 5000000);

    mpz_set_ui(result, 0);
    mpz_t powmod;
    mpz_init(powmod);
    mpz_set_ui(powmod, 1);
    while (!(0 == mpz_cmp(powmod, a)))
    {
        mpz_add_ui(result, result, 1);
        mul_mod_p(powmod, powmod, generator);
#ifdef DEBUG_PRINT
        print_base16(powmod);
#endif
    if(mpz_cmp(a,max_decryption) == 0){
        return false;
    }
    }
    return true;
}

void mod_q(mpz_t res, const mpz_t a) { mpz_mod(res, a, q); }

void add_mod_q(mpz_t res, const mpz_t l, const mpz_t r)
{
    mpz_add(res, l, r);
    mod_q(res, res);
}

void sub_mod_q(mpz_t res, const mpz_t l, const mpz_t r)
{
    mpz_sub(res, l, r);
    mod_q(res, res);
}

void mul_mod_q(mpz_t res, const mpz_t l, const mpz_t r)
{
    mpz_mul(res, l, r);
    mod_q(res, res);
}

void div_mod_p(mpz_t res, const mpz_t num, const mpz_t den)
{
    mpz_t inverse;
    mpz_init(inverse);

    mpz_invert(inverse, den, p);
    mul_mod_p(res, num, inverse);

    mpz_clear(inverse);
}

void div_mod_q(mpz_t res, const mpz_t num, const mpz_t den)
{
    mpz_t inverse;
    mpz_init(inverse);

    mpz_invert(inverse, den, q);
    mul_mod_q(res, num, inverse);

    mpz_clear(inverse);
}

// Export a mpz to a number of 64 bit ints. If the last int isn't filled out
// by the number, this function will fail
uint64_t *export_to_64_t(const mpz_t v, int ct)
{
    uint64_t *result = malloc(sizeof(uint64_t) * ct);
    if (result == NULL)
    {
        return NULL;
    }

    size_t written;
    // print_base16(v);
    mpz_export(result, &written, 1, 8, 0, 0, v);
    assert(written == ct);
    return result;
}

// Export a mpz to an array of 64 bit ints. Fills the most significant
// digits with 0 to fill the entire array. Will fail if more than
// ct are written.
uint64_t *export_to_64_t_pad(const mpz_t v, int ct){
    uint64_t *result = malloc(sizeof(uint64_t) * ct);
    if (result == NULL)
    {
        return NULL;
    }

    memset(result, 0, sizeof(uint64_t)*ct);
    uint64_t *tmp = malloc(sizeof(uint64_t) * ct);
    if (tmp == NULL)
    {
        free(result);
        return NULL;
    }

    size_t written;
    mpz_export(tmp, &written, 1, 8, 0, 0, v);

    assert(written <= ct);

    int j=0;
    for(int i = ct - written; i < ct; i++){
        result[i]=tmp[j++];
    }

    free(tmp);
    return result;
}

uint64_t *export_to_256(mpz_t v) { return export_to_64_t(v, 4); }

uint4096 export_to_uint4096(mpz_t v)
{
    uint4096 result = malloc(sizeof(struct uint4096_s));
    if (result == NULL)
    {
        return NULL;
    }

    size_t written;
    mpz_export(&result->words, &written, 1, 8, 0, 0, v);
    assert(written == 64);

#ifdef DEBUG_PRINT

    printf("Exporting %zu bits", written);

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

void import_uint64_ts(mpz_t op, uint64_t *v, int ct)
{
    mpz_import(op, ct, 1, 8, 0, 0, v);
}
