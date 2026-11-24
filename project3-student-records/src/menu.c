/*
 * menu.c - one handler per menu option. Handlers return 0 when input has
 * ended (EOF) so the program can stop cleanly.
 */
#include <stdio.h>
#include <stdlib.h>

#include "display.h"
#include "input.h"
#include "menu.h"
#include "report.h"
#include "search.h"
#include "sort.h"
#include "storage.h"
#include "validation.h"

#define DEFAULT_REPORT_FILE "academic_report.txt"

static void report_status(Status status, const char *success)
{
    if (status == STATUS_OK) {
        printf("%s\n", success);
    } else {
        printf("Error: %s\n", status_message(status));
    }
}

/* Asks for an ID and returns the student, or NULL after printing an error. */
static int ask_existing_student(App *app, Student **student)
{
    int id;

    *student = NULL;
    if (app->list.count == 0) {
        printf("No students registered yet.\n");
        return 1;
    }
    if (!read_int("Student ID: ", MIN_STUDENT_ID, MAX_STUDENT_ID, &id)) {
        return 0;
    }
    *student = list_get(&app->list, id);
    if (*student == NULL) {
        printf("Error: no student with ID %d.\n", id);
    }
    return 1;
}

static int register_student(App *app)
{
    char first[NAME_LEN], last[NAME_LEN], programme[PROGRAMME_LEN];
    int id, year;

    if (!read_int("Student ID: ", MIN_STUDENT_ID, MAX_STUDENT_ID, &id)) {
        return 0;
    }
    if (list_get(&app->list, id) != NULL) {
        printf("Error: student ID %d already exists.\n", id);
        return 1;
    }
    if (!read_text("First name: ", first, sizeof first) ||
        !read_text("Last name: ", last, sizeof last) ||
        !read_text("Programme of study: ", programme, sizeof programme) ||
        !read_int("Year of study (1-6): ", MIN_YEAR, MAX_YEAR, &year)) {
        return 0;
    }
    Status status = list_add(&app->list, id, first, last, programme, year);
    if (status == STATUS_OK) {
        app->unsaved_changes = 1;
    }
    report_status(status, "Student registered.");
    return 1;
}

static int view_student(App *app)
{
    Student *student;

    if (!ask_existing_student(app, &student)) {
        return 0;
    }
    if (student != NULL) {
        display_details(student);
    }
    return 1;
}

static int update_student(App *app)
{
    Student *student;

    if (!ask_existing_student(app, &student)) {
        return 0;
    }
    if (student == NULL) {
        return 1;
    }
    Student edited = *student;
    printf("Press Enter to keep the current value.\n");
    printf("First name [%s]: ", edited.first_name);
    if (!read_optional_text("", edited.first_name, NAME_LEN)) {
        return 0;
    }
    printf("Last name [%s]: ", edited.last_name);
    if (!read_optional_text("", edited.last_name, NAME_LEN)) {
        return 0;
    }
    printf("Programme [%s]: ", edited.programme);
    if (!read_optional_text("", edited.programme, PROGRAMME_LEN)) {
        return 0;
    }
    printf("Year [%d]: ", edited.year);
    if (!read_optional_int("", MIN_YEAR, MAX_YEAR, &edited.year)) {
        return 0;
    }
    Status status = student_update_info(student, edited.first_name, edited.last_name,
                                        edited.programme, edited.year);
    if (status == STATUS_OK) {
        app->unsaved_changes = 1;
    }
    report_status(status, "Student updated.");
    return 1;
}

static int delete_student(App *app)
{
    Student *student;
    int confirmed;

    if (!ask_existing_student(app, &student)) {
        return 0;
    }
    if (student == NULL) {
        return 1;
    }
    int id = student->id;
    printf("Delete %d %s %s and all their courses? ", id, student->first_name,
           student->last_name);
    if (!read_yes_no("(y/n): ", &confirmed)) {
        return 0;
    }
    if (!confirmed) {
        printf("Delete cancelled.\n");
        return 1;
    }
    Status status = list_remove(&app->list, id);
    if (status == STATUS_OK) {
        app->unsaved_changes = 1;
    }
    report_status(status, "Student deleted.");
    return 1;
}

static int add_course(App *app)
{
    Student *student;
    char code[CODE_LEN], name[COURSE_NAME_LEN];
    double max_marks, marks;

    if (!ask_existing_student(app, &student)) {
        return 0;
    }
    if (student == NULL) {
        return 1;
    }
    if (!read_text("Course code: ", code, sizeof code)) {
        return 0;
    }
    if (student_find_course(student, code) != NULL) {
        printf("Error: %s is already registered for this student.\n", code);
        return 1;
    }
    if (!read_text("Course name: ", name, sizeof name) ||
        !read_double("Maximum marks: ", 1, MAX_ALLOWED_MARKS, &max_marks) ||
        !read_double("Marks obtained: ", 0, max_marks, &marks)) {
        return 0;
    }
    Status status = student_add_course(student, code, name, marks, max_marks);
    if (status == STATUS_OK) {
        app->unsaved_changes = 1;
    }
    report_status(status, "Course added.");
    return 1;
}

