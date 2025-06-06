/**
 * @file parser.c
 * @brief Implements functionalities for parsing user command. Related to
 * parsing phase. Includes functions for allocating and freeing memory for
 * commands. Also includes functions for validating the position of special
 * characters. characters.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static const char *special_characters[OPERATORSLEN] = {">", "<", "&", ">>"};

Command *allocate_memory(size_t num_args);
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

Command *allocate_memory(size_t num_args) {
  Command *cmd = malloc(sizeof *cmd);
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

void free_struct_memory(Command *cmd) {
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

int parse(char *tokens[], Command **cmd_ptr, size_t num_tokens) {
  Command *cmd = allocate_memory(num_tokens);
  if (!cmd)
    return -1;
  *cmd_ptr = cmd;

  // check if the first token is a valid
  char *first_tok = tokens[0];
  if (!is_special_char(first_tok) && !is_background(first_tok) &&
      !is_append_output(first_tok) && !is_output_redirection(first_tok) &&
      !is_input_redirection(first_tok)) {
    cmd->argv[0] = strdup(first_tok);
  } else {
    fprintf(stderr, "parser: syntax error, first token is invalid\n");
    free_struct_memory(cmd);
    return -1;
  }

  size_t i = 1, argc = 1;
  while (i < num_tokens) {
    char *tok = tokens[i++];

    if (is_background(tok)) {
      if (i == num_tokens)
        cmd->background = 1;
      else {
        fprintf(stderr, "parser: syntax error, '&' must be the last token\n");
        free_struct_memory(cmd);
        return -1;
      }
    } else if (is_append_output(tok)) {
      if (i < num_tokens && !is_special_char(tokens[i])) {
        cmd->outfile = strdup(tokens[i++]);
        cmd->append_output = 1;
      } else {
        fprintf(stderr, "parser: syntax error after '>>'\n");
        free_struct_memory(cmd);
        return -1;
      }

    } else if (is_output_redirection(tok)) {
      if (i < num_tokens && !is_special_char(tokens[i])) {
        cmd->outfile = strdup(tokens[i++]);
      } else {
        fprintf(stderr, "parser: syntax error after '>'\n");
        free_struct_memory(cmd);
        return -1;
      }

    } else if (is_input_redirection(tok)) {
      if (i < num_tokens && !is_special_char(tokens[i])) {
        cmd->infile = strdup(tokens[i++]);
      } else {
        fprintf(stderr, "parser: syntax error after '<'\n");
        free_struct_memory(cmd);
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

void print_command_ptr(Command *cmd) {
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

int split_on_pipe(char *tokens[], size_t num_tokens, Command **cmd_ptr,
                  int indx) {
  char *process_command[num_tokens + 1];
  int j = 0;

  while (indx < (int)num_tokens && tokens[indx] != NULL &&
         strcmp(tokens[indx], "|") != 0) {
    process_command[j++] = tokens[indx++];
  }
  process_command[j] = NULL;

  int parser_status = parse(process_command, cmd_ptr, j);
  if (parser_status < 0) {
    fprintf(stderr, "parser: error\n");
    return -1;
  }

  return indx;
}

char *get_raw_input(char *line_buffer) {
  size_t length = strlen(line_buffer);
  char *raw_input = NULL;

  if (length > 0 && line_buffer[length - 1] == '&') {
    raw_input = malloc(length);
    if (raw_input == NULL) {
      perror("malloc failed");
      return NULL;
    }
    strncpy(raw_input, line_buffer, length - 1);
    raw_input[length - 1] = '\0';
  } else {
    raw_input = malloc(length + 1);
    if (raw_input == NULL) {
      perror("malloc failed");
      return NULL;
    }
    strcpy(raw_input, line_buffer);
  }

  return raw_input;
}

int is_background_char_valid(char *tokens[], size_t num_tokens) {
  for (size_t i = 0; i < num_tokens; i++) {
    if (strcmp(tokens[i], "&") == 0) {
      if (i == num_tokens - 1) {
        return 1;
      } else {
        fprintf(stderr, "parser: error on '&' character: must be last\n");
        return 0;
      }
    }
  }
  return 1;
}
