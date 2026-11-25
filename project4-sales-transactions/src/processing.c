/*
 * processing.c - the sales operations and the table that names them.
 */
#include "processing.h"

double op_total_revenue(const TransactionList *list, int *index)
{
    double revenue = 0.0;

    *index = -1;
    for (int i = 0; i < list->count; i++) {
        revenue += list->items[i].total;
    }
    return revenue;
}

/* Returns 0 for an empty list instead of dividing by zero. */
double op_average_value(const TransactionList *list, int *index)
{
    if (list->count == 0) {
        *index = -1;
        return 0.0;
    }
    return op_total_revenue(list, index) / list->count;
}

double op_highest_value(const TransactionList *list, int *index)
{
    *index = -1;
    for (int i = 0; i < list->count; i++) {
        if (*index < 0 || list->items[i].total > list->items[*index].total) {
            *index = i;
        }
    }
    return *index < 0 ? 0.0 : list->items[*index].total;
}

double op_lowest_value(const TransactionList *list, int *index)
{
    *index = -1;
    for (int i = 0; i < list->count; i++) {
        if (*index < 0 || list->items[i].total < list->items[*index].total) {
            *index = i;
        }
    }
    return *index < 0 ? 0.0 : list->items[*index].total;
}

double op_total_units(const TransactionList *list, int *index)
{
    double units = 0.0;

    *index = -1;
    for (int i = 0; i < list->count; i++) {
        units += list->items[i].quantity;
    }
    return units;
}

static const SalesOperationEntry operations[] = {
    {"Calculate total revenue", "op_total_revenue", op_total_revenue, 1},
    {"Calculate average transaction value", "op_average_value", op_average_value, 1},
    {"Identify highest-value transaction", "op_highest_value", op_highest_value, 1},
    {"Identify lowest-value transaction", "op_lowest_value", op_lowest_value, 1},
    {"Calculate total units sold", "op_total_units", op_total_units, 0},
};

const SalesOperationEntry *processing_operations(int *count)
{
    *count = (int)(sizeof operations / sizeof operations[0]);
    return operations;
}

double processing_execute(SalesOperation operation, const TransactionList *list, int *index)
{
    return operation(list, index);
}
