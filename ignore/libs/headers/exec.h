#ifndef EXEC_H
#define EXEC_H

void executeBuiltIn(char* name);
void findBinary(char* name);
int execCommand(char* command, bool builtIn);

#endif