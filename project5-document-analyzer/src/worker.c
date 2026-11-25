/*
 * worker.c - one thread per file. Each thread analyses its file and writes
 * its own report; the main thread only creates, joins and summarises.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "report.h"
#include "search.h"
#include "worker.h"

void *process_file(void *arg)
{
    ThreadTask *task = (ThreadTask *)arg;

    printf("[Thread %d] Processing %s ...\n", task->thread_number, task->input_path);

    task->status = analyze_file(task) ? TASK_COMPLETED : TASK_FAILED;
    if (!write_report(task)) {
        if (task->status == TASK_COMPLETED) {
            snprintf(task->error, ERROR_LEN, "cannot write report %s", task->report_path);
        }
        task->status = TASK_FAILED;
    }

    if (task->status == TASK_COMPLETED) {
        printf("[Thread %d] Successfully processed %s\n", task->thread_number,
               task->input_path);
    } else {
        printf("[Thread %d] Error: %s\n", task->thread_number, task->error);
    }
    return NULL;
}

/* Returns the number of threads that could not be created. Only threads
 * that were created are joined. */
int run_threads(ThreadTask *tasks, int count)
{
    pthread_t *threads = calloc((size_t)count, sizeof(pthread_t));
    int *created = calloc((size_t)count, sizeof(int));
    int failures = 0;

    if (threads == NULL || created == NULL) {
        free(threads);
        free(created);
        for (int i = 0; i < count; i++) {
            tasks[i].status = TASK_FAILED;
            snprintf(tasks[i].error, ERROR_LEN, "out of memory before starting threads");
        }
        return count;
    }

    for (int i = 0; i < count; i++) {
        int rc = pthread_create(&threads[i], NULL, process_file, &tasks[i]);

        if (rc == 0) {
            created[i] = 1;
            printf("[Main] Created thread %d for %s\n", tasks[i].thread_number,
                   tasks[i].input_path);
        } else {
            tasks[i].status = TASK_FAILED;
            snprintf(tasks[i].error, ERROR_LEN, "thread creation failed (%s)", strerror(rc));
            printf("[Main] Error: could not create thread %d for %s\n",
                   tasks[i].thread_number, tasks[i].input_path);
            failures++;
        }
    }

    for (int i = 0; i < count; i++) {
        if (created[i]) {
            pthread_join(threads[i], NULL);
        }
    }
    free(threads);
    free(created);
    return failures;
}

void display_results(const ThreadTask *tasks, int count)
{
    int completed = 0;

    printf("\n===== PROCESSING SUMMARY =====\n\n");
    for (int i = 0; i < count; i++) {
        const ThreadTask *t = &tasks[i];

        if (t->status == TASK_COMPLETED) {
            completed++;
            printf("%s: Completed - %ld occurrence(s) on %ld of %ld line(s)%s -> %s\n",
                   t->input_path, t->occurrences, t->matching_lines, t->total_lines,
                   t->occurrences == 0 ? " (keyword not found)" : "", t->report_path);
        } else {
            printf("%s: Failed - %s\n", t->input_path, t->error);
        }
    }

    printf("\n");
    if (completed == count) {
        printf("All processing tasks completed.\n");
        printf("Reports generated successfully.\n");
    } else {
        printf("%d of %d file(s) processed successfully. See errors above.\n", completed,
               count);
    }
}
