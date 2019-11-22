#ifndef __API_BASE_HASH_H__
#define __API_BASE_HASH_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <electionguard/crypto.h>
#include <electionguard/max_values.h>
#include <electionguard/api/config.h>

// Globally available
raw_hash base_hash_code;

void create_base_hash_code(struct api_config config);

#endif /* __API_BASE_HASH_H__ */