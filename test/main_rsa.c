#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmp.h>
#include <assert.h>
#include <time.h>
#include "main_rsa.h"

bool rsa_string_message(rsa_private_key* ku, rsa_public_key* kp, mpz_t M, mpz_t C,mpz_t DC) {
    bool ok = false;
    generate_keys(ku, kp);
    char *bn = mpz_get_str(NULL, 16, kp->n);
    printf("kp.n is [%s]\n", bn);
    free(bn);                                       // to avoid memory leak
    bn = mpz_get_str(NULL, 16, kp->e);
    printf("kp.e is [%s]\n\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->n);
    printf("ku.n is [%s]\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->e);
    printf("ku.e is [%s]\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->d);
    printf("ku.d is [%s]\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->p);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->p);
    printf("ku.p is [%s]\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->q);
    printf("ku.q is [%s]\n", bn);
    free(bn);
    char buf[6*BLOCK_SIZE]={0};
    const char *plaintext = "Test is always good.";
    mpz_import(M, strlen(plaintext), 1, 1, 0, 0, plaintext);
    bn = mpz_get_str(NULL, 16, M);
    char *origin = malloc(strlen(bn) + 1);
    memcpy(origin, bn, strlen(bn) + 1);
    printf("origin is [%s]\n", bn);
    free(bn);
    RSA_Encrypt(C, M, kp);
    bn = mpz_get_str(NULL, 16, C);
    printf("encrypted is [%s]\n", bn);
    free(bn);
    RSA_Decrypt(DC, C, ku);
    bn = mpz_get_str(NULL, 16, DC);
    char *decrypt = malloc(strlen(bn) + 1);
    memcpy(decrypt, bn, strlen(bn) + 1);
    printf("decrypted is [%s]\n", bn);
    free(bn);
    mpz_export(buf, NULL, 1, 1, 0, 0, DC);
    printf("As plaintext: %s\n", buf);
    ok = strcmp(origin,decrypt)==0;
    assert(ok);
    free(origin);
    free(decrypt);
    return ok;
}

bool rsa_num_message(rsa_private_key* ku, rsa_public_key* kp, mpz_t M, mpz_t C,mpz_t DC) {
    bool ok = false;
    generate_keys(ku, kp);
    char *bn = mpz_get_str(NULL, 16, kp->n);
    printf("kp.n is [%s]\n", bn);
    free(bn);                                       // to avoid memory leak
    bn = mpz_get_str(NULL, 16, kp->e);
    printf("kp.e is [%s]\n\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->n);
    printf("ku.n is [%s]\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->e);
    printf("ku.e is [%s]\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->d);
    printf("ku.d is [%s]\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->p);
    printf("ku.p is [%s]\n", bn);
    free(bn);
    bn = mpz_get_str(NULL, 16, ku->q);
    printf("ku.q is [%s]\n", bn);
    free(bn);

    char buf[BLOCK_SIZE];

    //Not windows compatible
    // for(int i = 0; i < BLOCK_SIZE; i++)
    //     buf[i] = random() % 0xFF;

    mpz_import(M, (BLOCK_SIZE), 1, sizeof(buf[0]), 0, 0, buf);
    bn = mpz_get_str(NULL, 16, M);
    char *origin = malloc(strlen(bn) + 1);
    memcpy(origin, bn, strlen(bn) + 1);
    printf("origin is [%s]\n", bn);
    free(bn);
    RSA_Encrypt(C, M, kp);
    bn = mpz_get_str(NULL, 16, C);
    printf("encrypted is [%s]\n", bn);
    free(bn);
    RSA_Decrypt(DC, C, ku);
    bn = mpz_get_str(NULL, 16, DC);
    char *decrypt = malloc(strlen(bn) + 1);
    memcpy(decrypt, bn, strlen(bn) + 1);
    printf("decrypted is [%s]\n", bn);
    free(bn);
    ok = strcmp(origin,decrypt)==0;
    assert(ok);
    free(origin);
    free(decrypt);
    return ok;
}

bool main_rsa(void)
{
    mpz_t M, C, DC;
    rsa_private_key ku;
    rsa_public_key kp;
    bool ok_1= false;
    bool ok_2 = false;

    mpz_inits(M, C, DC, NULL);
    mpz_inits(kp.n, kp.e, NULL);                    // Initialize public key
    mpz_inits(ku.n, ku.e, ku.d,ku.p, ku.q, NULL);   // Initialize private key

    ok_1= rsa_string_message(&ku, &kp, M, C, DC);

    mpz_clears(M, DC, C,NULL);
    mpz_clears(kp.n, kp.e, NULL);  // clear public key
    mpz_clears(ku.n, ku.e, ku.d,ku.p, ku.q, NULL);   // clear private key

    mpz_inits(M, C, DC, NULL);
    mpz_inits(kp.n, kp.e, NULL);                    // Initialize public key
    mpz_inits(ku.n, ku.e, ku.d,ku.p, ku.q, NULL);   // Initialize private key

    ok_2 = rsa_num_message(&ku, &kp, M, C, DC);

    mpz_clears(M, DC, C,NULL);
    mpz_clears(kp.n, kp.e, NULL);  // clear public key
    mpz_clears(ku.n, ku.e, ku.d,ku.p, ku.q, NULL);   // clear private key

    return ok_1 && ok_2;
}