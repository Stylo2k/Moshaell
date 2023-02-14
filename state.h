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

#endif