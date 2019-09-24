#include <stdlib.h>
#include <string.h>

#include "main_keyceremony.h"
#include "main_params.h"

// Initialize
static bool initialize_coordinator(void);
static bool initialize_trustees(void);

// Key Generation
static bool generate_keys(void);
static struct all_keys_received_message receive_keys(void);

// Share Generation
static bool generate_shares(struct all_keys_received_message all_keys_received);
static struct all_shares_received_message receive_shares(void);

// Verification
static bool
verify_shares(struct all_shares_received_message all_shares_received);
static struct joint_public_key publish_joint_key(void);

// Export trustees state
static bool export_trustee_states(struct trustee_state *trustee_states);

// Global state
static KeyCeremony_Coordinator coordinator;
static KeyCeremony_Trustee trustees[MAX_TRUSTEES];

bool key_ceremony(struct joint_public_key *joint_key,
                  struct trustee_state *trustee_states)
{
    bool ok = true;

    // Initialize

    if (ok)
        ok = initialize_coordinator();

    if (ok)
        ok = initialize_trustees();

    // Key Ceremony

    if (ok)
        ok = generate_keys();

    struct all_keys_received_message all_keys_received = {.bytes = NULL};

    if (ok)
    {
        all_keys_received = receive_keys();
        if (all_keys_received.bytes == NULL)
            ok = false;
    }

    // Share Generation

    if (ok)
        ok = generate_shares(all_keys_received);

    struct all_shares_received_message all_shares_received = {.bytes = NULL};

    if (ok)
    {
        all_shares_received = receive_shares();
        if (all_shares_received.bytes == NULL)
            ok = false;
    }

    // Verification

    if (ok)
        ok = verify_shares(all_shares_received);

    *joint_key = (struct joint_public_key){.bytes = NULL};

    if (ok)
    {
        *joint_key = publish_joint_key();
        if (joint_key->bytes == NULL)
            ok = false;
    }

    // Export trustee state

    if (ok)
        ok = export_trustee_states(trustee_states);

    // Cleanup

    if (all_shares_received.bytes != NULL)
    {
        free((void *)all_shares_received.bytes);
        all_shares_received.bytes = NULL;
    }

    if (all_keys_received.bytes != NULL)
    {
        free((void *)all_keys_received.bytes);
        all_keys_received.bytes = NULL;
    }

    for (uint32_t i = 0; i < NUM_TRUSTEES; i++)
    {
        if (trustees[i] != NULL)
            KeyCeremony_Trustee_free(trustees[i]);
    }

    if (coordinator != NULL)
        KeyCeremony_Coordinator_free(coordinator);

    return ok;
}

bool initialize_coordinator(void)
{
    bool ok = true;

    struct KeyCeremony_Coordinator_new_r result =
        KeyCeremony_Coordinator_new(NUM_TRUSTEES, THRESHOLD);

    if (result.status != KEYCEREMONY_COORDINATOR_SUCCESS)
        ok = false;
    else
        coordinator = result.coordinator;

    return ok;
}

bool initialize_trustees(void)
{
    bool ok = true;

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        struct KeyCeremony_Trustee_new_r result =
            KeyCeremony_Trustee_new(NUM_TRUSTEES, THRESHOLD, i);

        if (result.status != KEYCEREMONY_TRUSTEE_SUCCESS)
            ok = false;
        else
            trustees[i] = result.trustee;
    }

    return ok;
}

bool generate_keys(void)
{
    bool ok = true;

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        struct key_generated_message key_generated = {.bytes = NULL};

        {
            struct KeyCeremony_Trustee_generate_key_r result =
                KeyCeremony_Trustee_generate_key(trustees[i], BASE_HASH_CODE);

            if (result.status != KEYCEREMONY_TRUSTEE_SUCCESS)
                ok = false;
            else
                key_generated = result.message;
        }

        if (ok)
        {
            enum KeyCeremony_Coordinator_status status =
                KeyCeremony_Coordinator_receive_key_generated(coordinator,
                                                              key_generated);
            if (status != KEYCEREMONY_COORDINATOR_SUCCESS)
                ok = false;
        }

        if (key_generated.bytes != NULL)
        {
            free((void *)key_generated.bytes);
            key_generated.bytes = NULL;
        }
    }

    return ok;
}

struct all_keys_received_message receive_keys(void)
{
    struct all_keys_received_message message = {.bytes = NULL};

    struct KeyCeremony_Coordinator_all_keys_received_r result =
        KeyCeremony_Coordinator_all_keys_received(coordinator);

    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
        message = result.message;

    return message;
}

bool generate_shares(struct all_keys_received_message all_keys_received)
{
    bool ok = true;

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        struct shares_generated_message shares_generated = {.bytes = NULL};

        {
            struct KeyCeremony_Trustee_generate_shares_r result =
                KeyCeremony_Trustee_generate_shares(trustees[i],
                                                    all_keys_received);

            if (result.status != KEYCEREMONY_TRUSTEE_SUCCESS)
                ok = false;
            else
            {
                shares_generated = result.message;
            }
        }

        if (ok)
        {
            enum KeyCeremony_Coordinator_status cstatus =
                KeyCeremony_Coordinator_receive_shares_generated(
                    coordinator, shares_generated);
            if (cstatus != KEYCEREMONY_COORDINATOR_SUCCESS)
                ok = false;
        }

        if (shares_generated.bytes != NULL)
        {
            free((void *)shares_generated.bytes);
            shares_generated.bytes = NULL;
        }
    }

    return ok;
}

struct all_shares_received_message receive_shares()
{
    struct all_shares_received_message message = {.bytes = NULL};

    struct KeyCeremony_Coordinator_all_shares_received_r result =
        KeyCeremony_Coordinator_all_shares_received(coordinator);

    if (result.status == KEYCEREMONY_COORDINATOR_SUCCESS)
        message = result.message;

    return message;
}

bool verify_shares(struct all_shares_received_message all_shares_received)
{
    bool ok = true;

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        struct shares_verified_message shares_verified;

        {
            struct KeyCeremony_Trustee_verify_shares_r result =
                KeyCeremony_Trustee_verify_shares(trustees[i],
                                                  all_shares_received);
            if (result.status != KEYCEREMONY_TRUSTEE_SUCCESS)
                ok = false;
            else
                shares_verified = result.message;
        }

        if (ok)
        {
            enum KeyCeremony_Coordinator_status status =
                KeyCeremony_Coordinator_receive_shares_verified(
                    coordinator, shares_verified);
            if (status != KEYCEREMONY_COORDINATOR_SUCCESS)
                ok = false;
        }

        if (shares_verified.bytes != NULL)
        {
            free((void *)shares_verified.bytes);
            shares_verified.bytes = NULL;
        }
    }

    return ok;
}

struct joint_public_key publish_joint_key(void)
{
    struct joint_public_key key = {.bytes = NULL};

    struct KeyCeremony_Coordinator_publish_joint_key_r cresult =
        KeyCeremony_Coordinator_publish_joint_key(coordinator);

    if (cresult.status == KEYCEREMONY_COORDINATOR_SUCCESS)
        key = cresult.key;

    return key;
}

static bool export_trustee_states(struct trustee_state *trustee_states)
{

    bool ok = true;

    for (uint32_t i = 0; i < NUM_TRUSTEES && ok; i++)
    {
        struct KeyCeremony_Trustee_export_state_r result =
            KeyCeremony_Trustee_export_state(trustees[i]);

        if (result.status != KEYCEREMONY_TRUSTEE_SUCCESS)
            ok = false;
        else
            trustee_states[i] = result.state;
    }

    return ok;
}
