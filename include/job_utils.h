/*
 * file:   job_utils.h
 * author: Yegane
 * date:   2025-06-06
 * desc:   Functionalities for job creation phase and utilities for job control
 */

#ifndef JOB_UTILS_H
#define JOB_UTILS_H

#include <sys/types.h>

#include "parser.h"
#include "process_utils.h"

typedef struct Job {
  struct Job *next;
  char *command;
  Process *first_process;
  pid_t pgid;
  pid_t *pids;
  int job_num;
  int num_procs;
  int background;
} Job;

extern struct Pending {
  pid_t pid;
  int status;
} pending_bg_jobs[256];

extern int pending_indx;

Job *initialize_job_control(char *line_buffer, Command *cmd_ptr,
                            Process *proc_ptr, Job **job_head);
void free_job(Job *job, Job **head);
int get_num_procs(Job *job);
void free_all_jobs(Job **head);
void kill_jobs(Job **job_head);
Job *find_job(Process *proc, Job **job_head);
void queue_pending_procs(pid_t pid, int status);
void mark_bg_jobs(Job **job_head, struct Pending pending_bg_jobs[],
                  int pending_count);
void drain_remaining_statuses(Job *job);
void do_job_notification(Job *job, Job **job_head);
int job_is_stopped(Job *job);
int job_is_completed(Job *job);
void clear_stopped_mark(Job *job);
void notify_bg_jobs(Job **job_head);
void format_job_info(Job *job, char *status);

#endif