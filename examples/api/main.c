#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#include <electionguard/max_values.h>
#include <electionguard/api/create_election.h>

// Election Parameters
uint32_t const NUM_TRUSTEES = 3;
uint32_t const THRESHOLD = 2;
uint32_t const NUM_ENCRYPTERS = 3;
uint32_t const NUM_SELECTIONS = 3;
uint32_t const DECRYPTING_TRUSTEES = 2;

// ^The number of trustees that will participate in decryption. Will fail if this is less than THRESHOLD

int main()
{
    // Seed the RNG that we use to generate arbitrary ballots. This
    // relies on the fact that the current implementation of the
    // cryptography does not rely on the built in RNG.
    srand(100);

    struct api_config config = {
        .num_trustees = NUM_TRUSTEES,
        .threshold = THRESHOLD,
        .subgroup_order = 0,
        .election_meta = "placeholder"
    };

    // Outputs of the key ceremony (+ joint_key from return value)
    struct trustee_state trustee_states[MAX_TRUSTEES];

    // Key Ceremony

    struct joint_public_key joint_key = API_CreateElection(config, trustee_states);

    bool ok = joint_key.bytes != NULL;

    // Cleanup

    API_CreateElection_free(joint_key, trustee_states);

    if (ok)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
