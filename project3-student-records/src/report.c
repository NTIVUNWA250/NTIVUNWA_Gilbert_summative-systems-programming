/*
 * report.c - builds and prints the academic performance report.
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "report.h"
#include "textutil.h"

static CourseStat *find_or_add_course(Report *report, const Course *course)
{
    CourseStat *resized;
    CourseStat *stat;

    for (int i = 0; i < report->course_count; i++) {
        if (text_compare_ci(report->courses[i].code, course->code) == 0) {
            return &report->courses[i];
        }
    }
    resized = realloc(report->courses, (size_t)(report->course_count + 1) * sizeof(CourseStat));
    if (resized == NULL) {
        return NULL;
    }
    report->courses = resized;
    stat = &report->courses[report->course_count++];
    memset(stat, 0, sizeof *stat);
    text_copy(stat->code, CODE_LEN, course->code);
    text_copy(stat->name, COURSE_NAME_LEN, course->name);
    return stat;
}

static ProgrammeStat *find_or_add_programme(Report *report, const char *programme)
{
    ProgrammeStat *resized;
    ProgrammeStat *stat;

    for (int i = 0; i < report->programme_count; i++) {
        if (text_compare_ci(report->programmes[i].programme, programme) == 0) {
            return &report->programmes[i];
        }
    }
    resized = realloc(report->programmes,
                      (size_t)(report->programme_count + 1) * sizeof(ProgrammeStat));
    if (resized == NULL) {
        return NULL;
    }
    report->programmes = resized;
    stat = &report->programmes[report->programme_count++];
    stat->students = 0;
    text_copy(stat->programme, PROGRAMME_LEN, programme);
    return stat;
}

static void record_average(Report *report, const Student *student, double average)
{
    report->students_with_courses++;
    report->overall_average += average;
    if (average >= PASS_PERCENT) {
        report->passing++;
    } else {
        report->failing++;
    }
    if (report->best == NULL || average > report->best_average) {
        report->best = student;
        report->best_average = average;
    }
    if (report->worst == NULL || average < report->worst_average) {
        report->worst = student;
        report->worst_average = average;
    }
}

static Status record_courses(Report *report, const Student *student)
{
    for (int c = 0; c < student->course_count; c++) {
        const Course *course = &student->courses[c];
        CourseStat *stat = find_or_add_course(report, course);
        double percentage;

        if (stat == NULL) {
            return STATUS_NO_MEMORY;
        }
        if (course_percentage(course, &percentage)) {
            stat->percentage_sum += percentage;
            stat->enrolled++;
            stat->passed += course_passed(course);
        }
    }
    return STATUS_OK;
}

Status report_build(const StudentList *list, Report *report)
{
    memset(report, 0, sizeof *report);
    report->total_students = list->count;

    for (int i = 0; i < list->count; i++) {
        const Student *student = &list->students[i];
        ProgrammeStat *programme = find_or_add_programme(report, student->programme);
        double average;

        if (programme == NULL || record_courses(report, student) != STATUS_OK) {
            report_free(report);
            return STATUS_NO_MEMORY;
        }
        programme->students++;
        report->total_courses += student->course_count;

        if (student_average(student, &average)) {
            record_average(report, student, average);
        } else {
            report->students_without_courses++;
        }
    }
    /* overall_average held a running sum; divide only when it is safe. */
    if (report->students_with_courses > 0) {
        report->overall_average /= report->students_with_courses;
    }
    return STATUS_OK;
}

void report_print(const Report *report, FILE *out)
{
    char stamp[32];
    time_t now = time(NULL);

    strftime(stamp, sizeof stamp, "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(out, "\n========== ACADEMIC PERFORMANCE REPORT ==========\n");
    fprintf(out, "Generated: %s\n\n", stamp);
    fprintf(out, "Total registered students      : %d\n", report->total_students);
    fprintf(out, "Total course registrations     : %d\n", report->total_courses);
    fprintf(out, "Students with no courses       : %d\n", report->students_without_courses);

    if (report->students_with_courses == 0) {
        fprintf(out, "\nNo course marks recorded yet, so averages cannot be calculated.\n");
    } else {
        fprintf(out, "Overall average performance    : %.2f%%\n", report->overall_average);
        fprintf(out, "Highest average                : %d %s %s (%.2f%%)\n",
                report->best->id, report->best->first_name, report->best->last_name,
                report->best_average);
        fprintf(out, "Lowest average                 : %d %s %s (%.2f%%)\n",
                report->worst->id, report->worst->first_name, report->worst->last_name,
                report->worst_average);
        fprintf(out, "Students passing (avg >= %.0f%%)  : %d\n", PASS_PERCENT, report->passing);
        fprintf(out, "Students failing (avg <  %.0f%%)  : %d\n", PASS_PERCENT, report->failing);
    }

    if (report->course_count > 0) {
        fprintf(out, "\nAverage performance per course:\n");
        fprintf(out, "%-10s %-28s %8s %9s %7s\n", "Code", "Course", "Enrolled", "Average", "Passed");
        for (int i = 0; i < report->course_count; i++) {
            const CourseStat *stat = &report->courses[i];
            fprintf(out, "%-10s %-28.28s %8d %8.2f%% %7d\n", stat->code, stat->name,
                    stat->enrolled, stat->percentage_sum / stat->enrolled, stat->passed);
        }
    }

    if (report->programme_count > 0) {
        fprintf(out, "\nStudents per programme:\n");
        for (int i = 0; i < report->programme_count; i++) {
            fprintf(out, "  %-32s %d\n", report->programmes[i].programme,
                    report->programmes[i].students);
        }
    }
    fprintf(out, "=================================================\n");
}

void report_free(Report *report)
{
    free(report->courses);
    free(report->programmes);
    report->courses = NULL;
    report->programmes = NULL;
    report->course_count = 0;
    report->programme_count = 0;
}
