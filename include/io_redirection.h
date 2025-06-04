#ifndef IO_REDIRECTION_H
#define IO_REDIRECTION_H

#include "parser.h"
#include <stdlib.h>

int child_stdout_setup(Command *cmd, int (*pipes)[2], int proc_num,
                       int num_procs);
int child_stdin_setup(Command *cmd, int (*pipes)[2], int proc_num);
void close_pipe_ends(int num_procs, int (*pipes)[2]);
void free_pipes_and_pids(int (*pipes)[2]);

#endif