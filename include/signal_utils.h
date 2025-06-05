#ifndef SIGNAL_UTILS_H
#define SIGNAL_UTILS_H

#include <signal.h>

#include "job_utils.h"

extern volatile sig_atomic_t interrupted;
extern volatile sig_atomic_t child_changed;

void sigint_handler(int sig);
void sigquit_handler(int sig);
void sigchld_handler(int sig);
void ignore_job_control_signals(void);
void init_shell_signals(void);
int block_parent_signals(sigset_t *block_list, sigset_t *prev_list, Job *job);
void install_child_signal_handler();

#endif