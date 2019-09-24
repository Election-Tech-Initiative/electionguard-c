#include <stdlib.h>
#include <stdio.h>
#include "random_source.h"

struct RandomSource_s {
#ifdef HAVE_BCRYPTGENRANDOM
// These links will be helpful for fixing this error:
// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgenrandom
// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
#error Support for generating random byte sequences on Windows is not yet implemented.
#else
    FILE *dev_random;
#endif
};

struct RandomSource_new_r RandomSource_new(void) {
    struct RandomSource_new_r result;
    result.status = RANDOM_SOURCE_SUCCESS;

    if(RANDOM_SOURCE_SUCCESS == result.status) {
        result.source = malloc(sizeof(*result.source));
        if(NULL == result.source) {
            result.status = RANDOM_SOURCE_INSUFFICIENT_MEMORY;
        }
    }

    if(RANDOM_SOURCE_SUCCESS == result.status) {
#ifdef HAVE_BCRYPTGENRANDOM
#error Support for generating random byte sequences on Windows is not yet implemented.
#else
        result.source->dev_random = fopen("/dev/urandom", "r");
        if(NULL == result.source->dev_random) {
            result.status = RANDOM_SOURCE_IO_ERROR;
            free(result.source);
        }
#endif
    }

    return result;
}

void RandomSource_free(RandomSource source) {
#ifdef HAVE_BCRYPTGENRANDOM
#error Support for generating random byte sequences on Windows is not yet implemented.
#else
    fclose(source->dev_random);
    free(source);
#endif
}

enum RandomSource_status RandomSource_uniform_o(RandomSource source, uint4096 out) {
    uint8_t raw_bytes[UINT4096_SIZE_BYTES];
    size_t item_count;
    enum RandomSource_status result = RANDOM_SOURCE_SUCCESS;
    struct uint4096_s zero;
    uint4096_zext_o(&zero, NULL, 0);

    do {
        if(RANDOM_SOURCE_SUCCESS == result) {
#ifdef HAVE_BCRYPTGENRANDOM
#error Support for generating random byte sequences on Windows is not yet implemented.
#else
            item_count = fread(raw_bytes, UINT4096_SIZE_BYTES, 1, source->dev_random);
            if(1 != item_count) {
                result = RANDOM_SOURCE_IO_ERROR;
            }
#endif
        }

        if(RANDOM_SOURCE_SUCCESS == result) {
            uint4096_zext_o(out, raw_bytes, UINT4096_SIZE_BYTES);
        }
    } while(RANDOM_SOURCE_SUCCESS == result &&
            (uint4096_le(out, &zero) ||
             !uint4096_lt(out, uint4096_modulus_default)
            )
           );

    return result;
}

struct RandomSource_uniform_r RandomSource_uniform(RandomSource source) {
    struct RandomSource_uniform_r result;
    result.status = RANDOM_SOURCE_SUCCESS;

    if(RANDOM_SOURCE_SUCCESS == result.status) {
        result.result = uint4096_zext(NULL,0);
        if(NULL == result.result) {
            result.status = RANDOM_SOURCE_INSUFFICIENT_MEMORY;
        }
    }

    if(RANDOM_SOURCE_SUCCESS == result.status) {
        result.status = RandomSource_uniform_o(source, result.result);
        if(RANDOM_SOURCE_SUCCESS != result.status) {
            uint4096_free(result.result);
        }
    }

    return result;
}
