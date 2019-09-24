SRC += $(wildcard examples/simple/*.c)

examples/simple/main: LDFLAGS += -L.
examples/simple/main: $(patsubst %.c,%.o,$(wildcard examples/simple/*.c)) libelectionguard.a
	$(CC) $(CFLAGS) -o $@ $(patsubst %.c,%.o,$(wildcard examples/simple/*.c)) $(LDFLAGS) -lelectionguard
