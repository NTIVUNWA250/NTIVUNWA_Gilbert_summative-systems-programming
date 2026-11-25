/*
 * display.h - printing transactions as tables.
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include "transactions.h"

void display_header(void);
void display_row(const Transaction *t);
void display_footer(void);
void display_all(const TransactionList *list);
void display_matches(const TransactionList *list, const int *matches, int count);

#endif
