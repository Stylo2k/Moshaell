#ifndef LIB_H
#define LIB_H

void addToCurrentCommandArgs(char* yytext);
int execCommand();
void addOption(char* option);
void findBinary(char* yytext);
void printShellPrompt();
void cleanUp();
void executeBuiltIn(char* name);

#endif