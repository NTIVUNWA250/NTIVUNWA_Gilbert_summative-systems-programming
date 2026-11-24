#include <stdio.h>
#include <stdlib.h>

#include "search.h"
#include "test_util.h"

#define SAMPLE "build/search_sample.txt"

static void write_sample(const char *text)
{
    FILE *file = fopen(SAMPLE, "w");
    if (file != NULL) {
        fputs(text, file);
        fclose(file);
    }
}

static void test_count_occurrences(void)
{
    CHECK(count_occurrences("IoT devices and iot sensors", "IoT") == 2);
    CHECK(count_occurrences("IOT", "iot") == 1);
    CHECK(count_occurrences("no match here", "IoT") == 0);
    CHECK(count_occurrences("aaaa", "aa") == 2);
    CHECK(count_occurrences("", "IoT") == 0);
    CHECK(count_occurrences("IoT", "") == 0);
    CHECK(count_occurrences("Io", "IoT") == 0);
}

static void test_analyze_file(void)
{
    ThreadTask task = {0};

    write_sample("IoT one\nnothing\niot and IOT\n\nlast line IoT");
    task.input_path = SAMPLE;
    task.keyword = "IoT";
    CHECK(analyze_file(&task) == 1);
    CHECK(task.total_lines == 5);
    CHECK(task.occurrences == 4);
    CHECK(task.matching_lines == 3);
    CHECK(task.matching_lines == 3 && task.match_line_numbers[0] == 1);
    CHECK(task.matching_lines == 3 && task.match_line_numbers[1] == 3);
    CHECK(task.matching_lines == 3 && task.match_line_numbers[2] == 5);
    free(task.match_line_numbers);
}

static void test_keyword_not_found_and_empty(void)
{
    ThreadTask task = {0};

    write_sample("alpha\nbeta\n");
    task.input_path = SAMPLE;
    task.keyword = "gamma";
    CHECK(analyze_file(&task) == 1);
    CHECK(task.total_lines == 2 && task.occurrences == 0 && task.matching_lines == 0);

    ThreadTask empty = {0};
    write_sample("");
    empty.input_path = SAMPLE;
    empty.keyword = "x";
    CHECK(analyze_file(&empty) == 1 && empty.total_lines == 0);
}

static void test_many_matches_grow_the_array(void)
{
    ThreadTask task = {0};
    FILE *file = fopen(SAMPLE, "w");

    for (int i = 0; i < 100; i++) {
        fputs("IoT\n", file);
    }
    fclose(file);
    task.input_path = SAMPLE;
    task.keyword = "iot";
    CHECK(analyze_file(&task) == 1);
    CHECK(task.matching_lines == 100 && task.match_line_numbers[99] == 100);
    free(task.match_line_numbers);
}

static void test_missing_file(void)
{
    ThreadTask task = {0};

    task.input_path = "build/no_such_file.txt";
    task.keyword = "IoT";
    CHECK(analyze_file(&task) == 0);
    CHECK(task.error[0] != '\0');
}

int main(void)
{
    test_count_occurrences();
    test_analyze_file();
    test_keyword_not_found_and_empty();
    test_many_matches_grow_the_array();
    test_missing_file();
    return test_summary("test_search");
}
