#pragma once
#include "messages.h"

/* Produce a character representation of a ballot tracker, suitable for
 * presentation to a user. The pointer returned will be freshly allocated; it
 * is the caller's responsibility to free it. */
char *display_ballot_tracker(struct ballot_tracker tracker);
