Simple Client
===============

This simple client is designed to showcase the API by demonstrating
the correct sequence of function calls used in an elecition.

.. warning::

   This example client **does not** do any sort of error handling in
   order to more clearly highlight the "happy path" of an election.
   Please do not use this code in a real system.

:file:`main.c`
--------------

.. literalinclude:: /../examples/simple/main.c
   :language: c
   :linenos:
   :caption: main.c

:file:`main_keyceremony.c`
--------------------------

.. literalinclude:: /../examples/simple/main_keyceremony.c
   :language: c
   :linenos:
   :caption: main_keyceremony.c

:file:`main_voting.c`
---------------------

.. literalinclude:: /../examples/simple/main_voting.c
   :language: c
   :linenos:
   :caption: main_voting.c

:file:`main_decryption.c`
-------------------------

.. literalinclude:: /../examples/simple/main_decryption.c
   :language: c
   :linenos:
   :caption: main_decryption.c
