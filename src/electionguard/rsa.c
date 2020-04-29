/**********************************************************************
 *                                                                    *
 * Created by Adam Brockett && modified by Dragan Stosic  for         *
 *  Election Guard project.                                           *
 *                                                                    *
 * Copyright (c) 2010                                                 *
 *                                                                    *
 * Redistribution and use in source and binary forms, with or without *
 * modification is allowed.                                           *
 *                                                                    *
 * source : https://github.com/gilgad13/rsa-gmp/blob/master/rsa.c     *
 * Modified in order to fix memory leaks                              *
 **********************************************************************/
#include <gmp.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include "electionguard/rsa.h"
#include "random_source.h"

/* Assumes mpz_t's are initted in priv_key and pub_key */
void generate_keys(rsa_private_key* priv_key, rsa_public_key* pub_key)
{
    uint8_t buffer[BUFFER_SIZE];
    mpz_t lambda, gcd, tmp1, tmp2;
    mpz_inits(gcd,tmp1, tmp2, lambda, NULL);
    mpz_t u_1, u_2;
    mpz_init(u_1);
    mpz_init(u_2);

    RandomSource source;
    struct RandomSource_new_r source_r = RandomSource_new();
    source = source_r.source;

    mpz_set_ui(priv_key->e, 3);

    /* Select p */
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = RandomSource_get_byte(source);
    }

    buffer[0] |= 0xC0;                                  // Set the top two bits to 1 to ensure int(tmp) is relatively large
    buffer[BUFFER_SIZE - 1] |= 0x01;                    // Set the bottom bit to 1 to ensure int  is odd
    mpz_import(tmp1, BUFFER_SIZE, 1, sizeof(buffer[0]), 0, 0, buffer);
    mpz_nextprime(priv_key->p, tmp1);
    mpz_mod(tmp2, priv_key->p, priv_key->e);            // If p mod e == 1, gcd(lambda, e) != 1
    while(!mpz_cmp_ui(tmp2, 1))
    {
        mpz_nextprime(priv_key->p, priv_key->p);
        mpz_mod(tmp2, priv_key->p, priv_key->e);
    }


    /* Select q */
    do {
        for(int i = 0; i < BUFFER_SIZE; i++)
        {
            RandomSource_uniform_bignum_o(u_2, source);
            buffer[i] = mpz_get_ui(u_2) % 0xFF;
            mpz_clear(u_2);
            mpz_init(u_2);
        }

        buffer[0] |= 0xC0;                              // Set the top two bits to 1 to ensure int(tmp) is relatively large
        buffer[BUFFER_SIZE - 1] |= 0x01;                // Set the bottom bit to 1 to ensure int(tmp) is odd

        mpz_import(tmp1, (BUFFER_SIZE), 1, sizeof(buffer[0]), 0, 0, buffer);
        mpz_nextprime(priv_key->q, tmp1);
        mpz_mod(tmp2, priv_key->q, priv_key->e);
        while(!mpz_cmp_ui(tmp2, 1))
        {
            mpz_nextprime(priv_key->q, priv_key->q);
            mpz_mod(tmp2, priv_key->q, priv_key->e);
        }
    } while(mpz_cmp(priv_key->p, priv_key->q) == 0);    // If we have identical primes (unlikely), try again

    mpz_mul(priv_key->n, priv_key->p, priv_key->q);     // Calculate n = pq

    mpz_sub_ui(tmp1, priv_key->p, 1);
    mpz_sub_ui(tmp2, priv_key->q, 1);
    mpz_lcm(lambda, tmp1, tmp2);                        // Compute lambda(n) = (p-1)(q-1)
    assert(mpz_cmp(lambda, priv_key->e) > 0);

    mpz_gcd(gcd, priv_key->e, lambda);
    assert(mpz_cmp_ui(gcd, 1) == 0);

    mpz_invert(priv_key->d, priv_key->e, lambda);       // Calculate d (multiplicative inverse of e mod lambda)
    mpz_gcd(tmp1, priv_key->e, lambda);

    mpz_set(pub_key->e, priv_key->e);                   // Set public key
    mpz_set(pub_key->n, priv_key->n);
    mpz_clears(lambda, tmp1, tmp2, gcd, u_1, u_2, NULL);
    RandomSource_free(source);
}
/* Assumes mpz_t encrypted, mpz_t message, public_key pub_key are initialized */
void RSA_Encrypt(mpz_t encrypted, mpz_t message, rsa_public_key* pub_key)
{
    mpz_powm(encrypted, message, pub_key->e, pub_key->n);
}

/* Assumes mpz_t original, mpz_t encrypted, private_key priv_key are initialized */
void RSA_Decrypt(mpz_t original, mpz_t encrypted, rsa_private_key* priv_key)
{
    mpz_powm(original, encrypted, priv_key->d, priv_key->n);
}