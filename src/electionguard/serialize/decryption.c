#include "serialize/decryption.h"
#include "serialize/builtins.h"
#include "serialize/crypto.h"

void Serialize_reserve_decryption_share(struct serialize_state *state,
                                        struct decryption_share_rep const *data)
{
    Serialize_reserve_uint32(state, &data->trustee_index);
    Serialize_reserve_uint64(state, &data->num_tallies);
    for (uint64_t i = 0; i < data->num_tallies; i++)
        Serialize_reserve_encryption(state, &data->tally_share[i]);
}

void Serialize_write_decryption_share(struct serialize_state *state,
                                      struct decryption_share_rep const *data)
{
    Serialize_write_uint32(state, &data->trustee_index);
    Serialize_write_uint64(state, &data->num_tallies);
    for (uint64_t i = 0; i < data->num_tallies; i++)
        Serialize_write_encryption(state, &data->tally_share[i]);
}

void Serialize_read_decryption_share(struct serialize_state *state,
                                     struct decryption_share_rep *data)
{
    Serialize_read_uint32(state, &data->trustee_index);
    Serialize_read_uint64(state, &data->num_tallies);
    for (uint64_t i = 0; i < data->num_tallies; i++)
        Serialize_read_encryption(state, &data->tally_share[i]);
}

void Serialize_reserve_decryption_fragments_request(
    struct serialize_state *state,
    struct decryption_fragments_request_rep const *data)
{
    Serialize_reserve_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_reserve_bool(state, &data->requested[i]);
}

void Serialize_write_decryption_fragments_request(
    struct serialize_state *state,
    struct decryption_fragments_request_rep const *data)
{
    Serialize_write_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_write_bool(state, &data->requested[i]);
}

void Serialize_read_decryption_fragments_request(
    struct serialize_state *state,
    struct decryption_fragments_request_rep *data)
{
    Serialize_read_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_read_bool(state, &data->requested[i]);
}

void Serialize_reserve_decryption_fragments(
    struct serialize_state *state, struct decryption_fragments_rep const *data)
{
    Serialize_reserve_uint32(state, &data->trustee_index);
    Serialize_reserve_uint32(state, &data->num_trustees);
    Serialize_reserve_uint32(state, &data->num_selections);
    for (uint32_t i = 0; i < data->num_trustees; i++)
    {
        Serialize_reserve_bool(state, &data->requested[i]);
        if (data->requested[i])
        {
            for (int j = 0; j < data->num_selections ; j++)
                Serialize_reserve_uint4096(state, NULL);
        }
    }
    Serialize_reserve_uint256(state, NULL);
}

void Serialize_write_decryption_fragments(
    struct serialize_state *state, struct decryption_fragments_rep const *data)
{
    Serialize_write_uint32(state, &data->trustee_index);
    Serialize_write_uint32(state, &data->num_trustees);
    Serialize_write_uint32(state, &data->num_selections);
    for (uint32_t i = 0; i < data->num_trustees; i++)
    {
        Serialize_write_bool(state, &data->requested[i]);
        if (data->requested[i])
        {
            for (int j = 0; j < data->num_selections ; j++)
                Serialize_write_uint4096(state,
                        data->partial_decryption_M[i][j]);
        }
    }
    Serialize_write_uint256_pad(state, data->lagrange_coefficient);
}

void Serialize_read_decryption_fragments(struct serialize_state *state,
                                         struct decryption_fragments_rep *data)
{
    Serialize_read_uint32(state, &data->trustee_index);
    Serialize_read_uint32(state, &data->num_trustees);
    Serialize_read_uint32(state, &data->num_selections);
    for (uint32_t i = 0; i < data->num_trustees; i++)
    {
        Serialize_read_bool(state, &data->requested[i]);
        if (data->requested[i])
        {
            for (int j = 0; j < data->num_selections ; j++)
                Serialize_read_uint4096(state,
                        data->partial_decryption_M[i][j]);
        }
    }

    Serialize_read_uint256(state, data->lagrange_coefficient);
}
