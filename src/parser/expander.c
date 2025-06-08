/**
 * @file expander.c
 * @brief Implements functionality for expanding environment variables used in
 * execution phase.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "env_utils.h"
#include "expander.h"
#include "parser.h"

static const char *get_env(const char *name);
static char *expand_variable(const char *token);

static const char *get_env(const char *name) {
  Variable *vp = lookup(name);
  if (vp != NULL) {
    return vp->value;
  }
  return getenv(name);
}

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

  const char *env = get_env(arg);
  size_t env_len = env ? strlen(env) : 0;

  char *out = malloc(env_len + rest_len + 1);
  if (!out)
    return NULL;

  if (env_len)
    memcpy(out, env, env_len);
  if (rest_len)
    memcpy(out + env_len, token + i, rest_len);

  out[env_len + rest_len] = '\0';
  return out;
}

void expander(Command *cmd) {
  for (int i = 1; cmd->argv[i] != NULL; i++) {
    char *token = cmd->argv[i];
    if (token[0] == '$') {
      char *newstr = NULL;
      if (strcmp(token, "$$") == 0) {
        char pid_tmp[20];
        snprintf(pid_tmp, sizeof pid_tmp, "%d", getpid());
        newstr = strdup(pid_tmp);
      } else if (strcmp(token, "$?") == 0) {
        char exit_status[12];
        snprintf(exit_status, sizeof exit_status, "%d", last_exit_status);
        newstr = strdup(exit_status);
      } else {
        newstr = expand_variable(token);
        if (!newstr) {
          newstr = strdup(""); 
          fprintf(stderr, "expander: failed to expand variable '%s'\n", token);
        }
      }

      free(token);
      cmd->argv[i] = newstr;
    }
  }

  if (cmd->infile && cmd->infile[0] == '$') {
    char *old = cmd->infile;
    char *newstr = expand_variable(old);
    if (newstr) {
      cmd->infile = newstr;
      free(old);
    } else {
      fprintf(stderr, "expander: failed to expand infile variable '%s'\n", old);
      cmd->infile = strdup("");
      free(old);
    }
  }

  if (cmd->outfile && cmd->outfile[0] == '$') {
    char *old = cmd->outfile;
    char *newstr = expand_variable(old);
    if (newstr) {
      cmd->outfile = newstr;
      free(old);
    } else {
      fprintf(stderr, "expander: failed to expand outfile variable '%s'\n",
              old);
      cmd->outfile = strdup("");
      free(old);
    }
  }
}
