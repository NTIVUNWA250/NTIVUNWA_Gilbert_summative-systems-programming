# Project 2: Linux System Health and Resource Monitoring Manager

An interactive Bash script that monitors CPU, memory, disk and processes,
terminates processes safely, writes timestamped health reports and keeps an
activity log.

## Run on Linux

```
cd project2-linux-health-monitor
chmod +x health_monitor.sh
./health_monitor.sh
```

The activity log is written to `logs/activity.log` and reports to `reports/`
by default (set `HEALTH_LOG_FILE` or `HEALTH_REPORT_DIR` to change them).

## Run on macOS through Docker (Ubuntu)

```
cd project2-linux-health-monitor
docker run -it --rm -v "$PWD":/app -w /app ubuntu:24.04 bash -c \
  'apt-get update -qq && apt-get install -y -qq procps >/dev/null && bash'
# inside the container:
sleep 600 &            # a safe process to search for and terminate
./health_monitor.sh
```

## Tests

```
sh tests/run_in_docker.sh   # shellcheck + 59 tests as root + permission tests as a normal user
```

## Files

| Path | Purpose |
| --- | --- |
| health_monitor.sh | The application |
| tests/test_health_monitor.sh | Unit tests (sourced functions) and menu-driven integration tests |
| tests/run_in_docker.sh | Runs shellcheck and the tests in a fresh Ubuntu container |
| samples/sample_health_report.txt | Report generated on Ubuntu 24.04 |
| samples/sample_activity.log | Activity log from the same session |
