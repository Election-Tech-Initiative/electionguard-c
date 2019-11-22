#ifndef __API_CREATE_ELECTION_H__
#define __API_CREATE_ELECTION_H__

#include <electionguard/api/config.h>
#include <electionguard/crypto.h>
#include <electionguard/keyceremony/coordinator.h>
#include <electionguard/keyceremony/messages.h>
#include <electionguard/keyceremony/trustee.h>

/* Perform all the steps necessary to initialize a new election from the given config.
Perform the key ceremony, return joint key and assign the trustee_states in the provided pointer. */
struct joint_public_key API_CreateElection(struct api_config config,
                                           struct trustee_state *trustee_states);

#endif /* __API_CREATE_ELECTION_H__ */