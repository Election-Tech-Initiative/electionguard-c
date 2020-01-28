#ifndef __MAX_VALUES_H__
#define __MAX_VALUES_H__

#include <stdint.h>

// @design jwaksbaum We use enums here because enum members are
// constant expressions, allowing us to use them as array sizes, but
// they are not compiled away, making it easier to debug.

enum MAX_EXTERNAL_IDENIFIER_LENGTH_e
{
    MAX_EXTERNAL_ID_LENGTH = 4095
};

/** The maximum number of total trustees. */
enum MAX_TRUSTEES_e
{
    MAX_TRUSTEES = 5
};

/**
 * The maximum number of ballots that can be 
 * cast at one time by a voting single coordinator. 
 */
enum MAX_BALLOTS_e
{
    /** The Maximum ballots for an election 
     * that one Voting Coordinator can track
     */
    MAX_BALLOTS = 10000,

    /** The Maximum ballots that can be passed 
     * to a Voting Coordinator for 
     * Registration, Casting, or Spoiling at one time
     */
    MAX_BALLOT_PAYLOAD = 2000
};

/**
 * The maximum number of selections that 
 * can be present for a single contest. 
 */
enum MAX_SELECTIONS_e
{
    MAX_SELECTIONS = 1000
};

#endif /* __MAX_VALUES_H__ */
