/**
 * @file executor.h
 * @brief Implements functionality for expanding environment variables used in
 * execution phase.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef EXPANDER_H
#define EXPANDER_H

#include "parser.h"

#define MAXSIZ 1024

/**
 * @var last_exit_status
 * @brief The last exit status of a command.
 */
extern int last_exit_status;

/**
 * @brief Expands shell syntax for a given command.
 *
 * This function takes a Command struct and an array of environment variables as
 * input. It expands the shell syntax in the command using the provided
 * environment variables.
 *
 * @param cmd The Command struct to expand.
 * @param envp The array of environment variables.
 */
void expander(Command *cmd);

#endif