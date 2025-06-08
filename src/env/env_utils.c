/**
 * @file env_utils.c
 * @brief Utilities for handling environment variables.
 *         Adds, deletes, and updates environment variables.
 *         Creates the envp array of pointers.
 *         Envoronment variables are stored in a hash table
 *         implemented as a linked list.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "env_utils.h"

static unsigned hash(const char *s);

Variable *variable_table[TABLESIZE] = {NULL};

static unsigned hash(const char *s) {
  unsigned hashval = 0;
  while (*s)
    hashval = *s++ + 31 * hashval;
  return hashval % TABLESIZE;
}

/* --- Lookup a variable by key --- */
Variable *lookup(const char *key) {
  for (Variable *vp = variable_table[hash(key)]; vp; vp = vp->next)
    if (strcmp(key, vp->key) == 0)
      return vp;
  return NULL;
}

Variable *add_variable(const char *key, const char *value, int exported) {
  Variable *vp = lookup(key);
  if (!vp) {
    vp = malloc(sizeof(Variable));
    if (!vp) {
      perror("malloc");
      return NULL;
    }
    vp->key = strdup(key);
    vp->value = strdup(value);
    if (!vp->key || !vp->value) {
      perror("strdup");
      free(vp->key);
      free(vp->value);
      free(vp);
      return NULL;
    }
    vp->exported = exported;
    /* Insert at head of chain */
    unsigned idx = hash(key);
    vp->next = variable_table[idx];
    variable_table[idx] = vp;
  } else {
    /* Found: overwrite value and exported flag */
    free(vp->value);
    vp->value = strdup(value);
    if (!vp->value) {
      perror("strdup");
      return NULL;
    }
    vp->exported = exported;
  }
  return vp;
}

int remove_variable(const char *key) {
  unsigned idx = hash(key);
  Variable *vp = variable_table[idx];
  Variable *prev = NULL;

  while (vp) {
    if (strcmp(vp->key, key) == 0) {
      if (prev)
        prev->next = vp->next;
      else
        variable_table[idx] = vp->next;
      free(vp->key);
      free(vp->value);
      free(vp);
      return 0;
    }
    prev = vp;
    vp = vp->next;
  }
  return -1;
}

void dump_variables(void) {
  for (int i = 0; i < TABLESIZE; i++) {
    for (Variable *vp = variable_table[i]; vp; vp = vp->next) {
      printf("%s=%s %s\n", vp->key, vp->value,
             vp->exported ? "(exported)" : "");
    }
  }
}

int is_valid_identifier(const char *s) {
  if (!(isalpha((unsigned char)s[0]) || s[0] == '_'))
    return 0;
  for (size_t i = 1; s[i]; i++) {
    if (!(isalnum((unsigned char)s[i]) || s[i] == '_'))
      return 0;
  }
  return 1;
}

void free_variable_table(void) {
  for (int i = 0; i < TABLESIZE; i++) {
    Variable *vp = variable_table[i];
    while (vp) {
      Variable *next = vp->next;
      free(vp->key);
      free(vp->value);
      free(vp);
      vp = next;
    }
    variable_table[i] = NULL;
  }
}

int parse_key_value_inplace(char *input, char **key_out, char **val_out) {
  char *eq = strchr(input, '=');
  if (!eq)
    return -1;       
  *eq = '\0';       
  *key_out = input;  
  *val_out = eq + 1; 
  return 0;
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

int initialize_envp(char ***envpp) {
  *envpp = build_envp();
  if (!*envpp)
    return -1;
  return 0;
}

void free_envp(char **envp) {
  for (int i = 0; envp[i]; i++)
    free(envp[i]);
  free(envp);
}