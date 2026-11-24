/*
 * display.h - printing students, courses and search results.
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include "records.h"

void display_table_header(void);
void display_student_row(const Student *student);
void display_all(const StudentList *list);
void display_matches(const StudentList *list, const int *matches, int count);
void display_details(const Student *student);
void display_performance(const Student *student);

#endif
