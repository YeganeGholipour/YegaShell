#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define OPERATORSLEN 4

typedef struct {
  char **argv;
  char *infile;
  char *outfile;
  int append_output;
  int background;
} Command;

void free_struct_memory(Command *cmd);
int parse(char *tokens[], Command **command_ptr, size_t num_tokens);
void print_command_ptr(Command *command_ptr);
int split_on_pipe(char *tokens[], size_t num_tokens, Command **cmd_ptr,
                  int indx);
char *get_raw_input(char *line_buffer);
int is_background_char_valid(char *tokens[], size_t num_tokens);

#endif