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
  int background;
} Job;

int handle_job_control(char *tokens[], size_t num_tokens, COMMAND **cmd_ptr,
                       Process **proc_ptr, Job **job_ptr);

char *get_raw_input(char *line_buffer);

int get_num_procs(Job *job);

#endif