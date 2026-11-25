#include <string.h>

#include "task.h"
#include "test_util.h"

static void test_report_paths(void)
{
    char path[REPORT_PATH_LEN];

    build_report_path("document1.txt", path, sizeof path);
    CHECK(strcmp(path, "document1_report.txt") == 0);
    build_report_path("docs/notes.v2.txt", path, sizeof path);
    CHECK(strcmp(path, "notes.v2_report.txt") == 0);
    build_report_path("/tmp/README", path, sizeof path);
    CHECK(strcmp(path, "README_report.txt") == 0);
    build_report_path(".hidden", path, sizeof path);
    CHECK(strcmp(path, ".hidden_report.txt") == 0);
}

static void test_init_tasks(void)
{
    char *files[] = {"a/doc.txt", "b/doc.txt", "c/other.txt"};
    ThreadTask *tasks = init_thread_tasks(files, 3, "IoT");

    CHECK(tasks != NULL);
    if (tasks == NULL) {
        return;
    }
    CHECK(tasks[0].thread_number == 1 && tasks[2].thread_number == 3);
    CHECK(strcmp(tasks[1].input_path, "b/doc.txt") == 0);
    CHECK(strcmp(tasks[0].keyword, "IoT") == 0);
    CHECK(tasks[0].status == TASK_PENDING);
    CHECK(strcmp(tasks[0].report_path, "doc_report.txt") == 0);
    CHECK(strcmp(tasks[1].report_path, "doc_2_report.txt") == 0);
    CHECK(strcmp(tasks[2].report_path, "other_report.txt") == 0);
    CHECK(tasks[0].occurrences == 0 && tasks[0].match_line_numbers == NULL);
    free_thread_tasks(tasks, 3);
}

int main(void)
{
    test_report_paths();
    test_init_tasks();
    return test_summary("test_task");
}
