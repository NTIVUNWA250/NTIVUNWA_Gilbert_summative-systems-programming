# Project 5: Multi-threaded Document Search and Analysis System

Searches several text files at the same time with POSIX threads. Each thread
analyses one file (lines, keyword occurrences, matching line numbers,
case-insensitive) and writes its own `<name>_report.txt`.

## Build and run

```
cd project5-document-analyzer
make
./document_analyzer "IoT" documents/document1.txt documents/document2.txt documents/document3.txt
cat document1_report.txt
```

Error cases to try:

```
./document_analyzer "IoT" documents/document1.txt missing.txt   # missing file
./document_analyzer "IoT"                                       # usage message
```

Exit codes: 0 all files processed, 1 bad arguments, 2 at least one file failed.

## Tests

```
make test       # 4 unit test programs + end-to-end integration test
```

Thread and memory checks on Linux (Docker):

```
docker run --rm -v "$PWD":/work -w /work ubuntu:24.04 bash -c \
  'apt-get update -qq && apt-get install -y -qq gcc make valgrind >/dev/null && make -s && \
   valgrind --leak-check=full ./document_analyzer IoT documents/*.txt && \
   valgrind --tool=helgrind ./document_analyzer IoT documents/*.txt'
```

## Layout

| Path | Purpose |
| --- | --- |
| include/task.h, src/task.c | ThreadTask structure, argument setup, report names |
| include/search.h, src/search.c | Keyword counting and line-by-line file analysis |
| include/report.h, src/report.c | Writing each report file |
| include/worker.h, src/worker.c | Thread function, create/join, summary |
| src/main.c | Argument checks and program flow |
| documents/ | Sample input documents |
| samples/ | Sample reports and terminal output |
