#!/usr/bin/env bash
#
# health_monitor.sh - Linux System Health and Resource Monitoring Manager
#
# Interactive menu to monitor CPU, memory and disk usage, inspect and
# terminate processes, generate timestamped health reports and keep an
# activity log of everything the user does.
#
# Usage:   ./health_monitor.sh
# Options: HEALTH_LOG_FILE    path of the activity log (default: logs/activity.log)
#          HEALTH_REPORT_DIR  default report directory (default: reports/)

set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="${HEALTH_LOG_FILE:-$SCRIPT_DIR/logs/activity.log}"
DEFAULT_REPORT_DIR="${HEALTH_REPORT_DIR:-$SCRIPT_DIR/reports}"

CPU_THRESHOLD=80
MEM_THRESHOLD=80
DISK_THRESHOLD=80
TOP_COUNT=5
PROTECTED_NAMES="systemd init kthreadd sshd dbus-daemon systemd-journald systemd-logind login agetty"

WARNINGS=()
LOG_WARNED=0

# ---------------------------------------------------------------- helpers

timestamp() {
    date '+%Y-%m-%d %H:%M:%S'
}

# Appends "[time] [LEVEL] message" to the activity log; warns once if it cannot.
log_event() {
    local level="$1"
    shift
    if { mkdir -p "$(dirname "$LOG_FILE")" &&
        printf '[%s] [%s] %s\n' "$(timestamp)" "$level" "$*" >>"$LOG_FILE"; } 2>/dev/null; then
        return 0
    fi
    if [ "$LOG_WARNED" -eq 0 ]; then
        echo "Warning: cannot write to the activity log at $LOG_FILE" >&2
        LOG_WARNED=1
    fi
    return 1
}

error() {
    echo "Error: $*" >&2
}

print_header() {
    echo
    echo "===== $1 ====="
}

# Returns 1 and names every command in the list that is not installed.
require_commands() {
    local missing=() cmd
    for cmd in "$@"; do
        command -v "$cmd" >/dev/null 2>&1 || missing+=("$cmd")
    done
    if [ "${#missing[@]}" -gt 0 ]; then
        error "required command(s) not available: ${missing[*]}. Install them (e.g. the procps package) and try again."
        log_event ERROR "Missing commands: ${missing[*]}"
        return 1
    fi
}

input_ended() {
    echo
    log_event INFO "Input ended; session closed"
    exit 0
}

# ask VAR "prompt" - reads one line into VAR; exits cleanly on end of input.
ask() {
    local __reply
    read -r -p "$2" __reply || input_ended
    printf -v "$1" '%s' "$__reply"
}

confirm() {
    local answer
    while true; do
        ask answer "$1 (y/n): "
        case "$answer" in
        [Yy] | [Yy][Ee][Ss]) return 0 ;;
        [Nn] | [Nn][Oo]) return 1 ;;
        *) error "please answer y or n." ;;
        esac
    done
}

pause() {
    local _ignored
    if [ -t 0 ]; then
        read -r -p "Press Enter to return to the menu..." _ignored || true
    fi
}

add_warning() {
    WARNINGS+=("$1")
}

# Prints and logs the warnings collected by the last check, then resets them.
flush_warnings() {
    local warning
    for warning in "${WARNINGS[@]}"; do
        echo "WARNING: $warning"
        log_event WARNING "$warning"
    done
    WARNINGS=()
}

# ------------------------------------------------------------- validation

is_valid_threshold() {
    [[ "$1" =~ ^[0-9]{1,3}$ ]] && [ "$((10#$1))" -ge 1 ] && [ "$((10#$1))" -le 100 ]
}

is_valid_pid() {
    [[ "$1" =~ ^[0-9]{1,9}$ ]] && [ "$((10#$1))" -gt 0 ]
}

# True when the decimal number $1 is at least $2.
at_least() {
    awk -v a="$1" -v b="$2" 'BEGIN { exit !(a + 0 >= b + 0) }'
}

# read_threshold VAR "label" - asks until a value from 1 to 100 is given.
read_threshold() {
    local reply current="${!1}"
    while true; do
        ask reply "Enter the $2 warning threshold in % (1-100) [$current]: "
        if [ -z "$reply" ]; then
            return 0
        fi
        if is_valid_threshold "$reply"; then
            printf -v "$1" '%d' "$((10#$reply))"
            return 0
        fi
        error "the threshold must be a whole number from 1 to 100."
        log_event ERROR "Invalid $2 threshold entered: $reply"
    done
}

