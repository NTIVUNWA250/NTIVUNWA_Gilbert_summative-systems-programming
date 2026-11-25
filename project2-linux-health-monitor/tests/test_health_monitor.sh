#!/usr/bin/env bash
# Unit and integration tests for health_monitor.sh. Run on Linux:
#   bash tests/test_health_monitor.sh
set -u

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT="$HERE/../health_monitor.sh"
WORK="$(mktemp -d)"
export HEALTH_LOG_FILE="$WORK/activity.log"
export HEALTH_REPORT_DIR="$WORK/reports"
OUT="$WORK/out.txt"
passed=0
failed=0

ok() { passed=$((passed + 1)); echo "  ok   $1"; }
fail() { failed=$((failed + 1)); echo "  FAIL $1"; }
check() { if eval "$2"; then ok "$1"; else fail "$1"; fi; }
check_out() { if grep -q -- "$2" "$OUT"; then ok "$1"; else fail "$1 (missing '$2')"; fi; }
run_menu() { printf '%s\n' "$@" | bash "$SCRIPT" >"$OUT" 2>&1; }

echo "Unit tests (sourced functions)"
# shellcheck source=../health_monitor.sh
source "$SCRIPT"

check "threshold 1 is valid" "is_valid_threshold 1"
check "threshold 100 is valid" "is_valid_threshold 100"
check "threshold 080 is valid" "is_valid_threshold 080"
check "threshold 0 rejected" "! is_valid_threshold 0"
check "threshold 101 rejected" "! is_valid_threshold 101"
check "threshold -5 rejected" "! is_valid_threshold -5"
check "threshold text rejected" "! is_valid_threshold abc"
check "threshold decimal rejected" "! is_valid_threshold 50.5"
check "PID 1234 is valid" "is_valid_pid 1234"
check "PID 0 rejected" "! is_valid_pid 0"
check "PID text rejected" "! is_valid_pid 12a"
check "at_least compares decimals" "at_least 80.5 80 && ! at_least 79.9 80"
check "OS name is found" "[ -n \"\$(get_os_name)\" ]"
check "uptime is found" "[ -n \"\$(get_uptime)\" ]"
usage="$(cpu_usage_percent)"
check "CPU usage is a percentage" "at_least '$usage' 0 && at_least 100 '$usage'"
read -r total used available percent <<<"$(memory_stats)"
check "memory used + available = total" "[ $((used + available)) -eq $total ]"
check "memory percent within range" "at_least '$percent' 0 && at_least 100 '$percent'"

check "PID 1 is protected" "protection_reason 1 init 0 0 >/dev/null"
check "this shell is protected" "protection_reason $$ bash 0 1 >/dev/null"
check "protected service names" "protection_reason 999 sshd 0 1 >/dev/null"
check "kernel threads are protected" "protection_reason 999 kworker 0 2 >/dev/null"
check "ordinary process is not protected" "! protection_reason 999 sleep $(id -u) 1 >/dev/null"

log_event INFO "unit test entry"
check "log entry has timestamp and level" "grep -Eq '^\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9:]{8}\] \[INFO\] unit test entry$' '$HEALTH_LOG_FILE'"
: >"$HEALTH_LOG_FILE"

echo "Integration tests (menu driven)"
run_menu 1 10
check_out "system info shows hostname" "Hostname"
check_out "system info shows kernel" "Kernel version   : $(uname -r)"
check_out "session logged" ""
check "session start logged" "grep -q 'Session started' '$HEALTH_LOG_FILE'"

run_menu 2 abc 150 90 10
check_out "invalid threshold rejected" "whole number from 1 to 100"
check_out "CPU usage shown" "Overall CPU usage"
check_out "top processes shown" "Top 5 processes by CPU usage"

run_menu 3 1 10
check_out "memory totals shown" "Total memory"
check_out "memory warning at 1%" "WARNING: Memory usage"
check "memory warning logged" "grep -q '\[WARNING\] Memory usage' '$HEALTH_LOG_FILE'"

run_menu 4 1 10
check_out "disk table shown" "Mounted on"
check_out "disk warning at 1%" "WARNING: Filesystem"

sleep 300 &
target=$!
run_menu 5 2 sleep 5 3 "$target" 5 3 abc 5 3 999999 10
check_out "search by name finds sleep" "sleep"
check_out "search by PID shows the process" "$target"
check_out "invalid PID rejected" "a PID must be a positive whole number"
check_out "missing PID handled" "No running process has PID 999999"

run_menu 6 "$target" n 10
check_out "termination can be cancelled" "Termination cancelled."
check "process still running after cancel" "kill -0 $target 2>/dev/null"

run_menu 6 "$target" y 10
check_out "process terminated" "Process $target (sleep) terminated."
wait "$target" 2>/dev/null
check "process is gone" "! kill -0 $target 2>/dev/null"

run_menu 6 1 6 999999 6 xyz 10
check_out "PID 1 is refused" "refusing to terminate PID 1"
check_out "missing PID refused" "no running process has PID 999999"
check "blocked termination logged" "grep -q 'Blocked termination of PID 1' '$HEALTH_LOG_FILE'"

run_menu 7 "" 10
check "report saved in default dir" "ls '$HEALTH_REPORT_DIR'/health_report_*.txt >/dev/null 2>&1"
report="$(find "$HEALTH_REPORT_DIR" -name 'health_report_*.txt' | head -n 1)"
check "report has timestamp" "grep -q '^Generated: ' '$report'"
check "report has all sections" "grep -q -- '--- CPU' '$report' && grep -q -- '--- Memory' '$report' && grep -q -- '--- Disk' '$report' && grep -q 'System uptime' '$report'"

touch "$WORK/not_a_dir"
run_menu 7 "$WORK/not_a_dir" 7 "$WORK/custom" 10
check_out "file used as directory rejected" "exists but is not a directory"
check "report saved in chosen dir" "ls '$WORK/custom'/health_report_*.txt >/dev/null 2>&1"

run_menu 8 10
check_out "log view lists entries" "Health report generated"

run_menu 9 n 10
check "log kept after answering no" "grep -q 'Health report generated' '$HEALTH_LOG_FILE'"
run_menu 9 maybe y 10
check_out "confirmation re-asked" "please answer y or n"
check "log cleared" "! grep -q 'Health report generated' '$HEALTH_LOG_FILE'"
check "clear action logged" "grep -q 'Activity log cleared' '$HEALTH_LOG_FILE'"

run_menu 0 11 abc 10
check_out "invalid menu options rejected" "invalid option '11'"
check_out "exit says goodbye" "Goodbye."

printf '1\n' | bash "$SCRIPT" >"$OUT" 2>&1
check "end of input exits cleanly" "[ \$? -eq 0 ]"

rm -rf "$WORK"
echo "health_monitor tests: $passed passed, $failed failed"
[ "$failed" -eq 0 ]
