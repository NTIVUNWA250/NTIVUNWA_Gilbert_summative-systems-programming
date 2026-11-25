#include <stdlib.h>

#include "callbacks.h"
#include "fixtures.h"
#include "test_util.h"

static void test_filter_above_amount(void)
{
    TransactionList list;
    int *matches = NULL;
    double threshold = 250.0;
    int found;

    fixture_load(&list);
    found = filter_transactions(&list, is_above_amount, &threshold, &matches);
    CHECK(found == 3);
    CHECK(found == 3 && list.items[matches[0]].id == 1003);
    CHECK(found == 3 && list.items[matches[1]].id == 1002);
    CHECK(found == 3 && list.items[matches[2]].id == 1005);
    free(matches);
    list_free(&list);
}

static void test_product_and_counting(void)
{
    TransactionList list;
    int *matches = NULL;
    int min_quantity = 5;
    double limit = 100.0;

    fixture_load(&list);
    CHECK(filter_transactions(&list, is_product, "USB CABLE", &matches) == 2);
    free(matches);
    CHECK(count_transactions(&list, is_product, "laptop") == 2);
    CHECK(count_transactions(&list, quantity_at_least, &min_quantity) == 2);
    CHECK(count_transactions(&list, is_below_amount, &limit) == 2);
    CHECK(count_transactions(&list, product_contains, "board") == 1);
    CHECK(sum_matching_revenue(&list, is_product, "Laptop") == 3600.0);
    CHECK(sum_matching_revenue(&list, is_product, "Phone") == 0.0);
    list_free(&list);
}

static void test_comparisons(void)
{
    Transaction a = {1, "Apple", 1, 10.0, 10.0, "2026-11-01"};
    Transaction b = {2, "banana", 1, 5.0, 5.0, "2026-11-01"};

    CHECK(compare_by_id(&a, &b) < 0);
    CHECK(compare_by_product(&a, &b) < 0);
    CHECK(compare_by_amount(&a, &b) > 0);
    CHECK(compare_by_amount(&a, &a) == 0);
}

static void test_empty_filter(void)
{
    TransactionList list;
    int *matches = NULL;
    double threshold = 0.0;

    list_init(&list);
    CHECK(filter_transactions(&list, is_above_amount, &threshold, &matches) == 0);
    CHECK(matches == NULL);
    list_free(&list);
}

int main(void)
{
    test_filter_above_amount();
    test_product_and_counting();
    test_comparisons();
    test_empty_filter();
    return test_summary("test_callbacks");
}
