#include <ctype.h>
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
    perror("fork failed\n");
    return -1;
  }

  if (pid == 0) {
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);

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
      int out_fd;
      if (cmd->append_output)
        out_fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
      else
        out_fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

      if (out_fd < 0) {
        perror("failed to open output file\n");
        exit(EXIT_FAILURE);
      }
      dup2(out_fd, STDOUT_FILENO);
      close(out_fd);
    }

    execve(full_path, cmd->argv, envp);
    perror("execve failed");

    for (int i = 0; envp[i]; i++)
      free(envp[i]);
    free(envp);
    free(full_path);
    exit(EXIT_FAILURE);

  } else {
    if (!cmd->background) {
      int status;
      waitpid(pid, &status, 0);
      if (WIFEXITED(status))
        last_exit_status = WEXITSTATUS(status);
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
