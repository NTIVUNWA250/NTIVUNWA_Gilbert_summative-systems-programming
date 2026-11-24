/*
 * storage.c - text file persistence for students and their courses.
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "storage.h"
#include "validation.h"

#define LINE_SIZE 512
#define MAX_FIELDS 8

static Status write_records(const StudentList *list, FILE *file)
{
    fprintf(file, "# Student academic records\n");
    for (int i = 0; i < list->count; i++) {
        const Student *s = &list->students[i];

        fprintf(file, "S|%d|%s|%s|%s|%d|%d\n", s->id, s->first_name, s->last_name,
                s->programme, s->year, s->course_count);
        for (int c = 0; c < s->course_count; c++) {
            const Course *course = &s->courses[c];
            fprintf(file, "C|%s|%s|%.2f|%.2f\n", course->code, course->name,
                    course->marks, course->max_marks);
        }
    }
    return ferror(file) ? STATUS_FILE_ERROR : STATUS_OK;
}

/* Writes to a temporary file first, so a failed save never destroys the
 * previous data file. */
Status storage_save(const StudentList *list, const char *path)
{
    char temp_path[512];
    FILE *file;
    Status status;

    if (snprintf(temp_path, sizeof temp_path, "%s.tmp", path) >= (int)sizeof temp_path) {
        return STATUS_FILE_ERROR;
    }
    file = fopen(temp_path, "w");
    if (file == NULL) {
        return STATUS_FILE_ERROR;
    }
    status = write_records(list, file);
    if (fclose(file) != 0) {
        status = STATUS_FILE_ERROR;
    }
    if (status == STATUS_OK && rename(temp_path, path) != 0) {
        status = STATUS_FILE_ERROR;
    }
    if (status != STATUS_OK) {
        remove(temp_path);
    }
    return status;
}

static int split_fields(char *line, char **fields, int max_fields)
{
    int count = 0;
    char *start = line;

    for (char *p = line;; p++) {
        if (*p == '|' || *p == '\0') {
            int at_end = (*p == '\0');
            *p = '\0';
            if (count < max_fields) {
                fields[count] = start;
            }
            count++;
            if (at_end) {
                return count;
            }
            start = p + 1;
        }
    }
}

static Status parse_student(StudentList *list, char **fields, int *expected_courses)
{
    int id;
    int year;

    if (!parse_int(fields[1], &id) || !parse_int(fields[5], &year) ||
        !parse_int(fields[6], expected_courses) || *expected_courses < 0) {
        return STATUS_INVALID;
    }
    return list_add(list, id, fields[2], fields[3], fields[4], year);
}

static Status parse_course(Student *student, char **fields)
{
    double marks;
    double max_marks;

    if (!parse_double(fields[3], &marks) || !parse_double(fields[4], &max_marks)) {
        return STATUS_INVALID;
    }
    return student_add_course(student, fields[1], fields[2], marks, max_marks);
}

static Status parse_line(StudentList *list, char *line, int *expected_courses)
{
    char *fields[MAX_FIELDS];
    int count = split_fields(line, fields, MAX_FIELDS);
    Student *current = list->count > 0 ? &list->students[list->count - 1] : NULL;

    if (strcmp(fields[0], "S") == 0 && count == 7) {
        if (current != NULL && current->course_count != *expected_courses) {
            return STATUS_INVALID;
        }
        return parse_student(list, fields, expected_courses);
    }
    if (strcmp(fields[0], "C") == 0 && count == 5 && current != NULL &&
        current->course_count < *expected_courses) {
        return parse_course(current, fields);
    }
    return STATUS_INVALID;
}

/* Loads into a fresh list and only replaces *list when the whole file is
 * valid, so a corrupt file never leaves half-loaded data behind. */
Status storage_load(StudentList *list, const char *path, int *error_line)
{
    StudentList loaded;
    char line[LINE_SIZE];
    int line_number = 0;
    int expected_courses = 0;
    Status status = STATUS_OK;
    FILE *file = fopen(path, "r");

    if (error_line != NULL) {
        *error_line = 0;
    }
    if (file == NULL) {
        return errno == ENOENT ? STATUS_NOT_FOUND : STATUS_FILE_ERROR;
    }

    list_init(&loaded);
    while (status == STATUS_OK && fgets(line, sizeof line, file) != NULL) {
        line_number++;
        if (strchr(line, '\n') == NULL && !feof(file)) {
            status = STATUS_INVALID;
            break;
        }
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        status = parse_line(&loaded, line, &expected_courses);
    }
    if (status == STATUS_OK && ferror(file)) {
        status = STATUS_FILE_ERROR;
    }
    if (status == STATUS_OK && loaded.count > 0 &&
        loaded.students[loaded.count - 1].course_count != expected_courses) {
        status = STATUS_INVALID;
    }
    fclose(file);

    if (status != STATUS_OK) {
        if (error_line != NULL) {
            *error_line = line_number;
        }
        list_free(&loaded);
        return status;
    }
    list_free(list);
    *list = loaded;
    return STATUS_OK;
}
