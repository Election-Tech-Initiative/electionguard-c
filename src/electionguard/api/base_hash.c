#include "api/base_hash.h"

void create_base_hash_code(struct api_config config)
{
    // This is a temporary placeholder. In a real election, this should be
    // initialized by hashing:
    // 1. p (from bignum.h)
    // 2. The subgroup order (not yet named in the current implementation) (use config.subgroup_order?)
    // 3. generator (from bignum.h)
    // 4. config.num_trustees
    // 5. config.threshold
    // 6. The date of the election (part of config.election_meta?)
    // 7. Jurisdictional information for the election (part of config.election_meta?)
    // TODO: perform hashing with the given config and global Crypto values from bignum.h
    raw_hash initialized_hash = {0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(base_hash_code, initialized_hash, sizeof(initialized_hash));
}