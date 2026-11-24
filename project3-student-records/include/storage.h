/*
 * storage.h - saves and loads the student list as a text file.
 *
 * File format, one record per line, fields separated by '|':
 *   S|id|first name|last name|programme|year|number of courses
 *   C|course code|course name|marks|max marks
 * Each S line is followed by exactly its number of C lines. Lines starting
 * with '#' are comments.
 */
#ifndef STORAGE_H
#define STORAGE_H

#include "records.h"

Status storage_save(const StudentList *list, const char *path);
Status storage_load(StudentList *list, const char *path, int *error_line);

#endif
