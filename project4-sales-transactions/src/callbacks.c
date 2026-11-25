/*
 * callbacks.c - functions that take callbacks, and the callbacks themselves.
 */
#include <math.h>
#include <stdlib.h>

#include "callbacks.h"
#include "textutil.h"

#define AMOUNT_TOLERANCE 0.005

int filter_transactions(const TransactionList *list, TransactionPredicate predicate,
                        const void *context, int **matches)
{
    int found = 0;

    *matches = NULL;
    if (list->count == 0) {
        return 0;
    }
    *matches = malloc((size_t)list->count * sizeof(int));
    if (*matches == NULL) {
        return -1;
    }
    for (int i = 0; i < list->count; i++) {
        if (predicate(&list->items[i], context)) {
            (*matches)[found++] = i;
        }
    }
    return found;
}

int count_transactions(const TransactionList *list, TransactionPredicate predicate,
                       const void *context)
{
    int count = 0;

    for (int i = 0; i < list->count; i++) {
        count += predicate(&list->items[i], context) ? 1 : 0;
    }
    return count;
}

double sum_matching_revenue(const TransactionList *list, TransactionPredicate predicate,
                            const void *context)
{
    double sum = 0.0;

    for (int i = 0; i < list->count; i++) {
        if (predicate(&list->items[i], context)) {
            sum += list->items[i].total;
        }
    }
    return sum;
}

int is_above_amount(const Transaction *t, const void *context)
{
    return t->total > *(const double *)context;
}

int is_below_amount(const Transaction *t, const void *context)
{
    return t->total < *(const double *)context;
}

int is_product(const Transaction *t, const void *context)
{
    return text_compare_ci(t->product, (const char *)context) == 0;
}

int product_contains(const Transaction *t, const void *context)
{
    return text_contains_ci(t->product, (const char *)context);
}

int has_amount(const Transaction *t, const void *context)
{
    return fabs(t->total - *(const double *)context) < AMOUNT_TOLERANCE;
}

int quantity_at_least(const Transaction *t, const void *context)
{
    return t->quantity >= *(const int *)context;
}

int compare_by_id(const Transaction *a, const Transaction *b)
{
    return (a->id > b->id) - (a->id < b->id);
}

int compare_by_product(const Transaction *a, const Transaction *b)
{
    int result = text_compare_ci(a->product, b->product);
    return result != 0 ? result : compare_by_id(a, b);
}

int compare_by_amount(const Transaction *a, const Transaction *b)
{
    return (a->total > b->total) - (a->total < b->total);
}
