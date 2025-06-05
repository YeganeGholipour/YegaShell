#include <fcntl.h>
#include <unistd.h>

#include "io_redirection.h"

int child_stdin_setup(Command *cmd, int (*pipes)[2], int proc_num) {
  if (cmd->infile) {
    int in_fd = open(cmd->infile, O_RDONLY);
    if (in_fd < 0)
      return -1;

    dup2(in_fd, STDIN_FILENO);
    close(in_fd);
  } else if (proc_num > 0) {
    dup2(pipes[proc_num - 1][0], STDIN_FILENO);
    close(pipes[proc_num - 1][0]);
  }

  return 0;
}

int child_stdout_setup(Command *cmd, int (*pipes)[2], int proc_num,
                              int num_procs) {
  if (cmd->outfile) {
    int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
    int out_fd = open(cmd->outfile, flags, 0644);
    if (out_fd < 0)
      return -1;

    dup2(out_fd, STDOUT_FILENO);
    close(out_fd);
  } else if (proc_num < num_procs - 1) {
    dup2(pipes[proc_num][1], STDOUT_FILENO);
    close(pipes[proc_num][1]);
  }

  return 0;
}

void close_pipe_ends(int num_procs, int (*pipes)[2]) {
  for (int i = 0; i < num_procs - 1; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }
}

void free_pipes_and_pids(int (*pipes)[2]) { free(pipes); }