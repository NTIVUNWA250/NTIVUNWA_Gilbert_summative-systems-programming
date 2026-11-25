#include <stdlib.h>

#include "fixtures.h"
#include "search_sort.h"
#include "test_util.h"

static int ids_are(const TransactionList *list, const int *expected)
{
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].id != expected[i]) {
            return 0;
        }
    }
    return 1;
}

static void test_searches(void)
{
    TransactionList list;
    int *matches = NULL;
    int found;

    fixture_load(&list);
    CHECK(search_by_id(&list, 1002) == 2);
    CHECK(search_by_id(&list, 4242) == -1);

    found = search_by_product(&list, "cable", &matches);
    CHECK(found == 2 && list.items[matches[1]].id == 1004);
    free(matches);

    found = search_by_amount(&list, 1200.0, &matches);
    CHECK(found == 1 && list.items[matches[0]].id == 1005);
    free(matches);

    found = search_by_amount(&list, 999.0, &matches);
    CHECK(found == 0);
    free(matches);
    list_free(&list);
}

static void test_sorts(void)
{
    TransactionList list;
    const int by_id[] = {1001, 1002, 1003, 1004, 1005};
    const int by_product[] = {1002, 1003, 1005, 1001, 1004};
    const int by_amount_desc[] = {1003, 1005, 1002, 1001, 1004};
    const int by_amount_asc[] = {1004, 1001, 1002, 1005, 1003};

    fixture_load(&list);
    sort_transactions(&list, compare_by_id, 1);
    CHECK(ids_are(&list, by_id));
    sort_transactions(&list, compare_by_product, 1);
    CHECK(ids_are(&list, by_product));
    sort_transactions(&list, compare_by_amount, 0);
    CHECK(ids_are(&list, by_amount_desc));
    sort_transactions(&list, compare_by_amount, 1);
    CHECK(ids_are(&list, by_amount_asc));
    list_free(&list);
}

int main(void)
{
    test_searches();
    test_sorts();
    return test_summary("test_search_sort");
}
