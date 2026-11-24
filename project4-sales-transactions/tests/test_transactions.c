#include <string.h>

#include "fixtures.h"
#include "test_util.h"
#include "transactions.h"

static void test_init_and_growth(void)
{
    TransactionList list;

    CHECK(list_init(&list) == STATUS_OK);
    CHECK(list.capacity == INITIAL_CAPACITY && list.count == 0);
    list_free(&list);

    fixture_load(&list);
    CHECK(list.count == 5);
    CHECK(list.capacity == INITIAL_CAPACITY * 2);
    CHECK(list_get(&list, 1003)->total == 2400.0);
    CHECK(list_get(&list, 1001)->total == 50.0);
    list_free(&list);
    CHECK(list.items == NULL && list.capacity == 0);
}

static void test_add_rejects_bad_data(void)
{
    TransactionList list;

    fixture_load(&list);
    CHECK(list_add(&list, 1001, "Mouse", 1, 10, "2026-11-10") == STATUS_DUPLICATE);
    CHECK(list_add(&list, 2000, "Mouse", -1, 10, "2026-11-10") == STATUS_INVALID);
    CHECK(list_add(&list, 2000, "Mouse", 1, 0, "2026-11-10") == STATUS_INVALID);
    CHECK(list_add(&list, 2000, "Mouse", 1, -5, "2026-11-10") == STATUS_INVALID);
    CHECK(list_add(&list, 2000, "", 1, 10, "2026-11-10") == STATUS_INVALID);
    CHECK(list_add(&list, 2000, "Mouse", 1, 10, "2026-02-30") == STATUS_INVALID);
    CHECK(list_add(&list, 0, "Mouse", 1, 10, "2026-11-10") == STATUS_INVALID);
    CHECK(list.count == 5);
    list_free(&list);
}

static void test_update(void)
{
    TransactionList list;
    Transaction *t;

    fixture_load(&list);
    CHECK(list_update(&list, 1002, "Mechanical Keyboard", 3, 100, "2026-11-06") == STATUS_OK);
    t = list_get(&list, 1002);
    CHECK(strcmp(t->product, "Mechanical Keyboard") == 0 && t->total == 300.0);
    CHECK(strcmp(t->date, "2026-11-06") == 0);
    CHECK(list_update(&list, 1002, "Keyboard", 0, 100, "2026-11-06") == STATUS_INVALID);
    CHECK(t->quantity == 3);
    CHECK(list_update(&list, 7777, "Keyboard", 1, 1, "2026-11-06") == STATUS_NOT_FOUND);
    list_free(&list);
}

static void test_remove(void)
{
    TransactionList list;

    fixture_load(&list);
    CHECK(list_remove(&list, 1001) == STATUS_OK);
    CHECK(list.count == 4 && list_get(&list, 1001) == NULL);
    CHECK(list.items[1].id == 1002);
    CHECK(list_remove(&list, 1001) == STATUS_NOT_FOUND);
    list_free(&list);
}

int main(void)
{
    test_init_and_growth();
    test_add_rejects_bad_data();
    test_update();
    test_remove();
    return test_summary("test_transactions");
}
