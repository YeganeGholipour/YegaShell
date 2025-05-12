#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parser.h"
#include "expander.h"

static char *expand_variable(const char *token) {
    int i = 1;
    if (!(isalpha(token[i]) || token[i] == '_'))
        return NULL;
    char arg[MAXSIZ];
    int j = 0;
    while ((isalnum(token[i]) || token[i] == '_') && j < MAXSIZ - 1) {
        arg[j++] = token[i++];
    }
    arg[j] = '\0';

    size_t rest_len = strlen(token + i);

    const char *env = getenv(arg);
    size_t env_len = env ? strlen(env) : 0;

    char *out = malloc(env_len + rest_len + 1);
    if (!out) return NULL;

    if (env_len)
        memcpy(out, env, env_len);
    if (rest_len)
        memcpy(out + env_len, token + i, rest_len);

    out[env_len + rest_len] = '\0';
    return out;
}

void expander(COMMAND *cmd) {
  for (int i = 0; cmd->argv[i] != NULL; i++) {
    char *token = cmd->argv[i];
    if (token[0] == '$') {
      char *newstr;
      if (strcmp(token, "$$") == 0) {
        char pid_tmp[20];
        snprintf(pid_tmp, sizeof pid_tmp, "%d", getpid());
        newstr = strdup(pid_tmp);
      } else {
        newstr = expand_variable(token);
      }
      free(token);
      cmd->argv[i] = newstr;
    }
  }

  if (cmd->infile && cmd->infile[0] == '$') {
    char *old = cmd->infile;
    cmd->infile = expand_variable(old);
    free(old);
  }
  if (cmd->outfile && cmd->outfile[0] == '$') {
    char *old = cmd->outfile;
    cmd->outfile = expand_variable(old);
    free(old);
  }
}
