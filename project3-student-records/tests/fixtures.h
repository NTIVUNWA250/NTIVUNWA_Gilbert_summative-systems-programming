/*
 * fixtures.h - a small known data set shared by the tests.
 */
#ifndef FIXTURES_H
#define FIXTURES_H

#include "records.h"

/* Averages: 1003 Alice = 85%, 1001 Brian = 40%, 1002 Chantal = 62.5%,
 * 1004 David has no courses. */
static inline void fixture_load(StudentList *list)
{
    list_init(list);
    list_add(list, 1003, "Alice", "Uwase", "Computer Science", 2);
    list_add(list, 1001, "Brian", "Mugisha", "Business", 1);
    list_add(list, 1002, "Chantal", "Ingabire", "Computer Science", 3);
    list_add(list, 1004, "David", "Habimana", "Engineering", 1);

    student_add_course(list_get(list, 1003), "CS101", "Programming", 80, 100);
    student_add_course(list_get(list, 1003), "MA101", "Calculus", 90, 100);
    student_add_course(list_get(list, 1001), "BU101", "Accounting", 40, 100);
    student_add_course(list_get(list, 1002), "CS101", "Programming", 45, 100);
    student_add_course(list_get(list, 1002), "CS201", "Data Structures", 80, 100);
}

#endif
