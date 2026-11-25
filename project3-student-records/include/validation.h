/*
 * validation.h - rules for every value a user can type or a file can hold.
 * Each valid_* function returns 1 when the value is acceptable, 0 otherwise.
 */
#ifndef VALIDATION_H
#define VALIDATION_H

#include <stddef.h>

#define MIN_STUDENT_ID 1
#define MAX_STUDENT_ID 99999999
#define MIN_YEAR 1
#define MAX_YEAR 6
#define MAX_ALLOWED_MARKS 1000.0

int valid_student_id(int id);
int valid_year(int year);
int valid_text_field(const char *text, size_t capacity);
int valid_max_marks(double max_marks);
int valid_marks(double marks, double max_marks);

int parse_int(const char *text, int *out);
int parse_double(const char *text, double *out);

#endif