# ------------------------------------------------------- data collection

get_os_name() {
    if [ -r /etc/os-release ]; then
        # shellcheck source=/dev/null
        (. /etc/os-release && echo "${PRETTY_NAME:-${NAME:-Linux}}")
    else
        uname -s
    fi
}

get_uptime() {
    if uptime -p >/dev/null 2>&1; then
        uptime -p
    elif [ -r /proc/uptime ]; then
        awk '{ s = int($1); printf "up %d days, %d hours, %d minutes\n", s / 86400, (s % 86400) / 3600, (s % 3600) / 60 }' /proc/uptime
    else
        echo "unknown"
    fi
}

cpu_times() {
    awk '/^cpu / { total = 0; for (i = 2; i <= 9; i++) total += $i; print total, $5 + $6; exit }' /proc/stat
}

# Overall CPU busy percentage measured over one second from /proc/stat.
cpu_usage_percent() {
    local total1 idle1 total2 idle2
    [ -r /proc/stat ] || return 1
    read -r total1 idle1 < <(cpu_times)
    sleep 1
    read -r total2 idle2 < <(cpu_times)
    awk -v dt="$((total2 - total1))" -v di="$((idle2 - idle1))" \
        'BEGIN { if (dt <= 0) print "0.0"; else printf "%.1f\n", (dt - di) * 100 / dt }'
}

# Prints "total_kb used_kb available_kb percent" from /proc/meminfo.
memory_stats() {
    [ -r /proc/meminfo ] || return 1
    awk '/^MemTotal:/ { t = $2 } /^MemAvailable:/ { a = $2 }
         END { if (t > 0) printf "%d %d %d %.1f\n", t, t - a, a, (t - a) * 100 / t; else exit 1 }' /proc/meminfo
}

kb_to_human() {
    awk -v kb="$1" 'BEGIN { if (kb >= 1048576) printf "%.2f GiB", kb / 1048576; else printf "%.1f MiB", kb / 1024 }'
}

disk_usage() {
    df -hP -x tmpfs -x devtmpfs -x squashfs 2>/dev/null || df -hP 2>/dev/null
}

# ------------------------------------------------------- status printers

print_system_info() {
    echo "Hostname         : $(hostname 2>/dev/null || cat /etc/hostname 2>/dev/null || echo unknown)"
    echo "Operating system : $(get_os_name)"
    echo "Kernel version   : $(uname -r)"
    echo "Architecture     : $(uname -m)"
    echo "System uptime    : $(get_uptime)"
    echo "Current date/time: $(date '+%A %d %B %Y, %H:%M:%S %Z')"
}

print_cpu_status() {
    local usage cores
    cores="$(nproc 2>/dev/null || grep -c '^processor' /proc/cpuinfo 2>/dev/null || echo unknown)"
    echo "CPU cores             : $cores"
    if [ -r /proc/loadavg ]; then
        echo "Load average (1/5/15) : $(awk '{ print $1 ", " $2 ", " $3 }' /proc/loadavg)"
    fi
    if usage="$(cpu_usage_percent)"; then
        echo "Overall CPU usage     : ${usage}%  (threshold ${CPU_THRESHOLD}%)"
        if at_least "$usage" "$CPU_THRESHOLD"; then
            add_warning "Overall CPU usage ${usage}% reached the ${CPU_THRESHOLD}% threshold"
        fi
    else
        error "cannot read CPU statistics from /proc/stat."
    fi

    if command -v ps >/dev/null 2>&1; then
        echo
        echo "Top $TOP_COUNT processes by CPU usage:"
        ps -eo pid,user,%cpu,%mem,comm --sort=-%cpu | head -n "$((TOP_COUNT + 1))"
        while read -r pid cpu name; do
            add_warning "Process $name (PID $pid) is using ${cpu}% CPU (threshold ${CPU_THRESHOLD}%)"
        done < <(ps -eo pid=,%cpu=,comm= --sort=-%cpu | awk -v t="$CPU_THRESHOLD" '$2 + 0 >= t { print $1, $2, $3 }')
    else
        echo "(process list unavailable: ps is not installed)"
    fi
}

