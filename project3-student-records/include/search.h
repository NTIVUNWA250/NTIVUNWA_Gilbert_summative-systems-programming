/*
 * search.h - manual linear searches over the student list.
 * The name and programme searches fill a newly allocated array of matching
 * indexes; the caller frees it. They return the number of matches, or -1
 * if memory could not be allocated.
 */
#ifndef SEARCH_H
#define SEARCH_H

#include "records.h"

int search_by_id(const StudentList *list, int id);
int search_by_name(const StudentList *list, const char *query, int **matches);
int search_by_programme(const StudentList *list, const char *query, int **matches);

#endif
