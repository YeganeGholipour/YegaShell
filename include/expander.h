#ifndef EXPANDER_H
#define EXPANDER_H

#include "parser.h"

#define MAXSIZ 1024

extern int last_exit_status;

void expander(Command *cmd, char **envp);

#endif