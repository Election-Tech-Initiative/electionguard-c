API Reference
============================

This API is designed to allow clients to correctly implement an E2E
Verifiable election. In designing this API, we tried to balance the
following constraints and goals:

- *correct:* The API must provide the necessary tools to implement a correct
  E2E verifiable election system.

- *communication agnostic:* The client is responsible for managing
  communications between components. Those components may be a part of
  the same program and communicate via argument-passing, or they may
  be on entirely different servers and communicate over a network.
  This leads to a design in which the entities produce and consume
  messages that drive the election process forward and enforce an
  ordering on operations.

- *easy to use:* API should be easy to use correctly. This means
  reducing the number of functions calls where possible, as
  well as using the types of messages to guide the user towards
  correct usage of the API.

- *general:* We try to make no extra assumptions about the set-up of the election
  other than those required by the ElectionGuard E2E Verifiable specification.

There are 3 phases of an election:

1. :doc:`Key Ceremony </keyceremony>`: A group of `n` trustees jointly
   generate a joint public key such that for a predetermined threshold
   `k`, any `k` of the trustees can decrypt a message encrypted by the
   public key.
2. :doc:`Voting </voting>`: Voters cast their ballots at various
   polling locations. Their ballots are encrypted using the the joint
   public key.
3. :doc:`Decryption </decryption>`: A group of at least `k` of the
   trustees jointly decrypt the tally of all the ballots and publish
   sufficient information for anyone to verify that the tally and
   decryption was carried out correctly.

Within the API, these phases are represented by interactions between
various entities:

1. The key ceremony is carried out by `n` :type:`KeyCeremony_Trustee`
   objects representing the election trustees, and a single
   :type:`KeyCeremony_Coordinator` object that provides a central
   “location” to which :type:`KeyCeremony_Trustee`\ s submit pieces of
   information, and which then broadcasts that information to all the
   :type:`KeyCeremony_Trustee`\ s.
2. Voting is carried out by groups of many :type:`Voting_Encrypter`\ s
   (probably located within ballot marking devices) connected to a
   single :type:`Voting_Coordinator`. Each group represents a single
   polling location. The :type:`Voting_Encrypter` is responsible for
   encrypting ballots to produce :type:`ballot_tracker`\ s and
   :type:`ballot_identifier`\ s. The :type:`Voting_Coordinator`
   is responsible for keeping track of which ballots have been
   registered, cast, and spoiled.
3. Tallying is carried out by a group of at least `k`
   :type:`Decryption_Trustee` objects representing the election
   trustees, and a single :type:`Decryption_Coordinator` object in a
   structure very similar to that of the Key Ceremony.

We chose to separate the trustees and coordinators into
:type:`KeyCeremony_Trustee`\ s/\ :type:`Decryption_Trustee`\ s and
:type:`KeyCeremony_Coordinator`\ s/\ :type:`Decryption_Coordinator`\
s respectively. This is because conceptually a trustee has two
distinct tasks, key generation and decryption, that are linked by a
small kernel of persisted information such as the trustee’s private
key and key shares. Decryption will probably happen at least a day
after the key ceremony, so it is important that the information
necessary for decryption can be stored for a long period of time.
Making the distinction between :type:`KeyCeremony_Trustee`\ s and
:type:`Decryption_Trustee`\ s and making the persisted
:type:`trustee_state` explicit and serializable makes the APIs of each
component simpler and makes it easier to persist the
:type:`trustee_state` over the course of the election.
