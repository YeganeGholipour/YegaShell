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

int execute(Job *job);
int is_buitin(Process *proc);
char *get_full_path(const char *command);
char **build_envp(void);

Variable *variable_table[TABLESIZE] = {NULL};

int last_exit_status = 0;
int is_exit = -1;

int execute(Job *job) {
  int num_procs = get_num_procs(job);
  int(*pipes)[2] = NULL;
  if (num_procs > 1) {
    pipes = malloc(sizeof *pipes * (num_procs - 1));
    if (!pipes) {
      perror("malloc for pipes failed");
      return -1;
    }
  }
  Process *proc;
  int proc_num;
  pid_t pgid = 0, shell_pgid = getpid();
  pid_t *pids = malloc(sizeof *pids * num_procs);
  if (!pids) {
    perror("malloc for pids failed");
    free(pipes);
    return -1;
  }

  // TODO: install SIGCHLD handler later

  // Block Signals In Parent Until It has done its work
  sigset_t parent_block_mask, prev_mask;
  sigemptyset(&parent_block_mask);
  sigaddset(&parent_block_mask, SIGCHLD);

  if (!job->background) {
    sigaddset(&parent_block_mask, SIGINT);
    sigaddset(&parent_block_mask, SIGQUIT);
    sigaddset(&parent_block_mask, SIGTSTP);
  }

  if (sigprocmask(SIG_BLOCK, &parent_block_mask, &prev_mask) < 0) {
    perror("sigprocmask(block) before fork");
    return -1;
  }
  printf("[main] signals blocked before fork. PID=%d\n", getpid());

  for (proc = job->first_process, proc_num = 0; proc;
       proc = proc->next, proc_num++) {
    COMMAND *cmd = proc->cmd;

    // Create a new pipe if this is not the last process
    if (proc_num < num_procs - 1) {
      if (pipe(pipes[proc_num]) < 0) {
        perror("pipe failed");
        free(pipes);
        free(pids);
        return -1;
      }
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      for (int i = 0; i < proc_num; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
      }
      free(pipes);
      free(pids);
      return -1;
    }

    if (pid == 0) {
      // ── CHILD ──

      // install the signal handlers
      struct sigaction child_signals;
      child_signals.sa_flags = SA_RESTART;
      child_signals.sa_handler = SIG_DFL;
      sigemptyset(&child_signals.sa_mask);
      sigaction(SIGINT, &child_signals, NULL);
      sigaction(SIGQUIT, &child_signals, NULL);
      sigaction(SIGTSTP, &child_signals, NULL);

      // Join process group:
      if (setpgid(0, pgid) < 0) {
        perror("child: setpgid failed");
        exit(EXIT_FAILURE);
      }

      // unblock the terminal generated signals
      if (sigprocmask(SIG_SETMASK, &prev_mask, NULL) < 0) {
        perror("sigprocmask(unblock) in child");
        exit(1);
      }

      printf("  [child] signals unblocked. PID=%d\n", getpid());

      // ── SETUP STDIN ──
      if (cmd->infile) {
        int in_fd = open(cmd->infile, O_RDONLY);
        if (in_fd < 0) {
          perror("failed to open input file");
          exit(EXIT_FAILURE);
        }
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
      } else if (proc_num > 0) {
        dup2(pipes[proc_num - 1][0], STDIN_FILENO);
        close(pipes[proc_num - 1][0]);
      }

      // ── SETUP STDOUT ──
      if (cmd->outfile) {
        int flags =
            O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int out_fd = open(cmd->outfile, flags, 0644);
        if (out_fd < 0) {
          perror("failed to open output file");
          exit(EXIT_FAILURE);
        }
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
      } else if (proc_num < num_procs - 1) {
        dup2(pipes[proc_num][1], STDOUT_FILENO);
        close(pipes[proc_num][1]);
      }

      // ── CLOSE ALL PIPE ENDS ──
      for (int i = 0; i < num_procs - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
      }

      // ── EXEC ──
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
      perror("execve failed");
      exit(EXIT_FAILURE);
    }

    // ── PARENT ──
    if (proc_num == 0) {
      pgid = pid;
      job->pgid = pgid;
    }
    if (setpgid(pid, pgid) < 0 && errno != EACCES && errno != EINVAL) {
      perror("parent: setpgid failed");
    }
    pids[proc_num] = pid;

    // ── Immediately close pipe ends the parent doesn’t need ──
    if (proc_num > 0) {
      close(pipes[proc_num - 1][0]);
    }
    if (proc_num < num_procs - 1) {
      close(pipes[proc_num][1]);
    }
  }


  // ── WAIT FOR CHILDREN ──
  if (job->background) {
    if (sigprocmask(SIG_SETMASK, &prev_mask, NULL) < 0) {
      perror("sigprocmask(restore) in parent (bg)");
    }
    printf("[parent] signals unblocked. PID=%d\n", getpid());
  } else {
    if (tcsetpgrp(STDIN_FILENO, pgid) < 0)
      perror("parent: tcsetpgrp failed");

    // unblock signals in parent

    if (sigprocmask(SIG_SETMASK, &prev_mask, NULL) < 0) {
      perror("sigprocmask(restore) in parent (fg)");
    }
    printf("[parent] signals unblocked. PID=%d\n", getpid());

    int status;
    pid_t w;

    while (1) {
      w = waitpid(-job->pgid, &status, WUNTRACED);
      if (w > 0) {
        // child either exitted or is stopped
        if (w == pids[num_procs - 1]) {
          if (WIFEXITED(status)) {
            last_exit_status = WEXITSTATUS(status);
          } else if (WIFSIGNALED(status)) {
            last_exit_status = 128 + WTERMSIG(status);
          }
        }
        continue;
      }
      if (w == 0) {
        // no more children
        break;
      }
      if (w == -1) {
        if (errno == EINTR) {
          // some other signal interrupted waitpid
          continue;
        }
        if (errno == ECHILD) {
          // truly no children left
          break;
        }
        perror("waitpid");
        break;
      }
    }

    // ── RECLAIM TERMINAL ──
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
      perror("parent: couldn’t reclaim terminal");
    }
  }

  // ── CLEANUP ──
  for (int j = 0; j < num_procs - 1; j++) {
    close(pipes[j][0]);
    close(pipes[j][1]);
  }
  free(pipes);
  free(pids);

  return 0;
}

int executor(Job *job) {
  if (!job->first_process) {
    fprintf(stderr, "Invalid command\n");
    return -1;
  }

  int func_num = is_buitin(job->first_process);
  if (func_num == -1)
    return execute(job);
  else {
    last_exit_status = builtin_commands[func_num].func(job->first_process->cmd);
    if (strcmp(builtin_commands[func_num].name, "exit") == 0)
      is_exit = last_exit_status;
    return 0;
  }
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
