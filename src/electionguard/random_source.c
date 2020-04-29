#include "random_source.h"
#include <stdio.h>
#include <stdlib.h>
#include "bignum.h"

#ifdef HAVE_BCRYPTGENRANDOM
#include <windows.h>
#include <assert.h>
#include <ntstatus.h>
#include <bcrypt.h>
#endif

struct RandomSource_s
{
#ifdef HAVE_BCRYPTGENRANDOM
// For details of Windows crypto api, check the two urls below:
//   https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgenrandom
//   https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
// For the time being we are using the Windows' default RNG algorithm.
#else
    FILE *dev_random;
#endif
};

struct RandomSource_new_r RandomSource_new(void)
{
    struct RandomSource_new_r result;
    result.status = RANDOM_SOURCE_SUCCESS;
    result.source = malloc(sizeof(*result.source));
    if (NULL == result.source)
    {
        result.status = RANDOM_SOURCE_INSUFFICIENT_MEMORY;
    }

    if (RANDOM_SOURCE_SUCCESS == result.status)
    {
#ifdef HAVE_BCRYPTGENRANDOM
        //For Windows, we dont need to do anything particular here
#else
        result.source->dev_random = fopen("/dev/urandom", "r");
        if (NULL == result.source->dev_random)
        {
            result.status = RANDOM_SOURCE_IO_ERROR;
            free(result.source);
        }
#endif
    }

    return result;
}

void RandomSource_free(RandomSource source)
{
#ifdef HAVE_BCRYPTGENRANDOM
    free(source);
#else
    fclose(source->dev_random);
    free(source);
#endif
}

uint8_t RandomSource_get_byte(RandomSource source)
{
    size_t item_count;
    uint8_t ret;
#ifdef HAVE_BCRYPTGENRANDOM
    NTSTATUS ntstatus =
        BCryptGenRandom(NULL, &ret, 1, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    assert(ntstatus == STATUS_SUCCESS);
#else
    fread(&ret, 1, 1, source->dev_random);
#endif
    return ret;
}

enum RandomSource_status RandomSource_uniform_o(RandomSource source,
                                                uint4096 out)
{
    uint8_t raw_bytes[UINT4096_SIZE_BYTES];
    size_t item_count;
    enum RandomSource_status result = RANDOM_SOURCE_SUCCESS;
    struct uint4096_s zero;
    uint4096_zext_o(&zero, NULL, 0);

    do
    {
        if (RANDOM_SOURCE_SUCCESS == result)
        {
#ifdef HAVE_BCRYPTGENRANDOM
            NTSTATUS ntstatus =
                BCryptGenRandom(NULL, raw_bytes, UINT4096_SIZE_BYTES,
                                BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (ntstatus != STATUS_SUCCESS)
            {
                result = RANDOM_SOURCE_IO_ERROR;
            }
#else
            item_count =
                fread(raw_bytes, UINT4096_SIZE_BYTES, 1, source->dev_random);
            if (1 != item_count)
            {
                result = RANDOM_SOURCE_IO_ERROR;
            }
#endif
        }

        if (RANDOM_SOURCE_SUCCESS == result)
        {
            uint4096_zext_o(out, raw_bytes, UINT4096_SIZE_BYTES);
        }
    } while (RANDOM_SOURCE_SUCCESS == result &&
             (uint4096_le(out, &zero) ||
              !uint4096_lt(out, uint4096_modulus_default)));

    return result;
}

struct RandomSource_uniform_r RandomSource_uniform(RandomSource source)
{
    struct RandomSource_uniform_r result;
    result.status = RANDOM_SOURCE_SUCCESS;
    result.result = uint4096_zext(NULL, 0);
    if (NULL == result.result)
    {
        result.status = RANDOM_SOURCE_INSUFFICIENT_MEMORY;
    }
    if (RANDOM_SOURCE_SUCCESS == result.status)
    {
        result.status = RandomSource_uniform_o(source, result.result);
        if (RANDOM_SOURCE_SUCCESS != result.status)
        {
            uint4096_free(result.result);
        }
    }

    return result;
}

//4096 random bits into a mpz_t
enum RandomSource_status RandomSource_uniform_bignum_o(mpz_t out,
                                                       RandomSource source)
{
    struct RandomSource_uniform_r result = RandomSource_uniform(source);

    if (RANDOM_SOURCE_SUCCESS == result.status)
    {
        import_uint4096(out, result.result);
        free(result.result);
    }

    return result.status;
}

// Random uniform bignum from 1 to q
enum RandomSource_status RandomSource_uniform_bignum_o_q(mpz_t out,
                                                       RandomSource source)
{
    uint8_t raw_bytes[32];
    size_t item_count;
    enum RandomSource_status result = RANDOM_SOURCE_SUCCESS;

    do
    {
        if (RANDOM_SOURCE_SUCCESS == result)
        {
#ifdef HAVE_BCRYPTGENRANDOM
            NTSTATUS ntstatus =
                BCryptGenRandom(NULL, raw_bytes, 32,
                                BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (ntstatus != STATUS_SUCCESS)
            {
                result = RANDOM_SOURCE_IO_ERROR;
            }
#else
            item_count =
                fread(raw_bytes, 32, 1, source->dev_random);
            if (1 != item_count)
            {
                result = RANDOM_SOURCE_IO_ERROR;
            }
#endif
        }

        if (RANDOM_SOURCE_SUCCESS == result)
        {
            mpz_import(out, 32, 1, 1, 0, 0, raw_bytes);
        }
    } while (RANDOM_SOURCE_SUCCESS == result &&
             (mpz_cmp_ui(out, 0) == -1 ||
              mpz_cmp(out, q) == 1));

    return result;
}
