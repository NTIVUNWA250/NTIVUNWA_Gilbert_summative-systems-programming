/*
 * validation.c - input validation and strict number parsing.
 */
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "validation.h"

int valid_student_id(int id)
{
    return id >= MIN_STUDENT_ID && id <= MAX_STUDENT_ID;
}

int valid_year(int year)
{
    return year >= MIN_YEAR && year <= MAX_YEAR;
}

/* Text must be non-empty, fit its buffer, be printable ASCII and not
 * contain '|', which separates fields in the data file. */
int valid_text_field(const char *text, size_t capacity)
{
    size_t length = strlen(text);

    if (length == 0 || length >= capacity) {
        return 0;
    }
    for (size_t i = 0; i < length; i++) {
        unsigned char c = (unsigned char)text[i];
        if (c < 32 || c > 126 || c == '|') {
            return 0;
        }
    }
    return 1;
}

int valid_max_marks(double max_marks)
{
    return max_marks > 0.0 && max_marks <= MAX_ALLOWED_MARKS;
}

int valid_marks(double marks, double max_marks)
{
    return valid_max_marks(max_marks) && marks >= 0.0 && marks <= max_marks;
}

int parse_int(const char *text, int *out)
{
    char *end = NULL;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);
    if (end == text || errno == ERANGE || value < INT_MIN || value > INT_MAX) {
        return 0;
    }
    while (*end == ' ' || *end == '\t') {
        end++;
    }
    if (*end != '\0') {
        return 0;
    }
    *out = (int)value;
    return 1;
}

int parse_double(const char *text, double *out)
{
    char *end = NULL;
    double value;

    errno = 0;
    value = strtod(text, &end);
    if (end == text || errno == ERANGE || !isfinite(value)) {
        return 0;
    }
    while (*end == ' ' || *end == '\t') {
        end++;
    }
    if (*end != '\0') {
        return 0;
    }
    *out = value;
    return 1;
}
