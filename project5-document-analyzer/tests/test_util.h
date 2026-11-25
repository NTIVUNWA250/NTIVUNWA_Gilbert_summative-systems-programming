/*
 * test_util.h - a minimal check macro so tests need no external library.
 */
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdio.h>

static int checks_run = 0;
static int checks_failed = 0;

#define CHECK(condition)                                                  \
    do {                                                                  \
        checks_run++;                                                     \
        if (!(condition)) {                                               \
            checks_failed++;                                              \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        }                                                                 \
    } while (0)

static inline int test_summary(const char *suite)
{
    printf("%-18s %3d checks, %d failed\n", suite, checks_run, checks_failed);
    return checks_failed == 0 ? 0 : 1;
}

#endif
