# Decryption

After the election is over, the voting records from each polling
location are aggregated and tallied by the trustees. They each submit
a share of the decrypted tallies to the coordinator. The coordinator
may request fragments of shares from coordinators in order to
compensate for missing trustees. Then the coordinator combines the
shares to form the decrypted tallies, and publishes them along with
the necessary information for external verifiers to verify the
tallies, and for individual voters to verify that their vote was
included in the tally.

**Note:** It's easy to get confused if we aren't careful about
distinguishing between a trustee's portion of a decrypted message
(which I refer to as a share) and a trustee's portion of some other
missing trustees portion of a decrypted message (which I refer to as a
share fragment). I think it's okay if we also use share to refer to
the pieces of the other trustee's private keys, because in both cases
every trustee has exactly `n` of them.

## Tallying

After the election, the encrypted totals need to be transported to
each of the trustees individually and combined to form the voting
record of the election. Independently, each trustee then computes
their share of the decrypted tallies.

## Announcing

Each trustee announces their presence by publishing their share of the
encrypted tally. The proof of correctness ensures that each trustee is
actually in possession of that trustee's private key.

## Compensating

Once the coordinator decides that all trustees who
are going to announce have announced, it checks how many trustees
have announced.
a. If fewer than the threshold have announced, error.
b. If enough trustees have announced to reach the threshold, it returns
   a list of requests for some of the announced trustees. These
   requests must be passed to the trustees, who will respond with
   share fragments to fulfill those requests. It is up to the
   coordinator how to split the task of compensating for missing
   trustees between the announced trustees. The coordinator can choose
   not to produce requests for some of the announced trustees, or to
   produce requests that request no fragments.
