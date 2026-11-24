#include "fixtures.h"
#include "sort.h"
#include "test_util.h"

static int ids_are(const StudentList *list, int a, int b, int c, int d)
{
    return list->count == 4 && list->students[0].id == a &&
           list->students[1].id == b && list->students[2].id == c &&
           list->students[3].id == d;
}

static void test_by_id(void)
{
    StudentList list;

    fixture_load(&list);
    sort_students(&list, SORT_BY_ID, 1);
    CHECK(ids_are(&list, 1001, 1002, 1003, 1004));
    sort_students(&list, SORT_BY_ID, 0);
    CHECK(ids_are(&list, 1004, 1003, 1002, 1001));
    list_free(&list);
}

static void test_by_name(void)
{
    StudentList list;

    fixture_load(&list);
    sort_students(&list, SORT_BY_NAME, 1);
    CHECK(ids_are(&list, 1003, 1001, 1002, 1004));
    list_free(&list);
}

static void test_by_average(void)
{
    StudentList list;

    fixture_load(&list);
    sort_students(&list, SORT_BY_AVERAGE, 0);
    CHECK(ids_are(&list, 1003, 1002, 1001, 1004));
    sort_students(&list, SORT_BY_AVERAGE, 1);
    CHECK(ids_are(&list, 1004, 1001, 1002, 1003));
    list_free(&list);
}

static void test_by_programme(void)
{
    StudentList list;

    fixture_load(&list);
    sort_students(&list, SORT_BY_PROGRAMME, 1);
    CHECK(ids_are(&list, 1001, 1003, 1002, 1004));
    list_free(&list);
}

static void test_courses_follow_student(void)
{
    StudentList list;
    Student *alice;

    fixture_load(&list);
    sort_students(&list, SORT_BY_ID, 1);
    alice = list_get(&list, 1003);
    CHECK(alice->course_count == 2 && alice->courses[1].marks == 90);
    list_free(&list);
}

int main(void)
{
    test_by_id();
    test_by_name();
    test_by_average();
    test_by_programme();
    test_courses_follow_student();
    return test_summary("test_sort");
}
