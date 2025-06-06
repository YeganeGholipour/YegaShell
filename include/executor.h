/*
 * file:   executor.h
 * author: Yegane
 * date:   2025-06-06
 * desc:   High-level function for execution phase
 */

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "job_control.h"
#include "parser.h"


int executor(Job *job, Job **job_head);

#endif