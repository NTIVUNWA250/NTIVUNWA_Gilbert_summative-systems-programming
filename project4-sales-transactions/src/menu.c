/*
 * menu.c - one handler per menu option. Handlers return 0 when input has
 * ended (EOF) so the program can stop cleanly.
 */
#include <stdio.h>
#include <stdlib.h>

#include "callbacks.h"
#include "display.h"
#include "input.h"
#include "menu.h"
#include "processing.h"
#include "report.h"
#include "search_sort.h"
#include "storage.h"
#include "validation.h"

#define DEFAULT_REPORT_FILE "sales_report.txt"
#define ANSWER_LEN 64

static void print_status(Status status, const char *success)
{
    if (status == STATUS_OK) {
        printf("%s\n", success);
    } else {
        printf("Error: %s\n", status_message(status));
    }
}

static int has_transactions(const App *app)
{
    if (app->list.count == 0) {
        printf("No transactions recorded yet.\n");
        return 0;
    }
    return 1;
}

static int read_date(const char *prompt, char *date)
{
    for (;;) {
        if (!read_line(prompt, date, DATE_LEN + 2)) {
            return 0;
        }
        if (valid_date(date)) {
            return 1;
        }
        printf("  Error: enter a real date as YYYY-MM-DD (years %d-%d).\n", MIN_YEAR,
               MAX_YEAR);
    }
}

static int read_details(char *product, int *quantity, double *price, char *date)
{
    return read_text("Product name: ", product, PRODUCT_LEN) &&
           read_int("Quantity sold: ", 1, MAX_QUANTITY, quantity) &&
           read_double("Unit price: ", 0.01, MAX_UNIT_PRICE, price) &&
           read_date("Date (YYYY-MM-DD): ", date);
}

static int add_transaction(App *app)
{
    char product[PRODUCT_LEN], date[DATE_LEN + 2];
    int id, quantity;
    double price;

    if (!read_int("Transaction ID: ", MIN_TRANSACTION_ID, MAX_TRANSACTION_ID, &id)) {
        return 0;
    }
    if (list_get(&app->list, id) != NULL) {
        printf("Error: transaction ID %d already exists.\n", id);
        return 1;
    }
    if (!read_details(product, &quantity, &price, date)) {
        return 0;
    }
    Status status = list_add(&app->list, id, product, quantity, price, date);
    if (status == STATUS_OK) {
        app->unsaved_changes = 1;
        printf("Total amount: %.2f\n", list_get(&app->list, id)->total);
    }
    print_status(status, "Transaction added.");
    return 1;
}

static int update_transaction(App *app)
{
    char product[PRODUCT_LEN], date[DATE_LEN + 2];
    int id, quantity;
    double price;

    if (!has_transactions(app)) {
        return 1;
    }
    if (!read_int("Transaction ID to update: ", MIN_TRANSACTION_ID, MAX_TRANSACTION_ID, &id)) {
        return 0;
    }
    Transaction *t = list_get(&app->list, id);
    if (t == NULL) {
        printf("Error: no transaction with ID %d.\n", id);
        return 1;
    }
    display_header();
    display_row(t);
    display_footer();
    printf("Enter the new values.\n");
    if (!read_details(product, &quantity, &price, date)) {
        return 0;
    }
    Status status = list_update(&app->list, id, product, quantity, price, date);
    if (status == STATUS_OK) {
        app->unsaved_changes = 1;
    }
    print_status(status, "Transaction updated.");
    return 1;
}

static int delete_transaction(App *app)
{
    int id, confirmed;

    if (!has_transactions(app)) {
        return 1;
    }
    if (!read_int("Transaction ID to delete: ", MIN_TRANSACTION_ID, MAX_TRANSACTION_ID, &id)) {
        return 0;
    }
    if (list_get(&app->list, id) == NULL) {
        printf("Error: no transaction with ID %d.\n", id);
        return 1;
    }
    if (!read_yes_no("Delete this transaction? (y/n): ", &confirmed)) {
        return 0;
    }
    if (!confirmed) {
        printf("Delete cancelled.\n");
        return 1;
    }
    Status status = list_remove(&app->list, id);
    if (status == STATUS_OK) {
        app->unsaved_changes = 1;
    }
    print_status(status, "Transaction deleted.");
    return 1;
}

/* The user picks an entry from the operations table and the program calls
 * it through its function pointer. */
