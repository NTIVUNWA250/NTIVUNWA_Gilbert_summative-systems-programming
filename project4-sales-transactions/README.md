# Project 4: Sales Transaction Processing and Analysis System

A menu-driven C program that records sales transactions, runs sales
operations through function pointers, filters records with callbacks,
searches, sorts, reports and saves everything to a text file.

## Build and run

```
cd project4-sales-transactions
make                        # builds ./sales_system
./sales_system              # loads data/transactions.txt
./sales_system other.txt    # or another data file
```

## Tests

```
make test                   # 7 unit test programs + menu-driven integration test
```

Memory check on Linux (Docker):

```
docker run --rm -v "$PWD":/work -w /work ubuntu:24.04 bash -c \
  'apt-get update -qq && apt-get install -y -qq gcc make valgrind >/dev/null && \
   make -s && printf "2\n5\n1\n0\n" | valgrind --leak-check=full ./sales_system'
```

## Layout

| Path | Purpose |
| --- | --- |
| include/transactions.h, src/transactions.c | Transaction structure, calloc/realloc list, CRUD |
| include/processing.h, src/processing.c | Sales operations called through function pointers |
| include/callbacks.h, src/callbacks.c | Generic filter/count/sum functions and their callbacks |
| include/search_sort.h, src/search_sort.c | Manual search and insertion sort with a compare callback |
| include/report.h, src/report.c | Sales report and revenue per product |
| include/storage.h, src/storage.c | Saving and loading the data file |
| include/validation.h, src/validation.c | Field rules (ID, quantity, price, date) and number parsing |
| include/input.h, src/input.c | Re-asking keyboard input |
| include/display.h, src/display.c | Transaction tables |
| include/menu.h, src/menu.c | Menu handlers |
| src/main.c | Entry point |
| data/transactions.txt | Sample transactions |
| samples/sales_report.txt | Sample generated report |
