#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "builtin.h"
#include "executor.h"

int is_buitin(Process *proc) {
  char *command = proc->cmd->argv[0];
  for (int i = 0; builtin_commands[i].name != NULL; i++) {
    if (strcmp(command, builtin_commands[i].name) == 0)
      return i;
  }
  return -1;
}

int builtin_routine(int func_num, Process *proc_head, Job **job_struct,
                    Process **process_struct, Command **command_struct) {
  last_exit_status = builtin_commands[func_num].func(proc_head, job_struct);

  free(proc_head);
  free_struct_memory(*command_struct);
  *process_struct = NULL;
  *command_struct = NULL;

  if (strcmp(builtin_commands[func_num].name, "exit") == 0) {
    return -1;
  }
  return 0;
}

void clean_up(Job **job_struct, char *line_buffer) {
  kill_jobs(job_struct);
  free_all_jobs(job_struct);
  free(line_buffer);
}