#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "executor.h"
#include "expander.h"
#include "parser.h"
#include "tokenizer.h"

static void init_shell_signals(void);

static void init_shell_signals(void) {
  if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
    perror("signal SIGINT");
    exit(EXIT_FAILURE);
  }
  if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
    perror("signal SIGQUIT");
    exit(EXIT_FAILURE);
  }
  if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
    perror("signal SIGTSTP");
    exit(EXIT_FAILURE);
  }
  if (signal(SIGTTOU, SIG_IGN) == SIG_ERR) {
    perror("signal SIGTTOU");
    exit(EXIT_FAILURE);
  }
  if (signal(SIGTTIN, SIG_IGN) == SIG_ERR) {
    perror("signal SIGTTIN");
    exit(EXIT_FAILURE);
  }
}

int main(void) {
  char *tokens[MAXTOKENS];
  char *line_buffer = NULL;
  COMMAND *command_struct = NULL;
  size_t read;
  int token_status, token_num, prompt_status, status;

  pid_t shell_pgid = getpid();
  if (setpgid(shell_pgid, shell_pgid) < 0) {
    perror("shell: setpgid failed");
    exit(EXIT_FAILURE);
  }
  if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
    perror("shell: tcsetpgrp failed");
  }

  init_shell_signals();

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

    /* PARSING PHASE */
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
    fprintf(stderr, "Detected EOF (Ctrl+D), exiting...\n");
    return -1;
  }
  free(line_buffer);
  return 0;
}
