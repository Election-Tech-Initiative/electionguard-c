#include "api/filename.h"

#if defined(__STDC_WANT_SECURE_LIB__) || defined(__STDC_LIB_EXT1__)
#define __USE_SECURE_APIS__
#endif

bool generate_filename(char *path_in, char *prefix_in, char* default_prefix, char *filename_out)
{
    bool ok = true;

    char prefix[FILENAME_MAX];
    char path[FILENAME_MAX];
    
    // if path is provided, check the last char in the string to make sure it has the appropriate slash
    strcpy(path, path_in);
    size_t path_len = strlen(path);
    if (path_len > 0)
    {
        char *directory_separator;
#ifdef _WIN32
        directory_separator = "\\";
#else
        directory_separator = "/";
#endif
        char path_end_char = path_in[path_len-1];
        if (path_end_char != directory_separator[0])
        {
            strcat(path, directory_separator);
        }
    }

    // if prefix is provided for filename, use it, otherwise use the default
    size_t prefix_size = strlen(prefix_in);
    if (prefix_size > 0)
        strcpy(prefix, prefix_in);
    else
        strcpy(prefix, default_prefix);

    int32_t status = snprintf(filename_out, FILENAME_MAX, "%s%s", path, prefix);

    if (status < 0)
        ok = false;

    return ok;
}

bool generate_unique_filename(char *path_in, char *prefix_in, char* default_prefix, char *filename_out)
{
    bool ok = true;

    char prefix[FILENAME_MAX];
    char path[FILENAME_MAX];
    char *inUsePrefix = default_prefix;
    size_t path_len = 0;
    size_t prefix_size = 0;

    // if path is provided, check the last char in the string to make sure it has the appropriate slash
#ifdef __USE_SECURE_APIS__
    errno_t err = strcpy_s(path, FILENAME_MAX, path_in);
    if (0 != err)
    {
        ok = false;
        goto Exit;
    }
#else
    path_len = strlen(path_in);
    if (path_len >= FILENAME_MAX)
	{
        ok = false;
        goto Exit;
    }

    strcpy(path, path_in);
#endif


#ifdef __USE_SECURE_APIS__
    path_len = strnlen_s(path, FILENAME_MAX);
    if (path_len == 0 || path_len == FILENAME_MAX)
    {
        ok = false;
        goto Exit;
    }
#else
    path_len = strlen(path);
#endif


    if (path_len > 0)
    {
        char *directory_separator;
#ifdef _WIN32
        directory_separator = "\\";
#else
        directory_separator = "/";
#endif
        char path_end_char = path_in[path_len-1];
        if (path_end_char != directory_separator[0])
        {
#ifdef __USE_SECURE_APIS__
            err = strcat_s(path, FILENAME_MAX, directory_separator);
            if (0 != err)
            {
                ok = false;
                goto Exit;
            }
#else
            path_len += 1; 
            if (path_len >= FILENAME_MAX)
            {
                ok = false;
                goto Exit;
            }

            strcat(path, directory_separator);
#endif
        }
    }

	// if prefix is provided for filename, use it, otherwise use the default
    prefix_size = strlen(prefix_in);
    if (prefix_size > 0)
        inUsePrefix = prefix_in;
    else
        prefix_size = strlen(inUsePrefix);

#ifdef __USE_SECURE_APIS__
    err = strcpy_s(prefix, FILENAME_MAX, inUsePrefix);

	if (0 != err)
    {
        ok = false;
        goto Exit;
    }
#else
    if (prefix_size >= FILENAME_MAX)
    {
        ok = false;
        goto Exit;
    }

    strcpy(prefix, inUsePrefix);
#endif

    // get current epoch time
    time_t now = time(NULL);

    int32_t status = snprintf(filename_out, FILENAME_MAX, "%s%s%ld", path, prefix, now);


    if (status < 0 || status == FILENAME_MAX)
        ok = false;

Exit:
    return ok;
}
