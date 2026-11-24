#include <stdio.h>
#include <string.h>

#include "fixtures.h"
#include "storage.h"
#include "test_util.h"

#define TEST_FILE "build/test_transactions.txt"

static void write_file(const char *text)
{
    FILE *file = fopen(TEST_FILE, "w");
    if (file != NULL) {
        fputs(text, file);
        fclose(file);
    }
}

static void test_round_trip(void)
{
    TransactionList original;
    TransactionList loaded;
    Transaction *t;

    fixture_load(&original);
    CHECK(storage_save(&original, TEST_FILE) == STATUS_OK);
    list_init(&loaded);
    CHECK(storage_load(&loaded, TEST_FILE, NULL) == STATUS_OK);
    CHECK(loaded.count == 5);
    t = list_get(&loaded, 1003);
    CHECK(t != NULL && strcmp(t->product, "Laptop") == 0 && t->total == 2400.0);
    CHECK(t != NULL && strcmp(t->date, "2026-11-01") == 0);
    list_free(&original);
    list_free(&loaded);
}

static void test_errors(void)
{
    TransactionList list;
    int line = 0;

    fixture_load(&list);
    CHECK(storage_load(&list, "build/missing.txt", NULL) == STATUS_NOT_FOUND);
    CHECK(storage_save(&list, "no_such_dir/sales.txt") == STATUS_FILE_ERROR);

    write_file("T|1|Pen|2|1.50|2026-11-01\nT|2|Pen|-2|1.50|2026-11-01\n");
    CHECK(storage_load(&list, TEST_FILE, &line) == STATUS_INVALID && line == 2);

    write_file("T|1|Pen|2|1.50|2026-11-01\nT|1|Pen|2|1.50|2026-11-01\n");
    CHECK(storage_load(&list, TEST_FILE, &line) == STATUS_DUPLICATE && line == 2);

    write_file("T|1|Pen|2|1.50\n");
    CHECK(storage_load(&list, TEST_FILE, &line) == STATUS_INVALID && line == 1);

    CHECK(list.count == 5);
    list_free(&list);
}

int main(void)
{
    test_round_trip();
    test_errors();
    return test_summary("test_storage");
}
