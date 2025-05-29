#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"
#include "env_variable.h"
#include "executor.h"
#include "expander.h"

int execute(Job *job, Job **job_head);
int is_buitin(Process *proc);
char *get_full_path(const char *command);
char **build_envp(void);
static void exec_command(COMMAND *cmd);
static int block_parent_signals(sigset_t *block_list, sigset_t *prev_list,
                                Job *job);
static void free_pipes_and_pids(int (*pipes)[2], pid_t *pids);
static void close_pipe_ends(int num_procs, int (*pipes)[2]);
static void install_child_signal_handler();
static int child_stdin_setup(COMMAND *cmd, int (*pipes)[2], int proc_num);
static int child_stdout_setup(COMMAND *cmd, int (*pipes)[2], int proc_num,
                              int num_procs);
static void handle_foreground_job(sigset_t *prev_list, Job *job, int num_procs,
                                  int *pids, pid_t shell_pgid, pid_t pgid,
                                  Job **job_haed);
static void handle_background_job(sigset_t *prev_mask);
static int job_is_stopped(Job *job);
static int job_is_completed(Job *job);
static void do_job_notification(Job *job, Job **job_head);
void wait_for_children(Job *job, int *pids, int num_procs);

Variable *variable_table[TABLESIZE] = {NULL};

int last_exit_status = 0;
int is_exit = -1;

int execute(Job *job, Job **job_head) {
  Process *proc;
  COMMAND *cmd;
  pid_t shell_pgid = getpid(), pgid = 0;
  int num_procs = get_num_procs(job);
  int proc_num;
  int(*pipes)[2] = NULL;

  pipes = malloc(sizeof *pipes * (num_procs - 1));
  if (!pipes) {
    perror("malloc for pipes failed");
    return -1;
  }

  pid_t *pids = malloc(sizeof *pids * num_procs);
  if (!pids) {
    perror("malloc for pids failed");
    free(pipes);
    return -1;
  }

  for (int num = 0; num < num_procs - 1; num++) {
    if (pipe(pipes[num]) < 0) {
      perror("pipe failed");
      free_pipes_and_pids(pipes, pids);
      return -1;
    }
  }

  sigset_t parent_block_mask, prev_mask;
  if (block_parent_signals(&parent_block_mask, &prev_mask, job) < 0)
    return -1;

  for (proc = job->first_process, proc_num = 0; proc;
       proc = proc->next, proc_num++) {
    cmd = proc->cmd;

    pid_t pid = fork();

    if (pid < 0) {
      perror("fork failed");
      close_pipe_ends(num_procs, pipes);
      free_pipes_and_pids(pipes, pids);
      return -1;
    }

    if (pid == 0) {

      install_child_signal_handler();

      if (setpgid(0, pgid) < 0) {
        perror("child: setpgid failed");
        exit(EXIT_FAILURE);
      }

      if (sigprocmask(SIG_SETMASK, &prev_mask, NULL) < 0) {
        perror("sigprocmask(unblock) in child");
        exit(EXIT_FAILURE);
      }

      if (child_stdin_setup(cmd, pipes, proc_num) < 0) {
        perror("failed to open input file");
        exit(EXIT_FAILURE);
      }

      if (child_stdout_setup(cmd, pipes, proc_num, num_procs) < 0) {
        perror("failed to open output file");
        exit(EXIT_FAILURE);
      }

      close_pipe_ends(num_procs, pipes);
      exec_command(cmd);
      perror("execve failed");
      exit(EXIT_FAILURE);
    }

    if (proc_num == 0) {
      pgid = pid;
      job->pgid = pgid;
    }
    proc->pid = pid;
    if (setpgid(pid, pgid) < 0 && errno != EACCES && errno != EINVAL) {
      perror("parent: setpgid failed");
    }
    pids[proc_num] = pid;

    if (proc_num > 0) {
      close(pipes[proc_num - 1][0]);
    }
    if (proc_num < num_procs - 1) {
      close(pipes[proc_num][1]);
    }
  }

  if (job->background) {
    handle_background_job(&prev_mask);
  } else
    handle_foreground_job(&prev_mask, job, num_procs, pids, shell_pgid, pgid,
                          job_head);

  close_pipe_ends(num_procs, pipes);
  free_pipes_and_pids(pipes, pids);

  return 0;
}

int executor(Job *job, Job **job_head) {
  if (!job->first_process) {
    fprintf(stderr, "Invalid command\n");
    free_job(job, job_head);
    return -1;
  }

  int func_num = is_buitin(job->first_process);

  if (func_num == -1) {
    int execute_status = execute(job, job_head);
    if (execute_status < 0) {
      free_job(job, job_head);
      printf("job freed\n in execute_status < 0\n");
    }

    return execute_status;
  }

  else {
    last_exit_status = builtin_commands[func_num].func(job->first_process->cmd);
    if (strcmp(builtin_commands[func_num].name, "exit") == 0) {
      is_exit = last_exit_status;
      free_job(job, job_head);
    }
    return 0;
  }
}

/******************** THE UTILITY FUNCTIONS ********************/

static void handle_background_job(sigset_t *prev_mask) {
  if (sigprocmask(SIG_SETMASK, prev_mask, NULL) < 0) {
    perror("sigprocmask(restore) in parent (bg)");
  }
}

