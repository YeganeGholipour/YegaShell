#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#include "builtin.h"
#include "executor.h"
#include "expander.h"
#include "helper.h"
#include "job_control.h"
#include "parser.h"
#include "signal_setup.h"
#include "tokenizer.h"

int main(void) {
  char *tokens[MAXTOKENS];
  char *line_buffer = NULL;
  Command *command_ptr = NULL;
  Process *process_ptr = NULL;
  Job *job_ptr = NULL;
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

  /* ----- Prompt Phase ----- */
  while (1) {
    if (interrupted) {
      interrupted = 0;
      continue;
    }
    if (child_changed) {
      child_changed = 0;
      mark_bg_jobs(&job_ptr, pending_bg_jobs, pending_indx);
      notify_bg_jobs(&job_ptr);
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

    /* ----- Tokenization Phase ----- */
    token_num = 0;
    token_status =
        tokenize_line(line_buffer, tokens, MAXTOKENS, MAXLEN, &token_num);
    if (token_status < 0) {
      fprintf(stderr, "Error: tokenizing input\n");
      free_memory(tokens, token_num);
      continue;
    }
    if (token_num == 0)
      continue;

    /* ----- Process Phase ----- */
    Process *proc_head =
        handle_processes(tokens, token_num, &command_ptr, &process_ptr);

    int func_num = is_bulitin(proc_head);
    if (func_num != -1) {
      if (builtin_routine(func_num, proc_head, &job_ptr, &process_ptr,
                          &command_ptr) < 0) {
        exit_status = last_exit_status;
        free_memory(tokens, token_num);
        break;
      }
    }

    /* ---- Job Control Phase ----- */
    else {
      Job *new_job = handle_job_control(line_buffer, command_ptr, proc_head,
                                        &job_ptr);
      if (new_job == NULL) {
        fprintf(stderr, "Error: job control\n");
        free_memory(tokens, token_num);
        continue;
      }

      /* ---- Executor Phase ----- */
      executor_status = executor(new_job, &job_ptr);
      if (executor_status == -1) {
        fprintf(stderr, "failed to execute\n");
        exit_status = EXIT_FAILURE;
        free_memory(tokens, token_num);
        process_ptr = NULL;
        command_ptr = NULL;
        break;
      }
    }

    /* ----- Cleanup Phase ----- */
    free_memory(tokens, token_num);
    process_ptr = NULL;
    command_ptr = NULL;
  }

  /* ----- Exit Phase ----- */
  clean_up(&job_ptr, line_buffer);
  return exit_status;
}