static int update_marks(App *app)
{
    Student *student;
    char code[CODE_LEN];
    double marks;

    if (!ask_existing_student(app, &student)) {
        return 0;
    }
    if (student == NULL) {
        return 1;
    }
    if (student->course_count == 0) {
        printf("Error: this student has no courses.\n");
        return 1;
    }
    if (!read_text("Course code: ", code, sizeof code)) {
        return 0;
    }
    Course *course = student_find_course(student, code);
    if (course == NULL) {
        printf("Error: course %s not found for this student.\n", code);
        return 1;
    }
    printf("Current marks: %.2f / %.2f\n", course->marks, course->max_marks);
    if (!read_double("New marks: ", 0, course->max_marks, &marks)) {
        return 0;
    }
    Status status = student_update_marks(student, code, marks);
    if (status == STATUS_OK) {
        app->unsaved_changes = 1;
    }
    report_status(status, "Marks updated.");
    return 1;
}

static int show_performance(App *app)
{
    Student *student;

    if (!ask_existing_student(app, &student)) {
        return 0;
    }
    if (student != NULL) {
        display_performance(student);
    }
    return 1;
}

static int search_students(App *app)
{
    char query[PROGRAMME_LEN];
    int choice, id, found;
    int *matches = NULL;

    printf("Search by: 1. Student ID  2. First or last name  3. Programme\n");
    if (!read_int("Choice: ", 1, 3, &choice)) {
        return 0;
    }
    if (choice == 1) {
        if (!read_int("Student ID: ", MIN_STUDENT_ID, MAX_STUDENT_ID, &id)) {
            return 0;
        }
        int index = search_by_id(&app->list, id);
        display_matches(&app->list, &index, index >= 0 ? 1 : 0);
        return 1;
    }
    if (!read_text("Search text: ", query, sizeof query)) {
        return 0;
    }
    found = choice == 2 ? search_by_name(&app->list, query, &matches)
                        : search_by_programme(&app->list, query, &matches);
    if (found < 0) {
        printf("Error: %s\n", status_message(STATUS_NO_MEMORY));
    } else {
        display_matches(&app->list, matches, found);
    }
    free(matches);
    return 1;
}

static int sort_list(App *app)
{
    int key, order;

    if (app->list.count == 0) {
        printf("No students registered yet.\n");
        return 1;
    }
    printf("Sort by: 1. Student ID  2. Name  3. Overall average  4. Programme\n");
    if (!read_int("Choice: ", 1, 4, &key) ||
        !read_int("Order: 1. Ascending  2. Descending: ", 1, 2, &order)) {
        return 0;
    }
    sort_students(&app->list, (SortKey)(key - 1), order == 1);
    app->unsaved_changes = 1;
    display_all(&app->list);
    return 1;
}

static int show_report(App *app)
{
    Report report;
    char path[128];
    int save;

    if (report_build(&app->list, &report) != STATUS_OK) {
        printf("Error: %s\n", status_message(STATUS_NO_MEMORY));
        return 1;
    }
    report_print(&report, stdout);
    if (!read_yes_no("Save this report to a file? (y/n): ", &save)) {
        report_free(&report);
        return 0;
    }
    if (save) {
        if (!read_line("File name [" DEFAULT_REPORT_FILE "]: ", path, sizeof path)) {
            report_free(&report);
            return 0;
        }
        FILE *file = fopen(path[0] != '\0' ? path : DEFAULT_REPORT_FILE, "w");
        if (file == NULL) {
            printf("Error: could not create the report file.\n");
        } else {
            report_print(&report, file);
            printf(fclose(file) == 0 ? "Report saved.\n" : "Error: could not write the report.\n");
        }
    }
    report_free(&report);
    return 1;
}

static void save_records(App *app)
{
    Status status = storage_save(&app->list, app->data_path);

    if (status == STATUS_OK) {
        app->unsaved_changes = 0;
        printf("Saved %d student(s) to %s.\n", app->list.count, app->data_path);
    } else {
        printf("Error: could not save to %s.\n", app->data_path);
    }
}

static int confirm_exit(App *app)
{
    int save;

    if (!app->unsaved_changes) {
        return 1;
    }
    if (!read_yes_no("You have unsaved changes. Save before exiting? (y/n): ", &save)) {
        return 1;
    }
    if (save) {
        save_records(app);
    }
    return 1;
}

static void print_menu(void)
{
    printf("\n===== STUDENT ACADEMIC RECORDS SYSTEM =====\n");
    printf(" 1. Register new student\n");
    printf(" 2. Display all students\n");
    printf(" 3. View student details\n");
    printf(" 4. Update student information\n");
    printf(" 5. Delete student\n");
    printf(" 6. Add course to student\n");
    printf(" 7. Update course marks\n");
    printf(" 8. Display student performance\n");
    printf(" 9. Search students\n");
    printf("10. Sort students\n");
    printf("11. Academic performance report\n");
    printf("12. Save records\n");
    printf(" 0. Exit\n");
}

void menu_run(App *app)
{
    int choice;
    int running = 1;

    while (running) {
        print_menu();
        if (!read_int("Select an option: ", 0, 12, &choice)) {
            printf("\nInput ended.\n");
            break;
        }
        switch (choice) {
        case 1: running = register_student(app); break;
        case 2: display_all(&app->list); break;
        case 3: running = view_student(app); break;
        case 4: running = update_student(app); break;
        case 5: running = delete_student(app); break;
        case 6: running = add_course(app); break;
        case 7: running = update_marks(app); break;
        case 8: running = show_performance(app); break;
        case 9: running = search_students(app); break;
        case 10: running = sort_list(app); break;
        case 11: running = show_report(app); break;
        case 12: save_records(app); break;
        case 0:
            confirm_exit(app);
            running = 0;
            break;
        }
    }
    printf("Goodbye.\n");
}