print_memory_status() {
    local stats total used available percent
    if ! stats="$(memory_stats)"; then
        error "cannot read memory statistics from /proc/meminfo."
        return 1
    fi
    read -r total used available percent <<<"$stats"
    echo "Total memory     : $(kb_to_human "$total")"
    echo "Used memory      : $(kb_to_human "$used")"
    echo "Available memory : $(kb_to_human "$available")"
    echo "Memory usage     : ${percent}%  (threshold ${MEM_THRESHOLD}%)"
    if at_least "$percent" "$MEM_THRESHOLD"; then
        add_warning "Memory usage ${percent}% reached the ${MEM_THRESHOLD}% threshold"
    fi
}

print_disk_status() {
    local output
    if ! output="$(disk_usage)"; then
        error "cannot read disk usage (df failed)."
        return 1
    fi
    printf '%s\n' "$output" | awk 'NR == 1 { printf "%-28s %8s %8s %8s %6s  %s\n", "Filesystem", "Size", "Used", "Avail", "Use%", "Mounted on"; next }
                                  { printf "%-28s %8s %8s %8s %6s  %s\n", $1, $2, $3, $4, $5, $6 }'
    while read -r mount use fs; do
        add_warning "Filesystem $fs on $mount is ${use}% full (threshold ${DISK_THRESHOLD}%)"
    done < <(printf '%s\n' "$output" | awk -v t="$DISK_THRESHOLD" 'NR > 1 { u = $5; sub("%", "", u); if (u + 0 >= t) print $6, u, $1 }')
}

# --------------------------------------------------------- menu actions

show_system_info() {
    print_header "SYSTEM INFORMATION"
    print_system_info
    log_event INFO "Displayed system information"
}

monitor_cpu() {
    print_header "CPU UTILIZATION"
    read_threshold CPU_THRESHOLD "CPU"
    print_cpu_status
    if [ "${#WARNINGS[@]}" -eq 0 ]; then
        echo "OK: nothing reached the ${CPU_THRESHOLD}% CPU threshold."
    fi
    log_event INFO "CPU check completed (threshold ${CPU_THRESHOLD}%, ${#WARNINGS[@]} warning(s))"
    flush_warnings
}

monitor_memory() {
    print_header "MEMORY UTILIZATION"
    read_threshold MEM_THRESHOLD "memory"
    print_memory_status || return
    if [ "${#WARNINGS[@]}" -eq 0 ]; then
        echo "OK: memory usage is below the ${MEM_THRESHOLD}% threshold."
    fi
    log_event INFO "Memory check completed (threshold ${MEM_THRESHOLD}%, ${#WARNINGS[@]} warning(s))"
    flush_warnings
}

monitor_disk() {
    print_header "DISK UTILIZATION"
    read_threshold DISK_THRESHOLD "disk"
    print_disk_status || return
    if [ "${#WARNINGS[@]}" -eq 0 ]; then
        echo "OK: no filesystem reached the ${DISK_THRESHOLD}% threshold."
    fi
    log_event INFO "Disk check completed (threshold ${DISK_THRESHOLD}%, ${#WARNINGS[@]} warning(s))"
    flush_warnings
}

list_processes() {
    echo "Total running processes: $(ps -e --no-headers | wc -l)"
    echo "Top 20 by CPU usage:"
    ps -eo pid,user,%cpu,%mem,etime,comm --sort=-%cpu | head -n 21
    log_event INFO "Listed running processes"
}

search_process_by_name() {
    local name result
    ask name "Process name (or part of it): "
    if [[ ! "$name" =~ ^[A-Za-z0-9._:+-]+$ ]]; then
        error "enter a name using letters, digits and . _ : + - only."
        return
    fi
    result="$(ps -eo pid,user,%cpu,%mem,etime,comm --sort=-%cpu |
        awk -v n="$name" 'NR == 1 { header = $0; next } index(tolower($6), tolower(n)) { if (!found) print header; print; found = 1 } END { exit !found }')"
    if [ -n "$result" ]; then
        printf '%s\n' "$result"
        log_event INFO "Process search by name '$name': $(($(printf '%s\n' "$result" | wc -l) - 1)) match(es)"
    else
        echo "No running process matches '$name'."
        log_event INFO "Process search by name '$name': no matches"
    fi
}

search_process_by_pid() {
    local pid
    ask pid "PID: "
    if ! is_valid_pid "$pid"; then
        error "a PID must be a positive whole number."
        log_event ERROR "Invalid PID entered for search: $pid"
        return
    fi
    if ps -p "$((10#$pid))" -o pid,ppid,user,%cpu,%mem,etime,comm 2>/dev/null; then
        log_event INFO "Process search by PID $pid: found"
    else
        echo "No running process has PID $pid."
        log_event INFO "Process search by PID $pid: not found"
    fi
}

