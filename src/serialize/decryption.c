#include "serialize/decryption.h"
#include "serialize/builtins.h"
#include "serialize/crypto.h"

void Serialize_reserve_decryption_share(struct serialize_state *state,
                                        struct decryption_share_rep const *data)
{
    Serialize_reserve_uint32(state, &data->trustee_index);
    Serialize_reserve_uint64(state, &data->num_tallies);
    for (uint64_t i = 0; i < data->num_tallies; i++)
        Serialize_reserve_uint64(state, &data->tallies[i]);
}

void Serialize_write_decryption_share(struct serialize_state *state,
                                      struct decryption_share_rep const *data)
{
    Serialize_write_uint32(state, &data->trustee_index);
    Serialize_write_uint64(state, &data->num_tallies);
    for (uint64_t i = 0; i < data->num_tallies; i++)
        Serialize_write_uint64(state, &data->tallies[i]);
}

void Serialize_read_decryption_share(struct serialize_state *state,
                                     struct decryption_share_rep *data)
{
    Serialize_read_uint32(state, &data->trustee_index);
    Serialize_read_uint64(state, &data->num_tallies);
    for (uint64_t i = 0; i < data->num_tallies; i++)
        Serialize_read_uint64(state, &data->tallies[i]);
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
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_reserve_bool(state, &data->requested[i]);
}

void Serialize_write_decryption_fragments(
    struct serialize_state *state, struct decryption_fragments_rep const *data)
{
    Serialize_write_uint32(state, &data->trustee_index);
    Serialize_write_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_write_bool(state, &data->requested[i]);
}

void Serialize_read_decryption_fragments(struct serialize_state *state,
                                         struct decryption_fragments_rep *data)
{
    Serialize_read_uint32(state, &data->trustee_index);
    Serialize_read_uint32(state, &data->num_trustees);
    for (uint32_t i = 0; i < data->num_trustees; i++)
        Serialize_read_bool(state, &data->requested[i]);
}
