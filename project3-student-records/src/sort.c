/*
 * sort.c - insertion sort with a comparison chosen by the sort key.
 * Insertion sort is stable, so students that compare equal keep their order.
 */
#include "sort.h"
#include "textutil.h"

typedef int (*StudentCompare)(const Student *a, const Student *b);

static int compare_id(const Student *a, const Student *b)
{
    return (a->id > b->id) - (a->id < b->id);
}

static int compare_name(const Student *a, const Student *b)
{
    int result = text_compare_ci(a->first_name, b->first_name);

    if (result == 0) {
        result = text_compare_ci(a->last_name, b->last_name);
    }
    return result != 0 ? result : compare_id(a, b);
}

/* Students without courses get -1 so they sort below every real average. */
static double average_or_lowest(const Student *student)
{
    double average;
    return student_average(student, &average) ? average : -1.0;
}

static int compare_average(const Student *a, const Student *b)
{
    double x = average_or_lowest(a);
    double y = average_or_lowest(b);
    return (x > y) - (x < y);
}

static int compare_programme(const Student *a, const Student *b)
{
    int result = text_compare_ci(a->programme, b->programme);
    return result != 0 ? result : compare_name(a, b);
}

static StudentCompare compare_for(SortKey key)
{
    switch (key) {
    case SORT_BY_NAME:
        return compare_name;
    case SORT_BY_AVERAGE:
        return compare_average;
    case SORT_BY_PROGRAMME:
        return compare_programme;
    case SORT_BY_ID:
    default:
        return compare_id;
    }
}

void sort_students(StudentList *list, SortKey key, int ascending)
{
    StudentCompare compare = compare_for(key);

    for (int i = 1; i < list->count; i++) {
        Student current = list->students[i];
        int j = i - 1;

        while (j >= 0) {
            int order = compare(&list->students[j], &current);
            if (!ascending) {
                order = -order;
            }
            if (order <= 0) {
                break;
            }
            list->students[j + 1] = list->students[j];
            j--;
        }
        list->students[j + 1] = current;
    }
}
