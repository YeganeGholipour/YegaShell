/**
 * @file io_redirection.h
 * @brief Function prototypes for setting up input and output redirection for
 * child processes and closing pipe ends and freeing pipes.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef IO_REDIRECTION_H
#define IO_REDIRECTION_H

#include <stdlib.h>

#include "parser.h"

/**
 * @brief Sets up the output redirection for a child process.
 *
 * This function configures the output redirection for a child process based on
 * the provided command and pipe information.
 *
 * @param cmd        The command structure containing the output redirection
 * information.
 * @param pipes      A 2D array of pipe file descriptors.
 * @param proc_num   The process number of the child process.
 * @param num_procs  The total number of processes.
 *
 * @return An integer indicating the success or failure of the operation.
 */
int child_stdout_setup(Command *cmd, int (*pipes)[2], int proc_num,
                       int num_procs);

/**
 * @brief Sets up the input redirection for a child process.
 *
 * This function configures the input redirection for a child process based on
 * the provided command and pipe information.
 *
 * @param cmd        The command structure containing the input redirection
 * information.
 * @param pipes      A 2D array of pipe file descriptors.
 * @param proc_num   The process number of the child process.
 *
 * @return An integer indicating the success or failure of the operation.
 */
int child_stdin_setup(Command *cmd, int (*pipes)[2], int proc_num);

/**
 * @brief Closes the unused pipe ends for all processes.
 *
 * This function closes the unused pipe ends for all processes to prevent file
 * descriptor leaks.
 *
 * @param num_procs  The total number of processes.
 * @param pipes      A 2D array of pipe file descriptors.
 */
void close_pipe_ends(int num_procs, int (*pipes)[2]);

/**
 * @brief Frees the pipe resources.
 *
 * This function frees the pipe resources to prevent memory leaks.
 *
 * @param pipes      A 2D array of pipe file descriptors.
 */
void free_pipes(int (*pipes)[2]);

#endif