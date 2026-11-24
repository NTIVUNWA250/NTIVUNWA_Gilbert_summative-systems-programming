#include <math.h>
#include <string.h>

#include "fixtures.h"
#include "report.h"
#include "test_util.h"

static int near(double a, double b)
{
    return fabs(a - b) < 0.001;
}

static const CourseStat *course_stat(const Report *report, const char *code)
{
    for (int i = 0; i < report->course_count; i++) {
        if (strcmp(report->courses[i].code, code) == 0) {
            return &report->courses[i];
        }
    }
    return NULL;
}

static void test_totals_and_extremes(void)
{
    StudentList list;
    Report report;

    fixture_load(&list);
    CHECK(report_build(&list, &report) == STATUS_OK);
    CHECK(report.total_students == 4);
    CHECK(report.total_courses == 5);
    CHECK(report.students_with_courses == 3);
    CHECK(report.students_without_courses == 1);
    CHECK(near(report.overall_average, (85.0 + 40.0 + 62.5) / 3.0));
    CHECK(report.best != NULL && report.best->id == 1003);
    CHECK(report.worst != NULL && report.worst->id == 1001);
    CHECK(report.passing == 2 && report.failing == 1);
    report_free(&report);
    list_free(&list);
}

static void test_course_and_programme_stats(void)
{
    StudentList list;
    Report report;
    const CourseStat *cs101;

    fixture_load(&list);
    report_build(&list, &report);
    CHECK(report.course_count == 4);
    cs101 = course_stat(&report, "CS101");
    CHECK(cs101 != NULL && cs101->enrolled == 2 && cs101->passed == 1);
    CHECK(cs101 != NULL && near(cs101->percentage_sum / cs101->enrolled, 62.5));

    CHECK(report.programme_count == 3);
    CHECK(strcmp(report.programmes[0].programme, "Computer Science") == 0);
    CHECK(report.programmes[0].students == 2);
    report_free(&report);
    list_free(&list);
}

static void test_empty_list_is_safe(void)
{
    StudentList list;
    Report report;

    list_init(&list);
    CHECK(report_build(&list, &report) == STATUS_OK);
    CHECK(report.total_students == 0 && report.students_with_courses == 0);
    CHECK(report.overall_average == 0.0 && report.best == NULL);
    report_free(&report);
}

int main(void)
{
    test_totals_and_extremes();
    test_course_and_programme_stats();
    test_empty_list_is_safe();
    return test_summary("test_report");
}
