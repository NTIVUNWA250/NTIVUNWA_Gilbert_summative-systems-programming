/*
 * textutil.h - small string helpers shared by the whole program.
 */
#ifndef TEXTUTIL_H
#define TEXTUTIL_H

#include <stddef.h>

void text_trim(char *text);
void text_copy(char *dest, size_t size, const char *src);
int text_compare_ci(const char *a, const char *b);
int text_contains_ci(const char *haystack, const char *needle);

#endif
