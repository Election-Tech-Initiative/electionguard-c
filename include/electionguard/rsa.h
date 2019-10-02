#ifndef _RSA_H
#define _RSA_H

#define MODULUS_SIZE 4096                   /* This is the number of bits we want in the modulus */
#define BUFFER_SIZE ((MODULUS_SIZE/8) / 2)  /* This is the number of bytes in n and p */
#define BLOCK_SIZE (MODULUS_SIZE/8)         /* This is the size of a block that gets en/decrypted at once */


typedef struct {
    mpz_t n; /* Modulus */
    mpz_t e; /* Public Exponent */
} rsa_public_key;

typedef struct {
    mpz_t n;                                            // Modulus
    mpz_t e;                                            // Public Exponent
    mpz_t d;                                            // Private Exponent
    mpz_t p;                                            // Starting prime p
    mpz_t q;                                            // Starting prime q
} rsa_private_key;

void generate_keys(rsa_private_key* priv_key, rsa_public_key* pub_key);

void RSA_Encrypt(mpz_t encrypted, mpz_t message, rsa_public_key* pub_key);

void RSA_Decrypt(mpz_t original, mpz_t encrypted, rsa_private_key* priv_key);
#endif /* _RSA_H */