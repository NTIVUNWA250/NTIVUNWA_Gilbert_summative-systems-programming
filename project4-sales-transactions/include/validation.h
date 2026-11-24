/*
 * validation.h - rules for transaction fields and strict number parsing.
 * Each valid_* function returns 1 when the value is acceptable, 0 otherwise.
 */
#ifndef VALIDATION_H
#define VALIDATION_H

#include <stddef.h>

#define MIN_TRANSACTION_ID 1
#define MAX_TRANSACTION_ID 99999999
#define MAX_QUANTITY 1000000
#define MAX_UNIT_PRICE 10000000.0
#define MIN_YEAR 2000
#define MAX_YEAR 2100

int valid_transaction_id(int id);
int valid_quantity(int quantity);
int valid_unit_price(double price);
int valid_date(const char *date);
int valid_text_field(const char *text, size_t capacity);

int parse_int(const char *text, int *out);
int parse_double(const char *text, double *out);

#endif
