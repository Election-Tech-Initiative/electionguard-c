#ifndef __MAIN_DECRYPTION_H__
#define __MAIN_DECRYPTION_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <electionguard/crypto.h>
#include <electionguard/max_values.h>
#include <electionguard/trustee_state.h>

bool decryption(FILE *in, FILE *out, struct trustee_state *trustee_states);

#endif /* __MAIN_DECRYPTION_H__ */
