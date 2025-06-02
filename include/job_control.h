#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <sys/types.h>

#include "parser.h"

typedef struct Process {
  struct Process *next;
  COMMAND *cmd;
  pid_t pid;
  int completed;
  int stopped;
  int status;
} Process;

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

Job *handle_job_control(char *line_buffer, COMMAND *cmd_ptr, Process *proc_ptr,
                        Job **job_head);
Process *handle_processes(char *tokens[], size_t num_tokens, COMMAND **cmd_ptr,
                          Process **proc_ptr);
void free_job(Job *job, Job **head);
void free_process_list(Process *proc);
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