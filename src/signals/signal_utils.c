#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#include "signal_utils.h"
#include "job_utils.h"

volatile sig_atomic_t interrupted = 0;
volatile sig_atomic_t child_changed = 0;

void sigint_handler(int sig) {
  (void)sig;
  interrupted = 1;
  write(STDOUT_FILENO, "\n", 1);
}

void sigquit_handler(int sig) {
  (void)sig;
  interrupted = 1;
  write(STDOUT_FILENO, "\n", 1);
}

void sigchld_handler(int sig) {
  (void)sig;
  pid_t w;
  int status;
  while ((w = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    child_changed = 1;
    queue_pending_procs(w, status);
  }
}

void ignore_job_control_signals(void) {
  struct sigaction sa;
  sa.sa_flags = 0;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGTTIN, &sa, NULL) == -1) {
    perror("sigaction SIGTTIN");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTTOU, &sa, NULL) == -1) {
    perror("sigaction SIGTTOU");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTSTP, &sa, NULL) == -1) {
    perror("sigaction SIGTSTP");
    exit(EXIT_FAILURE);
  }
}

void init_shell_signals(void) {
  struct sigaction sa_sigint, sa_sigquit, sa_sigchld;

  sa_sigint.sa_handler = sigint_handler;
  sa_sigint.sa_flags = 0;
  sigemptyset(&sa_sigint.sa_mask);
  if (sigaction(SIGINT, &sa_sigint, NULL) == -1) {
    perror("sigaction SIGINT");
    exit(EXIT_FAILURE);
  }

  sa_sigquit.sa_handler = sigquit_handler;
  sa_sigquit.sa_flags = 0;
  sigemptyset(&sa_sigquit.sa_mask);
  if (sigaction(SIGQUIT, &sa_sigquit, NULL) == -1) {
    perror("sigaction SIGQUIT");
    exit(EXIT_FAILURE);
  }

  sa_sigchld.sa_handler = sigchld_handler;
  sa_sigchld.sa_flags = SA_RESTART;
  sigemptyset(&sa_sigchld.sa_mask);
  if (sigaction(SIGCHLD, &sa_sigchld, NULL) == -1) {
    perror("sigaction SIGCHLD");
    exit(EXIT_FAILURE);
  }
}

void install_child_signal_handler(void) {
  struct sigaction sa;
  sa.sa_flags = 0;
  sa.sa_handler = SIG_DFL;
  sigemptyset(&sa.sa_mask);

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGTSTP, &sa, NULL);
}

int block_parent_signals(sigset_t *block_list, sigset_t *prev_list, Job *job) {
  sigemptyset(block_list);
  sigaddset(block_list, SIGCHLD);

  if (!job->background) {
    sigaddset(block_list, SIGINT);
    sigaddset(block_list, SIGQUIT);
    sigaddset(block_list, SIGTSTP);
  }

  if (sigprocmask(SIG_BLOCK, block_list, prev_list) < 0) {
    perror("sigprocmask(block) before fork");
    return -1;
  }

  return 0;
}
