/**
 * @file process_utils.h
 * @brief Prototypes to handle process creation phase.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <sys/types.h>

#include "parser.h"

/**
 * @struct Process
 * @brief Represents a process in the process list.
 *
 * This struct contains information about a process, including its command,
 * process ID, completion status, and stop status.
 */
typedef struct Process {
  struct Process *next;
  Command *cmd;
  pid_t pid;
  int completed;
  int stopped;
  int status;
} Process;

/**
 * @brief Initializes a new process.
 *
 * This function creates a new process and initializes its fields.
 *
 * @param tokens        Array of tokens representing the command to execute.
 * @param num_tokens    Number of tokens in the tokens array.
 * @param cmd_ptr       Pointer to the command structure.
 * @param proc_ptr      Pointer to the process structure.
 *
 * @return A pointer to the head of the process list.
 */
Process *initalize_processes(char *tokens[], size_t num_tokens,
                             Command **cmd_ptr, Process **proc_ptr);

#endif