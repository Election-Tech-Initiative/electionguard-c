#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif
#include <electionguard/file.h>

// Create a new file according to template
FILE *File_new(char const *path)
{
    return fopen(path, "w+");
}

FILE *File_open_for_read(const char *path)
{
    return fopen(path, "r");
}

FILE *File_open_for_write(const char *path)
{
    return fopen(path, "a+");
}

// Close file
void File_close(FILE *file)
{
    fclose(file);
}

// Seek file to beginning
void File_seek(FILE *file)
{
    int seek_status = fseek(file, 0L, SEEK_SET);
}

bool File_exists(char const *path)
{
    FILE *file_pointer = File_open_for_read(path);
    if (file_pointer == NULL)
    {
        printf("doesnt exist");
        return false;
    }

    printf("exists");
    fclose(file_pointer);
    return true;
}