CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O0 -g
CPPFLAGS = -Iinclude

APP_SRC = $(wildcard src/*.c)
CORE_SRC = $(filter-out src/main.c src/menu.c src/input.c src/display.c,$(APP_SRC))
TESTS = $(patsubst tests/%.c,build/%,$(wildcard tests/test_*.c))

.PHONY: all test clean

all: sales_system

sales_system: $(APP_SRC) $(wildcard include/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(APP_SRC)

build/%: tests/%.c $(CORE_SRC) $(wildcard include/*.h) tests/test_util.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(CORE_SRC)

test: $(TESTS) sales_system
	@for t in $(TESTS); do ./$$t || exit 1; done
	@if [ -f tests/integration_test.sh ]; then sh tests/integration_test.sh; fi
	@echo "All tests passed."

clean:
	rm -rf build sales_system sales_system.dSYM
