#ifndef EXPANDER_H
#define EXPANDER_H

#include "parser.h"

#define MAXSIZ 1024

extern int last_exit_status;

void expander(COMMAND *cmd, char **envp);

#endif