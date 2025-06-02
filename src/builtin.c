#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "builtin.h"
#include "env_variable.h"
#include "executor.h"

Builtin builtin_commands[] = {
    {"cd", cd_func},   {"help", help_func},     {"exit", exit_func},
    {"pwd", pwd_func}, {"export", export_func}, {"fg", fg_func},
    {NULL, NULL}};

int fg_func(Job *job, Job **job_head) {
  // Step -1: drain all background jobs and mark them before running the fg job
  mark_bg_jobs(job_head, pending_bg_jobs, pending_indx);
  notify_bg_jobs(job_head);

  // Step 0: find the job
  Job *found_job = find_job(job, job_head);
  pid_t shell_pgid = getpgid(0);

  if (!found_job) {
    fprintf(stderr, "fg: no such job\n");
    return 1;
  }

  if (job_is_completed(found_job)) {
    fprintf(stderr, "fg: job %ld already completed\n", (long)found_job->pgid);
    return 1;
  }

  // Step 1: block signals in parent
  sigset_t parent_block_mask, prev_mask;
  if (block_parent_signals(&parent_block_mask, &prev_mask, found_job) < 0) {
    perror("fg");
    return 1;
  }

  // Step 2: send SIGCONT to the stopped job
  if (job_is_stopped(found_job)) {
    // clear the stopped member for each process
    clear_stopped_mark(found_job);
    // show the command
    printf("%s\n", found_job->command);
    if (kill(-found_job->pgid, SIGCONT) < 0) {
      perror("fg");
      sigprocmask(SIG_SETMASK, &prev_mask, NULL);
      return 1;
    }
  }

  // Step 3: handle the job in the foreground
  handle_foreground_job(&prev_mask, found_job, shell_pgid, job_head);
  return 0;
}

int cd_func(Job *job, Job **job_head) {
  (void)job_head;
  const char *path;
  COMMAND *cmd = job->first_process->cmd;

  if (cmd->argv[1] == NULL || strcmp(cmd->argv[1], "~") == 0) {
    path = getenv("HOME");
    if (path == NULL) {
      fprintf(stderr, "cd: HOME not set\n");
      return 1;
    }
  } else
    path = cmd->argv[1];

  if (chdir(path) != 0) {
    perror("cd");
    return 1;
  }
  return 0;
}

int help_func(Job *job, Job **job_head) {
  (void)job;
  (void)job_head;
  printf("Yega Shell\n");
  printf("Type the name of the command, and hit enter.\n");
  printf("Use the man command for information on other programs.\n");

  return 0;
}

int exit_func(Job *job, Job **job_head) {
  (void)job_head;
  COMMAND *cmd = job->first_process->cmd;
  int status = 0;
  if (cmd->argv[1])
    status = atoi(cmd->argv[1]);
  return status;
}

int pwd_func(Job *job, Job **job_head) {
  (void)job;
  (void)job_head;
  char *cwd = getcwd(NULL, 0);
  if (cwd) {
    printf("%s\n", cwd);
    free(cwd);
    return 0;
  } else {
    perror("pwd");
    return 1;
  }
}

int export_func(Job *job, Job **job_head) {
  (void)job_head;
  COMMAND *cmd = job->first_process->cmd;
  char *arg, *key, *value;

  if (!cmd->argv[1]) {
    dump_variables();
    return 0;
  }

  for (int i = 1; cmd->argv[i]; i++) {
    arg = cmd->argv[i];

    if (parse_key_value_inplace(arg, &key, &value) == 0) {
    } else {
      key = arg;
      value = lookup(key) ? lookup(key)->value : "";
    }

    if (!is_valid_identifier(key)) {
      fprintf(stderr, "export: `%s': not a valid identifier\n", key);
      continue;
    }

    if (!add_variable(key, value, 1)) {
      fprintf(stderr, "export: failed to set `%s'\n", key);
      return 1;
    }
  }
  return 0;
}
