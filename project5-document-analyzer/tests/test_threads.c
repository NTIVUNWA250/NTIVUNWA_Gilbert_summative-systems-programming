#include <stdio.h>
#include <string.h>

#include "task.h"
#include "test_util.h"
#include "worker.h"

static void write_file(const char *path, const char *text)
{
    FILE *file = fopen(path, "w");
    if (file != NULL) {
        fputs(text, file);
        fclose(file);
    }
}

static void test_each_thread_processes_its_own_file(void)
{
    char *files[] = {"build/thread_a.txt", "build/thread_b.txt", "build/thread_missing.txt"};
    ThreadTask *tasks;

    write_file(files[0], "IoT\nIoT IoT\n");
    write_file(files[1], "nothing\n");
    remove(files[2]);

    tasks = init_thread_tasks(files, 3, "iot");
    CHECK(tasks != NULL);
    if (tasks == NULL) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        char path[REPORT_PATH_LEN];
        snprintf(path, sizeof path, "build/%s", tasks[i].report_path);
        snprintf(tasks[i].report_path, REPORT_PATH_LEN, "%s", path);
    }

    CHECK(run_threads(tasks, 3) == 0);
    CHECK(tasks[0].status == TASK_COMPLETED);
    CHECK(tasks[0].occurrences == 3 && tasks[0].matching_lines == 2);
    CHECK(tasks[1].status == TASK_COMPLETED && tasks[1].occurrences == 0);
    CHECK(tasks[2].status == TASK_FAILED);
    CHECK(strstr(tasks[2].error, "cannot open") != NULL);

    FILE *report = fopen("build/thread_a_report.txt", "r");
    CHECK(report != NULL);
    if (report != NULL) {
        fclose(report);
    }
    report = fopen("build/thread_missing_report.txt", "r");
    CHECK(report != NULL);
    if (report != NULL) {
        fclose(report);
    }
    free_thread_tasks(tasks, 3);
}

int main(void)
{
    test_each_thread_processes_its_own_file();
    return test_summary("test_threads");
}
