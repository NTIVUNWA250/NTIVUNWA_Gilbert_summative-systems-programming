#include <stdio.h>
#include <string.h>

#include "report.h"
#include "test_util.h"

#define REPORT "build/unit_report.txt"

static int file_contains(const char *path, const char *text)
{
    char line[256];
    FILE *file = fopen(path, "r");
    int found = 0;

    if (file == NULL) {
        return 0;
    }
    while (!found && fgets(line, sizeof line, file) != NULL) {
        found = strstr(line, text) != NULL;
    }
    fclose(file);
    return found;
}

static void test_successful_report(void)
{
    long lines[] = {2, 5, 9};
    ThreadTask task = {0};

    task.thread_number = 2;
    task.input_path = "document2.txt";
    task.keyword = "IoT";
    snprintf(task.report_path, REPORT_PATH_LEN, "%s", REPORT);
    task.total_lines = 12;
    task.occurrences = 4;
    task.matching_lines = 3;
    task.match_line_numbers = lines;
    task.status = TASK_COMPLETED;

    CHECK(write_report(&task) == 1);
    CHECK(file_contains(REPORT, "Input file               : document2.txt"));
    CHECK(file_contains(REPORT, "Search keyword           : IoT"));
    CHECK(file_contains(REPORT, "Total lines processed    : 12"));
    CHECK(file_contains(REPORT, "Keyword occurrences      : 4"));
    CHECK(file_contains(REPORT, "Lines containing keyword : 3"));
    CHECK(file_contains(REPORT, "  2, 5, 9"));
    CHECK(file_contains(REPORT, "Processing status        : SUCCESS"));
}

static void test_not_found_and_failed_reports(void)
{
    ThreadTask task = {0};

    task.thread_number = 1;
    task.input_path = "empty.txt";
    task.keyword = "IoT";
    snprintf(task.report_path, REPORT_PATH_LEN, "%s", REPORT);
    task.total_lines = 3;
    task.status = TASK_COMPLETED;
    CHECK(write_report(&task) == 1);
    CHECK(file_contains(REPORT, "Line numbers with matches: none"));
    CHECK(file_contains(REPORT, "SUCCESS (keyword not found)"));

    task.status = TASK_FAILED;
    snprintf(task.error, ERROR_LEN, "cannot open empty.txt");
    CHECK(write_report(&task) == 1);
    CHECK(file_contains(REPORT, "Processing status        : FAILED"));
    CHECK(file_contains(REPORT, "Reason                   : cannot open empty.txt"));
}

static void test_unwritable_report(void)
{
    ThreadTask task = {0};

    task.input_path = "x.txt";
    task.keyword = "IoT";
    snprintf(task.report_path, REPORT_PATH_LEN, "no_such_dir/x_report.txt");
    CHECK(write_report(&task) == 0);
}

int main(void)
{
    test_successful_report();
    test_not_found_and_failed_reports();
    test_unwritable_report();
    return test_summary("test_report");
}
