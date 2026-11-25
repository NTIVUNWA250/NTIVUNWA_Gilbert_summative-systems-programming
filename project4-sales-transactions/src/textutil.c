/*
 * textutil.c - trimming, safe copying and case-insensitive comparison.
 */
#include <ctype.h>
#include <string.h>

#include "textutil.h"

void text_trim(char *text)
{
    size_t start = 0;
    size_t length = strlen(text);

    while (length > 0 && isspace((unsigned char)text[length - 1])) {
        text[--length] = '\0';
    }
    while (isspace((unsigned char)text[start])) {
        start++;
    }
    if (start > 0) {
        memmove(text, text + start, length - start + 1);
    }
}

void text_copy(char *dest, size_t size, const char *src)
{
    if (size == 0) {
        return;
    }
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

int text_compare_ci(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        int diff = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (diff != 0) {
            return diff;
        }
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int text_contains_ci(const char *haystack, const char *needle)
{
    size_t needle_length = strlen(needle);

    if (needle_length == 0) {
        return 1;
    }
    for (; *haystack != '\0'; haystack++) {
        size_t i = 0;
        while (i < needle_length && haystack[i] != '\0' &&
               tolower((unsigned char)haystack[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == needle_length) {
            return 1;
        }
    }
    return 0;
}
