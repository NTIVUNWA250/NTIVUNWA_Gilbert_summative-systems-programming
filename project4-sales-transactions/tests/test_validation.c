#include <string.h>

#include "test_util.h"
#include "textutil.h"
#include "validation.h"

static void test_numbers(void)
{
    int i = 0;
    double d = 0;

    CHECK(valid_transaction_id(1) && !valid_transaction_id(0));
    CHECK(valid_quantity(1) && valid_quantity(MAX_QUANTITY));
    CHECK(!valid_quantity(0) && !valid_quantity(-3));
    CHECK(valid_unit_price(0.5) && !valid_unit_price(0) && !valid_unit_price(-2));

    CHECK(parse_int("25", &i) && i == 25);
    CHECK(!parse_int("2x", &i));
    CHECK(parse_double("1499.99", &d) && d == 1499.99);
    CHECK(!parse_double("", &d));
}

static void test_dates(void)
{
    CHECK(valid_date("2026-11-24"));
    CHECK(valid_date("2024-02-29"));
    CHECK(!valid_date("2026-02-29"));
    CHECK(!valid_date("2026-13-01"));
    CHECK(!valid_date("2026-04-31"));
    CHECK(!valid_date("26-11-24"));
    CHECK(!valid_date("2026/11/24"));
    CHECK(!valid_date("1999-01-01"));
}

static void test_text(void)
{
    char text[16] = "  Laptop  ";

    text_trim(text);
    CHECK(strcmp(text, "Laptop") == 0);
    CHECK(valid_text_field("Laptop", 48));
    CHECK(!valid_text_field("", 48));
    CHECK(!valid_text_field("a|b", 48));
    CHECK(text_contains_ci("USB Cable", "cable"));
}

int main(void)
{
    test_numbers();
    test_dates();
    test_text();
    return test_summary("test_validation");
}
