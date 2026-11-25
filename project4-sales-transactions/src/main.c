/*
 * main.c - Sales Transaction Processing and Analysis System.
 * Usage: ./sales_system [data file]   (default: data/transactions.txt)
 */
#include <stdio.h>

#include "menu.h"
#include "storage.h"

#define DEFAULT_DATA_FILE "data/transactions.txt"

int main(int argc, char *argv[])
{
    App app;
    int error_line = 0;
    Status status;

    if (list_init(&app.list) != STATUS_OK) {
        fprintf(stderr, "Fatal: %s\n", status_message(STATUS_NO_MEMORY));
        return 1;
    }
    app.data_path = argc > 1 ? argv[1] : DEFAULT_DATA_FILE;
    app.unsaved_changes = 0;

    status = storage_load(&app.list, app.data_path, &error_line);
    if (status == STATUS_OK) {
        printf("Loaded %d transaction(s) from %s.\n", app.list.count, app.data_path);
    } else if (status == STATUS_NOT_FOUND) {
        printf("No saved transactions at %s. Starting empty.\n", app.data_path);
    } else {
        printf("Warning: could not load %s (line %d): %s\n", app.data_path, error_line,
               status_message(status));
        printf("Starting empty. Saving will overwrite that file.\n");
    }

    menu_run(&app);
    list_free(&app.list);
    return 0;
}
