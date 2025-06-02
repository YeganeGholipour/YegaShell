#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <wait.h>

#include "job_control.h"

static Job *create_job(Job **job_ptr, char *line_buffer, COMMAND *cmd);
static Process *create_process(Process **proc_ptr, COMMAND *cmd);

int pending_indx = 0;
struct Pending pending_bg_jobs[256] = {0};

Job *handle_job_control(char *tokens[], char *line_buffer, size_t num_tokens,
                        COMMAND **cmd_ptr, Process **proc_ptr, Job **job_head) {
  int i = 0;

  // see if bakground job character is valid
  if (is_background_char_valid(tokens, num_tokens) == 0)
    return NULL;

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

void queue_pending_procs(pid_t pid, int status) {
  pending_bg_jobs[pending_indx].pid = pid;
  pending_bg_jobs[pending_indx].status = status;
  pending_indx++;
}

void mark_bg_jobs(Job **job_head, struct Pending pending_bg_jobs[],
                  int pending_count) {
  for (int i = 0; i < pending_count; i++) {
    pid_t pid = pending_bg_jobs[i].pid;
    int status = pending_bg_jobs[i].status;
    int found = 0;

    for (Job *job = *job_head; job && !found; job = job->next) {
      for (Process *p = job->first_process; p; p = p->next) {
        if (p->pid == pid) {
          if (WIFSTOPPED(status)) {
            p->stopped = 1;
          } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
            p->completed = 1;
            p->status = status;
          }
          found = 1;
          break;
        }
      }
    }
  }

  pending_indx = 0;
}

void format_job_info(Job *job, char *status) {
  if (job->background)
    fprintf(stderr, "[%ld]  %s      %s &\n", (long)job->pgid, status, job->command);
  else
    fprintf(stderr, "[%ld]  %s      %s\n", (long)job->pgid, status, job->command);
}

void drain_remaining_statuses(Job *job) {
  pid_t w;
  int status;

  while ((w = waitpid(-job->pgid, &status, WNOHANG | WUNTRACED))) {
    if (w > 0) {
      Process *p;
      for (p = job->first_process; p; p = p->next) {
        if (p->pid == w) {
          if (WIFSTOPPED(status)) {
            p->stopped = 1;
            break;
          }
          if (WIFEXITED(status) || WIFSIGNALED(status)) {
            p->completed = 1;
            p->status = status;
            break;
          }
        }
      }
      continue;

    } else if (w == 0)
      return;

    else if (w == -1) {
      if (errno == EINTR) {
        continue;
      }
      if (errno == ECHILD) {
        return;
      }
      perror("waitpid");
      return;
    }
  }
}

void do_job_notification(Job *job, Job **job_head) {
  if (job_is_completed(job)) {
    if (job->background)
      format_job_info(job, "Done");
    free_job(job, job_head);
  } else if (job_is_stopped(job))
    format_job_info(job, "Stopped");
}

void notify_bg_jobs(Job **job_head) {
  Job *j = *job_head;
  Job *next = NULL;
  while (j) {
    next = j->next;
    do_job_notification(j, job_head);
    j = next;
  }
}

int job_is_stopped(Job *job) {
  Process *p;
  for (p = job->first_process; p; p = p->next)
    if (!p->stopped && !p->completed)
      return 0;
  return 1;
}

int job_is_completed(Job *job) {
  Process *p;
  for (p = job->first_process; p; p = p->next)
    if (!p->completed)
      return 0;
  return 1;
}

void clear_stopped_mark(Job *job) {
  Process *p;
  for (p = job->first_process; p; p = p->next) {
    p->stopped = 0;
  }
}