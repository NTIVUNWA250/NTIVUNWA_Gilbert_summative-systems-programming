#!/bin/sh
# Drives the compiled program through its menu and checks output and file.
set -u

APP=./sales_system
DATA=build/integration_sales.txt
OUT=build/integration_output.txt
failures=0

check() {
    if grep -q -- "$2" "$3"; then
        echo "  ok   $1"
    else
        echo "  FAIL $1 (expected '$2' in $3)"
        failures=$((failures + 1))
    fi
}

check_absent() {
    if grep -q -- "$2" "$3"; then
        echo "  FAIL $1 (did not expect '$2' in $3)"
        failures=$((failures + 1))
    else
        echo "  ok   $1"
    fi
}

mkdir -p build
rm -f "$DATA"

# Add three transactions (with bad quantity, price and date re-asked), reject a
# duplicate ID, update one, delete one, run operations and callbacks, save.
printf '%s\n' \
    1 1 Pen -5 3 0 2.50 2026-02-30 2026-11-01 \
    1 1 \
    1 2 Notebook 2 4.00 2026-11-02 \
    1 3 Bag 1 30.00 2026-11-03 \
    3 2 Notebook 5 4.00 2026-11-05 \
    4 3 y \
    5 1 \
    6 1 15 \
    8 3 2 \
    10 0 | "$APP" "$DATA" > "$OUT" 2>&1

check "starts empty" "Starting empty" "$OUT"
check "negative quantity re-asked" "enter a whole number from 1 to" "$OUT"
check "zero price re-asked" "enter a number from 0.01" "$OUT"
check "invalid date re-asked" "enter a real date" "$OUT"
check "duplicate ID rejected" "transaction ID 1 already exists" "$OUT"
check "update applied" "Transaction updated." "$OUT"
check "delete applied" "Transaction deleted." "$OUT"
check "function pointer revenue" "Calculate total revenue: 27.50" "$OUT"
check "callback filter used" "callback is_above_amount -> filter_transactions" "$OUT"
check "filter found the notebook" "1 match(es)" "$OUT"
check "sort callback used" "callback compare_by_amount -> sort_transactions" "$OUT"
check "file has updated notebook" "T|2|Notebook|5|4.00|2026-11-05" "$DATA"
check_absent "deleted bag not saved" "Bag" "$DATA"

# Restart on the saved file, search, report and handle a missing ID.
printf '%s\n' 7 2 note 7 1 99 9 pen n 0 | "$APP" "$DATA" > "$OUT" 2>&1
check "reload reports two transactions" "Loaded 2 transaction(s)" "$OUT"
check "product search finds notebook" "Notebook" "$OUT"
check "missing ID handled" "No matching transactions found." "$OUT"
check "product revenue" "Revenue from pen: 7.50 across 1 transaction(s)." "$OUT"

printf '%s\n' 12 x 0 | "$APP" "$DATA" > "$OUT" 2>&1
check "invalid menu choice rejected" "enter a whole number from 0 to 11" "$OUT"

if [ "$failures" -ne 0 ]; then
    echo "integration_test   $failures failure(s)"
    exit 1
fi
echo "integration_test   all checks passed"
