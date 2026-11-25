/*
 * report.h - academic performance statistics for the whole student list.
 * A student passes when their overall average is at least PASS_PERCENT.
 */
#ifndef REPORT_H
#define REPORT_H

#include <stdio.h>

#include "records.h"

typedef struct {
    char code[CODE_LEN];
    char name[COURSE_NAME_LEN];
    double percentage_sum;
    int enrolled;
    int passed;
} CourseStat;

typedef struct {
    char programme[PROGRAMME_LEN];
    int students;
} ProgrammeStat;

typedef struct {
    int total_students;
    int total_courses;
    int students_with_courses;
    int students_without_courses;
    int passing;
    int failing;
    double overall_average;
    const Student *best;
    double best_average;
    const Student *worst;
    double worst_average;
    CourseStat *courses;
    int course_count;
    ProgrammeStat *programmes;
    int programme_count;
} Report;

Status report_build(const StudentList *list, Report *report);
void report_print(const Report *report, FILE *out);
void report_free(Report *report);

#endif
