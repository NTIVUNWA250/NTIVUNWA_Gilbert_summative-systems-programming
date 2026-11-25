#include <stdlib.h>

#include "fixtures.h"
#include "search.h"
#include "test_util.h"

static void test_by_id(void)
{
    StudentList list;

    fixture_load(&list);
    CHECK(search_by_id(&list, 1003) == 0);
    CHECK(search_by_id(&list, 1004) == 3);
    CHECK(search_by_id(&list, 5555) == -1);
    list_free(&list);
}

static void test_by_name(void)
{
    StudentList list;
    int *matches = NULL;
    int found;

    fixture_load(&list);
    found = search_by_name(&list, "alice", &matches);
    CHECK(found == 1 && matches[0] == 0);
    free(matches);

    found = search_by_name(&list, "INGABIRE", &matches);
    CHECK(found == 1 && list.students[matches[0]].id == 1002);
    free(matches);

    found = search_by_name(&list, "Brian Mugisha", &matches);
    CHECK(found == 1 && list.students[matches[0]].id == 1001);
    free(matches);

    found = search_by_name(&list, "zzz", &matches);
    CHECK(found == 0);
    free(matches);
    list_free(&list);
}

static void test_by_programme(void)
{
    StudentList list;
    int *matches = NULL;
    int found;

    fixture_load(&list);
    found = search_by_programme(&list, "computer", &matches);
    CHECK(found == 2);
    CHECK(found == 2 && list.students[matches[0]].id == 1003);
    CHECK(found == 2 && list.students[matches[1]].id == 1002);
    free(matches);
    list_free(&list);
}

static void test_empty_list(void)
{
    StudentList list;
    int *matches = NULL;

    list_init(&list);
    CHECK(search_by_id(&list, 1) == -1);
    CHECK(search_by_name(&list, "a", &matches) == 0 && matches == NULL);
}

int main(void)
{
    test_by_id();
    test_by_name();
    test_by_programme();
    test_empty_list();
    return test_summary("test_search");
}
