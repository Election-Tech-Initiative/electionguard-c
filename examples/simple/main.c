#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif

#include "max_values.h"

#include "main_decryption.h"
#include "main_keyceremony.h"
#include "main_params.h"
#include "main_voting.h"

/** Create a new temporary file from a template. */
static FILE *fmkstemps(char const *template, const char *mode);

// Election Parameters
uint32_t const NUM_TRUSTEES = 5;
uint32_t const THRESHOLD = 4;
uint32_t const NUM_ENCRYPTERS = 3;
uint32_t const NUM_SELECTIONS = 3;

int main()
{
    // Seed the RNG that we use to generate arbitrary ballots. This
    // relies on the fact that the current implementation of the
    // cryptography does not rely on the built in RNG.
    srand(100);

    bool ok = true;

    // Outputs of the key ceremony
    struct trustee_state trustee_states[MAX_TRUSTEES];
    struct joint_public_key joint_key;

    // Key Ceremony
    if (ok)
        ok = key_ceremony(&joint_key, trustee_states);

    // Open the voting results file
    FILE *voting_results = NULL;

    if (ok)
    {
        voting_results = fmkstemps("voting_results-XXXXXX", "w+x");
        if (voting_results == NULL)
            ok = false;
    }

    // Voting
    if (ok)
        ok = voting(joint_key, voting_results);

    // Open the tally file
    FILE *tally = NULL;

    if (ok)
    {
        tally = fmkstemps("tally-XXXXXX", "wx");
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

    if (ok)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

FILE *fmkstemps(char const *template, const char *mode)
{
    bool ok = true;

    FILE *result = NULL;

    // Duplicate the template. It needs to be mutable for mkstemps.
    char *template_mut = strdup(template);
    if (template_mut == NULL)
        ok = false;

    // Create and open the temporary file
    if (ok)
    {
        template_mut = mktemp(template_mut);
        if (template_mut == NULL)
            ok = false;
    }

    // Convert the file descripter to a FILE*
    if (ok)
    {
        result = fopen(template_mut, mode);
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
