/**
 * @file signal_utils.h
 * @brief Implements functionality for handling signals in both parent and child
 * before and after fork.
 * @author Yegane Gholipur
 * @date 2025-06-06
 */

#ifndef SIGNAL_UTILS_H
#define SIGNAL_UTILS_H

#include <signal.h>

#include "job_utils.h"

/**
 * @var interrupted
 * @brief Flag indicating whether the shell was interrupted before executing a
 * command.
 */
extern volatile sig_atomic_t interrupted;

/**
 * @var child_changed
 * @brief Flag indicating whether a child background process has changed state.
 */
extern volatile sig_atomic_t child_changed;

/**
 * @brief Ignores job control signals.
 *
 * This function ignores job control signals, allowing the main loop of the
 * shell to continue execution without interruption.
 */
void ignore_job_control_signals(void);

/**
 * @brief Initializes shell signals.
 *
 * This function initializes shell signals, setting up the signal handling
 * mechanisms for the program.
 */
void init_shell_signals(void);

/**
 * @brief Blocks parent signals.
 *
 * This function blocks parent signals, preventing the parent process from
 * receiving signals while the process group is being set up.
 *
 * @param block_list  Pointer to the signal set to block.
 * @param prev_list   Pointer to the previous signal mask.
 * @param job         Pointer to the job structure.
 *
 * @return 0 on success, -1 on failure.
 */
int block_parent_signals(sigset_t *block_list, sigset_t *prev_list, Job *job);

/**
 * @brief Installs the child signal handler.
 *
 * This function installs the child signal handler, setting up the signal
 * handling mechanisms for the child process.
 */
void install_child_signal_handler();

#endif