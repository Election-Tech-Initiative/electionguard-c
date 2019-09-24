Decryption Coordinator
======================

.. doxygentypedef:: Decryption_Coordinator

Initialization and Freeing
--------------------------

.. doxygenfunction:: Decryption_Coordinator_new
.. doxygenstruct:: Decryption_Coordinator_new_r
   :members:
   :undoc-members:

.. doxygenfunction:: Decryption_Coordinator_free

Anouncing
---------

.. doxygenfunction:: Decryption_Coordinator_receive_share

.. doxygenfunction:: Decryption_Coordinator_all_shares_received
.. doxygenstruct:: Decryption_Coordinator_all_shares_received_r
   :members:
   :undoc-members:


Compensating
------------

.. doxygenfunction:: Decryption_Coordinator_receive_fragments

.. doxygenfunction:: Decryption_Coordinator_all_fragments_received

Status Codes
------------

.. doxygenenum:: Decryption_Coordinator_status
