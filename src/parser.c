#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static const char *special_characters[OPERATORSLEN] = {">", "<", "&", ">>"};

COMMAND *allocate_memory(size_t num_args);
static int is_background(char *tok) { return strcmp(tok, "&") == 0; }
static int is_append_output(char *tok) { return strcmp(tok, ">>") == 0; }
static int is_output_redirection(char *tok) { return strcmp(tok, ">") == 0; }
static int is_input_redirection(char *tok) { return strcmp(tok, "<") == 0; }
static int is_special_char(char *tok) {
  for (int i = 0; i < OPERATORSLEN; i++)
    if (strcmp(tok, special_characters[i]) == 0)
      return 1;
  return 0;
}

COMMAND *allocate_memory(size_t num_args) {
  COMMAND *cmd = malloc(sizeof *cmd);
  if (!cmd)
    return NULL;

  if (num_args > 0) {
    cmd->argv = calloc(num_args + 1, sizeof *cmd->argv);
    if (!cmd->argv) {
      free(cmd);
      return NULL;
    }
  } else {
    cmd->argv = NULL;
  }

  cmd->infile = NULL;
  cmd->outfile = NULL;
  cmd->append_output = 0;
  cmd->background = 0;
  return cmd;
}

void free_memory(COMMAND *cmd) {
  if (!cmd)
    return;

  if (cmd->argv) {
    for (char **p = cmd->argv; *p; ++p)
      free(*p);
    free(cmd->argv);
  }
  free(cmd->infile);
  free(cmd->outfile);
  free(cmd);
}

int parse(char *tokens[], COMMAND **cmd_ptr, size_t num_tokens) {
  if (num_tokens == 0) {
    fprintf(stderr, "parser: syntax error: empty tokens\n");
    return -1;
  }

  COMMAND *cmd = allocate_memory(num_tokens);
  if (!cmd)
    return -1;
  *cmd_ptr = cmd;

  size_t i = 0, argc = 0;
  while (i < num_tokens) {
    char *tok = tokens[i++];

    if (is_background(tok)) {
      cmd->background = 1;

    } else if (is_append_output(tok)) {
      if (i < num_tokens && !is_special_char(tokens[i])) {
        cmd->outfile = strdup(tokens[i++]);
        cmd->append_output = 1;
      } else {
        fprintf(stderr, "parser: syntax error after '>>'\n");
        free_memory(cmd);
        return -1;
      }

    } else if (is_output_redirection(tok)) {
      if (i < num_tokens && !is_special_char(tokens[i])) {
        cmd->outfile = strdup(tokens[i++]);
      } else {
        fprintf(stderr, "parser: syntax error after '>'\n");
        free_memory(cmd);
        return -1;
      }

    } else if (is_input_redirection(tok)) {
      if (i < num_tokens && !is_special_char(tokens[i])) {
        cmd->infile = strdup(tokens[i++]);
      } else {
        fprintf(stderr, "parser: syntax error after '<'\n");
        free_memory(cmd);
        return -1;
      }

    } else {
      cmd->argv[argc++] = strdup(tok);
    }
  }

  if (cmd->argv)
    cmd->argv[argc] = NULL;
  return 0;
}

void print_command_struct(COMMAND *cmd) {
  if (!cmd)
    return;
  printf("command: %s\n", cmd->argv[0]);
  printf("argv: [");
  if (cmd->argv) {
    for (int i = 0; cmd->argv[i]; i++)
      printf("%s ", cmd->argv[i]);
  }
  printf("]\n");
  printf("infile: %s\n", cmd->infile ? cmd->infile : "(null)");
  printf("outfile: %s\n", cmd->outfile ? cmd->outfile : "(null)");
  printf("append_output: %d\n", cmd->append_output);
  printf("background: %d\n", cmd->background);
}