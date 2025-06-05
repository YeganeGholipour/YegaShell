#ifndef PROCESS_CONTROL_H
#define PROCESS_CONTROL_H

#include <sys/types.h>
#include <signal.h>

#include "job_utils.h"
#include "process_utils.h"

typedef struct {
  int (*pipes)[2];
} JobResource;


int setup_exec_resource(Job *job, JobResource *job_res);
int create_pipes(Job *job, JobResource *job_res);
int fork_and_setup_processes(Job *job, JobResource job_res, int *pgid,
                             sigset_t *prev_mask);
int allocate_pipe(Job *job, JobResource *job_res);
int allocate_pids(Job *job);
void child_setup(pid_t pgid, sigset_t *prev_mask, int (*pipes)[],
                        int proc_num, Command *cmd, Job *job);
void parent_setup(pid_t *pgid, int pid, int proc_num, int (*pipes)[2],
                         Process *proc, Job *job);
void free_pipes_and_pids(int (*pipes)[2]);
#endif