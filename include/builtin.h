#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

typedef struct {
    char *name;
    void (*func)(COMMAND *);
} Builtin;

extern Builtin builtin_commands[];
void cd_func(COMMAND *cmd);
void help_func(COMMAND *cmd);
void exit_func(COMMAND *cmd);
void pwd_func(COMMAND *cmd);
void export_func(COMMAND *cmd);

#endif