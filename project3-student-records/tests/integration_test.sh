#!/bin/sh
# Drives the compiled program through its menu and checks the saved file.
set -u

APP=./student_records
DATA=build/integration_students.txt
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

# Register two students, reject a duplicate ID, add courses, update marks,
# delete one student, then save and exit.
printf '%s\n' \
    1 2001 Ana Keza Law 1 \
    1 2001 \
    1 2002 Ben Ishimwe Medicine 2 \
    6 2001 LW101 Torts 100 45 \
    6 2001 LW101 \
    7 2001 LW101 150 65 \
    5 2002 y \
    12 0 | "$APP" "$DATA" > "$OUT" 2>&1

check "starts with an empty list" "Starting with an empty list" "$OUT"
check "duplicate ID rejected" "student ID 2001 already exists" "$OUT"
check "duplicate course rejected" "LW101 is already registered" "$OUT"
check "marks outside range re-asked" "enter a number from 0.00 to 100.00" "$OUT"
check "student deleted" "Student deleted." "$OUT"
check "saved file has Ana" "S|2001|Ana|Keza|Law|1|1" "$DATA"
check "saved file has updated marks" "C|LW101|Torts|65.00|100.00" "$DATA"
check_absent "deleted student not saved" "2002" "$DATA"

# Restart the program on the saved file and confirm the data came back.
printf '%s\n' 8 2001 9 1 9999 0 | "$APP" "$DATA" > "$OUT" 2>&1
check "reload reports one student" "Loaded 1 student(s)" "$OUT"
check "performance shows pass" "65.00% - PASSING" "$OUT"
check "missing ID handled" "No matching students found." "$OUT"

printf '%s\n' 42 abc 0 | "$APP" "$DATA" > "$OUT" 2>&1
check "invalid menu choice rejected" "enter a whole number from 0 to 12" "$OUT"

if [ "$failures" -ne 0 ]; then
    echo "integration_test   $failures failure(s)"
    exit 1
fi
echo "integration_test   all checks passed"