static int process_sales(App *app)
{
    int count, choice, index;
    const SalesOperationEntry *ops = processing_operations(&count);

    if (!has_transactions(app)) {
        return 1;
    }
    printf("\nSales processing operations:\n");
    for (int i = 0; i < count; i++) {
        printf("%d. %s\n", i + 1, ops[i].name);
    }
    if (!read_int("Choice: ", 1, count, &choice)) {
        return 0;
    }
    const SalesOperationEntry *op = &ops[choice - 1];
    double result = processing_execute(op->run, &app->list, &index);

    printf("[function pointer -> %s]\n", op->symbol);
    printf(op->is_money ? "%s: %.2f\n" : "%s: %.0f\n", op->name, result);
    if (index >= 0) {
        display_header();
        display_row(&app->list.items[index]);
        display_footer();
    }
    return 1;
}

/* Each option passes a different callback to the same generic function. */
static int filter_menu(App *app)
{
    int choice, found, min_quantity;
    int *matches = NULL;
    double amount;
    char product[PRODUCT_LEN];

    if (!has_transactions(app)) {
        return 1;
    }
    printf("\nFilter using callbacks:\n");
    printf("1. Transactions above an amount\n");
    printf("2. Transactions below an amount\n");
    printf("3. Transactions for a product\n");
    printf("4. Count transactions with at least N units\n");
    if (!read_int("Choice: ", 1, 4, &choice)) {
        return 0;
    }
    if (choice == 4) {
        if (!read_int("Minimum units: ", 1, MAX_QUANTITY, &min_quantity)) {
            return 0;
        }
        printf("[callback quantity_at_least -> count_transactions]\n");
        printf("%d transaction(s) sold at least %d unit(s).\n",
               count_transactions(&app->list, quantity_at_least, &min_quantity), min_quantity);
        return 1;
    }
    if (choice == 3) {
        if (!read_text("Product name: ", product, sizeof product)) {
            return 0;
        }
        printf("[callback is_product -> filter_transactions]\n");
        found = filter_transactions(&app->list, is_product, product, &matches);
    } else {
        if (!read_double("Amount: ", 0, 1e12, &amount)) {
            return 0;
        }
        printf(choice == 1 ? "[callback is_above_amount -> filter_transactions]\n"
                           : "[callback is_below_amount -> filter_transactions]\n");
        found = filter_transactions(&app->list, choice == 1 ? is_above_amount : is_below_amount,
                                    &amount, &matches);
    }
    if (found < 0) {
        printf("Error: %s\n", status_message(STATUS_NO_MEMORY));
    } else {
        display_matches(&app->list, matches, found);
    }
    free(matches);
    return 1;
}

static int search_menu(App *app)
{
    int choice, id, found;
    int *matches = NULL;
    double amount;
    char product[PRODUCT_LEN];

    if (!has_transactions(app)) {
        return 1;
    }
    printf("Search by: 1. Transaction ID  2. Product name  3. Transaction amount\n");
    if (!read_int("Choice: ", 1, 3, &choice)) {
        return 0;
    }
    if (choice == 1) {
        if (!read_int("Transaction ID: ", MIN_TRANSACTION_ID, MAX_TRANSACTION_ID, &id)) {
            return 0;
        }
        int index = search_by_id(&app->list, id);
        display_matches(&app->list, &index, index >= 0 ? 1 : 0);
        return 1;
    }
    if (choice == 2) {
        if (!read_text("Product name (or part of it): ", product, sizeof product)) {
            return 0;
        }
        found = search_by_product(&app->list, product, &matches);
    } else {
        if (!read_double("Total amount: ", 0, 1e12, &amount)) {
            return 0;
        }
        found = search_by_amount(&app->list, amount, &matches);
    }
    if (found < 0) {
        printf("Error: %s\n", status_message(STATUS_NO_MEMORY));
    } else {
        display_matches(&app->list, matches, found);
    }
    free(matches);
    return 1;
}

static int sort_menu(App *app)
{
    static const TransactionCompare comparisons[] = {compare_by_id, compare_by_product,
                                                     compare_by_amount};
    static const char *names[] = {"compare_by_id", "compare_by_product", "compare_by_amount"};
    int key, order;

    if (!has_transactions(app)) {
        return 1;
    }
    printf("Sort by: 1. Transaction ID  2. Product name  3. Total amount\n");
    if (!read_int("Choice: ", 1, 3, &key) ||
        !read_int("Order: 1. Ascending  2. Descending: ", 1, 2, &order)) {
        return 0;
    }
    printf("[callback %s -> sort_transactions]\n", names[key - 1]);
    sort_transactions(&app->list, comparisons[key - 1], order == 1);
    app->unsaved_changes = 1;
    display_all(&app->list);
    return 1;
}

