/*
 * menu.h - the interactive menu that drives the application.
 */
#ifndef MENU_H
#define MENU_H

#include "records.h"

typedef struct {
    StudentList list;
    const char *data_path;
    int unsaved_changes;
} App;

void menu_run(App *app);

#endif
