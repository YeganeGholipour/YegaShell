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
} COMMAND;

void free_memory(COMMAND *cmd);
int parse(char *tokens[], COMMAND **command_struct, size_t num_tokens);
void print_command_struct(COMMAND *command_struct);

#endif