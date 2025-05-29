#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"
#include "job_control.h"

extern int is_exit;

int executor(Job *job, Job **job_head);

#endif