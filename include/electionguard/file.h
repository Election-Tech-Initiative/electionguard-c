#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif

/**
 * Open a file for writing.  Overwtites the file if it already exists
 */
FILE *File_new(char const *template);

void File_close(FILE *file);

FILE *File_open_for_read(const char *path);

/**
 * Open a file for reading and writing, appending data if it already exists
 */
FILE *File_open_for_write(const char *path);

void File_seek(FILE *file);

bool File_exists(char const *path);