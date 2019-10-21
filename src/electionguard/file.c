#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif
#include <electionguard/file.h>

// Create a new file according to template
FILE *File_new(char const *template)
{
    bool ok = true;
    int result_fd = -1;
    FILE *result = NULL;

    #ifdef _WIN32
        const char *mode = "w+";
    #else
        const char *mode = "w+x";
    #endif

    // Duplicate the template. It needs to be mutable for mkstemp.
    char *template_mut = strdup(template);
    if (template_mut == NULL)
        ok = false;

    // Create and open the temporary file
    if (ok)
    {
        result_fd = mkstemp(template_mut);
        if (-1 == result_fd)
            ok = false;
    }

    // Convert the file descriptor to a FILE*
    if (ok)
    {
        result = fdopen(result_fd, mode);
        if (result == NULL)
            ok = false;
    }

    // Free the duplicated template
    if (template_mut != NULL)
    {
        free(template_mut);
        template_mut = NULL;
    }

    return result;
}

// Close file
void File_close(FILE *file)
{
    fclose(file);
}