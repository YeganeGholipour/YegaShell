/**
 * @file builtin.h
 * @brief prototypes for builtin functions
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include "job_utils.h"

/**
 * @struct Builtin
 * @brief Represents a built-in command.
 *
 * @var name The name of the built-in command.
 * @var func A pointer to the function that implements the built-in command.
 */
typedef struct {
  char *name;
  int (*func)(Process *, Job **);
} Builtin;

/**
 * @var builtin_commands
 * @brief An array of built-in commands.
 *
 * This array contains all the built-in commands supported by the shell.
 */
extern Builtin builtin_commands[];

/**
 * @brief Changes the current working directory.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return 0 on success, 1 on failure.
 */
int cd_func(Process *proc, Job **job_head);

/**
 * @brief Displays help information for the shell.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return 0 on success.
 */
int help_func(Process *proc, Job **job_head);

/**
 * @brief Exits the shell.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return The exit status of the shell.
 */
int exit_func(Process *proc, Job **job_head);

/**
 * @brief Displays the current working directory.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return 0 on success, 1 on failure.
 */
int pwd_func(Process *proc, Job **job_head);

/**
 * @brief Sets an environment variable.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return 0 on success.
 */
int export_func(Process *proc, Job **job_head);

/**
 * @brief Unsets an environment variable.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return 0 on success, 1 on failure.
 */
int unset_func(Process *proc, Job **job_head);

/**
 * @brief Brings a job to the foreground.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return 0 on success.
 */
int fg_func(Process *proc, Job **job_head);

/**
 * @brief Sends a job to the background.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return 0 on success.
 */
int bg_func(Process *proc, Job **job_head);

/**
 * @brief Displays information about the jobs in the job list.
 *
 * @param proc The process that is executing the command.
 * @param job_head The head of the job list.
 * @return 0 on success.
 */
int jobs_func(Process *proc, Job **job_head);

#endif