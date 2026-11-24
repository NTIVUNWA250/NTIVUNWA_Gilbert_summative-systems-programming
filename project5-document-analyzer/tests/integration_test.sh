#!/bin/sh
# Runs the compiled analyzer on the sample documents and checks the output,
# the exit codes and every generated report.
set -u

APP=../../document_analyzer
WORK=build/integration
OUT=output.txt
failures=0

check() {
    if grep -q -- "$2" "$3"; then
        echo "  ok   $1"
    else
        echo "  FAIL $1 (expected '$2' in $3)"
        failures=$((failures + 1))
    fi
}

check_code() {
    if [ "$2" -eq "$3" ]; then
        echo "  ok   $1"
    else
        echo "  FAIL $1 (exit code $2, expected $3)"
        failures=$((failures + 1))
    fi
}

rm -rf "$WORK"
mkdir -p "$WORK"
cd "$WORK" || exit 1

"$APP" "IoT" ../../documents/document1.txt ../../documents/document2.txt \
    ../../documents/document3.txt > "$OUT" 2>&1
check_code "all files succeed" $? 0
check "keyword shown" "Search Keyword: IoT" "$OUT"
check "file count shown" "Number of Input Files: 3" "$OUT"
check "thread creation shown" "\[Main\] Created thread 3" "$OUT"
check "thread processing shown" "\[Thread 2\] Processing" "$OUT"
check "thread success shown" "\[Thread 1\] Successfully processed" "$OUT"
check "all completed message" "All processing tasks completed." "$OUT"
check "document1 counts" "Keyword occurrences      : 8" document1_report.txt
check "document1 matching lines" "Lines containing keyword : 6" document1_report.txt
check "document1 line numbers" "  1, 3, 6, 8, 12, 13" document1_report.txt
check "document2 counts" "Keyword occurrences      : 2" document2_report.txt
check "document3 not found" "SUCCESS (keyword not found)" document3_report.txt
check "document3 line total" "Total lines processed    : 9" document3_report.txt

"$APP" "IoT" ../../documents/document1.txt missing.txt > "$OUT" 2>&1
check_code "missing file gives exit code 2" $? 2
check "missing file error shown" "Error: cannot open missing.txt" "$OUT"
check "partial summary shown" "1 of 2 file(s) processed successfully" "$OUT"
check "failed report written" "Processing status        : FAILED" missing_report.txt

"$APP" "IoT" > "$OUT" 2>&1
check_code "missing arguments give exit code 1" $? 1
check "usage shown" "Usage:" "$OUT"

"$APP" "" ../../documents/document1.txt > "$OUT" 2>&1
check_code "empty keyword rejected" $? 1

if [ "$failures" -ne 0 ]; then
    echo "integration_test   $failures failure(s)"
    exit 1
fi
echo "integration_test   all checks passed"
