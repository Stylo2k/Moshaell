#ifndef PIPELINE_H
#define PIPELINE_H


typedef struct Command {
    char *name;
    char* path;
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


Command *newCommand(char* name, char** args, int argCount);
void setPathForCommand(Command *cmd, char* path);
void addCommandToPipeline(Command *cmd);
void addCommandToPipelineWithArgs(char* name, char** args, int argCount);

Command* getCommand(int index);
void resetPipeline();
Command* getCommandAt(int index);
void configureInput(Command *cmd, int fd);
void configureOutput(Command *cmd, int fd);
int getCommandCount();

#endif
