#ifndef STATE_H
#define STATE_H

// enum for the state of the shell
typedef enum wordState {
    COMMAND_STATE,
    OPTION_STATE,
    FILENAME_STATE,
    ARGUMENT,
    REDIRECT,
    PIPE,
    END
} WordState;


typedef struct options {
    char** commandArgs;
    int numArgs;
} Options;


typedef struct History {
    char** commands;
    char*** args;
    int* numArgs;
    int numCommands;
} History;

typedef struct Command {
    char *name;
    char *path;
    char **args;
    int argc;
    int in;
    int out;
    int err;
    bool isBuiltIn;
    struct Command *next;
} Command;

typedef struct Pipeline {
    Command* commands;
    int command_count;
} Pipeline;


#endif
