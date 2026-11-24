CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O0 -g
CPPFLAGS = -Iinclude -D_POSIX_C_SOURCE=200809L
LDLIBS = -pthread

APP_SRC = $(wildcard src/*.c)
CORE_SRC = $(filter-out src/main.c,$(APP_SRC))
TESTS = $(patsubst tests/%.c,build/%,$(wildcard tests/test_*.c))

.PHONY: all test clean

all: document_analyzer

document_analyzer: $(APP_SRC) $(wildcard include/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -o $@ $(APP_SRC) $(LDLIBS)

build/%: tests/%.c $(CORE_SRC) $(wildcard include/*.h) tests/test_util.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -o $@ $< $(CORE_SRC) $(LDLIBS)

test: $(TESTS) document_analyzer
	@for t in $(TESTS); do ./$$t || exit 1; done
	@sh tests/integration_test.sh
	@echo "All tests passed."

clean:
	rm -rf build document_analyzer document_analyzer.dSYM *_report.txt
