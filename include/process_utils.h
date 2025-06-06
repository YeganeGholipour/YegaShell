/*
 * file:   process_utils.h
 * author: Yegane
 * date:   2025-06-06
 * desc:   Prototypes to handle process creation phase.
 *         Includes a function for initalizing processes.  
 */

#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <sys/types.h>

#include "parser.h"

typedef struct Process {
  struct Process *next;
  Command *cmd;
  pid_t pid;
  int completed;
  int stopped;
  int status;
} Process;

Process *initalize_processes(char *tokens[], size_t num_tokens, Command **cmd_ptr,
                          Process **proc_ptr);

#endif