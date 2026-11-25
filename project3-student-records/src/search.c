/*
 * search.c - linear search by ID, by name and by programme.
 */
#include <stdio.h>
#include <stdlib.h>

#include "search.h"
#include "textutil.h"

typedef int (*StudentMatcher)(const Student *student, const char *query);

int search_by_id(const StudentList *list, int id)
{
    for (int i = 0; i < list->count; i++) {
        if (list->students[i].id == id) {
            return i;
        }
    }
    return -1;
}

static int name_matches(const Student *student, const char *query)
{
    char full_name[NAME_LEN * 2];

    snprintf(full_name, sizeof full_name, "%s %s", student->first_name,
             student->last_name);
    return text_contains_ci(full_name, query);
}

static int programme_matches(const Student *student, const char *query)
{
    return text_contains_ci(student->programme, query);
}

static int collect_matches(const StudentList *list, const char *query,
                           StudentMatcher matcher, int **matches)
{
    int found = 0;

    *matches = NULL;
    if (list->count == 0) {
        return 0;
    }
    *matches = malloc((size_t)list->count * sizeof(int));
    if (*matches == NULL) {
        return -1;
    }
    for (int i = 0; i < list->count; i++) {
        if (matcher(&list->students[i], query)) {
            (*matches)[found++] = i;
        }
    }
    return found;
}

int search_by_name(const StudentList *list, const char *query, int **matches)
{
    return collect_matches(list, query, name_matches, matches);
}

int search_by_programme(const StudentList *list, const char *query, int **matches)
{
    return collect_matches(list, query, programme_matches, matches);
}
