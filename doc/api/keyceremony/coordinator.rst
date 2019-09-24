Key Ceremony Coordinator
========================

.. doxygentypedef:: KeyCeremony_Coordinator

Initialization and Freeing
--------------------------

.. doxygenfunction:: KeyCeremony_Coordinator_new
.. doxygenstruct:: KeyCeremony_Coordinator_new_r
   :members:
   :undoc-members:

.. doxygenfunction:: KeyCeremony_Coordinator_free

Key Generation
--------------

.. doxygenfunction:: KeyCeremony_Coordinator_receive_key_generated

.. doxygenfunction:: KeyCeremony_Coordinator_all_keys_received
.. doxygenstruct:: KeyCeremony_Coordinator_all_keys_received_r
   :members:
   :undoc-members:

Share Generation
----------------

.. doxygenfunction:: KeyCeremony_Coordinator_receive_shares_generated

.. doxygenfunction:: KeyCeremony_Coordinator_all_shares_received
.. doxygenstruct:: KeyCeremony_Coordinator_all_shares_received_r
   :members:
   :undoc-members:

Verification
------------

.. doxygenfunction:: KeyCeremony_Coordinator_receive_shares_verified

.. doxygenfunction:: KeyCeremony_Coordinator_publish_joint_key
.. doxygenstruct:: KeyCeremony_Coordinator_publish_joint_key_r
   :members:
   :undoc-members:

Status Codes
------------

.. doxygenenum:: KeyCeremony_Coordinator_status
