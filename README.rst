ElectionGuard SDK No-Cryptography Implementation
=====================================

This implementation of the ElectionGuard SDK serves to showcase the API
provided by the SDK. It focuses on specifying and fixing the API.
Although it currently doesn't have encryption, programming against the
header files presented in the include document should allow you to
develop a voting system that is automatically improved with encryption
as the develoment of the ElectionGuard SDK continues.

For more details about that API, see the
:ref:`include`.

.. _building:

Building
--------

To enable cross-platform building, we use `cmake
<https://cmake.org/>`_. Any generator should work, but we describe
here how build using `Visual Studio
<https://visualstudio.microsoft.com/>`_ and using a Unix-like shell.

Visual Studio
~~~~~~~~~~~~~

As long as you have the C++ CMake tools for Windows installed,
:program:`cmake` should automatically integrate with Visual Studio.
This means you can build the project from the IDE, for example by
selecting ``Build > Build All`` in the menu.

Unix-like Systems
~~~~~~~~~~~~~~~~~

First create a build directory and configure the build.

.. code:: sh

    mkdir build
    cmake -S . -B build

To build the SDK static library ``libelectionguard.a``, run

.. code:: sh

   cmake --build build

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

To build and execute an example client of the SDK, run the tests,
for example by selecting ``Test > Run CTests for ElectionGuard SDK``.

The example client can also be built as a standalone project if it is
configured with the location of the SDK, but this is not covered here.


Unix-like Systems
~~~~~~~~~~~~~~~~~

To build and run an example client of the SDK, run the tests:

.. code:: sh

    cmake --build build --target test

Alternatively you can build the client as a stand-alone project.
Create a separate build directory for the client, configure the build
to refer to the built library, and build the client.

.. code:: sh

   mkdir simple_build
   ElectionGuard_DIR="$PWD/build/ElectionGuard" cmake -S examples/simple -B simple_build
   cmake --build simple_build --target simple

The built binary should be located at :file:`simple_build/simple`.


Debugging
---------

To enable debug builds suitable for running with debuggers like
:program:`lldb`, set the ``CMAKE_BUILD_TYPE`` cmake variable to
``Debug`` when configuring. From the command-line, this looks like

.. code:: sh

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

Developing
----------

Some development tools like :program:`ccls` or :program:`cquery` use a
JSON file called :file:`compile_commands.json` to lookup which build
flags are used to build different files. To produce such a file while
compiling, set the ``CMAKE_EXPORT_COMPILE_COMMANDS`` cmake variable.
From the command-line, this looks like

.. code:: sh

   cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

Documentation
-------------

To build the HTML documentation, you will need to have
:program:`doxygen` installed, as well as :program:`python` with the
``sphinx`` and ``breathe`` packages. Then configure your build with
the ``BUILD_DOCUMENTATION`` variable set and rebuild.

.. note::

   Make sure that you've initialized :program:`git` submodules
   correctly. The theme used for the documentation is in a submodule.

   .. code:: sh

       git submodule update --init --recursive


.. code:: sh

    cmake -S . -B build -DBUILD_DOCUMENTATION=ON
    cmake --build build

and the documentation will be built in the :file:`build/docs/html`
directory. You can browse it locally by opening
:file:`build/docs/html/index.html`, or by running a local server

.. code::sh

    # python2
    (cd build/docs/html && python -m SimpleHTTPServer)

    # python3
    python3 -m http.server --directory build/docs/html

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
