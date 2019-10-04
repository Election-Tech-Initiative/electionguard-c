#include <electionguard/secure_zero_memory.h>

#include <errno.h>
#include <assert.h>

/**
 * Best-effort function for zeroing a buffer that abstracts the API available for the platform */
void secure_zero_memory(
#ifdef _WIN32
#elif __STDC_LIB_EXT1__
#else
    volatile
#endif
    void* ptr,
    size_t cnt)
{
#ifdef _WIN32
    /* For Windows builds, use SecureZeroMemory, which guarantess that it will not be removed during optimization */
    SecureZeroMemory(ptr, cnt);
#elif __STDC_LIB_EXT1__
    /* For C11 (or higher) compilers, use memset_s, which guarantess that it will not be removed during optimization */
    int retval = memset_s(ptr, cnt, 0, cnt);

    /* Check on debug only */
    assert(retval == 0);
#else
    /* For any other platforms, use memset with teh argument marked as volatile to try to avoid optimization. Best-effort.*/
    memset((void*)ptr, 0, cnt);
#endif
}
