

#include <linux/limits.h>     /* PATH_MAX */
#include <sys/stat.h>   /* mkdir(2) */
#include <errno.h>

#include "directory.h"

#ifndef PATH_MAX
#define PATH_MAX=256
#endif

bool mkdir_p(const char *path)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[PATH_MAX];
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

            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return false; 
            }

            *p = directory_separator[0];
        }
    }   

    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return false; 
    }   

    return true;
}