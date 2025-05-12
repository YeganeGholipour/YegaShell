#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtin.h"
#include "env_variable.h"
#include "executor.h"

Builtin builtin_commands[] = {{"cd", cd_func},
                              {"help", help_func},
                              {"exit", exit_func},
                              {"pwd", pwd_func},
                              {"export", export_func}};

int cd_func(COMMAND *cmd) {
  const char *path;

  if (cmd->argv[1] == NULL || strcmp(cmd->argv[1], "~") == 0) {
    path = getenv("HOME");
    if (path == NULL) {
      fprintf(stderr, "cd: HOME not set\n");
      return -1;
    }
  } else {
    path = cmd->argv[1];
  }

  if (chdir(path) != 0) {
    perror("cd");
    return -1;
  }

  return 0;
}

int help_func(COMMAND *cmd) {
  printf("Yega Shell\n");
  printf("Type the name of the command, and hit enter.\n");
  printf("Use the man command for information on other programs.\n");

  return 0;
}

int exit_func(COMMAND *cmd) {
  int status = 0;
  if (cmd->argv[1])
    status = atoi(cmd->argv[1]);
  exit(status);
}

int pwd_func(COMMAND *cmd) {
  char *cwd = getcwd(NULL, 0);
  if (cwd) {
    printf("%s\n", cwd);
    free(cwd);
    return 0;
  } else {
    perror("pwd");
    return -1;
  }
}

int export_func(COMMAND *cmd) {
  char *arg, *key, *value;

  // If no args: dump all exported vars
  if (!cmd->argv[1]) {
    dump_variables(); // or write a specialized dump_exports()
    return 0;
  }

  // For each argument: KEY=VALUE or just KEY
  for (int i = 1; cmd->argv[i]; i++) {
    arg = cmd->argv[i];

    if (parse_key_value_inplace(arg, &key, &value) == 0) {
      // got KEY and VALUE
    } else {
      // no '=', so treat as mark-export-only: KEY
      key = arg;
      value = lookup(key) ? lookup(key)->value : "";
    }

    if (!is_valid_identifier(key)) {
      fprintf(stderr, "export: `%s': not a valid identifier\n", key);
      continue; // skip invalid names
    }

    // add or update; add_variable() does its own strdup()
    if (!add_variable(key, value, 1)) {
      fprintf(stderr, "export: failed to set `%s'\n", key);
      return -1;
    }
  }
  return 0;
}
