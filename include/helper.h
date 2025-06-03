#ifndef HELPER_H
#define HELPER_H

#include "job_control.h"

int is_buitin(Process *proc);
int builtin_routine(int func_num, Process *proc_head, Job **job_struct,
                    Process **process_struct, Command **command_struct);

void clean_up(Job **job_struct, char *line_buffer);

#endif