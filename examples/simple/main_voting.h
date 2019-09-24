#ifndef __MAIN_VOTING_H__
#define __MAIN_VOTING_H__

#include <electionguard/voting/coordinator.h>
#include <electionguard/voting/encrypter.h>

bool voting(struct joint_public_key joint_key, FILE *out);

#endif /* __MAIN_VOTING_H__ */
