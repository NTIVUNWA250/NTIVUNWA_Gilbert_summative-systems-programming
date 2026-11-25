/*
 * transactions.h - the sales transaction record and the dynamic list that
 * stores them. The list starts with INITIAL_CAPACITY slots from calloc()
 * and doubles with realloc() when it is full.
 */
#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#define PRODUCT_LEN 48
#define DATE_LEN 11
#define INITIAL_CAPACITY 4

typedef enum {
    STATUS_OK = 0,
    STATUS_NO_MEMORY,
    STATUS_DUPLICATE,
    STATUS_NOT_FOUND,
    STATUS_INVALID,
    STATUS_FILE_ERROR
} Status;

typedef struct {
    int id;
    char product[PRODUCT_LEN];
    int quantity;
    double unit_price;
    double total;
    char date[DATE_LEN];
} Transaction;

typedef struct {
    Transaction *items;
    int count;
    int capacity;
} TransactionList;

const char *status_message(Status status);

Status list_init(TransactionList *list);
void list_free(TransactionList *list);
int list_index_of(const TransactionList *list, int id);
Transaction *list_get(TransactionList *list, int id);
Status list_add(TransactionList *list, int id, const char *product, int quantity,
                double unit_price, const char *date);
Status list_update(TransactionList *list, int id, const char *product, int quantity,
                   double unit_price, const char *date);
Status list_remove(TransactionList *list, int id);

#endif
