

#include <stdio.h>     /* FILENAME_MAX */
#include <sys/stat.h>   /* mkdir(2) */
#include <errno.h>

#include "directory.h"

#ifndef FILENAME_MAX
#define FILENAME_MAX=256
#endif

bool create_directory(const char *path)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[FILENAME_MAX];
    char *p; 

    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path)-1) {
        errno = ENAMETOOLONG;
        return false; 
    }   
    strcpy(_path, path);

    char *directory_separator;
#ifdef _WIN32
        directory_separator = "\\";
#else
        directory_separator = "/";
#endif

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == directory_separator[0]) {
            /* Temporarily truncate */
            *p = '\0';

            int mk_dir_res = -1;
#ifdef _WIN32
            mk_dir_res = mkdir(_path);
#else
            mk_dir_res = mkdir(_path, S_IRWXU);
#endif
            if (mk_dir_res != 0) {
                if (errno != EEXIST)
                    return false; 
            }

            *p = directory_separator[0];
        }
    }   

    int mk_dir_res = -1;
#ifdef _WIN32
    mk_dir_res = mkdir(_path);
#else
    mk_dir_res = mkdir(_path, S_IRWXU);
#endif

    if (mk_dir_res != 0) {
        if (errno != EEXIST)
            return false; 
    }   

    return true;
}