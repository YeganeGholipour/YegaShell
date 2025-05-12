#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

typedef struct {
    char *name;
    int (*func)(COMMAND *);
} Builtin;

extern Builtin builtin_commands[];
int cd_func(COMMAND *cmd);
int help_func(COMMAND *cmd);
int exit_func(COMMAND *cmd);
int pwd_func(COMMAND *cmd);
int export_func(COMMAND *cmd);

#endif