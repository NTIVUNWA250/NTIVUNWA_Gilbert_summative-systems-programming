#include "fixtures.h"
#include "report.h"
#include "test_util.h"

static void test_report_values(void)
{
    TransactionList list;
    SalesReport report;
    int count = 0;

    fixture_load(&list);
    report_build(&list, &report);
    CHECK(report.count == 5);
    CHECK(report.revenue == 3980.0);
    CHECK(report.average == 796.0);
    CHECK(report.highest == 2400.0 && list.items[report.highest_index].id == 1003);
    CHECK(report.lowest == 30.0 && list.items[report.lowest_index].id == 1004);
    CHECK(report.units == 23.0);

    CHECK(report_product_revenue(&list, "laptop", &count) == 3600.0 && count == 2);
    CHECK(report_product_revenue(&list, "Phone", &count) == 0.0 && count == 0);
    list_free(&list);
}

static void test_empty_report(void)
{
    TransactionList list;
    SalesReport report;

    list_init(&list);
    report_build(&list, &report);
    CHECK(report.count == 0 && report.average == 0.0);
    CHECK(report.highest_index == -1 && report.lowest_index == -1);
    list_free(&list);
}

int main(void)
{
    test_report_values();
    test_empty_report();
    return test_summary("test_report");
}
