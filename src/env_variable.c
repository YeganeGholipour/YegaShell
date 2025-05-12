#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "env_variable.h"

/* --- Hash function (K&R style) --- */
unsigned hash(const char *s) {
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

/* --- Add or update a variable --- */
Variable *add_variable(const char *key, const char *value, int exported) {
  Variable *vp = lookup(key);
  if (!vp) {
    /* Not found: create new node */
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

/* --- Remove a variable; returns 0 on success, -1 if not found --- */
int remove_variable(const char *key) {
  unsigned idx = hash(key);
  Variable *vp = variable_table[idx];
  Variable *prev = NULL;

  while (vp) {
    if (strcmp(vp->key, key) == 0) {
      /* Unlink */
      if (prev)
        prev->next = vp->next;
      else
        variable_table[idx] = vp->next;
      /* Free memory */
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

/* --- Free entire table (e.g., at shell exit) --- */
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

/* --- (Optional) Debug: print all variables --- */
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

int parse_key_value_inplace(char *input, char **key_out, char **val_out) {
  char *eq = strchr(input, '=');
  if (!eq)
    return -1;       // invalid, no '='
  *eq = '\0';        // replace '=' with NUL
  *key_out = input;  // KEY is at input
  *val_out = eq + 1; // VALUE is right after
  return 0;
}