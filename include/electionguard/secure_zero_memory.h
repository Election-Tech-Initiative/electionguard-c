#ifndef SECURE_ZERO_MEMORY_H
#define SECURE_ZERO_MEMORY_H

#ifdef _WIN32
#include <Windows.h>
#else
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#endif

/**
 * Best-effort function for zeroing a buffer that abstracts the API available for the platform */
void secure_zero_memory(
#ifdef _WIN32
#elif __STDC_LIB_EXT1__
#else
    volatile
#endif
    void* ptr,
    size_t cnt);

#endif
