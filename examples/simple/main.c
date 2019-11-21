#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#include <electionguard/max_values.h>

#include "main_decryption.h"
#include "main_keyceremony.h"
#include "main_params.h"
#include "main_rsa.h"
#include "main_voting.h"

/* LGTM TEST - Do not PR */
int LgtmAlertExample01(int i)
{
    int intArray[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int *intPointer = intArray;
    // BAD: the offset is already automatically scaled by sizeof(int),
    // so this code will compute the wrong offset.
    return *(intPointer + (i * sizeof(int)));
}
/* LGTM TEST - Do not PR */

/** Create a new temporary file from a template. */
static FILE *fmkstemps(char const *template, const char *mode);

// Election Parameters
uint32_t const NUM_TRUSTEES = 3;
uint32_t const THRESHOLD = 2;
uint32_t const NUM_ENCRYPTERS = 3;
uint32_t const NUM_SELECTIONS = 3;
uint32_t const DECRYPTING_TRUSTEES = 2;

// ^The number of trustees that will participate in decryption. Will fail if this is less than THRESHOLD

// This is a temporary placeholder. In a real election, this should be
// initialized by hashing:
// 1. p (from bignum.h)
// 2. The subgroup order (not yet named in the current implementation)
// 3. generator (from bignum.h)
// 4. NUM_TRUSTEES
// 5. THRESHOLD
// 6. The date of the election
// 7. Jurisdictional information for the election
raw_hash BASE_HASH_CODE = {0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int main()
{
	// Seed the RNG that we use to generate arbitrary ballots. This
    // relies on the fact that the current implementation of the
    // cryptography does not rely on the built in RNG.
    srand(100);

    Crypto_parameters_new();

    // Outputs of the key ceremony
    struct trustee_state trustee_states[MAX_TRUSTEES];
    struct joint_public_key joint_key;

    // Key Ceremony

    bool ok = key_ceremony(&joint_key, trustee_states);

    // Open the voting results file
    FILE *voting_results = NULL;

    if (ok)
    {
//Gcc on windows appears to not support the "x" flag at the time of writing
#ifdef _WIN32
        voting_results = fmkstemps("voting_results-XXXXXX", "w+");
#else
        voting_results = fmkstemps("voting_results-XXXXXX", "w+x");
#endif

        if (voting_results == NULL)
            ok = false;
    }

    // Voting
    if (ok)
        ok = voting(joint_key, voting_results);

    // Open the tally file
    FILE *tally = NULL;

    //  // RSA encrypt decrypt check
    //  if(ok)
    //      ok = main_rsa();
    if (ok)
    {
//Gcc on windows appears to not support the "x" flag at the time of writing
#ifdef _WIN32
        tally = fmkstemps("tally-XXXXXX", "w");
#else
        tally = fmkstemps("tally-XXXXXX", "wx");
#endif

        if (tally == NULL)
            ok = false;
    }

    // Decryption
    if (ok)
        ok = decryption(voting_results, tally, trustee_states);

    // Cleanup

    if (voting_results != NULL)
    {
        fclose(voting_results);
        voting_results = NULL;
    }

    if (tally != NULL)
    {
        fclose(tally);
        tally = NULL;
    }

    for (uint32_t i = 0; i < NUM_TRUSTEES; i++)
        if (trustee_states[i].bytes != NULL)
        {
            free((void *)trustee_states[i].bytes);
            trustee_states[i].bytes = NULL;
        }

    if (joint_key.bytes != NULL)
    {
        free((void *)joint_key.bytes);
        joint_key.bytes = NULL;
    }

    Crypto_parameters_free();

    if (ok)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

FILE *fmkstemps(char const *template, const char *mode)
{
    bool ok = true;

    int result_fd = -1;
    FILE *result = NULL;

    // Duplicate the template. It needs to be mutable for mkstemp.
    char *template_mut = strdup(template);
    if (template_mut == NULL)
        ok = false;

    // Create and open the temporary file
    if (ok)
    {
        result_fd = mkstemp(template_mut);
        if (-1 == result_fd)
            ok = false;
    }

    // Convert the file descriptor to a FILE*
    if (ok)
    {
        result = fdopen(result_fd, mode);
        if (result == NULL)
            ok = false;
    }

    // Free the duplicated template
    if (template_mut != NULL)
    {
        free(template_mut);
        template_mut = NULL;
    }

    return result;
}
