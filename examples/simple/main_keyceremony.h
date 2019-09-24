#ifndef __MAIN_KEY_CEREMONY_H__
#define __MAIN_KEY_CEREMONY_H__

#include "keyceremony/coordinator.h"
#include "keyceremony/messages.h"
#include "keyceremony/trustee.h"

/* Carry out the key ceremony phase of an election, storing the joint
key and trustee_states in the provided pointers. */
bool key_ceremony(struct joint_public_key *joint_key,
                  struct trustee_state *trustee_states);

#endif /* __MAIN_KEY_CEREMONY_H__ */
