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

#endif
