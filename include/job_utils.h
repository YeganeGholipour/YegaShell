/**
 * @file job_utils.h
 * @brief Functionalities for job creation phase and utilities for job control.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef JOB_UTILS_H
#define JOB_UTILS_H

#include <sys/types.h>

#include "parser.h"
#include "process_utils.h"

/**
 * @struct Job
 * @brief Represents a job in the job list.
 *
 * This struct contains information about a job, including its command, process
 * group ID, and job number.
 */
typedef struct Job {
  struct Job *next;
  char *command;
  Process *first_process;
  pid_t pgid;
  pid_t *pids;
  int job_num;
  int num_procs;
  int background;
} Job;

extern struct Pending {
  pid_t pid;
  int status;
} pending_bg_jobs[256];

extern int pending_indx;

/**
 * @brief Initializes a new job control structure.
 *
 * This function creates a new job control structure and initializes its fields.
 *
 * @param line_buffer The command line buffer.
 * @param cmd_ptr     The command structure.
 * @param proc_ptr    The process structure.
 * @param job_head    The head of the job list.
 *
 * @return A pointer to the newly created job structure.
 */
Job *initialize_job_control(char *line_buffer, Command *cmd_ptr,
                            Process *proc_ptr, Job **job_head);

/**
 * @brief Frees a job structure and its associated resources.
 *
 * This function frees a job structure and its associated resources, including
 * the command string and process list.
 *
 * @param job  The job structure to free.
 * @param head The head of the job list.
 */
void free_job(Job *job, Job **head);

/**
 * @brief Gets the number of processes in a job.
 *
 * This function returns the number of processes in a given job.
 *
 * @param job The job structure.
 *
 * @return The number of processes in the job.
 */
int get_num_procs(Job *job);

/**
 * @brief Frees all jobs in the job list.
 *
 * This function frees all jobs in the job list and their associated resources.
 *
 * @param head The head of the job list.
 */
void free_all_jobs(Job **head);

/**
 * @brief Kills all jobs in the job list.
 *
 * This function sends a signal to all jobs in the job list to terminate them.
 *
 * @param job_head The head of the job list.
 */
void kill_jobs(Job **job_head);

/**
 * @brief Finds a job in the job list based on a process.
 *
 * This function searches for a job in the job list that contains a given
 * process.
 *
 * @param proc    The process structure.
 * @param job_head The head of the job list.
 *
 * @return A pointer to the job structure that contains the process, or NULL if
 * not found.
 */
Job *find_job(Process *proc, Job **job_head);

/**
 * @brief Queues a pending process.
 *
 * This function adds a pending process to the queue of pending processes.
 *
 * @param pid   The process ID of the pending process.
 * @param status The status of the pending process.
 */
void queue_pending_procs(pid_t pid, int status);

/**
 * @brief Marks background jobs as pending.
 *
 * This function marks all background jobs as pending.
 *
 * @param job_head       The head of the job list.
 * @param pending_bg_jobs An array of pending background jobs.
 * @param pending_count  The number of pending background jobs.
 */
void mark_bg_jobs(Job **job_head, struct Pending pending_bg_jobs[],
                  int pending_count);

/**
 * @brief Drains the remaining statuses of a job.
 *
 * This function drains the remaining statuses of a job and updates the job's
 * status accordingly.
 *
 * @param job The job structure.
 */
void drain_remaining_statuses(Job *job);

/**
 * @brief Notifies the user about a job.
 *
 * This function notifies the user about a job, including its status and any
 * errors that occurred.
 *
 * @param job     The job structure.
 * @param job_head The head of the job list.
 */
void do_job_notification(Job *job, Job **job_head);

/**
 * @brief Checks if a job is stopped.
 *
 * This function checks if a job is stopped and returns a boolean value
 * indicating the result.
 *
 * @param job The job structure.
 *
 * @return 1 if the job is stopped, 0 otherwise.
 */
int job_is_stopped(Job *job);

/**
 * @brief Checks if a job is completed.
 *
 * This function checks if a job is completed and returns a boolean value
 * indicating the result.
 *
 * @param job The job structure.
 *
 * @return 1 if the job is completed, 0 otherwise.
 */
int job_is_completed(Job *job);

/**
 * @brief Clears the stopped mark for a job.
 *
 * This function clears the stopped mark for a job, indicating that it is no
 * longer stopped.
 *
 * @param job The job structure.
 */
void clear_stopped_mark(Job *job);

/**
 * @brief Notifies the user about background jobs.
 *
 * This function notifies the user about background jobs, including their status
 * and any errors that occurred.
 *
 * @param job_head The head of the job list.
 */
void notify_bg_jobs(Job **job_head);

/**
 * @brief Formats job information for display.
 *
 * This function formats job information, including the job number, process
 * group ID, and status, for display.
 *
 * @param job   The job structure.
 * @param status A buffer to store the formatted job information.
 */
void format_job_info(Job *job, char *status);

#endif