# Voting

Voters cast their ballots at polling locations, which consist of
multiple devices connected on a local network. The types of devices
and their functions can vary based on the details of the election
system, but the ElectionGuard system relies on a voting process in
which voters first create ballots that contain their preferences, and
then can choose to cast or spoil those ballots. To that end, we
provide encrypters to encrypt ballots, and coordinators, which track
which ballots have been created, cast, and spoiled. You can think of
the encrypters as running on the same machine as the ballot marking
devices, and the coordinator as running on the same machine as a
single ballot box.

The flow for casting a ballot is:
1. An encrypter receives an unencrypted ballot and produces
     - an *encrypted ballot*, containing all of the selection
       information
     - a *ballot tracker*, which will be kept by the voter and
       used to identify their vote in the list of encrypted
       ballots that are included in the final tally
     - a *ballot identifier*, which is used to uniquely identify a
       ballot in a voting place for the duration of its liveness.
2. The ballot marking device prints
   - the ballot tracker
   - the ballot identifier, perhaps attached to the unencrypted
     contents of the voter's ballot so that they can review their
     choices before casting their ballot
3. The encrypted ballot along with the ballot identifier is registered
   with the coordinator.
4. The voter may choose to cast or spoil their ballot:
   a. If the voter wishes to cast their ballot, they insert the paper
      containing the ballot identifier into the ballot box, which
      scans it and tells the coordinator to mark the corresponding
      ballot as cast.
   b. If the voter wishes to spoil their ballot, they take it to a
      poll worker where it is marked as spoiled. Optionally, the poll
      worker can notify the coordinator that the ballot was spoiled to
      ensure that the vote cannot be cast. Otherwise, once the
      lifetime of the ballot has passed, the ballot will automatically
      be considered as spoiled.

The reason for the addition of the ballot identifier is that the
encrypted ballot will be on the order of megabytes, so we need
another way of referring to a registered ballot.

After voting is over, the coordinator exports the voting record to be
transported to the trustees and decrypted.
