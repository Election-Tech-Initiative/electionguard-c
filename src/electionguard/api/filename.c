#include "api/filename.h"


bool generate_unique_filename(char *path_in, char *prefix_in, char* default_prefix, char *filename_out)
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

    // get current epoch time
    time_t now = time(NULL);

    int32_t status = snprintf(filename_out, FILENAME_MAX, "%s%s%ld", path, prefix, now);


    if (status < 0)
        ok = false;

    return ok;
}