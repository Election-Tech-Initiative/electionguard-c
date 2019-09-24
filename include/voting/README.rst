Voting
======

Voters cast their ballots at polling locations, which consist of
multiple devices connected on a local network. The types of devices and
their functions can vary based on the details of the election system,
but the ElectionGuard system relies on a voting process in which voters
first create ballots that contain their preferences, and then can choose
to cast or spoil those ballots.

To that end, we provide :type:`encrypter <Voting_Encrypter>`\ s to
encrypt ballots, and :type:`coordinators <Voting_Coordinator>`, which
track which ballots have been created, cast, and spoiled. In a system
with many ballot marking devices and a single ballot box, you can
think of the encrypters as running on the same machine as the ballot
marking devices, and the coordinator as running on the same machine as
the ballot box.

Initialization
--------------

At the beginning of the election, each :type:`encrypter
<Voting_Encrypter>` must be initialized with the :type:`joint key
<joint_public_key>` that was produced at the end of the key ceremony,
so that the ballots can be encrypted with it. In addition, each
:type:`encrypter <Voting_Encrypter>` is initialized with a
:type:`unique identifier <uid>` that must be unique within a single
polling location.

Casting a Ballot
----------------

The process for casting a ballot is:

1. An encrypter receives an unencrypted ballot (from some sort of
   frontend) and produces [#]_

   - an :type:`encrypted ballot <register_ballot_message>`, containing
     all of the selection information
   - a :type:`ballot tracker <ballot_tracker>`, which will be kept by
     the voter and used to identify their vote in the list of
     encrypted ballots that are included in the final tally
   - a :type:`ballot identifier <ballot_identifier>`, which is used to
     uniquely identify a ballot in a voting place for the duration of
     its liveness. This identifier should not contain any information
     about the voter; its purpose is to allow the physical piece of
     paper used to cast or spoil a ballot to refer to a specific
     ballot without having to encode all of the ballot's contents

.. [#] :func:`Voting_Encrypter_encrypt_ballot()`

2. The ballot marking device, or whatever device the voter is
   interacting with, prints

   - the :type:`ballot tracker <ballot_tracker>`
   - the :type:`ballot identifier <ballot_identifier>`, perhaps
     attached to the unencrypted contents of the voter’s ballot so
     that they can review their choices before casting their ballot.
     Ideally the `ballot identifier <ballot_identifier>` should be
     encoded transparently to the voter so they can see that no
     personal information is being associated with their ballot.

3. The :type:`encrypted ballot <register_ballot_message>` along with
   the ballot identifier is registered with the coordinator so that it
   knows which ballot the identifier refers to. [#]_

.. [#] :func:`Voting_Coordinator_register_ballot()`

4. The voter may choose to cast or spoil their ballot:

   a. If the voter wishes to cast their ballot, they insert the paper
      containing the ballot identifier into the ballot box or a
      similar device, which scans it and tells the coordinator to mark
      the corresponding ballot as cast. [#]_
   b. If the voter wishes to spoil their ballot, they take it to a
      poll worker where it is marked as spoiled. Optionally, the poll
      worker can notify the coordinator that the ballot was spoiled to
      ensure that the vote cannot be cast. [#]_ Otherwise, once the
      lifetime of the ballot has passed, the ballot will automatically
      be considered to be spoiled.

.. [#] :func:`Voting_Coordinator_cast_ballot()`
.. [#] :func:`Voting_Coordinator_spoil_ballot()`

Exporting the Voting Record
---------------------------

After voting is over, the coordinator exports the voting record to be
transported to the trustees and decrypted. [#]_

.. [#] :func:`Voting_Coordinator_export_ballots()`
