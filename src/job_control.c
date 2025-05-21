#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "job_control.h"

int handle_job_control(char *tokens[], size_t num_tokens, COMMAND **cmd_ptr,
                       Process **proc_ptr, Job **job_ptr) {
  int i = 0;

  while (tokens[i] != NULL) {
    int split_indx = split_on_pipe(tokens, num_tokens, cmd_ptr, i);
    if (split_indx < 0) {
      return -1;
    }

    *proc_ptr = create_process(proc_ptr, *cmd_ptr);

    i = split_indx + 1;
  }

  *job_ptr = create_job(job_ptr);
  return 0;
}

static Job *create_job(Job **job_ptr) {
  if (*job_ptr == NULL) {
    *job_ptr = calloc(1, sizeof(Job));
  }

  (*job_ptr)->next = create_job(&(*job_ptr)->next);

  return *job_ptr;
}

static Process *create_process(Process **proc_ptr, COMMAND *cmd) {
  if (*proc_ptr == NULL) {
    *proc_ptr = calloc(1, sizeof(Process));
    (*proc_ptr)->cmd = cmd;
    return *proc_ptr;
  }

  (*proc_ptr)->next = create_process(&(*proc_ptr)->next, cmd);
  return *proc_ptr;
}

static int split_on_pipe(char *tokens[], size_t num_tokens, COMMAND **cmd_ptr,
                         int indx) {
  char *process_command[num_tokens];
  int j = 0;

  while (indx < num_tokens && tokens[indx] != NULL &&
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

char *get_raw_input(char *line_buffer) {
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

// Job *create_job(Job **job_ptr, char *line_buffer) {
//   if (*job_ptr != NULL) {
//     *job_ptr = calloc(1, sizeof(Job));
//     // get raw user command
//     (*job_ptr)->command = get_raw_input(line_buffer);
//   }

//   (*job_ptr)->next = create_job(&(*job_ptr)->next, line_buffer);
// }