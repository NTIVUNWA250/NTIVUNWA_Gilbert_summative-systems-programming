/*
 * main.c - Multi-threaded Document Search and Analysis System.
 * Usage: ./document_analyzer "keyword" file1.txt [file2.txt ...]
 */
#include <stdio.h>
#include <string.h>

#include "task.h"
#include "worker.h"

static void print_usage(const char *program)
{
    fprintf(stderr, "Usage: %s \"keyword\" file1.txt [file2.txt ...]\n", program);
}

int main(int argc, char *argv[])
{
    const char *keyword;
    int file_count;
    ThreadTask *tasks;
    int all_completed = 1;

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }
    keyword = argv[1];
    if (strlen(keyword) == 0) {
        fprintf(stderr, "Error: the search keyword cannot be empty.\n");
        return 1;
    }
    file_count = argc - 2;

    printf("===== MULTI-THREADED DOCUMENT ANALYZER =====\n\n");
    printf("Search Keyword: %s\n", keyword);
    printf("Number of Input Files: %d\n\n", file_count);

    tasks = init_thread_tasks(&argv[2], file_count, keyword);
    if (tasks == NULL) {
        fprintf(stderr, "Error: could not allocate thread arguments.\n");
        return 1;
    }

    run_threads(tasks, file_count);
    display_results(tasks, file_count);

    for (int i = 0; i < file_count; i++) {
        if (tasks[i].status != TASK_COMPLETED) {
            all_completed = 0;
        }
    }
    free_thread_tasks(tasks, file_count);
    return all_completed ? 0 : 2;
}
