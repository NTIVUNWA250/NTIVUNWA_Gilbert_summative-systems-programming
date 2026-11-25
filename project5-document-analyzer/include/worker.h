/*
 * worker.h - the thread function and the code that starts and joins threads.
 */
#ifndef WORKER_H
#define WORKER_H

#include "task.h"

void *process_file(void *arg);
int run_threads(ThreadTask *tasks, int count);
void display_results(const ThreadTask *tasks, int count);

#endif
