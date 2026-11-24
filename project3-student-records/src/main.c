/*
 * main.c - Student Academic Records and Performance Analytics System.
 * Usage: ./student_records [data file]   (default: data/students.txt)
 */
#include <stdio.h>

#include "menu.h"
#include "storage.h"

#define DEFAULT_DATA_FILE "data/students.txt"

int main(int argc, char *argv[])
{
    App app;
    int error_line = 0;
    Status status;

    list_init(&app.list);
    app.data_path = argc > 1 ? argv[1] : DEFAULT_DATA_FILE;
    app.unsaved_changes = 0;

    status = storage_load(&app.list, app.data_path, &error_line);
    if (status == STATUS_OK) {
        printf("Loaded %d student(s) from %s.\n", app.list.count, app.data_path);
    } else if (status == STATUS_NOT_FOUND) {
        printf("No saved records at %s. Starting with an empty list.\n", app.data_path);
    } else {
        printf("Warning: could not load %s (line %d): %s\n", app.data_path, error_line,
               status_message(status));
        printf("Starting with an empty list. Saving will overwrite that file.\n");
    }

    menu_run(&app);
    list_free(&app.list);
    return 0;
}
