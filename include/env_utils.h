/*
 * file:   env_utils.h
 * author: Yegane
 * date:   2025-06-06
 * desc:   Utilities for handling environment variables.
 *         Adds, deletes, and updates environment variables.
 *         Creates the envp array of pointers.
 *         Envoronment variables are stored in a hash table
 *         implemented as a linked list.
 */

#ifndef ENV_UTILS_H
#define ENV_UTILS_H

#define TABLESIZE 100

typedef struct Variable {
  char *key;
  char *value;
  int exported;
  struct Variable *next;
} Variable;

extern Variable *variable_table[];

Variable *lookup(const char *key);
Variable *add_variable(const char *key, const char *value, int exported);
int remove_variable(const char *key);
void dump_variables(void);
int is_valid_identifier(const char *s);
int parse_key_value_inplace(char *input, char **key_out, char **val_out);
char *get_full_path(const char *command);
char **build_envp(void);

#endif