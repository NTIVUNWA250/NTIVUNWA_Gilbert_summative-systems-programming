/*
 * storage.h - saves and loads transactions as a text file.
 * Format, one transaction per line, fields separated by '|':
 *   T|id|product|quantity|unit price|date
 * The total is recalculated on load. Lines starting with '#' are comments.
 */
#ifndef STORAGE_H
#define STORAGE_H

#include "transactions.h"

Status storage_save(const TransactionList *list, const char *path);
Status storage_load(TransactionList *list, const char *path, int *error_line);

#endif
