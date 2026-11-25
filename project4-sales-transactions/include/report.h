/*
 * report.h - sales summary built from the processing operations.
 */
#ifndef REPORT_H
#define REPORT_H

#include <stdio.h>

#include "transactions.h"

typedef struct {
    int count;
    double revenue;
    double average;
    double highest;
    int highest_index;
    double lowest;
    int lowest_index;
    double units;
} SalesReport;

void report_build(const TransactionList *list, SalesReport *report);
void report_print(const SalesReport *report, const TransactionList *list, FILE *out);
double report_product_revenue(const TransactionList *list, const char *product, int *count);

#endif
