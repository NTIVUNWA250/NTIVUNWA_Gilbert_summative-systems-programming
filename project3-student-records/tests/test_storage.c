#include <stdio.h>
#include <string.h>

#include "fixtures.h"
#include "storage.h"
#include "test_util.h"

#define TEST_FILE "build/test_students.txt"

static void write_file(const char *text)
{
    FILE *file = fopen(TEST_FILE, "w");
    if (file != NULL) {
        fputs(text, file);
        fclose(file);
    }
}

static void test_round_trip(void)
{
    StudentList original;
    StudentList loaded;
    Student *alice;

    fixture_load(&original);
    CHECK(storage_save(&original, TEST_FILE) == STATUS_OK);

    list_init(&loaded);
    CHECK(storage_load(&loaded, TEST_FILE, NULL) == STATUS_OK);
    CHECK(loaded.count == 4);
    alice = list_get(&loaded, 1003);
    CHECK(alice != NULL && strcmp(alice->programme, "Computer Science") == 0);
    CHECK(alice != NULL && alice->course_count == 2);
    CHECK(alice != NULL && strcmp(alice->courses[1].code, "MA101") == 0);
    CHECK(alice != NULL && alice->courses[1].marks == 90);
    CHECK(list_get(&loaded, 1004)->course_count == 0);

    list_free(&original);
    list_free(&loaded);
}

static void test_missing_file(void)
{
    StudentList list;

    list_init(&list);
    CHECK(storage_load(&list, "build/does_not_exist.txt", NULL) == STATUS_NOT_FOUND);
    CHECK(list.count == 0);
}

static void test_bad_directory(void)
{
    StudentList list;

    fixture_load(&list);
    CHECK(storage_save(&list, "no_such_dir/students.txt") == STATUS_FILE_ERROR);
    list_free(&list);
}

static void test_corrupt_files_are_rejected(void)
{
    StudentList list;
    int line = 0;

    fixture_load(&list);

    write_file("S|1|Ana|Keza|Law|1|1\nC|L1|Torts|abc|100\n");
    CHECK(storage_load(&list, TEST_FILE, &line) == STATUS_INVALID && line == 2);

    write_file("S|1|Ana|Keza|Law|1|2\nC|L1|Torts|50|100\n");
    CHECK(storage_load(&list, TEST_FILE, &line) == STATUS_INVALID);

    write_file("S|1|Ana|Keza|Law|1|0\nS|1|Copy|Keza|Law|1|0\n");
    CHECK(storage_load(&list, TEST_FILE, &line) == STATUS_DUPLICATE && line == 2);

    write_file("C|L1|Torts|50|100\n");
    CHECK(storage_load(&list, TEST_FILE, &line) == STATUS_INVALID && line == 1);

    CHECK(list.count == 4);
    list_free(&list);
}

int main(void)
{
    test_round_trip();
    test_missing_file();
    test_bad_directory();
    test_corrupt_files_are_rejected();
    return test_summary("test_storage");
}
