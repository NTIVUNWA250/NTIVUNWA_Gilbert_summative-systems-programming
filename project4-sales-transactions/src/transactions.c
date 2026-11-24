/*
 * transactions.c - create, update and delete transactions in a dynamic array.
 */
#include <stdlib.h>
#include <string.h>

#include "textutil.h"
#include "transactions.h"
#include "validation.h"

const char *status_message(Status status)
{
    switch (status) {
    case STATUS_OK:
        return "Success.";
    case STATUS_NO_MEMORY:
        return "Memory allocation failed.";
    case STATUS_DUPLICATE:
        return "A transaction with that ID already exists.";
    case STATUS_NOT_FOUND:
        return "Transaction not found.";
    case STATUS_INVALID:
        return "Invalid transaction data.";
    case STATUS_FILE_ERROR:
        return "File access error.";
    }
    return "Unknown error.";
}

Status list_init(TransactionList *list)
{
    list->items = calloc(INITIAL_CAPACITY, sizeof(Transaction));
    list->count = 0;
    list->capacity = list->items == NULL ? 0 : INITIAL_CAPACITY;
    return list->items == NULL ? STATUS_NO_MEMORY : STATUS_OK;
}

void list_free(TransactionList *list)
{
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

int list_index_of(const TransactionList *list, int id)
{
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].id == id) {
            return i;
        }
    }
    return -1;
}

Transaction *list_get(TransactionList *list, int id)
{
    int index = list_index_of(list, id);
    return index < 0 ? NULL : &list->items[index];
}

static Status validate_fields(const char *product, int quantity, double unit_price,
                              const char *date)
{
    if (!valid_text_field(product, PRODUCT_LEN) || !valid_quantity(quantity) ||
        !valid_unit_price(unit_price) || !valid_date(date)) {
        return STATUS_INVALID;
    }
    return STATUS_OK;
}

static void fill(Transaction *t, const char *product, int quantity, double unit_price,
                 const char *date)
{
    text_copy(t->product, PRODUCT_LEN, product);
    text_copy(t->date, DATE_LEN, date);
    t->quantity = quantity;
    t->unit_price = unit_price;
    t->total = quantity * unit_price;
}

static Status ensure_capacity(TransactionList *list)
{
    Transaction *resized;
    int new_capacity;

    if (list->count < list->capacity) {
        return STATUS_OK;
    }
    new_capacity = list->capacity > 0 ? list->capacity * 2 : INITIAL_CAPACITY;
    resized = realloc(list->items, (size_t)new_capacity * sizeof(Transaction));
    if (resized == NULL) {
        return STATUS_NO_MEMORY;
    }
    list->items = resized;
    list->capacity = new_capacity;
    return STATUS_OK;
}

Status list_add(TransactionList *list, int id, const char *product, int quantity,
                double unit_price, const char *date)
{
    Transaction *t;

    if (!valid_transaction_id(id) ||
        validate_fields(product, quantity, unit_price, date) != STATUS_OK) {
        return STATUS_INVALID;
    }
    if (list_index_of(list, id) >= 0) {
        return STATUS_DUPLICATE;
    }
    if (ensure_capacity(list) != STATUS_OK) {
        return STATUS_NO_MEMORY;
    }
    t = &list->items[list->count];
    memset(t, 0, sizeof *t);
    t->id = id;
    fill(t, product, quantity, unit_price, date);
    list->count++;
    return STATUS_OK;
}

Status list_update(TransactionList *list, int id, const char *product, int quantity,
                   double unit_price, const char *date)
{
    Transaction *t = list_get(list, id);

    if (t == NULL) {
        return STATUS_NOT_FOUND;
    }
    if (validate_fields(product, quantity, unit_price, date) != STATUS_OK) {
        return STATUS_INVALID;
    }
    fill(t, product, quantity, unit_price, date);
    return STATUS_OK;
}

Status list_remove(TransactionList *list, int id)
{
    int index = list_index_of(list, id);

    if (index < 0) {
        return STATUS_NOT_FOUND;
    }
    for (int i = index; i < list->count - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    list->count--;
    return STATUS_OK;
}
