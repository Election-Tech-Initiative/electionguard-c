Example Clients
===============

These examples demonstrate correct usage of the ElectionGuard SDK.

.. toctree::
   :maxdepth: 1
   :hidden:

   Simple Client <examples/simple>

- The :doc:`simple client <examples/simple>` carries out an election where all
  entities communicate via argument-passing.

  It should successfully exit with return status ``0``, and produce two files
  in the current directory with names of the form :file:`voting_results-XXXXXX`
  and :file:`tally-XXXXXX` containing ballots and the decrypted tallies
  respectively. It should also print all of the ballot trackers on STDOUT using
  a representation where each byte is converted to a specific word.

