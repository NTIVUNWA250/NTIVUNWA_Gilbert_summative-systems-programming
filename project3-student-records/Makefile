CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O0 -g
CPPFLAGS = -Iinclude

CORE_SRC = $(wildcard src/*.c)
CORE_SRC := $(filter-out src/main.c src/menu.c src/input.c src/display.c,$(CORE_SRC))
APP_SRC = $(wildcard src/*.c)
TESTS = $(patsubst tests/%.c,build/%,$(wildcard tests/test_*.c))

.PHONY: all test clean

all: student_records

student_records: $(APP_SRC) $(wildcard include/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(APP_SRC)

build/%: tests/%.c $(CORE_SRC) $(wildcard include/*.h) tests/test_util.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(CORE_SRC)

test: $(TESTS) student_records
	@for t in $(TESTS); do ./$$t || exit 1; done
	@sh tests/integration_test.sh
	@echo "All tests passed."

clean:
	rm -rf build student_records
