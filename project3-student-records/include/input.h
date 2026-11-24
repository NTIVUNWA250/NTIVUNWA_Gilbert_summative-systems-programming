/*
 * input.h - safe keyboard input. Every reader returns 1 on success and 0
 * when input has ended (EOF), so the menu can exit cleanly.
 */
#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

int read_line(const char *prompt, char *buffer, size_t size);
int read_text(const char *prompt, char *buffer, size_t size);
int read_optional_text(const char *prompt, char *buffer, size_t size);
int read_int(const char *prompt, int min, int max, int *out);
int read_optional_int(const char *prompt, int min, int max, int *out);
int read_double(const char *prompt, double min, double max, double *out);
int read_yes_no(const char *prompt, int *answer);

#endif
