/*
 * storage.c - text file persistence for transactions.
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "storage.h"
#include "validation.h"

#define LINE_SIZE 256
#define FIELD_COUNT 6

/* Writes to a temporary file first, so a failed save never destroys the
 * previous data file. */
Status storage_save(const TransactionList *list, const char *path)
{
    char temp_path[512];
    FILE *file;
    Status status = STATUS_OK;

    if (snprintf(temp_path, sizeof temp_path, "%s.tmp", path) >= (int)sizeof temp_path) {
        return STATUS_FILE_ERROR;
    }
    file = fopen(temp_path, "w");
    if (file == NULL) {
        return STATUS_FILE_ERROR;
    }
    fprintf(file, "# Sales transactions\n");
    for (int i = 0; i < list->count; i++) {
        const Transaction *t = &list->items[i];
        fprintf(file, "T|%d|%s|%d|%.2f|%s\n", t->id, t->product, t->quantity,
                t->unit_price, t->date);
    }
    if (ferror(file)) {
        status = STATUS_FILE_ERROR;
    }
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

static Status parse_line(TransactionList *list, char *line)
{
    char *fields[FIELD_COUNT];
    int id, quantity;
    double price;

    if (split_fields(line, fields, FIELD_COUNT) != FIELD_COUNT ||
        strcmp(fields[0], "T") != 0 || !parse_int(fields[1], &id) ||
        !parse_int(fields[3], &quantity) || !parse_double(fields[4], &price)) {
        return STATUS_INVALID;
    }
    return list_add(list, id, fields[2], quantity, price, fields[5]);
}

/* Loads into a fresh list and replaces *list only when the whole file is
 * valid, so a corrupt file never leaves half-loaded data. */
Status storage_load(TransactionList *list, const char *path, int *error_line)
{
    TransactionList loaded;
    char line[LINE_SIZE];
    int line_number = 0;
    Status status;
    FILE *file;

    if (error_line != NULL) {
        *error_line = 0;
    }
    file = fopen(path, "r");
    if (file == NULL) {
        return errno == ENOENT ? STATUS_NOT_FOUND : STATUS_FILE_ERROR;
    }
    status = list_init(&loaded);
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
        status = parse_line(&loaded, line);
    }
    if (status == STATUS_OK && ferror(file)) {
        status = STATUS_FILE_ERROR;
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
