/*
 * search.c - reads one file line by line and records keyword matches.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "search.h"

/* Counts non-overlapping matches, ignoring case: "aaaa" holds "aa" twice. */
long count_occurrences(const char *line, const char *keyword)
{
    size_t keyword_length = strlen(keyword);
    long count = 0;

    if (keyword_length == 0) {
        return 0;
    }
    while (*line != '\0') {
        size_t i = 0;
        while (i < keyword_length && line[i] != '\0' &&
               tolower((unsigned char)line[i]) == tolower((unsigned char)keyword[i])) {
            i++;
        }
        if (i == keyword_length) {
            count++;
            line += keyword_length;
        } else {
            line++;
        }
    }
    return count;
}

static int remember_line(ThreadTask *task, long line_number)
{
    if ((size_t)task->matching_lines == task->match_capacity) {
        size_t new_capacity = task->match_capacity == 0 ? 16 : task->match_capacity * 2;
        long *resized = realloc(task->match_line_numbers, new_capacity * sizeof(long));

        if (resized == NULL) {
            return 0;
        }
        task->match_line_numbers = resized;
        task->match_capacity = new_capacity;
    }
    task->match_line_numbers[task->matching_lines++] = line_number;
    return 1;
}

/* Returns 1 on success. On failure the task holds the error message. */
int analyze_file(ThreadTask *task)
{
    FILE *file = fopen(task->input_path, "r");
    char *line = NULL;
    size_t capacity = 0;
    int ok = 1;

    if (file == NULL) {
        snprintf(task->error, ERROR_LEN, "cannot open %s (%s)", task->input_path,
                 strerror(errno));
        return 0;
    }
    while (getline(&line, &capacity, file) != -1) {
        long found = count_occurrences(line, task->keyword);

        task->total_lines++;
        if (found > 0) {
            task->occurrences += found;
            if (!remember_line(task, task->total_lines)) {
                snprintf(task->error, ERROR_LEN, "out of memory while reading %s",
                         task->input_path);
                ok = 0;
                break;
            }
        }
    }
    if (ok && ferror(file)) {
        snprintf(task->error, ERROR_LEN, "read error in %s", task->input_path);
        ok = 0;
    }
    free(line);
    fclose(file);
    return ok;
}
