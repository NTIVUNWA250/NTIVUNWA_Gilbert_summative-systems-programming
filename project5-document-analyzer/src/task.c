/*
 * task.c - creating and releasing the per-thread arguments.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"

/* "docs/document1.txt" becomes "document1_report.txt" in the current folder. */
void build_report_path(const char *input_path, char *out, size_t size)
{
    const char *name = strrchr(input_path, '/');
    const char *dot;
    int length;

    name = name == NULL ? input_path : name + 1;
    dot = strrchr(name, '.');
    length = (dot == NULL || dot == name) ? (int)strlen(name) : (int)(dot - name);
    snprintf(out, size, "%.*s_report.txt", length, name);
}

/* Two inputs with the same base name would write the same report, so later
 * ones get their thread number added to keep every report separate. */
static void make_unique(ThreadTask *tasks, int index)
{
    for (int i = 0; i < index; i++) {
        if (strcmp(tasks[i].report_path, tasks[index].report_path) == 0) {
            char base[REPORT_PATH_LEN];
            size_t keep = strlen(tasks[index].report_path) - strlen("_report.txt");

            snprintf(base, sizeof base, "%.*s", (int)keep, tasks[index].report_path);
            snprintf(tasks[index].report_path, REPORT_PATH_LEN, "%s_%d_report.txt", base,
                     tasks[index].thread_number);
            return;
        }
    }
}

ThreadTask *init_thread_tasks(char **files, int count, const char *keyword)
{
    ThreadTask *tasks = calloc((size_t)count, sizeof(ThreadTask));

    if (tasks == NULL) {
        return NULL;
    }
    for (int i = 0; i < count; i++) {
        tasks[i].thread_number = i + 1;
        tasks[i].input_path = files[i];
        tasks[i].keyword = keyword;
        tasks[i].status = TASK_PENDING;
        build_report_path(files[i], tasks[i].report_path, REPORT_PATH_LEN);
        make_unique(tasks, i);
    }
    return tasks;
}

void free_thread_tasks(ThreadTask *tasks, int count)
{
    if (tasks == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        free(tasks[i].match_line_numbers);
    }
    free(tasks);
}
