#include <electionguard/voting/messages.h>
#include <gmp.h>
#include "serialize/voting.h"

bool Messages_are_equal(struct register_ballot_message *left_message, struct register_ballot_message *right_message)
{
    bool ok = true;
    struct encrypted_ballot_rep left_ballot_rep;
    ok = Serialize_deserialize_register_ballot_message(left_message, &left_ballot_rep);

    struct encrypted_ballot_rep right_vallot_rep;
    ok = Serialize_deserialize_register_ballot_message(right_message, &right_vallot_rep);

#ifdef DEBUG_PRINT
    printf("\nMessages_are_equal: comparing left_id %lu right_id: %lu:", 
        left_ballot_rep.id, right_vallot_rep.id);
#endif

    ok = left_ballot_rep.id == right_vallot_rep.id
        && left_ballot_rep.num_selections == right_vallot_rep.num_selections;

    if (ok)
    {
        for (uint32_t i = 0; i < left_ballot_rep.num_selections && ok; i++)
        {
            bool comp_nonce = mpz_cmp(
                    left_ballot_rep.selections[i].nonce_encoding, 
                    right_vallot_rep.selections[i].nonce_encoding
                ) == 0;

            bool comp_msg = mpz_cmp(
                    left_ballot_rep.selections[i].message_encoding, 
                    right_vallot_rep.selections[i].message_encoding
                ) == 0;

#ifdef DEBUG_PRINT

            if (!comp_nonce)
            {
                printf("\nMessages_are_equal: ERROR! nonce_encoding did not match! :\n");
                print_base16(left_ballot_rep.selections[i].nonce_encoding);
                print_base16(right_vallot_rep.selections[i].nonce_encoding);
            }

            if (!comp_msg)
            {
                printf("\nMessages_are_equal: ERROR! message_encoding did not match! :\n");
                print_base16(left_ballot_rep.selections[i].message_encoding);
                print_base16(right_vallot_rep.selections[i].message_encoding);
            }
#endif
            ok = comp_nonce && comp_msg;
        }
    }

    if (!ok)
    {

#ifdef DEBUG_PRINT
        printf("\nMessage_are_equal: false!\n");
#endif

    }

    return ok;
}