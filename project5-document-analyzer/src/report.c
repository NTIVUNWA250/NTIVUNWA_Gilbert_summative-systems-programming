/*
 * report.c - formats a task's results into its own report file.
 */
#include <stdio.h>

#include "report.h"

#define LINES_PER_ROW 10

static void write_line_numbers(FILE *out, const ThreadTask *task)
{
    if (task->matching_lines == 0) {
        fprintf(out, "Line numbers with matches: none\n");
        return;
    }
    fprintf(out, "Line numbers with matches:\n");
    for (long i = 0; i < task->matching_lines; i++) {
        fprintf(out, "%s%ld", i % LINES_PER_ROW == 0 ? "  " : ", ",
                task->match_line_numbers[i]);
        if (i % LINES_PER_ROW == LINES_PER_ROW - 1 || i == task->matching_lines - 1) {
            fprintf(out, "\n");
        }
    }
}

/* Returns 1 when the report was written completely. */
int write_report(const ThreadTask *task)
{
    FILE *out = fopen(task->report_path, "w");

    if (out == NULL) {
        return 0;
    }
    fprintf(out, "===== DOCUMENT ANALYSIS REPORT =====\n");
    fprintf(out, "Input file               : %s\n", task->input_path);
    fprintf(out, "Search keyword           : %s\n", task->keyword);
    fprintf(out, "Processed by thread      : %d\n", task->thread_number);

    if (task->status == TASK_FAILED) {
        fprintf(out, "Processing status        : FAILED\n");
        fprintf(out, "Reason                   : %s\n", task->error);
    } else {
        fprintf(out, "Total lines processed    : %ld\n", task->total_lines);
        fprintf(out, "Keyword occurrences      : %ld\n", task->occurrences);
        fprintf(out, "Lines containing keyword : %ld\n", task->matching_lines);
        write_line_numbers(out, task);
        fprintf(out, "Processing status        : SUCCESS%s\n",
                task->occurrences == 0 ? " (keyword not found)" : "");
    }
    fprintf(out, "====================================\n");

    if (ferror(out)) {
        fclose(out);
        return 0;
    }
    return fclose(out) == 0;
}
