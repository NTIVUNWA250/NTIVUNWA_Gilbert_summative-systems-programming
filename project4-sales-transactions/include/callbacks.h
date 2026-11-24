/*
 * callbacks.h - generic functions that receive a callback and call it for
 * each transaction. The callbacks decide which records match or how two
 * records compare; the generic functions do the looping.
 */
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "transactions.h"

typedef int (*TransactionPredicate)(const Transaction *t, const void *context);
typedef int (*TransactionCompare)(const Transaction *a, const Transaction *b);

/* Fills *matches with a newly allocated array of indexes (caller frees).
 * Returns the number of matches, or -1 if memory allocation failed. */
int filter_transactions(const TransactionList *list, TransactionPredicate predicate,
                        const void *context, int **matches);
int count_transactions(const TransactionList *list, TransactionPredicate predicate,
                       const void *context);
double sum_matching_revenue(const TransactionList *list, TransactionPredicate predicate,
                            const void *context);

int is_above_amount(const Transaction *t, const void *context);
int is_below_amount(const Transaction *t, const void *context);
int is_product(const Transaction *t, const void *context);
int product_contains(const Transaction *t, const void *context);
int has_amount(const Transaction *t, const void *context);
int quantity_at_least(const Transaction *t, const void *context);

int compare_by_id(const Transaction *a, const Transaction *b);
int compare_by_product(const Transaction *a, const Transaction *b);
int compare_by_amount(const Transaction *a, const Transaction *b);

#endif
