/**
 * @file process_control.h
 * @brief Prototypes for process control functions
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef PROCESS_CONTROL_H
#define PROCESS_CONTROL_H

#include <signal.h>
#include <sys/types.h>

#include "job_utils.h"
#include "process_utils.h"

/**
 * @struct JobResource
 * @brief Represents resources for a job.
 *
 * This struct contains information about the resources for a job, including
 * the pipes
 */
typedef struct {
  int (*pipes)[2]; /**< Array of pipes for the job. */
} JobResource;

/**
 * @brief Sets up the execution resource for a job.
 *
 * This function sets up the execution resource for a job, including allocating
 * memory for the pipes and process group ID.
 *
 * @param job The job to set up the execution resource for.
 * @param job_res The JobResource struct to fill.
 * @return 0 on success, -1 on failure.
 */
int setup_exec_resource(Job *job, JobResource *job_res);

/**
 * @brief Creates pipes for a job.
 *
 * This function creates pipes for a job, including allocating memory for the
 * pipes and setting up the pipe file descriptors.
 *
 * @param job The job to create pipes for.
 * @param job_res The JobResource struct containing the pipes.
 * @return 0 on success, -1 on failure.
 */
int create_pipes(Job *job, JobResource *job_res);

/**
 * @brief Forks and sets up child and parent processes for a job.
 *
 * This function forks and sets up child and parent processes for a job,
 * including setting up the process group ID and signal handling.
 *
 * @param job The job to fork and set up processes for.
 * @param job_res The JobResource struct containing the pipes.
 * @param pgid The process group ID of the job.
 * @param prev_mask The previous signal mask.
 * @return 0 on success, -1 on failure.
 */
int fork_and_setup_processes(Job *job, JobResource job_res, int *pgid,
                             sigset_t *prev_mask);

#endif