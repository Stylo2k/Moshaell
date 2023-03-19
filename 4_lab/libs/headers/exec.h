#ifndef EXEC_H
#define EXEC_H

#include "pipeline.h"
#include <signal.h>

void executeBuiltIn(Command* command);
char* findBinary(char* name);
int execCommand(Command* command, bool backGround);
bool isAlwaysTrue();
void setAlwaysTrue(bool value);
void setExitCode(int code);
int getExitCode();
bool doesBinaryExist(char* name);
int execCommands(Command* commands, bool backGround);

void handleSigInt(int signo, siginfo_t *info, void *other);
void handleSigChld(int signo, siginfo_t *info, void *other);

void statusFunc(char* name);
void historyFunc(char* name);
void exitFunc(char* name);
void changeDirectory(char* name);
void sourceFunc(char* name);
void aliasFunc(char* name);
void popDir(char* name);
void printDirStack(char* name);
void printBGProcesses(char* _);
void killProcess(char* _);
void handleEOF();

#endif
