/*
 * file:   job_control.h
 * author: Yegane
 * date:   2025-06-06
 * desc:   Functionality for setting up job control in execution phase
 *         Includes functions for handling foreground and background jobs.
 */

#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "job_utils.h"

extern int last_exit_status;

void setup_job_control(Job *job, Job **job_head, sigset_t *prev_mask,
                       pid_t shell_pgid);
void handle_foreground_job(sigset_t *prev_list, Job *job, pid_t shell_pgid,
                           Job **job_head);
void handle_background_job(sigset_t *prev_mask, Job *job);

#endif