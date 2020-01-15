#include "api/filename.h"


bool generate_unique_filename(char *path_in, char *prefix_in, char* default_prefix, char *filename_out)
{
    bool ok = true;

    char prefix[FILENAME_MAX];
    char path[FILENAME_MAX];
    char *inUsePrefix = default_prefix;

    // if path is provided, check the last char in the string to make sure it has the appropriate slash
#ifdef _WIN32
#elif __STDC_LIB_EXT1__
    errno_t err = strcpy_s(path, FILENAME_MAX, path_in);
    if (0 != err)
    {
        ok = false;
        goto Exit;
    }
#else
    size_t tmp_len = strlen(path_in);
    if (tmp_len >= FILENAME_MAX)
	{
        ok = false;
        goto Exit;
    }

    strcpy(path, path_in);
#endif

size_t path_len = 0;

#ifdef _WIN32
#elif __STDC_LIB_EXT1__
    path_len = strlen_s(path, FILENAME_MAX);
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
#ifdef _WIN32
#elif __STDC_LIB_EXT1__
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
    size_t prefix_size = strlen(prefix_in);
    if (prefix_size > 0)
        inUsePrefix = prefix_in;
    else
        prefix_size = strlen(inUsePrefix);

#ifdef _WIN32
#elif __STDC_LIB_EXT1__
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