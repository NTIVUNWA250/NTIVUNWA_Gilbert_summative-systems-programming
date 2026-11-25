/*
 * display.c - formatted transaction tables.
 */
#include <stdio.h>

#include "display.h"

#define TABLE_LINE "--------------------------------------------------------------------------\n"

void display_header(void)
{
    printf(TABLE_LINE);
    printf("%-9s %-24s %8s %12s %12s %-10s\n", "ID", "Product", "Qty", "Unit price",
           "Total", "Date");
    printf(TABLE_LINE);
}

void display_row(const Transaction *t)
{
    printf("%-9d %-24.24s %8d %12.2f %12.2f %-10s\n", t->id, t->product, t->quantity,
           t->unit_price, t->total, t->date);
}

void display_footer(void)
{
    printf(TABLE_LINE);
}

void display_all(const TransactionList *list)
{
    if (list->count == 0) {
        printf("No transactions recorded.\n");
        return;
    }
    display_header();
    for (int i = 0; i < list->count; i++) {
        display_row(&list->items[i]);
    }
    display_footer();
    printf("%d transaction(s)\n", list->count);
}

void display_matches(const TransactionList *list, const int *matches, int count)
{
    if (count <= 0) {
        printf("No matching transactions found.\n");
        return;
    }
    display_header();
    for (int i = 0; i < count; i++) {
        display_row(&list->items[matches[i]]);
    }
    display_footer();
    printf("%d match(es)\n", count);
}
