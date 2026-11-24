/*
 * sort.h - manual insertion sort of the student list (no qsort).
 */
#ifndef SORT_H
#define SORT_H

#include "records.h"

typedef enum {
    SORT_BY_ID,
    SORT_BY_NAME,
    SORT_BY_AVERAGE,
    SORT_BY_PROGRAMME
} SortKey;

void sort_students(StudentList *list, SortKey key, int ascending);

#endif
