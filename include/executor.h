#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "job_control.h"
#include "parser.h"

extern int is_exit;

int executor(Job *job, Job **job_head);
int block_parent_signals(sigset_t *block_list, sigset_t *prev_list, Job *job);
void handle_foreground_job(sigset_t *prev_list, Job *job, pid_t shell_pgid,
                           Job **job_haed);
void handle_background_job(sigset_t *prev_mask);
int job_is_stopped(Job *job);
int job_is_completed(Job *job);
void clear_stopped_mark(Job *job);

#endif