static void handle_foreground_job(sigset_t *prev_list, Job *job, int num_procs,
                                  int *pids, pid_t shell_pgid, pid_t pgid,
                                  Job **job_head) {

  if (tcsetpgrp(STDIN_FILENO, pgid) < 0)
    perror("parent: tcsetpgrp failed");

  pid_t current_fg = tcgetpgrp(STDIN_FILENO);
  fprintf(stderr,
          "DEBUG: after handing terminal, tcgetpgrp=%d (should == %d)\n",
          current_fg, pgid);

  wait_for_children(job, pids, num_procs);

  if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
    perror("parent: couldn't reclaim terminal");
  }

  fprintf(stderr,
          "DEBUG: shell reclaimed terminal, tcgetpgrp=%d (should == %d)\n",
          tcgetpgrp(STDIN_FILENO), shell_pgid);

  do_job_notification(job, job_head);

  if (sigprocmask(SIG_SETMASK, prev_list, NULL) < 0) {
    perror("sigprocmask(restore) in parent (fg)");
  }
}

void wait_for_children(Job *job, int *pids, int num_procs) {
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

static void format_job_info(Job *job, char *status) {
  fprintf(stderr, "[%ld]  %s    %s\n", (long)job->pgid, status, job->command);
}

static void do_job_notification(Job *job, Job **job_head) {
  if (job_is_stopped(job))
    format_job_info(job, "Stopped");

  else if (job_is_completed(job)) {
    free_job(job, job_head);
  }
}

static int job_is_stopped(Job *job) {
  Process *p;
  for (p = job->first_process; p; p = p->next)
    if (!p->stopped)
      return 0;
  return 1;
}

static int job_is_completed(Job *job) {
  Process *p;
  for (p = job->first_process; p; p = p->next)
    if (!p->completed)
      return 0;
  return 1;
}

static void exec_command(COMMAND *cmd) {
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

static int block_parent_signals(sigset_t *block_list, sigset_t *prev_list,
                                Job *job) {
  sigemptyset(block_list);
  sigaddset(block_list, SIGCHLD);

  if (!job->background) {
    sigaddset(block_list, SIGINT);
    sigaddset(block_list, SIGQUIT);
  }

  if (sigprocmask(SIG_BLOCK, block_list, prev_list) < 0) {
    perror("sigprocmask(block) before fork");
    return -1;
  }

  return 0;
}

static void free_pipes_and_pids(int (*pipes)[2], pid_t *pids) {
  free(pipes);
  free(pids);
}

static void close_pipe_ends(int num_procs, int (*pipes)[2]) {
  for (int i = 0; i < num_procs - 1; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }
}

static void install_child_signal_handler(void) {
  struct sigaction sa;
  sa.sa_flags = 0;
  sa.sa_handler = SIG_DFL;
  sigemptyset(&sa.sa_mask);

  // Make sure child uses default for SIGINT, SIGQUIT, SIGTSTP
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGTSTP, &sa, NULL);

  // Optional debug:
  char buf[100];
  int len = snprintf(buf, sizeof(buf),
                     "DEBUG(child:%d): PGID=%d, SIGTSTP=SIG_DFL now\n",
                     getpid(), getpgid(0));
  write(STDERR_FILENO, buf, len);
}

static int child_stdin_setup(COMMAND *cmd, int (*pipes)[2], int proc_num) {
  if (cmd->infile) {
    int in_fd = open(cmd->infile, O_RDONLY);
    if (in_fd < 0)
      return -1;

    dup2(in_fd, STDIN_FILENO);
    close(in_fd);
  } else if (proc_num > 0) {
    dup2(pipes[proc_num - 1][0], STDIN_FILENO);
    close(pipes[proc_num - 1][0]);
  }

  return 0;
}

static int child_stdout_setup(COMMAND *cmd, int (*pipes)[2], int proc_num,
                              int num_procs) {
  if (cmd->outfile) {
    int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
    int out_fd = open(cmd->outfile, flags, 0644);
    if (out_fd < 0)
      return -1;

    dup2(out_fd, STDOUT_FILENO);
    close(out_fd);
  } else if (proc_num < num_procs - 1) {
    dup2(pipes[proc_num][1], STDOUT_FILENO);
    close(pipes[proc_num][1]);
  }

  return 0;
}

int is_buitin(Process *proc) {
  char *command = proc->cmd->argv[0];
  for (int i = 0; builtin_commands[i].name != NULL; i++) {
    if (strcmp(command, builtin_commands[i].name) == 0)
      return i;
  }
  return -1;
}

char *get_full_path(const char *command) {
  if (strchr(command, '/')) {
    if (access(command, X_OK) == 0)
      return strdup(command);
    return NULL;
  }

  char *path = getenv("PATH");
  if (!path)
    return NULL;

  char *paths = strdup(path);
  if (!paths)
    return NULL;

  char *dir = strtok(paths, ":");
  static char full_path[512];

  while (dir) {
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
    if (access(full_path, X_OK) == 0) {
      free(paths);
      return strdup(full_path);
    }
    dir = strtok(NULL, ":");
  }

  free(paths);
  return NULL;
}

char **build_envp(void) {
  int count = 0;
  for (int i = 0; i < TABLESIZE; i++)
    for (Variable *vp = variable_table[i]; vp; vp = vp->next)
      if (vp->exported)
        count++;

  char **envp = malloc((count + 1) * sizeof(char *));
  if (!envp) {
    perror("malloc envp array");
    return NULL;
  }

  int idx = 0;
  for (int i = 0; i < TABLESIZE; i++) {
    for (Variable *vp = variable_table[i]; vp; vp = vp->next) {
      if (!vp->exported)
        continue;

      size_t len = strlen(vp->key) + 1 + strlen(vp->value) + 1;
      char *entry = malloc(len);
      if (!entry) {
        perror("malloc envp entry");
        for (int k = 0; k < idx; k++)
          free(envp[k]);
        free(envp);
        return NULL;
      }

      snprintf(entry, len, "%s=%s", vp->key, vp->value);
      envp[idx++] = entry;
    }
  }

  envp[idx] = NULL;
  return envp;
}
