Key Ceremony
============

The key ceremony is carried out between `n` :type:`trustee
<KeyCeremony_Trustee>`\ s and a single :type:`coordinator
<KeyCeremony_Coordinator>`. Its purpose is to ensure that the
coordinator possesses an joint public key, and that each trustee
possesses: - a private key - all the trustees’ public keys - a set of
key shares that allow the trustee to, as a part of a group of at least
`k` trustees, to decrypt a missing trustee’s share of a message
encrypted with the aggregate public key.

Key Generation
--------------

1. Each trustee generates a key pair, [#]_ producing a
   :type:`key-generated message <key_generated_message>`.
2. These `n` :type:`key-generated message <key_generated_message>`\ s are passed to the
   coordinator. [#]_
3. The coordinator produces an :type:`all-keys-received message <all_keys_received_message>`, [#]_
   which must be passed back to each trustee. [#]_

.. [#] :func:`KeyCeremony_Trustee_generate_key()`
.. [#] :func:`KeyCeremony_Coordinator_receive_key_generated()`
.. [#] :func:`KeyCeremony_Coordinator_all_keys_received()`
.. [#] :func:`KeyCeremony_Trustee_generate_shares()`

Share Generation
----------------

1. Each trustee, having received an :type:`all-keys-received message
   <all_keys_received_message>`, computes a key share for every
   trustee, and encrypts each key share with the corresponding
   trustee’s public key to produce a :type:`shares-generated message
   <shares_generated_message>` [#]_.
2. These `n` :type:`shares-generated message
   <shares_generated_message>`\ s are passed to the coordinator [#]_.
3. The coordinator produces a :type:`all-shares-received message
   <all_shares_received_message>`, [#]_ which must be passed back to
   each trustee. [#]_

.. [#] :func:`KeyCeremony_Trustee_generate_shares()`
.. [#] :func:`KeyCeremony_Coordinator_receive_shares_generated()`
.. [#] :func:`KeyCeremony_Coordinator_all_shares_received()`
.. [#] :func:`KeyCeremony_Trustee_verify_shares()`

Verification
------------

1. Each trustee, having received an :type:`all-shares-received message
   <all_shares_received_message>`, decrypts its shares of the other
   trustees keys and verifies that they match the commitments in their
   public keys, producing a :type:`shares-verified message
   <shares_verified_message>`. [#]_
2. These `n` :type:`shares-verified message
   <shares_verified_message>`\ s are passed to the coordinator. [#]_
3. The coordinator produces a :type:`joint public key
   <joint_public_key>`. [#]_

.. [#] :func:`KeyCeremony_Trustee_verify_shares()`
.. [#] :func:`KeyCeremony_Coordinator_receive_shares_verified`
.. [#] :func:`KeyCeremony_Coordinator_publish_joint_key`
