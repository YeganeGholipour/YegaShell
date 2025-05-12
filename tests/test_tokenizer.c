#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "tokenizer.h"

void test_simple_command() {
  char *tokens[MAXTOKENS];
  int token_num = 0;
  char line[] = "ls -l /home/user";

  int status = tokenize_line(line, tokens, MAXTOKENS, MAXLEN, &token_num);

  assert(status == 0);
  assert(token_num == 3);
  assert(strcmp(tokens[0], "ls") == 0);
  assert(strcmp(tokens[1], "-l") == 0);
  assert(strcmp(tokens[2], "/home/user") == 0);

  freeMemory(tokens, token_num);
  printf("test_simple_command passed.\n");
}

void test_double_quotes() {
  char *tokens[MAXTOKENS];
  int token_num = 0;
  char line[] = "echo \"hello world to you\"";

  int status = tokenize_line(line, tokens, MAXTOKENS, MAXLEN, &token_num);

  assert(status == 0);
  assert(token_num == 2);
  assert(strcmp(tokens[0], "echo") == 0);
  assert(strcmp(tokens[1], "hello world to you") == 0);

  freeMemory(tokens, token_num);
  printf("test_double_quotes passed.\n");
}

void test_single_quotes() {
  char *tokens[MAXTOKENS];
  int token_num = 0;
  char line[] = "echo 'h$llo' to you";

  int status = tokenize_line(line, tokens, MAXTOKENS, MAXLEN, &token_num);

  assert(status == 0);
  assert(token_num == 4);
  assert(strcmp(tokens[0], "echo") == 0);
  assert(strcmp(tokens[1], "h$llo") == 0);
  assert(strcmp(tokens[2], "to") == 0);
  assert(strcmp(tokens[3], "you") == 0);

  freeMemory(tokens, token_num);
  printf("test_single_quotes passed.\n");
}

void test_special_characters() {
  char *tokens[MAXTOKENS];
  int token_num = 0;
  char line[] = "cat file.txt> output.txt";

  int status = tokenize_line(line, tokens, MAXTOKENS, MAXLEN, &token_num);

  assert(status == 0);
  assert(token_num == 4);
  assert(strcmp(tokens[0], "cat") == 0);
  assert(strcmp(tokens[1], "file.txt") == 0);
  assert(strcmp(tokens[2], ">") == 0);
  assert(strcmp(tokens[3], "output.txt") == 0);

  freeMemory(tokens, token_num);
  printf("test_special_characters passed.\n");
}

void test_double_operator() {
  char *tokens[MAXTOKENS];
  int token_num = 0;
  char line[] = "cmd >> logfile";

  int status = tokenize_line(line, tokens, MAXTOKENS, MAXLEN, &token_num);

  assert(status == 0);
  assert(token_num == 3);
  assert(strcmp(tokens[0], "cmd") == 0);
  assert(strcmp(tokens[1], ">>") == 0);
  assert(strcmp(tokens[2], "logfile") == 0);

  freeMemory(tokens, token_num);
  printf("test_double_operator passed.\n");
}

void test_unclosed_quote() {
  char *tokens[MAXTOKENS];
  int token_num = 0;
  char line[] = "echo \"missing end";

  int status = tokenize_line(line, tokens, MAXTOKENS, MAXLEN, &token_num);

  assert(status == -1);
  printf("test_unclosed_quote passed.\n");
}

int main(void) {
  test_simple_command();
  test_double_quotes();
  test_single_quotes();
  test_special_characters();
  test_double_operator();

  printf("All tests passed!\n");
  return 0;
}