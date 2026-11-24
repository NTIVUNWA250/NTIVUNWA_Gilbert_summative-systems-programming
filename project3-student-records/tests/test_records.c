#include <string.h>

#include "fixtures.h"
#include "records.h"
#include "test_util.h"

static void test_add_and_get(void)
{
    StudentList list;
    Student *student;

    list_init(&list);
    CHECK(list_add(&list, 1, "Ana", "Keza", "Law", 1) == STATUS_OK);
    CHECK(list_add(&list, 2, "Ben", "Ishimwe", "Law", 2) == STATUS_OK);
    CHECK(list.count == 2);

    student = list_get(&list, 2);
    CHECK(student != NULL && strcmp(student->first_name, "Ben") == 0);
    CHECK(student->course_count == 0 && student->courses == NULL);
    CHECK(list_get(&list, 99) == NULL);
    list_free(&list);
    CHECK(list.count == 0 && list.students == NULL);
}

static void test_add_rejects_bad_input(void)
{
    StudentList list;

    list_init(&list);
    CHECK(list_add(&list, 1, "Ana", "Keza", "Law", 1) == STATUS_OK);
    CHECK(list_add(&list, 1, "Copy", "Keza", "Law", 1) == STATUS_DUPLICATE);
    CHECK(list_add(&list, 0, "Ana", "Keza", "Law", 1) == STATUS_INVALID);
    CHECK(list_add(&list, 5, "", "Keza", "Law", 1) == STATUS_INVALID);
    CHECK(list_add(&list, 5, "Ana", "Keza", "Law", 9) == STATUS_INVALID);
    CHECK(list.count == 1);
    list_free(&list);
}

static void test_remove(void)
{
    StudentList list;

    fixture_load(&list);
    CHECK(list_remove(&list, 1001) == STATUS_OK);
    CHECK(list.count == 3);
    CHECK(list_get(&list, 1001) == NULL);
    CHECK(list_get(&list, 1002) != NULL);
    CHECK(list_remove(&list, 1001) == STATUS_NOT_FOUND);

    list_remove(&list, 1002);
    list_remove(&list, 1003);
    list_remove(&list, 1004);
    CHECK(list.count == 0 && list.students == NULL);
    list_free(&list);
}

static void test_update_info(void)
{
    StudentList list;
    Student *student;

    fixture_load(&list);
    student = list_get(&list, 1001);
    CHECK(student_update_info(student, "Bryan", "Mugisha", "Finance", 2) == STATUS_OK);
    CHECK(strcmp(student->first_name, "Bryan") == 0 && student->year == 2);
    CHECK(student_update_info(student, "Bryan", "", "Finance", 2) == STATUS_INVALID);
    CHECK(strcmp(student->last_name, "Mugisha") == 0);
    list_free(&list);
}

static void test_courses(void)
{
    StudentList list;
    Student *student;

    fixture_load(&list);
    student = list_get(&list, 1004);
    CHECK(student_add_course(student, "EN101", "Mechanics", 70, 100) == STATUS_OK);
    CHECK(student_add_course(student, "en101", "Again", 70, 100) == STATUS_DUPLICATE);
    CHECK(student_add_course(student, "EN102", "Circuits", 120, 100) == STATUS_INVALID);
    CHECK(student_add_course(student, "EN103", "Drawing", 10, 0) == STATUS_INVALID);
    CHECK(student->course_count == 1);

    CHECK(student_update_marks(student, "EN101", 55) == STATUS_OK);
    CHECK(student->courses[0].marks == 55);
    CHECK(student_update_marks(student, "EN101", 101) == STATUS_INVALID);
    CHECK(student_update_marks(student, "XX999", 50) == STATUS_NOT_FOUND);
    list_free(&list);
}

static void test_averages(void)
{
    StudentList list;
    double average = 0;
    Course pass = {"A", "A", 50, 100};
    Course fail = {"B", "B", 49.5, 100};

    fixture_load(&list);
    CHECK(student_average(list_get(&list, 1003), &average) && average == 85.0);
    CHECK(student_average(list_get(&list, 1002), &average) && average == 62.5);
    CHECK(!student_average(list_get(&list, 1004), &average));
    CHECK(course_passed(&pass));
    CHECK(!course_passed(&fail));
    list_free(&list);
}

static void test_weighted_average(void)
{
    StudentList list;
    double average = 0;

    list_init(&list);
    list_add(&list, 7, "Eve", "Mutoni", "Law", 1);
    student_add_course(list_get(&list, 7), "L1", "Torts", 20, 50);
    student_add_course(list_get(&list, 7), "L2", "Contracts", 80, 150);
    CHECK(student_average(list_get(&list, 7), &average) && average == 50.0);
    list_free(&list);
}

int main(void)
{
    test_add_and_get();
    test_add_rejects_bad_input();
    test_remove();
    test_update_info();
    test_courses();
    test_averages();
    test_weighted_average();
    return test_summary("test_records");
}
