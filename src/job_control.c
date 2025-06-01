#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "job_control.h"

static int split_on_pipe(char *tokens[], size_t num_tokens, COMMAND **cmd_ptr,
                         int indx);
static Job *create_job(Job **job_ptr, char *line_buffer, COMMAND *cmd);
static Process *create_process(Process **proc_ptr, COMMAND *cmd);
static char *get_raw_input(char *line_buffer);

Job *handle_job_control(char *tokens[], char *line_buffer, size_t num_tokens,
                        COMMAND **cmd_ptr, Process **proc_ptr, Job **job_head) {
  int i = 0;

  while (i < (int)num_tokens) {
    int split_indx = split_on_pipe(tokens, num_tokens, cmd_ptr, i);
    if (split_indx < 0) {
      return NULL;
    }

    *proc_ptr = create_process(proc_ptr, *cmd_ptr);

    if (split_indx < (int)num_tokens && strcmp(tokens[split_indx], "|") == 0) {
      i = split_indx + 1;
    } else {
      break;
    }
  }

  Job *new_job = create_job(job_head, line_buffer, *cmd_ptr);
  new_job->first_process = *proc_ptr;
  return new_job;
}

void kill_jobs(Job **job_head) {
  for (Job *j = *job_head; j; j = j->next) {
    kill(-j->pgid, SIGHUP);
    kill(-j->pgid, SIGCONT);
    kill(-j->pgid, SIGTERM);
  }
}

void free_all_jobs(Job **head) {
  Job *curr = *head;
  Job *next;

  while (curr) {
    next = curr->next;
    free_job(curr, head);
    curr = next;
  }
}

static void free_process_list(Process *proc) {
  Process *curr = proc;
  Process *next;

  while (curr) {
    next = curr->next;
    free_struct_memory(curr->cmd);
    free(curr);
    curr = next;
  }
}

void free_job(Job *job, Job **head) {
  Job *prev = NULL;
  Job *curr = *head;

  while (curr) {
    if (curr == job) {
      if (prev)
        prev->next = curr->next;
      else
        *head = curr->next;

      free_process_list(curr->first_process);
      free(curr->command);
      free(curr->pids);
      free(curr);
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}

static Job *create_job(Job **job_head_ptr, char *line_buffer, COMMAND *cmd) {
  if (*job_head_ptr == NULL) {
    *job_head_ptr = calloc(1, sizeof(Job));
    if (!*job_head_ptr) {
      perror("calloc for Job failed");
      return NULL;
    }
    (*job_head_ptr)->command = get_raw_input(line_buffer);
    (*job_head_ptr)->background = (cmd->background ? 1 : 0);

    return *job_head_ptr;
  }

  Job *curr = *job_head_ptr;
  while (curr->next)
    curr = curr->next;

  curr->next = calloc(1, sizeof(Job));
  if (!curr->next) {
    perror("calloc for Job failed");
    return NULL;
  }
  curr->next->command = get_raw_input(line_buffer);
  curr->next->background = (cmd->background ? 1 : 0);

  return curr->next;
}

static Process *create_process(Process **proc_ptr, COMMAND *cmd) {
  if (*proc_ptr == NULL) {
    *proc_ptr = calloc(1, sizeof(Process));
    (*proc_ptr)->cmd = cmd;
  } else
    (*proc_ptr)->next = create_process(&(*proc_ptr)->next, cmd);

  return *proc_ptr;
}

static int split_on_pipe(char *tokens[], size_t num_tokens, COMMAND **cmd_ptr,
                         int indx) {
  char *process_command[num_tokens + 1];
  int j = 0;

  while (indx < (int)num_tokens && tokens[indx] != NULL &&
         strcmp(tokens[indx], "|") != 0) {
    process_command[j++] = tokens[indx++];
  }
  process_command[j] = NULL;

  int parser_status = parse(process_command, cmd_ptr, j);
  if (parser_status < 0) {
    fprintf(stderr, "parser: error\n");
    return -1;
  }

  return indx;
}

static char *get_raw_input(char *line_buffer) {
  size_t length = strlen(line_buffer);
  char *raw_input = malloc(length + 1);

  if (raw_input == NULL) {
    perror("malloc failed");
    return NULL;
  }

  strcpy(raw_input, line_buffer);
  return raw_input;
}

int get_num_procs(Job *job) {
  int num = 0;
  Process *proc;
  for (proc = job->first_process; proc; proc = proc->next)
    num++;

  return num;
}

Job *find_job(Job *job, Job **job_head) {
  char **argv = job->first_process->cmd->argv;
  long job_num = -1;

  if (argv[1] == NULL) {
    Job *curr = *job_head;
    Job *last = NULL;
    while (curr) {
      last = curr;
      curr = curr->next;
    }
    return last;
  }

  if (argv[1][0] != '%' || argv[1][1] == '\0') {
    return NULL;
  }

  char *endptr;
  job_num = strtol(argv[1] + 1, &endptr, 10);

  if (*endptr != '\0' || job_num <= 0) {
    return NULL;
  }

  Job *curr = *job_head;
  while (curr) {
    if ((long)curr->pgid == job_num) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}
