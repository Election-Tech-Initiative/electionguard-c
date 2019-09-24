# ElectionGuard SDK Mock Implementation

This implementation of the ElectionGuard SDK serves to showcase the
API provided by the SDK. For more details about that API, see the
[`include`](./include) directory.

## Table of Contents

- The [`include`](./include) directory contains the header files that
  form the public API of the SDK.
- The [`examples`](./examples) directory contains example clients of
  the API, showing how it can be used to run an election.
- The [`src`](./src) directory contains an implementation of the SDK.
  At this time, it does not actually implement any cryptographic features,
  and can only run elections that take place on a single machine.
  However, the API was designed to allow for elections run across many
  machines.

## Building

To build the SDK static library `libelectionguard.a`, run
```sh
make libelectionguard.a
```

To build an example client of the SDK, run
```sh
make examples/simple/main
```

### Debugging

To enable debug builds suitable for running with debuggers like `gdb`
or `lldb`, set the `DEBUG` environment variable:
``` sh
DEBUG=1 make examples/simple/main
```

When the `CC` environment variable is set to `clang`, debug builds
will also be built with the address sanitizer enabled, which helps to
catch memory management errors.
``` sh
CC=clang DEBUG=1 make examples/simple/main
```

**Note:** The current implementation allocates most things statically,
leading to a large stack. This leads to stack-overflow errors with
address sanitizer, unless the default stack size is increased. In our
experience, using `ulimit` like this fixes the issue:
``` sh
ulimit -s 32768
```
In addition, this causes issues with `valgrind`. The error messages are usually
pretty helpful. As of the last time this was updated, this valgrind command
works without errors:
```sh
valgrind --main-stacksize=33445532 --max-stackframe=27686064 ./examples/simple/main
```

### Developing

Some development tools like `ccls` or `cquery` use a JSON file called
`compile_commands.json` to lookup which build flags are used to build
different files. To produce such a file while compiling, set the
`COMPILE_COMMANDS` environment variable and then run the
`compile_commands.json` target.
``` sh
COMPILE_COMMANDS=1 make examples/simple/main compile_commands.json
```

## Testing

Build the example client as described in [Building](#Building), and
run it. It should successfully exit with return status `0`, and
produce two files in the current directory with names of the form
`voting_results-XXXXXX.txt` and `tally-XXXXXX.txt` containing ballots
and the decrypted tallies respectively. For more details see [the
source of the example client](./examples/simple).

## Memory Management/Ownership: Who frees what?

Any pointers *returned by* functions in the SDK are considered to be
owned by the caller. This means that the SDK will retain no references
to them, and that the caller must free them when they are done.

Any pointers *passed to* functions in the SDK as arguments are
considered to be borrowed by the function, which means that they will
not be freed by that function, and it is still the responsibility of
the caller to free the pointer. This of course excludes functions
whose purpose is to free an opaque data type, like
`Key_Ceremony_Trustee_free`.

This only applies when functions return with a successful status. If a
function returns with an error status, the client does not need to
free any memory that may have been allocated by the function; it will
clean up after itself.

## Naming Conventions

All public functions are prefixed with the name of their "class" or
module, capitalized.

There are a few different kinds of types, and they each have their own
naming conventions. The rationale is that for types that we rely on
the fact that they are enums or structs, we should not `typedef` them so
that it is clear that they are enums and structs. If that changes, we
will have to go fix it everywhere, which is good, because now we
cannot rely on their representation anymore. Abstract types should be
`typedef`ed because we don't rely on their implementation.

### Abstract Type
A type whose implementation we want to be hidden from clients. This
means that it must be hidden behind a pointer so its size doesn't need
to be known.

**Naming convention:** uppercase, with their structs suffixed with `_s`.
```c
typedef struct Car_s *Car;
```

### Status Enum
A enum whose values represent possible statuses that we want to
return.

**Naming convention:** prefixed by module or scope, then lowercase,
and no `typedef`.
```c
enum Car_status {
  CAR_SUCCESS,
  CAR_ON_FIRE,
};
```

### Return Struct
A struct whose sole purpose is to allow us to return multiple values,
often a status enum and a payload.

**Naming convention:** prefixed by module or scope, then lowercase,
then `_r`, and no `typedef`. If only used for a single function, make
the name identical to the function name, then `_r`. You can forward
declare in the return type.
```c
struct Car_drive_r Car_drive(Car c);

struct Car_drive_r {
  enum Car_status status;
  int x;
  int y;
};
```

### Internal Struct

A type we want to name, but whose implementation need not be hidden.
In fact, we might rely on the details of its representation.

**Naming convention:** all lowercase, no `typedef`.
```c
struct model {
  int year;
  enum color color;
};
```