view_processes() {
    local choice
    print_header "RUNNING PROCESSES"
    require_commands ps || return
    echo "1. List running processes"
    echo "2. Search by process name"
    echo "3. Search by PID"
    echo "0. Back to main menu"
    ask choice "Choice: "
    case "$choice" in
    1) list_processes ;;
    2) search_process_by_name ;;
    3) search_process_by_pid ;;
    0) return ;;
    *) error "invalid choice '$choice'." ;;
    esac
}

# Prints why a process must not be terminated, or returns 1 if it may be.
protection_reason() {
    local pid="$1" name="$2" uid="$3" ppid="$4" protected
    if [ "$pid" -le 2 ]; then
        echo "PID $pid is a core system process"
        return 0
    fi
    if [ "$pid" -eq "$$" ] || [ "$pid" -eq "$PPID" ]; then
        echo "it is this monitor or the shell that started it"
        return 0
    fi
    if [ "$ppid" -eq 2 ]; then
        echo "it is a kernel thread"
        return 0
    fi
    for protected in $PROTECTED_NAMES; do
        if [ "$name" = "$protected" ]; then
            echo "$name is a protected system service"
            return 0
        fi
    done
    if [ "$(id -u)" -ne 0 ] && [ "$uid" -ne "$(id -u)" ]; then
        echo "it belongs to another user and you do not have permission"
        return 0
    fi
    return 1
}

terminate_process() {
    local pid name uid ppid reason
    print_header "TERMINATE A PROCESS"
    require_commands ps kill || return
    ask pid "PID of the process to terminate: "
    if ! is_valid_pid "$pid"; then
        error "a PID must be a positive whole number."
        log_event ERROR "Invalid PID entered for termination: $pid"
        return
    fi
    pid="$((10#$pid))"
    if ! ps -p "$pid" >/dev/null 2>&1; then
        error "no running process has PID $pid."
        log_event ERROR "Termination failed: PID $pid does not exist"
        return
    fi
    read -r name uid ppid < <(ps -p "$pid" -o comm=,uid=,ppid=)
    ps -p "$pid" -o pid,ppid,user,%cpu,%mem,etime,comm

    if reason="$(protection_reason "$pid" "$name" "$uid" "$ppid")"; then
        error "refusing to terminate PID $pid ($name): $reason."
        log_event WARNING "Blocked termination of PID $pid ($name): $reason"
        return
    fi
    if ! confirm "Terminate PID $pid ($name)?"; then
        echo "Termination cancelled."
        log_event INFO "Termination of PID $pid ($name) cancelled by user"
        return
    fi
    if ! kill -TERM "$pid" 2>/dev/null; then
        error "could not signal PID $pid (permission denied or it already exited)."
        log_event ERROR "Termination of PID $pid ($name) failed"
        return
    fi
    sleep 1
    if ps -p "$pid" >/dev/null 2>&1 && confirm "PID $pid is still running. Force it to stop (SIGKILL)?"; then
        kill -KILL "$pid" 2>/dev/null
        sleep 1
    fi
    if ps -p "$pid" >/dev/null 2>&1; then
        echo "PID $pid ($name) is still running."
        log_event WARNING "PID $pid ($name) did not stop after SIGTERM"
    else
        echo "Process $pid ($name) terminated."
        log_event INFO "Terminated process PID $pid ($name)"
    fi
}

write_report() {
    echo "=========== SYSTEM HEALTH REPORT ==========="
    echo "Generated: $(timestamp)"
    echo "Generated by: $(id -un 2>/dev/null || echo unknown)"
    echo
    echo "--- System ---"
    print_system_info
    echo
    echo "--- CPU (threshold ${CPU_THRESHOLD}%) ---"
    print_cpu_status
    echo
    echo "--- Memory (threshold ${MEM_THRESHOLD}%) ---"
    print_memory_status
    echo
    echo "--- Disk (threshold ${DISK_THRESHOLD}%) ---"
    print_disk_status
    echo
    echo "--- Warnings ---"
    if [ "${#WARNINGS[@]}" -eq 0 ]; then
        echo "None"
    else
        printf 'WARNING: %s\n' "${WARNINGS[@]}"
    fi
    echo "============================================"
}

