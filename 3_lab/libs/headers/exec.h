#ifndef EXEC_H
#define EXEC_H

#include "pipeline.h"

void executeBuiltIn(char* name);
void findBinary(char* name);
int execCommand(Command* command);
bool isAlwaysTrue();
void setAlwaysTrue(bool value);
void setExitCode(int code);
int getExitCode();
bool doesBinaryExist(char* name);

#endif
