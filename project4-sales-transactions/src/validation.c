/*
 * validation.c - transaction field rules and strict number parsing.
 */
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "validation.h"

int valid_transaction_id(int id)
{
    return id >= MIN_TRANSACTION_ID && id <= MAX_TRANSACTION_ID;
}

int valid_quantity(int quantity)
{
    return quantity >= 1 && quantity <= MAX_QUANTITY;
}

int valid_unit_price(double price)
{
    return isfinite(price) && price > 0.0 && price <= MAX_UNIT_PRICE;
}

static int days_in_month(int year, int month)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;

    return month == 2 && leap ? 29 : days[month - 1];
}

/* Accepts a real calendar date written as YYYY-MM-DD. */
int valid_date(const char *date)
{
    int year, month, day;

    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return 0;
    }
    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit((unsigned char)date[i])) {
            return 0;
        }
    }
    year = atoi(date);
    month = atoi(date + 5);
    day = atoi(date + 8);
    if (year < MIN_YEAR || year > MAX_YEAR || month < 1 || month > 12) {
        return 0;
    }
    return day >= 1 && day <= days_in_month(year, month);
}

/* Text must be non-empty, fit its buffer, be printable ASCII and not
 * contain '|', which separates fields in the data file. */
int valid_text_field(const char *text, size_t capacity)
{
    size_t length = strlen(text);

    if (length == 0 || length >= capacity) {
        return 0;
    }
    for (size_t i = 0; i < length; i++) {
        unsigned char c = (unsigned char)text[i];
        if (c < 32 || c > 126 || c == '|') {
            return 0;
        }
    }
    return 1;
}

int parse_int(const char *text, int *out)
{
    char *end = NULL;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);
    if (end == text || errno == ERANGE || value < INT_MIN || value > INT_MAX) {
        return 0;
    }
    while (*end == ' ' || *end == '\t') {
        end++;
    }
    if (*end != '\0') {
        return 0;
    }
    *out = (int)value;
    return 1;
}

int parse_double(const char *text, double *out)
{
    char *end = NULL;
    double value;

    errno = 0;
    value = strtod(text, &end);
    if (end == text || errno == ERANGE || !isfinite(value)) {
        return 0;
    }
    while (*end == ' ' || *end == '\t') {
        end++;
    }
    if (*end != '\0') {
        return 0;
    }
    *out = value;
    return 1;
}
