/*
 * search_sort.c - linear searches and an insertion sort driven by a
 * comparison callback.
 */
#include "search_sort.h"

int search_by_id(const TransactionList *list, int id)
{
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].id == id) {
            return i;
        }
    }
    return -1;
}

int search_by_product(const TransactionList *list, const char *query, int **matches)
{
    return filter_transactions(list, product_contains, query, matches);
}

int search_by_amount(const TransactionList *list, double amount, int **matches)
{
    return filter_transactions(list, has_amount, &amount, matches);
}

void sort_transactions(TransactionList *list, TransactionCompare compare, int ascending)
{
    for (int i = 1; i < list->count; i++) {
        Transaction current = list->items[i];
        int j = i - 1;

        while (j >= 0) {
            int order = compare(&list->items[j], &current);
            if (!ascending) {
                order = -order;
            }
            if (order <= 0) {
                break;
            }
            list->items[j + 1] = list->items[j];
            j--;
        }
        list->items[j + 1] = current;
    }
}
