#ifndef __API_FILENAME_H__
#define __API_FILENAME_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool generate_unique_filename(char *path_in, char *prefix_in, char* default_prefix, char *filename_out);

#endif /* __API_FILENAME_H__ */