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

int execute(COMMAND *cmd);
int is_buitin(COMMAND *cmd);
char *get_full_path(const char *command);
char **build_envp(void);

Variable *variable_table[TABLESIZE] = {NULL};

int last_exit_status = 0;

int execute(COMMAND *cmd) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    return -1;
  }

  if (pid == 0) {
    /* CHILD */
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    
    if (setpgid(0, 0) < 0) {
      perror("child: setpgid failed");
      exit(EXIT_FAILURE);
    }
    pid_t child_pgid = getpid();

    if (tcsetpgrp(STDIN_FILENO, child_pgid) < 0) {
      perror("child: tcsetpgrp failed");
    }

    if (cmd->infile) {
      int in_fd = open(cmd->infile, O_RDONLY);
      if (in_fd < 0) {
        perror("failed to open input file");
        exit(EXIT_FAILURE);
      }
      dup2(in_fd, STDIN_FILENO);
      close(in_fd);
    }
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
      exit(EXIT_FAILURE);
    }
    expander(cmd, envp);

    execve(full_path, cmd->argv, envp);
    perror("execve failed");
    for (int i = 0; envp[i]; i++)
      free(envp[i]);
    free(envp);
    free(full_path);
    exit(EXIT_FAILURE);

  } else {
    /* PARENT */
    if (setpgid(pid, pid) < 0 && errno != EACCES && errno != EINVAL) {
      perror("parent: setpgid failed");
    }

    if (!cmd->background) {
      if (tcsetpgrp(STDIN_FILENO, pid) < 0) {
        perror("parent: tcsetpgrp failed");
      }

      int status;
      if (waitpid(pid, &status, WUNTRACED) < 0) {
        perror("waitpid failed");
      }

      if (tcsetpgrp(STDIN_FILENO, getpgrp()) < 0) {
        perror("parent: couldn’t reclaim terminal");
      }

      if (WIFEXITED(status)) {
        last_exit_status = WEXITSTATUS(status);
      } else if (WIFSIGNALED(status)) {
        last_exit_status = 128 + WTERMSIG(status);
      } else if (WIFSTOPPED(status)) {
        last_exit_status = 128 + WSTOPSIG(status);
        fprintf(stderr, "\n[%d]+  Stopped\t%s\n", pid, cmd->argv[0]);
        /* track job if you want a proper job table */
      }
    } else {
      printf("Background job %d started\n", pid);
    }
  }
  return 0;
}

int executor(COMMAND *cmd) {
  if (!cmd || !cmd->argv || !cmd->argv[0]) {
    fprintf(stderr, "Invalid command\n");
    return -1;
  }

  int func_num = is_buitin(cmd);
  if (func_num == -1)
    return execute(cmd);
  else {
    last_exit_status = builtin_commands[func_num].func(cmd);
    return 0;
  }
}

int is_buitin(COMMAND *cmd) {
  char *command = cmd->argv[0];
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
