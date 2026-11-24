/*
 * task.h - the data handed to each thread. Every thread gets its own
 * ThreadTask and only writes to that one, so no locking is needed.
 */
#ifndef TASK_H
#define TASK_H

#include <stddef.h>

#define REPORT_PATH_LEN 512
#define ERROR_LEN 160

typedef enum {
    TASK_PENDING,
    TASK_COMPLETED,
    TASK_FAILED
} TaskStatus;

typedef struct {
    int thread_number;
    const char *input_path;
    const char *keyword;
    char report_path[REPORT_PATH_LEN];

    long total_lines;
    long occurrences;
    long matching_lines;
    long *match_line_numbers;
    size_t match_capacity;

    TaskStatus status;
    char error[ERROR_LEN];
} ThreadTask;

ThreadTask *init_thread_tasks(char **files, int count, const char *keyword);
void free_thread_tasks(ThreadTask *tasks, int count);
void build_report_path(const char *input_path, char *out, size_t size);

#endif
