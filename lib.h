#ifndef LIB_H
#define LIB_H


#define CMD_NOT_FOUND 127

void addToCurrentCommandArgs(char* yytext);
int execCommand(char* command, bool builtIn);
void addOption(char* option);
void findBinary(char* yytext);
void printShellPrompt();
void cleanUp();
void executeBuiltIn(char* name);
void DEBUG(const char *fmt, ...);

#endif