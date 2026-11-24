#!/bin/sh
# Runs shellcheck and the test suite inside a fresh Ubuntu container, first
# as root and then as a normal user to exercise permission handling.
#   sh tests/run_in_docker.sh
set -e

cd "$(dirname "$0")/.."
docker run --rm -v "$PWD":/app:ro ubuntu:24.04 bash -c '
    set -e
    apt-get update -qq >/dev/null
    apt-get install -y -qq procps shellcheck >/dev/null 2>&1
    echo "== shellcheck =="
    shellcheck -x -P SCRIPTDIR /app/health_monitor.sh /app/tests/test_health_monitor.sh && echo "shellcheck: no issues"
    echo "== tests as root =="
    bash /app/tests/test_health_monitor.sh
    echo "== permission checks as a normal user =="
    useradd -m tester
    sleep 300 &
    root_pid=$!
    cd /tmp
    out=$(printf "6\n%s\n7\n/root/reports\n10\n" "$root_pid" |
        su tester -c "HEALTH_LOG_FILE=/tmp/tester.log bash /app/health_monitor.sh" 2>&1)
    echo "$out" | grep -q "belongs to another user" && echo "  ok   root process protected from normal user"
    echo "$out" | grep -qE "cannot create directory|do not have permission" && echo "  ok   inaccessible report directory rejected"
    kill "$root_pid"
'
