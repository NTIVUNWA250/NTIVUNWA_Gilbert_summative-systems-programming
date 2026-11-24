/*
 * input.c - line-based input that re-asks until the value is valid.
 */
#include <stdio.h>
#include <string.h>

#include "input.h"
#include "textutil.h"
#include "validation.h"

#define INPUT_SIZE 128

int read_line(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    if (strchr(buffer, '\n') == NULL && !feof(stdin)) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }
        printf("  Error: input is too long.\n");
        buffer[0] = '\0';
        return 1;
    }
    buffer[strcspn(buffer, "\r\n")] = '\0';
    text_trim(buffer);
    return 1;
}

int read_text(const char *prompt, char *buffer, size_t size)
{
    for (;;) {
        if (!read_line(prompt, buffer, size)) {
            return 0;
        }
        if (valid_text_field(buffer, size)) {
            return 1;
        }
        printf("  Error: enter 1 to %zu plain characters (no '|').\n", size - 1);
    }
}

/* An empty answer keeps the current value, which the buffer already holds. */
int read_optional_text(const char *prompt, char *buffer, size_t size)
{
    char entry[INPUT_SIZE];

    for (;;) {
        if (!read_line(prompt, entry, sizeof entry)) {
            return 0;
        }
        if (entry[0] == '\0') {
            return 1;
        }
        if (valid_text_field(entry, size)) {
            text_copy(buffer, size, entry);
            return 1;
        }
        printf("  Error: enter 1 to %zu plain characters (no '|').\n", size - 1);
    }
}

int read_int(const char *prompt, int min, int max, int *out)
{
    char entry[INPUT_SIZE];
    int value;

    for (;;) {
        if (!read_line(prompt, entry, sizeof entry)) {
            return 0;
        }
        if (parse_int(entry, &value) && value >= min && value <= max) {
            *out = value;
            return 1;
        }
        printf("  Error: enter a whole number from %d to %d.\n", min, max);
    }
}

int read_optional_int(const char *prompt, int min, int max, int *out)
{
    char entry[INPUT_SIZE];
    int value;

    for (;;) {
        if (!read_line(prompt, entry, sizeof entry)) {
            return 0;
        }
        if (entry[0] == '\0') {
            return 1;
        }
        if (parse_int(entry, &value) && value >= min && value <= max) {
            *out = value;
            return 1;
        }
        printf("  Error: enter a whole number from %d to %d.\n", min, max);
    }
}

int read_double(const char *prompt, double min, double max, double *out)
{
    char entry[INPUT_SIZE];
    double value;

    for (;;) {
        if (!read_line(prompt, entry, sizeof entry)) {
            return 0;
        }
        if (parse_double(entry, &value) && value >= min && value <= max) {
            *out = value;
            return 1;
        }
        printf("  Error: enter a number from %.2f to %.2f.\n", min, max);
    }
}

int read_yes_no(const char *prompt, int *answer)
{
    char entry[INPUT_SIZE];

    for (;;) {
        if (!read_line(prompt, entry, sizeof entry)) {
            return 0;
        }
        if (text_compare_ci(entry, "y") == 0 || text_compare_ci(entry, "yes") == 0) {
            *answer = 1;
            return 1;
        }
        if (text_compare_ci(entry, "n") == 0 || text_compare_ci(entry, "no") == 0) {
            *answer = 0;
            return 1;
        }
        printf("  Error: answer y or n.\n");
    }
}
