Decryption
==========

After the election is over, the voting records from each polling
location are aggregated and tallied by the :type:`trustee
<Decryption_Trustee>`\ s. [#]_ They each submit a :type:`share
<decryption_share>` of the decrypted tallies to the :type:`coordinator
<Decryption_Coordinator>`. [#]_ The :type:`coordinator
<Decryption_Coordinator>` may request :type:`fragments of shares
<decryption_fragments>` from :type:`trustee <Decryption_Trustee>`\ s in order to
compensate for missing :type:`trustee <Decryption_Trustee>`\ s. [#]_
Then the :type:`coordinator <Decryption_Coordinator>` combines the
shares to form the decrypted tallies, and publishes them along with
the necessary information for external verifiers to verify the
tallies, and for individual voters to verify that their vote was
included in the tally. [#]_

.. [#] :func:`Decryption_Trustee_tally_voting_record()`
.. [#] :func:`Decryption_Coordinator_receive_share()`
.. [#] :func:`Decryption_Coordinator_all_shares_received()`, :func:`Decryption_Trustee_compute_fragments()`
.. [#] :func:`Decryption_Coordinator_all_fragments_received`

.. note::

  It’s easy to get confused if we aren’t careful about distinguishing
  between a :type:`trustee <Decryption_Trustee>`\ 's portion of a
  decrypted total (which I refer to as a share) and a :type:`trustee
  <Decryption_Trustee>`\ 's portion of some other missing
  :type:`trustee <Decryption_Trustee>`\ 's portion of a decrypted
  total (which I refer to as a share fragment). I think it’s okay if
  we also use share to refer to the pieces of the other :type:`trustee
  <Decryption_Trustee>`\ 's private keys, because in both cases every
  :type:`trustee <Decryption_Trustee>` has exactly `n` of them.

Tallying
--------

After the election, the encrypted totals need to be transported to
each :type:`trustee <Decryption_Trustee>` individually and combined to
form the voting record of the election. [#]_ Independently, each trustee
then computes their share of the decrypted tallies. [#]_

.. [#] :func:`Decryption_Trustee_tally_voting_record()`
.. [#] :func:`Decryption_Trustee_compute_share()`

Announcing
----------

Each :type:`trustee <Decryption_Trustee>` announces their presence by
publishing their share of the encrypted tally. [#]_ The proof of
correctness ensures that each :type:`trustee <Decryption_Trustee>` is
actually in possession of that :type:`trustee <Decryption_Trustee>`\
's private key.

.. [#] :func:`Decryption_Coordinator_receive_share()`

Compensating
------------

Once the :type:`coordinator <Decryption_Coordinator>` decides that all
:type:`trustee <Decryption_Trustee>`\ s who are going to announce have
announced, it checks how many :type:`trustee <Decryption_Trustee>`\ s
have announced. [#]_

a. If fewer than the threshold have announced, error.
b. If enough :type:`trustee <Decryption_Trustee>`\ s have announced to
   reach the threshold, it returns a list of :type:`request
   <decryption_fragments_request>`\ s for some of the announced :type:`trustee
   <Decryption_Trustee>`\ s. These :type:`request
   <decryption_fragments_request>`\ s must be passed to the :type:`trustee
   <Decryption_Trustee>`\ s, who will respond with :type:`share
   fragments <decryption_fragments>` to fulfill those requests. [#]_

   It is up to the :type:`coordinator <Decryption_Coordinator>` how to
   split the task of compensating for missing :type:`trustee
   <Decryption_Trustee>`\ s between the announced :type:`trustee
   <Decryption_Trustee>`\ s. The coordinator can choose not to produce
   requests for some of the announced :type:`trustee
   <Decryption_Trustee>`\ s, or to produce requests that request no
   fragments.

Publishing
----------

After all missing trustees have been compensated for, the
:type:`coordinator <Decryption_Coordinator>` publishes the decrypted
totals and the necessary information to verify the election. [#]_

.. [#] :func:`Decryption_Coordinator_all_shares_received()`
.. [#] :func:`Decryption_Trustee_compute_fragments()`
.. [#] :func:`Decryption_Coordinator_all_fragments_received()`
