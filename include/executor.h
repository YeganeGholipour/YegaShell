/**
 * @file executor.h
 * @brief High-level function for execution phase
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "job_control.h"
#include "parser.h"

/**
 * @brief Executes a job.
 *
 * This function takes a job and the head of the job list as input, and executes
 * the job.
 *
 * @param job The job to execute.
 * @param job_head The head of the job list.
 * @return The status of the execution.
 */
int executor(Job *job, Job **job_head);

#endif