/*
 * file:   expander.h
 * author: Yegane
 * date:   2025-06-06
 * desc:   Implements functionality for expanding environment variables used in execution phase.
 */

#ifndef EXPANDER_H
#define EXPANDER_H

#include "parser.h"

#define MAXSIZ 1024

extern int last_exit_status;

void expander(Command *cmd, char **envp);

#endif