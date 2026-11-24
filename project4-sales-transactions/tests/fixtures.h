/*
 * fixtures.h - a known set of transactions shared by the tests.
 * Totals: 1003 = 2400, 1001 = 50, 1002 = 300, 1004 = 30, 1005 = 1200.
 */
#ifndef FIXTURES_H
#define FIXTURES_H

#include "transactions.h"

static inline void fixture_load(TransactionList *list)
{
    list_init(list);
    list_add(list, 1003, "Laptop", 2, 1200.0, "2026-11-01");
    list_add(list, 1001, "USB Cable", 10, 5.0, "2026-11-02");
    list_add(list, 1002, "Keyboard", 4, 75.0, "2026-11-03");
    list_add(list, 1004, "usb cable", 6, 5.0, "2026-11-04");
    list_add(list, 1005, "Laptop", 1, 1200.0, "2026-11-05");
}

#endif
