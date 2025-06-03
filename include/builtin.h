#ifndef BUILTIN_H
#define BUILTIN_H

#include "job_control.h"

typedef struct {
  char *name;
  int (*func)(Process *, Job **);
} Builtin;

extern Builtin builtin_commands[];
int cd_func(Process *proc, Job **job_head);
int help_func(Process *proc, Job **job_head);
int exit_func(Process *proc, Job **job_head);
int pwd_func(Process *proc, Job **job_head);
int export_func(Process *proc, Job **job_head);
int fg_func(Process *proc, Job **job_head);
int bg_func(Process *proc, Job **job_head);
int jobs_func(Process *proc, Job **job_head);

#endif