/**
 * @file helper.h
 * @brief Collection of utility functions that provide supporting functionality
 * for the YegaShell program.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef HELPER_H
#define HELPER_H

#include "job_utils.h"

/**
 * @brief Checks whether a process is a built-in command.
 *
 * This function checks whether the given process is a built-in command.
 *
 * @param proc The process to check.
 * @return 1 if the process is a built-in command, 0 otherwise.
 */
int is_bulitin(Process *proc);

/**
 * @brief Executes a built-in routine.
 *
 * This function executes a built-in routine based on the given function number
 * and process information.
 *
 * @param func_num The function number of the built-in routine to execute.
 * @param proc_head The head of the process list.
 * @param job_ptr A pointer to the job structure.
 * @param process_ptr A pointer to the process structure.
 * @param command_ptr A pointer to the command structure.
 * @return The result of the built-in routine execution.
 */
int builtin_routine(int func_num, Process *proc_head, Job **job_ptr,
                    Process **process_ptr, Command **command_ptr);

/**
 * @brief Cleans up resources after a job is completed.
 *
 * This function cleans up resources such as the job structure, process list,
 * and command structure after a job is completed.
 *
 * @param job_ptr A pointer to the job structure.
 * @param line_buffer The line buffer containing the command input.
 */
void clean_up(Job **job_ptr, char *line_buffer);

#endif