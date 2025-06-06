/**
 * @file job_control.h
 * @brief Functionality for setting up job control in execution phase
 *        Includes functions for handling foreground and background jobs.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "job_utils.h"

/**
 * @var last_exit_status
 * @brief The last exit status of a job.
 *
 * This variable stores the exit status of the last job executed.
 */
extern int last_exit_status;

/**
 * @brief Sets up job control for a given job.
 *
 * This function sets up job control for a given job, including handling
 * foreground and background jobs.
 *
 * @param job        The job to set up job control for.
 * @param job_head   The head of the job list.
 * @param prev_mask  A pointer to the previous signal mask.
 * @param shell_pgid The process group ID of the shell.
 */
void setup_job_control(Job *job, Job **job_head, sigset_t *prev_mask,
                       pid_t shell_pgid);

/**
 * @brief Handles a foreground job.
 *
 * This function handles a foreground job, including setting up signal handling
 * and process group IDs.
 *
 * @param prev_list  A pointer to the previous signal mask.
 * @param job        The job to handle.
 * @param shell_pgid The process group ID of the shell.
 * @param job_head   The head of the job list.
 */
void handle_foreground_job(sigset_t *prev_list, Job *job, pid_t shell_pgid,
                           Job **job_head);

/**
 * @brief Handles a background job.
 *
 * This function handles a background job, including setting up signal handling
 * and process group IDs.
 *
 * @param prev_mask  A pointer to the previous signal mask.
 * @param job        The job to handle.
 */
void handle_background_job(sigset_t *prev_mask, Job *job);

#endif