#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "job_control.h"
#include "parser.h"


int executor(Job *job, Job **job_head);

#endif