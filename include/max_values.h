#ifndef __MAX_VALUES_H__
#define __MAX_VALUES_H__

#include <stdint.h>

// @design jwaksbaum We use enums here because enum members are
// constant expressions, allowing us to use them as array sizes, but
// they are not compiled away, making it easier to debug.

/* The maximum number of total trustees. */
enum MAX_TRUSTEES_e
{
    MAX_TRUSTEES = 30
};

/* The maximum number of ballots that can be cast by a single ballot
   box. */
enum MAX_BALLOTS_e
{
    MAX_BALLOTS = 1000
};

/* The maximum number of selections that can be present for a single
   election. This is across all races. */
enum MAX_SELECTIONS_e
{
    MAX_SELECTIONS = 1000
};

#endif /* __MAX_VALUES_H__ */
