/*
 * file:   builtin.h
 * author: Yegane
 * date:   2025-06-06
 * desc:   prototypes for builtin functions     
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include "job_utils.h"

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
int unset_func(Process *proc, Job **job_head);
int fg_func(Process *proc, Job **job_head);
int bg_func(Process *proc, Job **job_head);
int jobs_func(Process *proc, Job **job_head);

#endif