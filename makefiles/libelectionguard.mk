LIBELECTIONGUARD_SRC = $(shell find src -name '*.c')
LIBELECTIONGUARD_OBJ = $(patsubst %.c,%.o,$(LIBELECTIONGUARD_SRC))

$(LIBELECTIONGUARD_OBJ): CFLAGS += -Isrc
libelectionguard.a: $(LIBELECTIONGUARD_OBJ)
	$(AR) rcs $@ $^
