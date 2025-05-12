#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "executor.h"
#include "expander.h"
#include "parser.h"
#include "tokenizer.h"

/* custom handler for ctrl+c */
void handle_sigint(int sig);

int main(void) {
  char *tokens[MAXTOKENS];
  char *line_buffer = NULL;
  COMMAND *command_struct = NULL;
  size_t read;
  int token_status, token_num, prompt_status, status;

  signal(SIGINT, handle_sigint);
  signal(SIGQUIT, SIG_IGN);

  /* PROMPT PHASE */
  while ((prompt_status = prompt_and_read(&line_buffer, &read)) == 0) {

    if (line_buffer[read - 1] == '\n')
      line_buffer[read - 1] = '\0';

    /* TOKENIZE PHASE */
    token_num = 0;
    token_status =
        tokenize_line(line_buffer, tokens, MAXTOKENS, MAXLEN, &token_num);

    if (token_status < 0) {
      fprintf(stderr, "Error: tokenizing input\n");
      continue;
    }

    /* PARSING PHASE*/
    if (parse(tokens, &command_struct, token_num) < 0) {
      fprintf(stderr, "parser: error\n");
      return 1;
    }

    /* EXECUTION PHASE */
    status = executor(command_struct);
    if (status == -1) {
      fprintf(stderr, "failed to execute\n");
      exit(EXIT_FAILURE);
    }

    /* FREE MEMORY - LAST STEP */
    freeMemory(tokens, token_num);
  }

  if (prompt_status < 0) {
    fprintf(stderr, " Detected EOF (Ctrl+D), exiting...\n");
    return -1;
  }

  free(line_buffer);
  return 0;
}

void handle_sigint(int sig) {
  (void)sig;
  write(STDOUT_FILENO, "\nYegaShell>", 12);
}
