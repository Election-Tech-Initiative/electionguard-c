Example Clients
===============

These examples demonstrate usage of the ElectionGuard SDK.

.. toctree::
   :maxdepth: 1
   :hidden:

   API Client <examples/api>

- The :doc:`api client <examples/api>` carries out an election where all
  entities communicate via argument-passing.  It demonstrates using the 
  high level API's

- It should successfully exit with return status ``0``, and produce three files:
    - ballots/registered-ballots_YYY_MM_DD
    - ballots_encrypter/encrypted-ballots_YYYY_MM_DD
    - tallies/tallies_YYYY_MM_DD

- It should also print all of the ballot trackers on STDOUT using
  a representation where each byte is converted to a specific word.

- It should execute verification steps as part of the example