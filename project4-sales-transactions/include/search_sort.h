/*
 * search_sort.h - manual searching and manual sorting (no qsort).
 */
#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

#include "callbacks.h"
#include "transactions.h"

int search_by_id(const TransactionList *list, int id);
int search_by_product(const TransactionList *list, const char *query, int **matches);
int search_by_amount(const TransactionList *list, double amount, int **matches);

void sort_transactions(TransactionList *list, TransactionCompare compare, int ascending);

#endif
