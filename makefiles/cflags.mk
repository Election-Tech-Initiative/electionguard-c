# Includes
CFLAGS += -Iinclude

# Warnings
CFLAGS += -Wall -Werror -Wextra -pedantic -pedantic-errors -Wmissing-field-initializers
# Compiler specific warnings
ifeq ($(CC),gcc)
CFLAGS += -Wenum-compare
else ifeq ($(CC),clnag)
CFLAGS += -Wgnu-empty-initializer
endif

# Debug builds by setting DEBUG environment variable
ifdef DEBUG
CFLAGS += -DDEBUG -g
# TODO gcc with nixpkgs address sanitizer doesn't work
ifeq ($(CC),clang)
CFLAGS += -fsanitize=address
endif
endif

# Create compile_commands.json by setting COMPILE_COMMANDS while building, and
# then building the compile_commands.json target
ifdef COMPILE_COMMANDS
CFLAGS += -MJ $*.o.json
endif

# Automatic dependency generation based on
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/#traditional
#
# The additional flags we need to pass to CC to produce the dep file
CFLAGS += -MT $@ -MMD -MP -MF $*.d
