#ifndef HELPER_H
#define HELPER_H

#include "job_utils.h"

int is_bulitin(Process *proc);
int builtin_routine(int func_num, Process *proc_head, Job **job_ptr,
                    Process **process_ptr, Command **command_ptr);

void clean_up(Job **job_ptr, char *line_buffer);

#endif