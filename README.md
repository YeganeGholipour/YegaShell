# YegaShell

> **My First Shell in C: The Messy Truth About What Worked and What Failed**  
> A minimal, educational Unix‐style shell written in C.  
> Read the full blog post [here](https://dev.to/yeganegholipour/my-first-shell-project-in-c-the-messy-truth-about-what-worked-and-what-failed-5ge8), where I break down my mistake  and lesson learned.

---

## Table of Contents

1. [Project Overview](#project-overview)  
2. [Features](#features)  
3. [Prerequisites](#prerequisites)  
4. [Building & Installation](#building--installation)  
5. [Usage](#usage)  
6. [Directory Structure](#directory-structure)  
7. [Tests](#tests)  
8. [Known Issues](#known-issues)  
9. [Planned Improvements](#planned-improvements)  
10. [How to Contribute](#how-to-contribute)  
11. [License](#license)  

---

## Project Overview

YegaShell is my very first attempt at writing a user‐level shell in C. The goal was to:

- Learn how Linux system calls (`fork`, `exec`, `pipe`, `signal`) actually work.  
- Gain hands‐on experience with process groups, job control, and pipeline management.  
- Document every mistake and misstep along the way.

This shell is far from production‐ready. It was built as an educational exercise, so expect rough edges, missing features, and edge‐case bugs. 

---

## Features

These are the features that currently work (or mostly work):

- **Tokenizer**  
  - Handles single quotes (`'`) and double quotes (`"`)  
  - Supports escape sequences (`\"`, `\n`, etc.) inside double quotes  

- **Expander**  
  - Expands environment variables: `$VARIABLE`, `$?`, `$$`  

- **External Command Execution**  
  - Runs binaries found in `$PATH`, including `ls`, `echo`, `grep`, etc.  

- **Built‐in Commands**  
  - `cd`  
  - `pwd`  
  - `export` / `unset`  
  - `exit`  
  - `help`  
  - Job control: `jobs`, `fg`, `bg`  

- **Job Control & Process Groups**  
  - Enables background (`&`) and foreground execution  
  - Handles `SIGINT` (`Ctrl+C`) and `SIGTSTP` (`Ctrl+Z`) and `SIGQUIT` `(Ctrl + D)` correctly for child processes  
  - Updates job status on demand  

- **Pipelines**  
  - Supports pipeline (`|`) chains (e.g., `ls | grep foo`)  

- **I/O Redirection**  
  - Simple redirection: `>`, `<` (e.g., `grep hello < input.txt > out.txt`)  

- **Error Handling**  
  - Prints appropriate error messages when commands fail, pipes deadlock, or system calls error out  

---

## Prerequisites

Before you compile and run YegaShell, make sure you have:

- **A Unix‐like operating system (Linux, macOS)**  
- **GCC (≥ 7.0) or Clang**  
- **Make** (for the provided Makefile)  
- **Git** (optional, for cloning)  
- **GDB & Valgrind** (recommended for debugging)

---

## Building & Installation

1. **Clone the repository**  
   ```bash
   git clone https://github.com/YeganeGholipour/YegaShell.git
   cd YegaShell
   ```

2. **Run `make`**  
   All object files and the final executable will be placed under the `build/` directory.  
   ```bash
   make
   ```
   - If you just type `make`, it builds everything.  
   - To rebuild from scratch, run:
     ```bash
     make clean
     make
     ```

3. **Locate the executable**  
   After `make` completes, the main shell binary is:
   ```
   build/my_program
   ```
   You can rename or move this binary anywhere you like.

---

## Usage

1. **Launch the shell**  
   ```bash
   ./build/my_program
   ```
   You should see a custom prompt (e.g., `YegaShell> `).

2. **Run commands**  
   - **External commands**:  
     ```bash
     YegaShell> ls -la
     YegaShell> echo "Hello, world!"
     ```
   - **Built‐in commands**:  
     ```bash
     YegaShell> cd /path/to/directory
     YegaShell> pwd
     YegaShell> export FOO=bar
     YegaShell> unset FOO
     YegaShell> help
     YegaShell> exit
     ```
   - **Pipelines**:  
     ```bash
     YegaShell> ls | grep e | wc -c
     ```
   - **Background jobs**:  
     ```bash
     YegaShell> sleep 10 &
     YegaShell> jobs
     YegaShell> fg %1
     ```
   - **I/O redirection**:  
     ```bash
     YegaShell> grep foo < input.txt > output.txt
     ```

3. **Known quirks**  
   - Pressing `Ctrl+C` or `Ctrl+Z` will send signals to the currently foreground job, not the shell itself. If there is no foreground job, the shell prints a new line character just like bash .  
   - Job notifications do not always display immediately; you may need to run `jobs` again to refresh statuses.  

---

## Directory Structure

```
.
├── build                # Compiled object files and final binaries (ignored by Git)
│   ├── builtin
│   ├── core
│   ├── env
│   ├── exec
│   ├── io
│   ├── job
│   ├── my_program       # Main executable
│   ├── parser
│   ├── signals
│   ├── tokenizer
│   └── utils
├── include              # Public headers for each module
│   ├── builtin.h
│   ├── env_utils.h
│   ├── executor.h
│   ├── expander.h
│   ├── helper.h
│   ├── io_redirection.h
│   ├── job_control.h
│   ├── job_utils.h
│   ├── parser.h
│   ├── process_control.h
│   ├── process_utils.h
│   ├── shell.h
│   ├── signal_utils.h
│   └── tokenizer.h
├── Makefile             # Build instructions (rules to compile *.c → *.o → my_program)
├── README.md            # ← You are here
├── src                  # Source files, organized by functionality
│   ├── builtin
│   │   └── builtin.c
│   ├── core
│   │   ├── main.c
│   │   └── shell.c
│   ├── env
│   │   └── env_utils.c
│   ├── exec
│   │   ├── executor.c
│   │   ├── process_control.c
│   │   └── process_utils.c
│   ├── io
│   │   └── io_redirection.c
│   ├── job
│   │   ├── job_control.c
│   │   └── job_utils.c
│   ├── parser
│   │   ├── expander.c
│   │   └── parser.c
│   ├── signals
│   │   └── signal_utils.c
│   ├── tokenizer
│   │   └── tokenizer.c
│   └── utils
│       └── helper.c
└── tests                # Unit & integration tests
    ├── integration      # (Empty or implementation‐specific scripts)
    └── unittests
        ├── env
        │   └── test_env.c
        ├── expander
        │   └── test_expander.c
        ├── job
        │   └── test_job.c
        ├── parser
        │   └── test_parser.c
        └── tokenizer
            └── test_tokenizer.c
```

- **build/**: Contains object files (*.o) and subdirectories mirroring `src/`. The final executable is `build/my_program`. The entire `build/` tree is ignored in Git (`.gitignore`), so you won’t see compiled artifacts in the repo.  
- **include/**: Header files for each module. Other modules `#include` these to share helper functions, types, and constants.  
- **src/**: All C source code, organized by feature:  
  - `core/`: Main entrypoint (`main.c`) and core shell loop (`shell.c`).  
  - `tokenizer/`: Splits input lines into tokens (handles quotes, escapes).  
  - `parser/`: Builds command data structures, performs variable expansion.  
  - `exec/`: Spawns child processes, manages `fork()`/`execve()`, sets up pipes.  
  - `job/`: Implements job control logic, process groups, and status tracking.  
  - `signals/`: Utility functions for blocking/unblocking signals, `SIGCHLD` handling.  
  - `io/`: Handles I/O redirection (`<`, `>`).  
  - `env/`: Manages environment variables (`export`, `unset`).  
  - `builtin/`: Implements built‐in commands (`cd`, `pwd`, `help`, `exit`, etc.).  
  - `utils/`: Miscellaneous helper functions (string utilities, error wrappers).  
- **tests/**: Contains unit tests (under `unittests/`) for some modules but it's not finished yet.

---

## Known Issues

- **Background Job Notifications**  
  Sometimes a background job finishes, but the shell does not immediately print a notification. You may need to manually run `jobs` or wait for the next prompt.  
- **Limited Pipeline Depth**  
  The shell currently supports simple pipelines (two or three commands). Deep or complex pipelines may fail silently.  
- **Basic I/O Redirection**  
  Only single‐file redirection is supported. Nested or multiple redirections (e.g., `cmd < in > out 2> err`) are not fully tested.  
- **No Command History / Tab Completion**  
  The shell does not yet support navigating command history (↑/↓) or tab completion.  
- **Edge‐Case Quoting & Escapes**  
  Some rare combinations of quotes, escapes, or variable expansions may not work as expected.  
- **Signal Races**  
  In rare cases, quickly typing `Ctrl+C` or `Ctrl+Z` while a process is spawning can cause the shell to exit or hang.  

If you find other bugs or weird behavior, please [open an issue](https://github.com/YeganeGholipour/YegaShell/issues/new) with as much detail as possible.

---

## Planned Improvements

Based on my ongoing learning and reader feedback (from the [blog post](https://dev.to/yeganegholipour/my-first-shell-project-in-c-the-messy-truth-about-what-worked-and-what-failed-5ge8)):

- **Command History**  
  Press ↑/↓ to navigate previous commands (using `readline` or a custom implementation).  
- **Tab Completion**  
  Auto‐complete file names and built‐in commands.  
- **Heredoc Support**  
  Handle here‐doc syntax (`<<EOF ... EOF`).  
- **Robust Job Notifications**  
  Fix missing background notifications so that `jobs` always reflects real‐time status.  
- **Improved I/O Redirection**  
  Support `2>`  and multiple redirections in a single command.  
- **Configurable Prompt**  
  Let users customize prompt (e.g., colors, current directory).  
- **Optimize Data Structures**  
  Replace simple linked lists with balanced trees or hash tables for faster job lookups.  
- **Unit & Integration Test Coverage**  
  Expand tests to cover `exec/`, `signals/`, and end‐to‐end scenarios.  


---

## How to Contribute

I’ll review your PR. I'm still learning so I appreciate any feedback from you.


---

## License

This project is released under the **MIT License**. See [LICENSE](LICENSE) for details.

