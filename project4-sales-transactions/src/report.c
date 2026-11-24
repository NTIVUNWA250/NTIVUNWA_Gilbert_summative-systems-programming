/*
 * report.c - sales report built by running each operation through its
 * function pointer, plus revenue for one product using callbacks.
 */
#include <time.h>

#include "callbacks.h"
#include "processing.h"
#include "report.h"

void report_build(const TransactionList *list, SalesReport *report)
{
    int unused;

    report->count = list->count;
    report->revenue = processing_execute(op_total_revenue, list, &unused);
    report->average = processing_execute(op_average_value, list, &unused);
    report->highest = processing_execute(op_highest_value, list, &report->highest_index);
    report->lowest = processing_execute(op_lowest_value, list, &report->lowest_index);
    report->units = processing_execute(op_total_units, list, &unused);
}

static void print_extreme(FILE *out, const char *label, double value, int index,
                          const TransactionList *list)
{
    const Transaction *t = &list->items[index];

    fprintf(out, "%-28s: %.2f (ID %d, %s, %s)\n", label, value, t->id, t->product, t->date);
}

void report_print(const SalesReport *report, const TransactionList *list, FILE *out)
{
    char stamp[32];
    time_t now = time(NULL);

    strftime(stamp, sizeof stamp, "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(out, "\n=============== SALES REPORT ===============\n");
    fprintf(out, "Generated: %s\n\n", stamp);
    fprintf(out, "%-28s: %d\n", "Total transactions", report->count);
    if (report->count == 0) {
        fprintf(out, "No transactions recorded, so there is nothing to summarise.\n");
    } else {
        fprintf(out, "%-28s: %.2f\n", "Total revenue", report->revenue);
        fprintf(out, "%-28s: %.2f\n", "Average transaction value", report->average);
        print_extreme(out, "Highest transaction value", report->highest,
                      report->highest_index, list);
        print_extreme(out, "Lowest transaction value", report->lowest,
                      report->lowest_index, list);
        fprintf(out, "%-28s: %.0f\n", "Total units sold", report->units);
    }
    fprintf(out, "============================================\n");
}

double report_product_revenue(const TransactionList *list, const char *product, int *count)
{
    *count = count_transactions(list, is_product, product);
    return sum_matching_revenue(list, is_product, product);
}
