/**
 * @file process_utils.c
 * @brief Functions to handle process creation phase
 *         Includes functionality for creating and initalizing processes.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "process_utils.h"

static Process *create_process(Process **proc_ptr, Command *cmd);

Process *initalize_processes(char *tokens[], size_t num_tokens,
                             Command **cmd_ptr, Process **proc_ptr) {
  int i = 0;

  // see if bakground job character is valid
  if (is_background_char_valid(tokens, num_tokens) == 0)
    return NULL;

  while (i < (int)num_tokens) {
    int split_indx = split_on_pipe(tokens, num_tokens, cmd_ptr, i);
    if (split_indx < 0) {
      return NULL;
    }

    *proc_ptr = create_process(proc_ptr, *cmd_ptr);

    if (split_indx < (int)num_tokens && strcmp(tokens[split_indx], "|") == 0) {
      i = split_indx + 1;
    } else {
      break;
    }
  }
  return *proc_ptr;
}

static Process *create_process(Process **proc_ptr, Command *cmd) {
  if (*proc_ptr == NULL) {
    *proc_ptr = calloc(1, sizeof(Process));
    (*proc_ptr)->cmd = cmd;
  } else
    (*proc_ptr)->next = create_process(&(*proc_ptr)->next, cmd);

  return *proc_ptr;
}