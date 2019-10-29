#include "serialize/builtins.h"
#include "serialize/crypto.h"
#include "trustee_state_rep.h"

void Serialize_reserve_trustee_state(struct serialize_state *state,
                                     struct trustee_state_rep const *data, uint32_t num_trustees)
{
    Serialize_reserve_uint32(state, &data->index);
    Serialize_reserve_private_key(state, &data->private_key);
    Serialize_reserve_rsa_private_key(state, NULL);
    for(uint32_t i = 0; i < num_trustees; i++ ){
        Serialize_reserve_encrypted_key_share(state, NULL);
    }
}

void Serialize_write_trustee_state(struct serialize_state *state,
                                   struct trustee_state_rep const *data, uint32_t num_trustees)
{
    Serialize_write_uint32(state, &data->index);
    Serialize_write_private_key(state, &data->private_key);
    Serialize_write_rsa_private_key(state, &data->rsa_private_key);
    for(uint32_t i = 0; i < num_trustees; i++ ){
        Serialize_write_encrypted_key_share(state, &data->my_key_shares[i]);
    }
}

void Serialize_read_trustee_state(struct serialize_state *state,
                                  struct trustee_state_rep *data, uint32_t num_trustees)
{
    Serialize_read_uint32(state, &data->index);
    Serialize_read_private_key(state, &data->private_key);
    Serialize_read_rsa_private_key(state, &data->rsa_private_key);
    for(uint32_t i = 0; i < num_trustees; i++ ){
        Serialize_read_encrypted_key_share(state, &data->my_key_shares[i]);
    }
}
