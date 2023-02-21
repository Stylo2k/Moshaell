#ifndef LIB_H
#define LIB_H


#define CMD_NOT_FOUND 127

int execCommand(char* command, bool builtIn);
void addOption(char* option);
void findBinary(char* yytext);
void printShellPrompt();
void cleanUp();
void executeBuiltIn(char* name);
void DEBUG(const char *fmt, ...);
void executeBuiltIn(char* name);
void findBinary(char* name);
void printPlainPrompt();
void printShellPrompt();
void setExitCode(int code);
int getExitCode();
bool isAlwaysTrue();
void setAlwaysTrue(bool value);

#endif
