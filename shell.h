#ifndef SHELL_H
#define SHELL_H

#include "keyboard.h"

// Maximum lengths for input handling
#define MAX_INPUT_LENGTH 256
#define MAX_ARGS 16
#define MAX_ARG_LENGTH 32

// Command function type
typedef void (*cmd_function)(int argc, char **argv);

// Command structure
typedef struct
{
    const char *name;
    cmd_function func;
    const char *help;
} command_t;

// Function declarations
void shell_init(void);
void shell_run(void);

#endif