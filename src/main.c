#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "executor.h"
#include "expander.h"
#include "job_control.h"
#include "parser.h"
#include "tokenizer.h"

volatile sig_atomic_t interrupted = 0;

void sigint_handler(int sig) {
  (void)sig;
  interrupted = 1;
  write(STDOUT_FILENO, "\n", 1);
}

void sigquit_handler(int sig) {
  (void)sig;
  interrupted = 1;
  write(STDOUT_FILENO, "\n", 1);
}

static void ignore_job_control_signals(void) {
  struct sigaction sa;
  sa.sa_flags = 0;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGTTIN, &sa, NULL) == -1) {
    perror("sigaction SIGTTIN");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTTOU, &sa, NULL) == -1) {
    perror("sigaction SIGTTOU");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTSTP, &sa, NULL) == -1) {
    perror("sigaction SIGTSTP");
    exit(EXIT_FAILURE);
  }
}

static void init_shell_signals(void) {
  struct sigaction sa_sigint, sa_sigquit;

  sa_sigint.sa_handler = sigint_handler;
  sa_sigint.sa_flags = 0;
  sigemptyset(&sa_sigint.sa_mask);
  if (sigaction(SIGINT, &sa_sigint, NULL) == -1) {
    perror("sigaction SIGINT");
    exit(EXIT_FAILURE);
  }

  sa_sigquit.sa_handler = sigquit_handler;
  sa_sigquit.sa_flags = 0;
  sigemptyset(&sa_sigquit.sa_mask);
  if (sigaction(SIGQUIT, &sa_sigquit, NULL) == -1) {
    perror("sigaction SIGQUIT");
    exit(EXIT_FAILURE);
  }
}

int main(void) {
  char *tokens[MAXTOKENS];
  char *line_buffer = NULL;
  COMMAND *command_struct = NULL;
  Process *process_struct = NULL;
  Job *job_struct = NULL;
  ssize_t read;
  size_t buffsize = 0;
  int token_num, token_status, prompt_status, executor_status;
  int exit_status = 0;

  pid_t shell_pgid = getpid();
  if (setpgid(shell_pgid, shell_pgid) < 0) {
    perror("shell: setpgid failed");
    exit(EXIT_FAILURE);
  }
  if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
    perror("shell: tcsetpgrp failed");
  }

  ignore_job_control_signals();
  init_shell_signals();

  /* PROMPT PHASE */
  while (1) {
    if (interrupted) {
      interrupted = 0;
      continue;
    }
    prompt_status = prompt_and_read(&line_buffer, &read, &buffsize);
    if (prompt_status < 0) {
      if (errno == EINTR) {
        clearerr(stdin);
        continue;
      } else if (feof(stdin)) {
        fprintf(stderr, " Detected EOF (Ctrl+D), exiting...\n");
        break; // safe
      } else {
        perror("getline");
        exit_status = EXIT_FAILURE;
        break;
      }
    }
    if (read > 0 && line_buffer[read - 1] == '\n')
      line_buffer[read - 1] = '\0';

    /* TOKENIZE PHASE */
    token_num = 0;
    token_status =
        tokenize_line(line_buffer, tokens, MAXTOKENS, MAXLEN, &token_num);
    if (token_status < 0) {
      fprintf(stderr, "Error: tokenizing input\n");
      continue;
    }
    if (token_num == 0)
      continue;

    /* JOB CONTROL PHASE */
    Job *new_job =
        handle_job_control(tokens, line_buffer, token_num, &command_struct,
                           &process_struct, &job_struct);
    if (new_job == NULL) {
      fprintf(stderr, "Error: job control\n");
      continue;
    }

    /* EXECUTION PHASE */
    executor_status = executor(new_job, &job_struct);
    if (executor_status == -1) {
      fprintf(stderr, "failed to execute\n");
      exit_status = EXIT_FAILURE;
      freeMemory(tokens, token_num);
      new_job = NULL;
      process_struct = NULL;
      command_struct = NULL;
      break;
    }

    if (is_exit != -1) {
      exit_status = is_exit;
      freeMemory(tokens, token_num);
      new_job = NULL;
      process_struct = NULL;
      command_struct = NULL;
      break;
    }

    /* FREE MEMORY - LAST STEP */
    // free_job(new_job, &job_struct);

    freeMemory(tokens, token_num);
    new_job = NULL;
    process_struct = NULL;
    command_struct = NULL;
  }

  kill_jobs(&job_struct);
  free_all_jobs(&job_struct);
  free(line_buffer);
  return exit_status;
}
