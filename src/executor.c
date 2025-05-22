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

  int syncpipe[2];
  int proc_num;
  Process *proc;
  pid_t pgid = 0;
  pid_t shell_pgid = getpid();

  pid_t *pids = malloc(sizeof *pids * num_procs);
  if (!pids) {
    perror("malloc for pids failed");
    free(pipes);
    return -1;
  }

  sigset_t mask_all, prev_mask;
  sigfillset(&mask_all);

  if (!job->background) {
    sigaddset(&mask_all, SIGINT);
    sigaddset(&mask_all, SIGQUIT);
    sigaddset(&mask_all, SIGTSTP);
  }
  sigaddset(&mask_all, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);

  if (pipe(syncpipe) < 0) {
    perror("pipe failed");
    free(pipes);
    free(pids);
    return -1;
  }

  fcntl(syncpipe[0], F_SETFD, FD_CLOEXEC);

  for (proc = job->first_process, proc_num = 0; proc;
       proc = proc->next, proc_num++) {
    COMMAND *cmd = proc->cmd;

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
      return -1;
    }

    /******************************/
    /* ------ CHILD PROCESS ------*/
    /******************************/
    if (pid == 0) {
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);
      signal(SIGCHLD, SIG_DFL);
      // TODO: add a SIGCHLD handler

      close(syncpipe[1]);
      char buf;
      read(syncpipe[0], &buf, 1);

      if (setpgid(0, pgid) < 0) {
        perror("child: setpgid failed");
        exit(EXIT_FAILURE);
      }

      /* STDIN SETUP*/

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
      }

      /* STDOUT SETUP */

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
      }

      for (int i = 0; i < num_procs - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
      }

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
      for (int i = 0; envp[i]; i++)
        free(envp[i]);
      free(envp);
      free(full_path);
      close(syncpipe[0]);
      exit(EXIT_FAILURE);
    }

    /**************************************/
    /* ------ PARENT PROCESS IN LOOP -----*/
    /**************************************/

    if (proc_num == 0) {
      pgid = pid;
      job->pgid = pgid;
    }

    if (setpgid(pid, pgid) < 0 && errno != EACCES && errno != EINVAL) {
      perror("parent: setpgid failed");
    }

    pids[proc_num] = pid;
  }

  /**********************************************/
  /* ------ PARENT PROCESS OUTSIDE OF LOOP -----*/
  /**********************************************/

  close(syncpipe[0]);
  for (int i = 0; i < num_procs; i++) {
    write(syncpipe[1], " ", 1);
  }
  close(syncpipe[1]);

  if (!job->background) {
    if (tcsetpgrp(STDIN_FILENO, pgid) < 0)
      perror("parent: tcsetpgrp failed");

    sigprocmask(SIG_SETMASK, &prev_mask, NULL);

    int status;
    for (int i = 0; i < num_procs; i++) {
      if (waitpid(pids[i], &status, WUNTRACED) < 0) {
        perror("waitpid failed");
      }
      if (i == num_procs - 1) {
        if (WIFEXITED(status)) {
          last_exit_status = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
          last_exit_status = 128 + WTERMSIG(status);
        }
        // TODO: implement sigstop and sigcont
      }
    }

    if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
      perror("parent: couldn’t reclaim terminal");
    }
  } else {
    // background
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
  }

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

// if (!cmd->background) {
//       if (tcsetpgrp(STDIN_FILENO, pid) < 0) {
//         perror("parent: tcsetpgrp failed");
//       }

//       int status;
//       if (waitpid(pid, &status, WUNTRACED) < 0) {
//         perror("waitpid failed");
//       }

//       if (tcsetpgrp(STDIN_FILENO, getpgrp()) < 0) {
//         perror("parent: couldn’t reclaim terminal");
//       }

//       if (WIFEXITED(status)) {
//         last_exit_status = WEXITSTATUS(status);
//       } else if (WIFSIGNALED(status)) {
//         last_exit_status = 128 + WTERMSIG(status);
//       } else if (WIFSTOPPED(status)) {
//         last_exit_status = 128 + WSTOPSIG(status);
//         fprintf(stderr, "\n[%d]+  Stopped\t%s\n", pid, cmd->argv[0]);
//         /* track job if you want a proper job table */
//       }
//     }