# Project 3: Student Academic Records and Performance Analytics System

A menu-driven C program that registers students, manages their course marks,
searches and sorts records, produces academic reports and saves everything to
a text file.

## Build and run

```
cd project3-student-records
make                      # builds ./student_records
./student_records         # loads data/students.txt
./student_records my.txt  # or use another data file
```

## Tests

```
make test                 # 6 unit test programs + a menu-driven integration test
```

Memory check on Linux (Docker):

```
docker run --rm -v "$PWD":/work -w /work ubuntu:24.04 bash -c \
  'apt-get update -qq && apt-get install -y -qq gcc make valgrind >/dev/null && \
   make -s && printf "2\n11\nn\n0\n" | valgrind --leak-check=full ./student_records'
```

## Layout

| Path | Purpose |
| --- | --- |
| include/records.h, src/records.c | Student and Course structures, dynamic memory, CRUD |
| include/search.h, src/search.c | Manual linear search by ID, name, programme |
| include/sort.h, src/sort.c | Manual insertion sort by ID, name, average, programme |
| include/report.h, src/report.c | Academic statistics and report printing |
| include/storage.h, src/storage.c | Saving and loading the text data file |
| include/validation.h, src/validation.c | Input rules and strict number parsing |
| include/input.h, src/input.c | Re-asking keyboard input |
| include/display.h, src/display.c | Tables and student detail output |
| include/menu.h, src/menu.c | Menu handlers |
| src/main.c | Entry point: load file, run menu, free memory |
| data/students.txt | Sample records (8 students) |
| samples/academic_report.txt | Sample generated report |
| tests/ | Unit tests and integration test |
