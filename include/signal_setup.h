#ifndef SIGNAL_SETUP_H
#define SIGNAL_SETUP_H

#include <signal.h>

extern volatile sig_atomic_t interrupted;
extern volatile sig_atomic_t child_changed;

void sigint_handler(int sig);
void sigquit_handler(int sig);
void sigchld_handler(int sig);
void ignore_job_control_signals(void);
void init_shell_signals(void);

#endif