static int report_menu(App *app)
{
    SalesReport report;
    char product[PRODUCT_LEN], path[128];
    int count, save;

    report_build(&app->list, &report);
    report_print(&report, &app->list, stdout);

    if (app->list.count > 0) {
        if (!read_text("Product for revenue breakdown: ", product, sizeof product)) {
            return 0;
        }
        double revenue = report_product_revenue(&app->list, product, &count);
        if (count == 0) {
            printf("No transactions found for \"%s\".\n", product);
        } else {
            printf("Revenue from %s: %.2f across %d transaction(s).\n", product, revenue, count);
        }
    }
    if (!read_yes_no("Save the report to a file? (y/n): ", &save)) {
        return 0;
    }
    if (save) {
        if (!read_line("File name [" DEFAULT_REPORT_FILE "]: ", path, sizeof path)) {
            return 0;
        }
        FILE *file = fopen(path[0] != '\0' ? path : DEFAULT_REPORT_FILE, "w");
        if (file == NULL) {
            printf("Error: could not create the report file.\n");
            return 1;
        }
        report_print(&report, &app->list, file);
        printf(fclose(file) == 0 ? "Report saved.\n" : "Error: could not write the report.\n");
    }
    return 1;
}

static void save_transactions(App *app)
{
    if (storage_save(&app->list, app->data_path) == STATUS_OK) {
        app->unsaved_changes = 0;
        printf("Saved %d transaction(s) to %s.\n", app->list.count, app->data_path);
    } else {
        printf("Error: could not save to %s.\n", app->data_path);
    }
}

static int load_transactions(App *app)
{
    int confirmed, line;

    if (app->unsaved_changes) {
        if (!read_yes_no("Discard unsaved changes and reload from file? (y/n): ", &confirmed)) {
            return 0;
        }
        if (!confirmed) {
            printf("Load cancelled.\n");
            return 1;
        }
    }
    Status status = storage_load(&app->list, app->data_path, &line);
    if (status == STATUS_OK) {
        app->unsaved_changes = 0;
        printf("Loaded %d transaction(s) from %s.\n", app->list.count, app->data_path);
    } else if (status == STATUS_NOT_FOUND) {
        printf("Error: %s does not exist yet.\n", app->data_path);
    } else {
        printf("Error: could not load %s (line %d): %s\n", app->data_path, line,
               status_message(status));
    }
    return 1;
}

static void confirm_exit(App *app)
{
    int save;

    if (app->unsaved_changes &&
        read_yes_no("You have unsaved changes. Save before exiting? (y/n): ", &save) && save) {
        save_transactions(app);
    }
}

static void print_menu(void)
{
    printf("\n===== SALES TRANSACTION MANAGEMENT SYSTEM =====\n");
    printf(" 1. Add New Transaction\n");
    printf(" 2. Display All Transactions\n");
    printf(" 3. Update Transaction\n");
    printf(" 4. Delete Transaction\n");
    printf(" 5. Process Sales Data\n");
    printf(" 6. Filter Transactions (Callbacks)\n");
    printf(" 7. Search Transactions\n");
    printf(" 8. Sort Transactions\n");
    printf(" 9. Generate Sales Report\n");
    printf("10. Save Transactions\n");
    printf("11. Load Transactions\n");
    printf(" 0. Exit\n");
}

void menu_run(App *app)
{
    int choice;
    int running = 1;

    while (running) {
        print_menu();
        if (!read_int("Select an option: ", 0, 11, &choice)) {
            printf("\nInput ended.\n");
            break;
        }
        switch (choice) {
        case 1: running = add_transaction(app); break;
        case 2: display_all(&app->list); break;
        case 3: running = update_transaction(app); break;
        case 4: running = delete_transaction(app); break;
        case 5: running = process_sales(app); break;
        case 6: running = filter_menu(app); break;
        case 7: running = search_menu(app); break;
        case 8: running = sort_menu(app); break;
        case 9: running = report_menu(app); break;
        case 10: save_transactions(app); break;
        case 11: running = load_transactions(app); break;
        case 0:
            confirm_exit(app);
            running = 0;
            break;
        }
    }
    printf("Goodbye.\n");
}
