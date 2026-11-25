/*
 * processing.h - sales operations selected at runtime through function
 * pointers. Every operation has the same signature, so the menu can store
 * them in a table and call whichever one the user picks.
 */
#ifndef PROCESSING_H
#define PROCESSING_H

#include "transactions.h"

/* Returns the computed value. For operations that identify a transaction,
 * *index is set to its position; otherwise it is set to -1. */
typedef double (*SalesOperation)(const TransactionList *list, int *index);

typedef struct {
    const char *name;
    const char *symbol;
    SalesOperation run;
    int is_money;
} SalesOperationEntry;

double op_total_revenue(const TransactionList *list, int *index);
double op_average_value(const TransactionList *list, int *index);
double op_highest_value(const TransactionList *list, int *index);
double op_lowest_value(const TransactionList *list, int *index);
double op_total_units(const TransactionList *list, int *index);

const SalesOperationEntry *processing_operations(int *count);
double processing_execute(SalesOperation operation, const TransactionList *list, int *index);

#endif
