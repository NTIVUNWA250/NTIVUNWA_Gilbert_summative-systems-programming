#include <string.h>

#include "fixtures.h"
#include "processing.h"
#include "test_util.h"

static void test_operations(void)
{
    TransactionList list;
    int index = 99;

    fixture_load(&list);
    CHECK(op_total_revenue(&list, &index) == 3980.0 && index == -1);
    CHECK(op_average_value(&list, &index) == 796.0);
    CHECK(op_highest_value(&list, &index) == 2400.0 && list.items[index].id == 1003);
    CHECK(op_lowest_value(&list, &index) == 30.0 && list.items[index].id == 1004);
    CHECK(op_total_units(&list, &index) == 23.0);
    list_free(&list);
}

static void test_empty_list(void)
{
    TransactionList list;
    int index = 99;

    list_init(&list);
    CHECK(op_average_value(&list, &index) == 0.0 && index == -1);
    CHECK(op_highest_value(&list, &index) == 0.0 && index == -1);
    CHECK(op_lowest_value(&list, &index) == 0.0 && index == -1);
    list_free(&list);
}

static void test_table_runs_through_pointers(void)
{
    TransactionList list;
    int count = 0;
    int index;
    const SalesOperationEntry *ops = processing_operations(&count);

    fixture_load(&list);
    CHECK(count == 5);
    CHECK(strcmp(ops[0].name, "Calculate total revenue") == 0);
    CHECK(ops[0].run == op_total_revenue);
    CHECK(processing_execute(ops[0].run, &list, &index) == 3980.0);
    CHECK(processing_execute(ops[4].run, &list, &index) == 23.0);
    CHECK(!ops[4].is_money);
    list_free(&list);
}

int main(void)
{
    test_operations();
    test_empty_list();
    test_table_runs_through_pointers();
    return test_summary("test_processing");
}
