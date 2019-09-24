# Key Ceremony

The key ceremony is carried out between `n` trustees and a single
coordinator. Its purpose is to ensure that the coordinator
possesses an joint public key, and that each trustee possesses:
  - a private key
  - all the trustees' public keys
  - a set of key shares that allow the trustee to, as a part of a
    group of at least `k` trustees, to decrypt a missing trustee's
    share of a message encrypted with the aggregate public key.

## Key Generation

The first phase is **key generation**:
1. Each trustee generates a key pair, producing a
   `struct key_generated_message`.
2. These `n` `struct key_generated_message`s are passed to the
   coordinator.
3. The coordinator produces an `struct all_keys_received_message`,
   which must be passed back to each trustee.

## Share Generation

The second phase is **share generation**:
1. Each trustee, having received an `struct
   all_keys_received_message`, computes a key share for each other
   trustee, and encrypts it with that trustee's public key, producing
   a `struct shares_generated_message`.
2. These `n` `struct shares_generated_message`s are passed to the
   coordinator.
3. The coordinator produces a `struct all_shares_received_message`,
   which must be passed back to each trustee.

## Verification

The third phase is **verification**:
1. Each trustee, having received an `struct
   all_shares_received_message`, decrypts its shares of the other
   trustees keys and verifies that they match the commitments in their
   public keys, producing a `struct shares_verified_message`.
2. These `n` `struct shares_verified_message`s are passed to the
   coordinator.
3. The coordinator produces a `struct joint_public_key`.
