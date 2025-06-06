/**
 * @file process_control.c
 * @brief Handles forking and setting up child and parent processes.
 *         Includes utilities for creating and freeing pipes.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "env_utils.h"
#include "expander.h"
#include "io_redirection.h"
#include "job_utils.h"
#include "process_control.h"
#include "signal_utils.h"

static void exec_command(Command *cmd);
static int allocate_pipe(Job *job, JobResource *job_res);
static int allocate_pids(Job *job);
static void child_setup(pid_t pgid, sigset_t *prev_mask, int (*pipes)[],
                        int proc_num, Command *cmd, Job *job);
static void parent_setup(pid_t *pgid, int pid, int proc_num, int (*pipes)[2],
                         Process *proc, Job *job);

int setup_exec_resource(Job *job, JobResource *job_res) {
  job->num_procs = get_num_procs(job);

  if (allocate_pipe(job, job_res) < 0)
    return -1;

  if (allocate_pids(job) < 0) {
    free(job_res->pipes);
    return -1;
  }

  return 0;
}

int create_pipes(Job *job, JobResource *job_res) {
  for (int num = 0; num < job->num_procs - 1; num++) {
    if (pipe(job_res->pipes[num]) < 0) {
      perror("pipe failed");
      free_pipes(job_res->pipes);
      return -1;
    }
  }
  return 0;
}

int fork_and_setup_processes(Job *job, JobResource job_res, int *pgid,
                             sigset_t *prev_mask) {
  Process *proc;
  int proc_num;
  Command *cmd;

  for (proc = job->first_process, proc_num = 0; proc;
       proc = proc->next, proc_num++) {
    cmd = proc->cmd;

    pid_t pid = fork();

    if (pid < 0) {
      perror("fork failed");
      close_pipe_ends(job->num_procs, job_res.pipes);
      free_pipes(job_res.pipes);
      return -1;
    }

    if (pid == 0)
      child_setup(*pgid, prev_mask, job_res.pipes, proc_num, cmd, job);
    else
      parent_setup(pgid, pid, proc_num, job_res.pipes, proc, job);
  }
  return 0;
}

static int allocate_pipe(Job *job, JobResource *job_res) {
  int(*pipes)[2];

  pipes = malloc(sizeof *pipes * (job->num_procs - 1));
  if (pipes == NULL) {
    perror("malloc for pipes failed");
    return -1;
  }
  job_res->pipes = pipes;

  return 0;
}

static int allocate_pids(Job *job) {
  pid_t *pids;

  pids = malloc(sizeof(pid_t) * job->num_procs);
  if (pids == NULL) {
    perror("malloc for pids failed");
    return -1;
  }

  job->pids = pids;
  return 0;
}

static void child_setup(pid_t pgid, sigset_t *prev_mask, int (*pipes)[],
                        int proc_num, Command *cmd, Job *job) {
  install_child_signal_handler();

  if (setpgid(0, pgid) < 0) {
    perror("child: setpgid failed");
    exit(EXIT_FAILURE);
  }

  if (sigprocmask(SIG_SETMASK, prev_mask, NULL) < 0) {
    perror("sigprocmask(unblock) in child");
    exit(EXIT_FAILURE);
  }

  if (child_stdin_setup(cmd, pipes, proc_num) < 0) {
    perror("failed to open input file");
    exit(EXIT_FAILURE);
  }

  if (child_stdout_setup(cmd, pipes, proc_num, job->num_procs) < 0) {
    perror("failed to open output file");
    exit(EXIT_FAILURE);
  }

  close_pipe_ends(job->num_procs, pipes);
  exec_command(cmd);
  perror("execve failed");
  exit(EXIT_FAILURE);
}

static void parent_setup(pid_t *pgid, int pid, int proc_num, int (*pipes)[2],
                         Process *proc, Job *job) {
  if (proc_num == 0) {
    *pgid = pid;
    job->pgid = *pgid;
  }
  proc->pid = pid;
  if (setpgid(pid, *pgid) < 0 && errno != EACCES && errno != EINVAL) {
    perror("parent: setpgid failed");
  }
  job->pids[proc_num] = pid;

  if (proc_num > 0) {
    close(pipes[proc_num - 1][0]);
  }
  if (proc_num < job->num_procs - 1) {
    close(pipes[proc_num][1]);
  }
}

static void exec_command(Command *cmd) {
  char *full_path = get_full_path(cmd->argv[0]);
  if (!full_path) {
    fprintf(stderr, "%s: command not found\n", cmd->argv[0]);
    exit(EXIT_FAILURE);
  }
  char **envp = build_envp();
  if (!envp) {
    fprintf(stderr, "Failed to build environment\n");
    free(full_path);
    free(envp);
    exit(EXIT_FAILURE);
  }
  expander(cmd, envp);

  execve(full_path, cmd->argv, envp);
}
