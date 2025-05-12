#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expander.h"
#include "parser.h"

int compare_string_arrays(char *a[], char *b[]) {
  int i = 0;
  while (a[i] && b[i]) {
    if (strcmp(a[i], b[i]) != 0)
      return -1;
    i++;
  }
  return (a[i] == NULL && b[i] == NULL) ? 0 : -1;
}

void test_only_command() {
  char *tokens[] = {"pwd"};
  size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);
  COMMAND *cmd = NULL;

  int status = parse(tokens, &cmd, num_tokens);

  assert(status == 0);
  assert(strcmp(cmd->command, "pwd") == 0);
  assert(cmd->argv == NULL);
  assert(cmd->infile == NULL);
  assert(cmd->outfile == NULL);
  assert(cmd->background == 0);
  assert(cmd->append_output == 0);

  free_memory(cmd);
  printf("test_only_command passed.\n");
}

void test_argv_command() {
  char *tokens[] = {"ls", "-l", "/home/user"};
  size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);
  COMMAND *cmd = NULL;

  int status = parse(tokens, &cmd, num_tokens);

  assert(status == 0);
  assert(strcmp(cmd->command, "ls") == 0);
  char *expected[] = {"-l", "/home/user", NULL};
  assert(compare_string_arrays(cmd->argv, expected) == 0);
  assert(cmd->infile == NULL);
  assert(cmd->outfile == NULL);
  assert(cmd->background == 0);
  assert(cmd->append_output == 0);

  free_memory(cmd);
  printf("test_argv_command passed.\n");
}

void test_redirection_command() {
  char *tokens[] = {"grep", "foo", ">", "out.txt", "<", "in.txt"};
  size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);
  COMMAND *cmd = NULL;

  int status = parse(tokens, &cmd, num_tokens);

  assert(status == 0);
  assert(strcmp(cmd->command, "grep") == 0);
  char *expected[] = {"foo", NULL};
  assert(compare_string_arrays(cmd->argv, expected) == 0);
  assert(strcmp(cmd->infile, "in.txt") == 0);
  assert(strcmp(cmd->outfile, "out.txt") == 0);
  assert(cmd->background == 0);
  assert(cmd->append_output == 0);

  free_memory(cmd);
  printf("test_redirection_command passed.\n");
}

void test_background_command() {
  char *tokens[] = {"sleep", "5", "&"};
  size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);
  COMMAND *cmd = NULL;

  int status = parse(tokens, &cmd, num_tokens);

  assert(status == 0);
  assert(strcmp(cmd->command, "sleep") == 0);
  char *expected[] = {"5", NULL};
  assert(compare_string_arrays(cmd->argv, expected) == 0);
  assert(cmd->infile == NULL);
  assert(cmd->outfile == NULL);
  assert(cmd->background == 1);
  assert(cmd->append_output == 0);

  free_memory(cmd);
  printf("test_background_command passed.\n");
}

void test_environment_variable_command() {
  char *tokens[] = {"echo", "$HOME/mymy"};
  size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);
  COMMAND *cmd = NULL;

  int status = parse(tokens, &cmd, num_tokens);
  assert(status == 0);

  expander(cmd);
  assert(strcmp(cmd->command, "echo") == 0);

  char full_path[512];
  snprintf(full_path, sizeof(full_path), "%s/mymy", getenv("HOME"));
  char *expected[] = {full_path, NULL};
  assert(compare_string_arrays(cmd->argv, expected) == 0);
  assert(cmd->infile == NULL);
  assert(cmd->outfile == NULL);
  assert(cmd->background == 0);
  assert(cmd->append_output == 0);

  free_memory(cmd);
  printf("test_environment_variable_command passed.\n");
}

int main(void) {
  test_only_command();
  test_argv_command();
  test_redirection_command();
  test_background_command();
  test_environment_variable_command();

  printf("All tests passed!\n");
  return 0;
}
