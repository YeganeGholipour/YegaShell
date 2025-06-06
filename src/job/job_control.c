/*
 * file:   job_control.c
 * author: Yegane
 * date:   2025-06-06
 * desc:   Functionality for setting up job control in execution phase
 *         Includes functions for handling foreground and background jobs.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "job_control.h"

static void wait_for_children(Job *job, int *pids, int num_procs);

int last_exit_status = 0;

void setup_job_control(Job *job, Job **job_head, sigset_t *prev_mask,
                       pid_t shell_pgid) {
  if (job->background) {
    handle_background_job(prev_mask, job);
  } else
    handle_foreground_job(prev_mask, job, shell_pgid, job_head);
}

void handle_foreground_job(sigset_t *prev_list, Job *job, pid_t shell_pgid,
                           Job **job_head) {

  if (tcsetpgrp(STDIN_FILENO, job->pgid) < 0)
    perror("parent: tcsetpgrp failed");

  wait_for_children(job, job->pids, job->num_procs);

  drain_remaining_statuses(job);

  if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
    perror("parent: couldn't reclaim terminal");
  }

  do_job_notification(job, job_head);

  if (sigprocmask(SIG_SETMASK, prev_list, NULL) < 0) {
    perror("sigprocmask(restore) in parent (fg)");
  }
}

void handle_background_job(sigset_t *prev_mask, Job *job) {
  if (sigprocmask(SIG_SETMASK, prev_mask, NULL) < 0) {
    perror("sigprocmask(restore) in parent (bg)");
  }

  fprintf(stderr, "[%ld]  %ld\n", (long)job->job_num, (long)job->pgid);
}

static void wait_for_children(Job *job, int *pids, int num_procs) {
  int status;
  pid_t w;

  while (1) {
    w = waitpid(-job->pgid, &status, WUNTRACED);
    if (w > 0) {
      Process *p;
      for (p = job->first_process; p; p = p->next) {
        if (p->pid == w) {
          if (w == pids[num_procs - 1]) {
            if (WIFEXITED(status)) {
              last_exit_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
              last_exit_status = 128 + WTERMSIG(status);
            }
          }
          if (WIFSTOPPED(status)) {
            p->stopped = 1;
            return;
          }
          if (WIFEXITED(status) || WIFSIGNALED(status)) {
            p->completed = 1;
            p->status = status;
            break;
          }
        }
      }
      continue;
    }
    if (w == 0) {
      return;
    }
    if (w == -1) {
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