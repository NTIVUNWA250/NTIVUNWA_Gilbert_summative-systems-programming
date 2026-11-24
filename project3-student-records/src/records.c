/*
 * records.c - dynamic storage of students and their courses.
 */
#include <stdlib.h>
#include <string.h>

#include "records.h"
#include "search.h"
#include "textutil.h"
#include "validation.h"

const char *status_message(Status status)
{
    switch (status) {
    case STATUS_OK:
        return "Success.";
    case STATUS_NO_MEMORY:
        return "Memory allocation failed.";
    case STATUS_DUPLICATE:
        return "A record with that identifier already exists.";
    case STATUS_NOT_FOUND:
        return "Record not found.";
    case STATUS_INVALID:
        return "Invalid data.";
    case STATUS_FILE_ERROR:
        return "File access error.";
    }
    return "Unknown error.";
}

void list_init(StudentList *list)
{
    list->students = NULL;
    list->count = 0;
}

void list_free(StudentList *list)
{
    for (int i = 0; i < list->count; i++) {
        free(list->students[i].courses);
    }
    free(list->students);
    list_init(list);
}

Student *list_get(StudentList *list, int id)
{
    int index = search_by_id(list, id);
    return index < 0 ? NULL : &list->students[index];
}

static Status validate_info(const char *first_name, const char *last_name,
                            const char *programme, int year)
{
    if (!valid_text_field(first_name, NAME_LEN) ||
        !valid_text_field(last_name, NAME_LEN) ||
        !valid_text_field(programme, PROGRAMME_LEN) || !valid_year(year)) {
        return STATUS_INVALID;
    }
    return STATUS_OK;
}

Status list_add(StudentList *list, int id, const char *first_name,
                const char *last_name, const char *programme, int year)
{
    Student *resized;
    Student *student;
    size_t new_size = (size_t)(list->count + 1) * sizeof(Student);

    if (!valid_student_id(id) ||
        validate_info(first_name, last_name, programme, year) != STATUS_OK) {
        return STATUS_INVALID;
    }
    if (search_by_id(list, id) >= 0) {
        return STATUS_DUPLICATE;
    }

    resized = list->students == NULL ? malloc(new_size)
                                     : realloc(list->students, new_size);
    if (resized == NULL) {
        return STATUS_NO_MEMORY;
    }
    list->students = resized;

    student = &list->students[list->count];
    memset(student, 0, sizeof *student);
    student->id = id;
    student->year = year;
    student->courses = NULL;
    text_copy(student->first_name, NAME_LEN, first_name);
    text_copy(student->last_name, NAME_LEN, last_name);
    text_copy(student->programme, PROGRAMME_LEN, programme);
    list->count++;
    return STATUS_OK;
}

Status list_remove(StudentList *list, int id)
{
    int index = search_by_id(list, id);
    Student *resized;

    if (index < 0) {
        return STATUS_NOT_FOUND;
    }
    free(list->students[index].courses);
    for (int i = index; i < list->count - 1; i++) {
        list->students[i] = list->students[i + 1];
    }
    list->count--;

    if (list->count == 0) {
        free(list->students);
        list->students = NULL;
        return STATUS_OK;
    }
    /* If shrinking fails the old, larger block is still valid, so keep it. */
    resized = realloc(list->students, (size_t)list->count * sizeof(Student));
    if (resized != NULL) {
        list->students = resized;
    }
    return STATUS_OK;
}

Status student_update_info(Student *student, const char *first_name,
                           const char *last_name, const char *programme, int year)
{
    Status status = validate_info(first_name, last_name, programme, year);

    if (status != STATUS_OK) {
        return status;
    }
    text_copy(student->first_name, NAME_LEN, first_name);
    text_copy(student->last_name, NAME_LEN, last_name);
    text_copy(student->programme, PROGRAMME_LEN, programme);
    student->year = year;
    return STATUS_OK;
}

Course *student_find_course(Student *student, const char *code)
{
    for (int i = 0; i < student->course_count; i++) {
        if (text_compare_ci(student->courses[i].code, code) == 0) {
            return &student->courses[i];
        }
    }
    return NULL;
}

Status student_add_course(Student *student, const char *code, const char *name,
                          double marks, double max_marks)
{
    Course *resized;
    Course *course;
    size_t new_size = (size_t)(student->course_count + 1) * sizeof(Course);

    if (!valid_text_field(code, CODE_LEN) ||
        !valid_text_field(name, COURSE_NAME_LEN) || !valid_marks(marks, max_marks)) {
        return STATUS_INVALID;
    }
    if (student_find_course(student, code) != NULL) {
        return STATUS_DUPLICATE;
    }

    resized = student->courses == NULL ? malloc(new_size)
                                       : realloc(student->courses, new_size);
    if (resized == NULL) {
        return STATUS_NO_MEMORY;
    }
    student->courses = resized;

    course = &student->courses[student->course_count];
    text_copy(course->code, CODE_LEN, code);
    text_copy(course->name, COURSE_NAME_LEN, name);
    course->marks = marks;
    course->max_marks = max_marks;
    student->course_count++;
    return STATUS_OK;
}

Status student_update_marks(Student *student, const char *code, double marks)
{
    Course *course = student_find_course(student, code);

    if (course == NULL) {
        return STATUS_NOT_FOUND;
    }
    if (!valid_marks(marks, course->max_marks)) {
        return STATUS_INVALID;
    }
    course->marks = marks;
    return STATUS_OK;
}

/* The average is total marks over total maximum marks, so courses with a
 * bigger maximum weigh more. Returns 0 when there is nothing to average. */
int student_average(const Student *student, double *average)
{
    double obtained = 0.0;
    double possible = 0.0;

    for (int i = 0; i < student->course_count; i++) {
        obtained += student->courses[i].marks;
        possible += student->courses[i].max_marks;
    }
    if (student->course_count == 0 || possible <= 0.0) {
        return 0;
    }
    *average = obtained / possible * 100.0;
    return 1;
}

int course_percentage(const Course *course, double *percentage)
{
    if (course->max_marks <= 0.0) {
        return 0;
    }
    *percentage = course->marks / course->max_marks * 100.0;
    return 1;
}

int course_passed(const Course *course)
{
    double percentage;
    return course_percentage(course, &percentage) && percentage >= PASS_PERCENT;
}
