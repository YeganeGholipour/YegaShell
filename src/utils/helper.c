/*
 * file:   helper.c
 * author: [Your Name]
 * date:   [Today's Date]
 * desc:   Collection of utility functions that provide supporting functionality for the YegaShell program.       
 */

#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "builtin.h"
#include "executor.h"

int is_bulitin(Process *proc) {
  char *command = proc->cmd->argv[0];
  for (int i = 0; builtin_commands[i].name != NULL; i++) {
    if (strcmp(command, builtin_commands[i].name) == 0)
      return i;
  }
  return -1;
}

int builtin_routine(int func_num, Process *proc_head, Job **job_ptr,
                    Process **process_ptr, Command **command_ptr) {
  last_exit_status = builtin_commands[func_num].func(proc_head, job_ptr);

  free(proc_head);
  free_struct_memory(*command_ptr);
  *process_ptr = NULL;
  *command_ptr = NULL;

  if (strcmp(builtin_commands[func_num].name, "exit") == 0) {
    return -1;
  }
  return 0;
}

void clean_up(Job **job_ptr, char *line_buffer) {
  kill_jobs(job_ptr);
  free_all_jobs(job_ptr);
  free(line_buffer);
}