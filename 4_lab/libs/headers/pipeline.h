#ifndef PIPELINE_H
#define PIPELINE_H


#include "../../types.h"

Command *newCommand(char* name, char** args, int argCount);
void addCommandToPipeline(Command *cmd);
void addCommandToPipelineWithArgs(char* name, char** args, int argCount);

Command* getCommand(int index);
void resetPipeline();
Command* getCommandAt(int index);
void addBuiltInToPipelineWithArgs(char* name, char** args, int argCount);
void configureInput(Command *cmd, int fd);
void configureOutput(Command *cmd, int fd);
int getCommandCount();
Command* getFirstCommand();
Command* getLastCommand();
void configureError(Command *cmd, int fd);
void freeAtExit();

#endif
