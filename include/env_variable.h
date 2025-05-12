#ifndef VARIABLE_H
#define VARIABLE_H

#define TABLESIZE 100

typedef struct Variable {
  char *key;
  char *value;
  int exported;
  struct Variable *next;
} Variable;

extern Variable *variable_table[];

unsigned hash(const char *s);
Variable *lookup(const char *key);
Variable *add_variable(const char *key, const char *value, int exported);
int remove_variable(const char *key);
void free_variable_table(void);
void dump_variables(void);
int is_valid_identifier(const char *s);
int parse_key_value_inplace(char *input, char **key_out, char **val_out);

#endif