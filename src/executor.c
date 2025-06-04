#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <unistd.h>

#include "process_control.h"
#include "signal_utils.h"
#include "job_control.h"
#include "env_utils.h"
#include "io_redirection.h"

int execute(Job *job, Job **job_head);
void cleanup_job_execution(int num_procs, JobResource *job_res);

int execute(Job *job, Job **job_head) {
  JobResource job_res;
  pid_t shell_pgid = getpid();
  pid_t pgid = 0;
  sigset_t parent_block_mask, prev_mask;
  int local_num_procs = job->num_procs;

  if (setup_exec_resource(job, &job_res) < 0)
    return -1;

  if (create_pipes(job, &job_res) < 0)
    return -1;

  if (block_parent_signals(&parent_block_mask, &prev_mask, job) < 0)
    return -1;

  if (fork_and_setup_processes(job, job_res, &pgid, &prev_mask) < 0)
    return -1;

  setup_job_control(job, job_head, &prev_mask, shell_pgid);

  cleanup_job_execution(local_num_procs, &job_res);

  return 0;
}

int executor(Job *job, Job **job_head) {
  if (!job->first_process) {
    fprintf(stderr, "Invalid command\n");
    free_job(job, job_head);
    return -1;
  }

  int execute_status = execute(job, job_head);
  if (execute_status < 0) {
    free_job(job, job_head);
    printf("job freed\n in execute_status < 0\n");
  }

  return execute_status;
}



void cleanup_job_execution(int num_procs, JobResource *job_res) {
  close_pipe_ends(num_procs, job_res->pipes);
  free_pipes_and_pids(job_res->pipes);
}


/* --------------------------------------------------------- */


