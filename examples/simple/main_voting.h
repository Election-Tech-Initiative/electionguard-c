#ifndef __MAIN_VOTING_H__
#define __MAIN_VOTING_H__

#include "voting/coordinator.h"
#include "voting/encrypter.h"

bool voting(struct joint_public_key joint_key, FILE *out);

#endif /* __MAIN_VOTING_H__ */
