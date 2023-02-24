#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

void finalizeLexer();
int yylex(void);
void showErrorLine();
void initLexer(FILE *f);
void readConfigFile();

#endif