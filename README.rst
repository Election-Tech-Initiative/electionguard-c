ElectionGuard SDK Mock Implementation
=====================================

This implementation of the ElectionGuard SDK serves to showcase the API
provided by the SDK. For more details about that API, see the
:ref:`include`.

.. _building:

Building
--------

To enable cross-platform building, we use `cmake <https://cmake.org/>`_. Here
we describe how to use Microsoft Visual Studio and :program:`make`, but other
backends like :program:`ninja` should work as well.

Visual Studio
~~~~~~~~~~~~~

As long as you have the C++ CMake tools for Windows installed, :program:`cmake`
should automatically integrate with Visual Studio. This means you can build the
project from the IDE, for example by right-clicking :file:`CMakeLists.txt` and
then selecting ``Build``.

Unix-like Systems
~~~~~~~~~~~~~~~~~

First use :program:`cmake` to generate build instructions for some
generator like :program:`make` or :program:`ninja`. You can see a list
of available generators by running ``cmake -G``. We will assume from
now on that you are using the :program:`make` generator, but it should
be straightforward to build the corresponding targets with whatever
generator you choose to use.

.. code:: sh

    mkdir build
    cd build
    cmake ..

To build the SDK static library ``libelectionguard.a``, run

.. code:: sh

   make electionguard

To build an example client of the SDK, run

.. code:: sh

   make simple

Testing
--------

Currently you can exercise the SDK by running the :doc:`example client
<examples/simple>`. We include a cmake test to do so automatically. You can
also execute the client directly to better examine the output it produces.

.. warning::

  The current implementation allocates most things statically, leading
  to a large stack. This can cause stack-overflows.

  The size of the stack mostly depends on the value of :data:`MAX_TRUSTEES` in
  :file:`include/max_trustees.h`, so one way to fix the problem is to reduce
  that number and recompile.

  You can also increase the stack size, for example using :command:`ulimit`.

  In addition, this causes issues with :program:`valgrind`. The error messages
  are usually pretty helpful, and setting ``--main-stacksize`` and
  ``--main-stackframe`` according to its reccomendations usually fixes the issue.

Visual Studio
~~~~~~~~~~~~~

As with building, you should be able to use the IDE to run the tests, for
example by right-clicking :file:`CMakeLists.txt` and then selecing ``Run Tests``.

Unix-like Systems
~~~~~~~~~~~~~~~~~

From the build directory, run

.. code:: sh

   make test

Debugging
---------

To enable debug builds suitable for running with debuggers like
:program:`lldb`, set the ``CMAKE_BUILD_TYPE`` cmake variable to
``Debug``. From the command-line, this looks like

.. code:: sh

    cmake -DCMAKE_BUILD_TYPE=Debug ..

Developing
----------

Some development tools like :program:`ccls` or :program:`cquery` use a
JSON file called :file:`compile_commands.json` to lookup which build
flags are used to build different files. To produce such a file while
compiling, set the ``CMAKE_EXPORT_COMPILE_COMMANDS`` cmake variable.
From the command-line, this looks like

.. code:: sh

   cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

Documentation
-------------

To build the HTML documentation, you will need to have
:program:`doxygen` installed, as well as :program:`python` with the
``sphinx`` and ``breathe`` packages. Then run

.. code:: sh

    make docs

and the documentation will be built in the :file:`html` directory. You
can browse it locally by opening :file:`html/index.html`, or by
running a local server

.. code::sh

    # python2
    (cd html && python -m SimpleHTTPServer)

    # python3
    python3 -m http.server --directory html


.. note::

   Make sure that you've initialized :program:`git` submodules
   correctly. The theme used for the documentation is in a submodule.

   .. code:: sh

       git submodule update --init --recursive

Memory Management/Ownership: Who frees what?
--------------------------------------------

Any pointers *returned by* functions in the SDK are considered to be
owned by the caller. This means that the SDK will retain no references
to them, and that the caller must free them when they are done.

Any pointers *passed to* functions in the SDK as arguments are
considered to be borrowed by the function, which means that they will
not be freed by that function, and it is still the responsibility of the
caller to free the pointer. This of course excludes functions whose
purpose is to free an opaque data type, like
:func:`KeyCeremony_Trustee_free()`.

This only applies when functions return with a successful status. If a
function returns with an error status, the client does not need to free
any memory that may have been allocated by the function; it will clean
up after itself.

Naming Conventions
------------------

All public functions are prefixed with the name of their “class” or
module, capitalized.

There are a few different kinds of types, and they each have their own
naming conventions. The rationale is that for types that we rely on the
fact that they are enums or structs, we should not ``typedef`` them so
that it is clear that they are enums and structs. If that changes, we
will have to go fix it everywhere, which is good, because now we cannot
rely on their representation anymore. Abstract types should be
``typedef``\ ed because we don’t rely on their implementation.

Abstract Type
~~~~~~~~~~~~~

A type whose implementation we want to be hidden from clients. This
means that it must be hidden behind a pointer so its size doesn’t need
to be known.

**Naming convention:** uppercase, with their structs suffixed with
``_s``.

.. code:: c

   typedef struct Car_s *Car;

Status Enum
~~~~~~~~~~~

A enum whose values represent possible statuses that we want to return.

**Naming convention:** prefixed by module or scope, then lowercase, and
no ``typedef``.

.. code:: c

   enum Car_status {
     CAR_SUCCESS,
     CAR_ON_FIRE,
   };

Return Struct
~~~~~~~~~~~~~

A struct whose sole purpose is to allow us to return multiple values,
often a status enum and a payload.

**Naming convention:** prefixed by module or scope, then lowercase, then
``_r``, and no ``typedef``. If only used for a single function, make the
name identical to the function name, then ``_r``. You can forward
declare in the return type.

.. code:: c

   struct Car_drive_r Car_drive(Car c);

   struct Car_drive_r {
     enum Car_status status;
     int x;
     int y;
   };

Internal Struct
~~~~~~~~~~~~~~~

A type we want to name, but whose implementation need not be hidden. In
fact, we might rely on the details of its representation.

**Naming convention:** all lowercase, no ``typedef``.

.. code:: c

   struct model {
     int year;
     enum color color;
   };
