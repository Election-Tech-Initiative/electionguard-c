include makefiles/cflags.mk
include makefiles/libelectionguard.mk
include examples/simple.mk

.DEFAULT_GOAL := libelectionguard.a

all: examples/simple/main

FIND_O_JSON = find . -name '*.o.json'
compile_commands.json: $(shell $(FIND_O_JSON))
	$(FIND_O_JSON) | xargs sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' > $@ $<

.PHONY: spell
spell:
	for f in $$(find . -iname '*.c' -o -iname '*.h' -o -iname '*.md'); do \
	  aspell --lang=en --personal=./.aspell-wordlist.txt check "$$f"; \
	done

DEP = $(shell find . -name '*.d')
$(DEP):
include $(wildcard $(DEP))
