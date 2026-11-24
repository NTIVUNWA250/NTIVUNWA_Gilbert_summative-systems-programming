/*
 * display.c - formatted output of student records.
 */
#include <stdio.h>

#include "display.h"

#define TABLE_LINE "------------------------------------------------------------------------------\n"

static void print_average(const Student *student)
{
    double average;

    if (student_average(student, &average)) {
        printf("%7.2f%%", average);
    } else {
        printf("%8s", "N/A");
    }
}

void display_table_header(void)
{
    printf(TABLE_LINE);
    printf("%-9s %-26s %-24s %4s %7s %8s\n", "ID", "Name", "Programme", "Year",
           "Courses", "Average");
    printf(TABLE_LINE);
}

void display_student_row(const Student *student)
{
    char name[NAME_LEN * 2];

    snprintf(name, sizeof name, "%s %s", student->first_name, student->last_name);
    printf("%-9d %-26.26s %-24.24s %4d %7d ", student->id, name, student->programme,
           student->year, student->course_count);
    print_average(student);
    printf("\n");
}

void display_all(const StudentList *list)
{
    if (list->count == 0) {
        printf("No students registered yet.\n");
        return;
    }
    display_table_header();
    for (int i = 0; i < list->count; i++) {
        display_student_row(&list->students[i]);
    }
    printf(TABLE_LINE);
    printf("%d student(s)\n", list->count);
}

void display_matches(const StudentList *list, const int *matches, int count)
{
    if (count <= 0) {
        printf("No matching students found.\n");
        return;
    }
    display_table_header();
    for (int i = 0; i < count; i++) {
        display_student_row(&list->students[matches[i]]);
    }
    printf(TABLE_LINE);
    printf("%d match(es)\n", count);
}

void display_details(const Student *student)
{
    printf("\nStudent ID   : %d\n", student->id);
    printf("Name         : %s %s\n", student->first_name, student->last_name);
    printf("Programme    : %s\n", student->programme);
    printf("Year         : %d\n", student->year);
    printf("Courses      : %d\n", student->course_count);
    printf("Average      : ");
    print_average(student);
    printf("\n");
}

void display_performance(const Student *student)
{
    double average;

    display_details(student);
    if (student->course_count == 0) {
        printf("\nThis student has no registered courses yet.\n");
        return;
    }
    printf("\n%-10s %-30s %8s %8s %8s %6s\n", "Code", "Course", "Marks", "Max", "Percent",
           "Result");
    printf(TABLE_LINE);
    for (int i = 0; i < student->course_count; i++) {
        const Course *course = &student->courses[i];
        double percentage = 0.0;

        course_percentage(course, &percentage);
        printf("%-10s %-30.30s %8.2f %8.2f %7.2f%% %6s\n", course->code, course->name,
               course->marks, course->max_marks, percentage,
               course_passed(course) ? "PASS" : "FAIL");
    }
    printf(TABLE_LINE);
    if (student_average(student, &average)) {
        printf("Overall average: %.2f%% - %s\n", average,
               average >= PASS_PERCENT ? "PASSING" : "FAILING");
    }
}
