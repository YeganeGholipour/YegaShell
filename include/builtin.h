#ifndef BUILTIN_H
#define BUILTIN_H

#include "job_control.h"

typedef struct {
  char *name;
  int (*func)(Job *, Job **);
} Builtin;

extern Builtin builtin_commands[];
int cd_func(Job *job, Job **job_head);
int help_func(Job *job, Job **job_head);
int exit_func(Job *job, Job **job_head);
int pwd_func(Job *job, Job **job_head);
int export_func(Job *job, Job **job_head);
int fg_func(Job *job, Job **job_head);
int bg_func(Job *job, Job **job_head);

#endif