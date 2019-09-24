# ElectionGuard SDK Public API

This API is designed to allow clients to correctly implement an E2E
Verifiable election. In designing this API, we tried to balance the
following constraints and goals:
- *correct*: The API must provide the necessary tools to implement a
  correct E2E verifiable election system.
- *communication agnostic*: The client is responsible for managing
  communication between components. Those components may be a part of
  the same program and communicate via argument-passing, or they may
  be on entirely different servers and communicate over a network.
  This leads to a design in which the entities produce and consume
  messages that drive the election process forward and enforce an
  ordering on operations.
- *easy to use*: We want the API to be easy to use correctly. This
  means reducing the number of methods and calls as much as possible,
  as well as using the types of messages to guide the user towards
  correct usage of the API.
- *general*: We try to no extra assumptions about the set-up of the
  election other than those required by the ElectionGuard E2E
  Verifiable specification.

## Overview of an election

There are 3 phases of an election:
1. **Key Ceremony**: A group of `n` trustees jointly generate a joint
   public key such that for a predetermined threshold `k`, any `k` of
   the trustees can decrypt a message encrypted by the public key.
2. **Voting**: Voters cast their ballots at various polling locations.
   Their ballots are encrypted using the the joint public key.
3. **Tallying and Decryption**: A group of at least `k` of the
   trustees jointly decrypt the tally of all the ballots and publish
   sufficient information for anyone to verify that the tally and
   decryption was carried out correctly.

For more information about these phases, see the READMEs in
[`key_ceremony`](./key_ceremony), [`voting`](./voting),
[`decryption`](./decryption) respectively.

Within the API, these phases are represented by interactions between
various entities:
1. The key ceremony is carried out by `n` `Key_Ceremony_Trustee` objects
   representing the election trustees, and a single
   `Key_Ceremony_Coordinator` object that provides a central "location"
   to which `Key_Ceremony_Trustee`s submit pieces of information and
   which then broadcasts the information of all the `Key_Ceremony_Trustee`s
   back to each `Trustee`.
2. Voting is carried out by groups of many `Voting_Encrypters`s
   (probably located within Ballot Marking Devices) connected to a
   single `Voting_Coordinator`, where each group represents a single
   polling location. The `Voting_Coordinator` is responsible for
   keeping track of which ballots have been registered, cast, and
   spoiled.
3. Tallying is carried out by a group of at least `k`
   `Decryption_Trustee` objects representing the election trustees,
   and a single `Decryption_Coordinator` object in a structure very
   similar to that of the Key Ceremony.

We chose to separate the trustees and coordinators
into `Key_Ceremony_Trustee`s/`Decryption_Trustee`s and
`Key_Ceremony_Coordinator`s/`Decryption_Coordinator`s respectively.
This is because conceptually a trustee has two distinct tasks, key
generation and decryption, that are linked by a small kernel of
persisted information such as the trustee's private key and key
shares. Decryption will probably happen at least a day after the key
ceremony, so it is important that the information necessary for
decryption can be stored for a long period of time. Making the
distinction between `Key_Ceremony_Trustee`s and `Decryption_Trustee`s
and making the persisted `struct trustee_state` explicit and serializable
makes the APIs of each component simpler and makes it easier to
persist the `struct trustee_state` over the course of the election.

## Table of Contents

- [`keyceremony`](/.keyceremony)
  - [`messages.h`](./keyceremony/messages.h) defines the types of the
    messages used in the key ceremony.
  - [`trustee.h`](./keyceremony/trustee.h) defines the
    `Key_Ceremony_Trustee` type.
  - [`coordinator.h`](./keyceremony/coordinator.h) defines the
    `Key_Ceremony_Coordinator` type.
- [`voting`](./voting)
  - [`messages.h`](./voting/messages.h) defines the types of messages used in voting.
  - [`encrypter.h`](./voting/encrypter.h) defines the
    type `Voting_Encrypter`.
  - [`coordinator.h`](./voting/coordinator.h) defines the type `Voting_Coordinator`.
- [`decryption`](./decryption)
  - [`messages.h`](./decryption/messages.h) defines the types of the
    messages used in decryption.
  - [`trustee.h`](./decryption/trustee.h) defines the
    `Decryption_Trustee` type.
  - [`coordinator.h`](./decryption/coordinator.h) defines the
    `Decryption_Coordinator` type.
- [`trustee_state.h`](./trustee_state.h) defines the `struct trustee_state`
    type, which links `Key_Ceremony_Trustee`s to `Decryption_Trustee`s.
- [`crypto.h`](./crypto.h) defines the type `struct joint_public_key`
    which represents the joint public key produced at the end of the
    key ceremony.
- [`max_values.h`](./max_values.h) defines compile-time upper bounds
    on certain values, such as the number of trustees. This allows us
    to use static allocation in many contexts.