generate_report() {
    local dir file warning
    print_header "SYSTEM HEALTH REPORT"
    ask dir "Directory to save the report [$DEFAULT_REPORT_DIR]: "
    dir="${dir:-$DEFAULT_REPORT_DIR}"
    dir="${dir/#\~/$HOME}"

    if [ -e "$dir" ] && [ ! -d "$dir" ]; then
        error "$dir exists but is not a directory."
        log_event ERROR "Report not generated: $dir is not a directory"
        return
    fi
    if [ ! -d "$dir" ] && ! mkdir -p "$dir" 2>/dev/null; then
        error "cannot create directory $dir (check the path and your permissions)."
        log_event ERROR "Report not generated: cannot create $dir"
        return
    fi
    if [ ! -w "$dir" ] || [ ! -x "$dir" ]; then
        error "you do not have permission to write in $dir."
        log_event ERROR "Report not generated: no write permission for $dir"
        return
    fi

    file="$dir/health_report_$(date '+%Y%m%d_%H%M%S').txt"
    echo "Collecting data..."
    if ! write_report >"$file" 2>/dev/null; then
        error "failed to write $file."
        log_event ERROR "Report could not be written to $file"
        rm -f "$file"
        WARNINGS=()
        return
    fi
    for warning in "${WARNINGS[@]}"; do
        log_event WARNING "$warning"
    done
    WARNINGS=()
    echo "Report saved to $file"
    log_event INFO "Health report generated: $file"
}

view_log() {
    local lines
    print_header "ACTIVITY HISTORY"
    if [ ! -e "$LOG_FILE" ] || [ ! -s "$LOG_FILE" ]; then
        echo "The activity log is empty."
        return
    fi
    if [ ! -r "$LOG_FILE" ]; then
        error "you do not have permission to read $LOG_FILE."
        return
    fi
    lines="$(wc -l <"$LOG_FILE" | tr -d ' ')"
    echo "Log file: $LOG_FILE ($lines entries, showing the last 50)"
    tail -n 50 "$LOG_FILE"
    log_event INFO "Viewed activity log"
}

clear_log() {
    print_header "CLEAR ACTIVITY LOG"
    if [ ! -s "$LOG_FILE" ]; then
        echo "The activity log is already empty."
        return
    fi
    if ! confirm "Delete all $(wc -l <"$LOG_FILE" | tr -d ' ') log entries?"; then
        echo "The activity log was not cleared."
        log_event INFO "Clear log cancelled by user"
        return
    fi
    if ! : >"$LOG_FILE" 2>/dev/null; then
        error "you do not have permission to clear $LOG_FILE."
        return
    fi
    echo "Activity log cleared."
    log_event INFO "Activity log cleared by $(id -un 2>/dev/null || echo unknown)"
}

# ----------------------------------------------------------------- main

show_menu() {
    echo
    echo "========== LINUX SYSTEM HEALTH MONITOR =========="
    echo " 1. Display system information"
    echo " 2. Monitor CPU utilization"
    echo " 3. Monitor memory utilization"
    echo " 4. Monitor disk utilization"
    echo " 5. View and search running processes"
    echo " 6. Terminate a process"
    echo " 7. Generate a system health report"
    echo " 8. View activity history"
    echo " 9. Clear activity log"
    echo "10. Exit"
}

check_environment() {
    if [ "$(uname -s)" != "Linux" ]; then
        error "this tool must run on Linux (detected $(uname -s))."
        exit 1
    fi
    require_commands awk df date uname || exit 1
    if ! command -v ps >/dev/null 2>&1; then
        echo "Note: ps is not installed, so process features are disabled (install procps)."
    fi
}

main() {
    local choice
    check_environment
    log_event INFO "Session started by $(id -un 2>/dev/null || echo unknown)"
    while true; do
        show_menu
        ask choice "Select an option (1-10): "
        case "$choice" in
        1) show_system_info ;;
        2) monitor_cpu ;;
        3) monitor_memory ;;
        4) monitor_disk ;;
        5) view_processes ;;
        6) terminate_process ;;
        7) generate_report ;;
        8) view_log ;;
        9) clear_log ;;
        10)
            log_event INFO "Session ended"
            echo "Goodbye."
            exit 0
            ;;
        *)
            error "invalid option '$choice'. Enter a number from 1 to 10."
            log_event ERROR "Invalid menu option: $choice"
            continue
            ;;
        esac
        pause
    done
}

# Run the menu only when executed, so tests can source the functions.
if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
    main "$@"
fi
