#ifndef EXEC_H
#define EXEC_H

#include "pipeline.h"

void executeBuiltIn(Command* command);
char* findBinary(char* name);
int execCommand(Command* command, bool backGround);
bool isAlwaysTrue();
void setAlwaysTrue(bool value);
void setExitCode(int code);
int getExitCode();
bool doesBinaryExist(char* name);
int execCommands(Command* commands);

#endif
