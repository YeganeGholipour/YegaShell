/*
 * file:   process_control.h
 * author: Yegane
 * date:   2025-06-06
 * desc:   Prototypes for process control functions
 *         Includes functions for setting up pipes, fork and setting up child and parent processes
 */

#ifndef PROCESS_CONTROL_H
#define PROCESS_CONTROL_H

#include <signal.h>
#include <sys/types.h>

#include "job_utils.h"
#include "process_utils.h"

typedef struct {
  int (*pipes)[2];
} JobResource;

int setup_exec_resource(Job *job, JobResource *job_res);
int create_pipes(Job *job, JobResource *job_res);
int fork_and_setup_processes(Job *job, JobResource job_res, int *pgid,
                             sigset_t *prev_mask);
void free_pipes(int (*pipes)[2]);

#endif