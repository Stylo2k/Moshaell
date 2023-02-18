%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <stdbool.h>
  #include <math.h>
  #include <string.h>
  #include "lib.h"
  #include <ctype.h>
  

  void yyerror(char *msg);    /* forward declaration */
  /* exported by the lexer (made with flex) */
  extern int yylex(void);
  extern char *yytext;
  extern void showErrorLine();
  extern void initLexer(FILE *f);
  extern void finalizeLexer();
  extern int wasQuotes;
  
  void findBinary(char* name);

  extern int exitCode;
  extern char** commandArgs;

  int isOR = -1;
  int isAND = -1;
  int isBuiltIn = 0;
  int alwaysTrue = 0;
  char* latestCMD;
%}


%union {
  int ival;     /* used for passing int values from lexer to parser */
  double dval;  /* used for passing double values from lexer to parser */
  /* add here anything you may need */
  /*....*/  
  char* str;
}

%token EXECUTABLE OPTION FILENAME AMP AND_OP OR_OP SEMICOLON BUILTIN GT LT PIPE_OP NEWLINE

%type <str> EXECUTABLE OPTION BUILTIN
%type <ival> BinOp

%start InputLine

%%
InputLine :   Chain       AMP        InputLine 
            | BinOp NEWLINE {printShellPrompt();}    InputLine
            | BinOp SEMICOLON  InputLine 
            | 
            ;

BinOp :    BinOp OR_OP  {isOR = 1;}  BinOp  { isOR = -1; exitCode = alwaysTrue || $1 || $4; isBuiltIn = 0; alwaysTrue = 0; $$ = alwaysTrue || $1 || $4;}
        |  BinOp AND_OP {isOR = 0;}  BinOp  { isOR = -1; exitCode = alwaysTrue && $1 && $4; isBuiltIn = 0; alwaysTrue = 0; $$ = alwaysTrue && $1 && $4;}
        |  Chain { 
                  if (isOR == -1) { // first time
                    if (isBuiltIn) {
                      executeBuiltIn(latestCMD);
                      $$ = exitCode;
                    } else {
                      findBinary(latestCMD);
                      $$ = execCommand(); 
                    }
                  } else if (isOR && exitCode == EXIT_SUCCESS) {
                    cleanUp();
                    $$ = EXIT_SUCCESS;
                  } else if (isOR && exitCode == EXIT_FAILURE) {
                    if (isBuiltIn) {
                      executeBuiltIn(latestCMD);
                      $$ = exitCode;
                    } else {
                      findBinary(latestCMD);
                      $$ = execCommand(); 
                    }
                  } else if (!isOR && exitCode == EXIT_SUCCESS) {
                    if (isBuiltIn) {
                      executeBuiltIn(latestCMD);
                      $$ = exitCode;
                    } else {
                      findBinary(latestCMD);
                      $$ = execCommand(); 
                    }
                  } else if (!isOR && exitCode == EXIT_FAILURE) {
                    cleanUp();
                    $$ = EXIT_FAILURE;
                  }
                }
        ;

Chain : Pipeline Redirections
        | BUILTIN {latestCMD = $1;} Options {isBuiltIn = 1;}
        ;

Redirections : LT FILENAME GT FILENAME
              | GT FILENAME LT FILENAME
              | GT FILENAME
              | LT FILENAME
              |
              ;

Pipeline :  Command PIPE_OP Pipeline
          | Command
          ;

Command: EXECUTABLE {latestCMD = $1;} Options;

Options: OPTION {
                  addOption($1);
                } 
        Options
        |
        ;

%%


void printToken(int token) {
  /* single character tokens */
  if (token < 256) {
    if (token < 33) {
      /* non-printable character */
      printf("chr(%d)", token);
    } else {
      printf("'%c'", token);
    }
    return;
  }
  /* standard tokens (>255) */
  switch (token) {
  case EXECUTABLE   : printf("EXECUTABLE");
    break;
  case NEWLINE     : printf("NEWLINE");
    break;
  case OPTION: printf("OPTION<%s>", yytext);
    break;
  case FILENAME       : printf("FILENAME<%s>", yytext);
    break;
  case AMP     : printf("AMP");
    break;
  case AND_OP     : printf("AND_OP");
    break;
  case OR_OP : printf("OR_OP");
    break;
  case SEMICOLON: printf("SEMICOLON");
    break;
  case BUILTIN        : printf("BUILTIN");
    break;
  case GT   : printf("GT");
    break;
  case LT      : printf("LT");
    break;
  case PIPE_OP  : printf("PIPE_OP");
    break;
  }
}

void yyerror (char *msg) {
  // showErrorLine();
  // printToken(yychar);
  // printf(").\n");
  printf("Error: invalid syntax!\n");
  // exit(EXIT_SUCCESS);  /* EXIT_SUCCESS because we use Themis */
  printShellPrompt();
  yyparse();
}


int main(int argc, char *argv[]) {
  if (argc > 2) {
    return EXIT_FAILURE;
  }
  
  FILE *f = stdin;
  if (argc == 2) {
    f = fopen(argv[1], "r");
    if (f == NULL) {
      exit(EXIT_FAILURE);
    }
  }
  printShellPrompt();

  initLexer(f);
  yyparse();
  finalizeLexer();
  return EXIT_SUCCESS;
}
