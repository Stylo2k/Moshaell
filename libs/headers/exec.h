#ifndef EXEC_H
#define EXEC_H

void executeBuiltIn(char* name);
void findBinary(char* name);
int execCommand(char* command, bool builtIn);
bool isAlwaysTrue();
void setAlwaysTrue(bool value);
void setExitCode(int code);
int getExitCode();

#endif