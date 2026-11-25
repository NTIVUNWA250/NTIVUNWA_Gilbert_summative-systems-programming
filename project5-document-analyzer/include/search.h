/*
 * search.h - case-insensitive keyword counting and whole-file analysis.
 */
#ifndef SEARCH_H
#define SEARCH_H

#include "task.h"

long count_occurrences(const char *line, const char *keyword);
int analyze_file(ThreadTask *task);

#endif
