%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <stdbool.h>
  #include <math.h>
  #include <string.h>
  #include "lib.h"
  #include <ctype.h>
  
  bool silent = true;
  bool experimental = false;

  void yyerror(char *msg);
  extern int yylex(void);
  extern char *yytext;
  extern void showErrorLine();
  extern void initLexer(FILE *f);
  extern void finalizeLexer();
  extern int exitCode;
  extern char** commandArgs;
  
  void findBinary(char* name);
  void resetFlags();

  int isOR = -1;
  bool isBuiltIn = false;
  bool alwaysTrue = false;
  char* latestCMD;

  int execChain();

  bool nomalizeCode(int code);
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
%type <ival> InputLine Chain

%start program

%%

program : InputLine;

InputLine :   Chain     AMP        InputLine 
            | Chain OR_OP {isOR = 1;} InputLine  {isOR = -1; exitCode = alwaysTrue || nomalizeCode($1) || nomalizeCode($4); $$ = exitCode;}
            | Chain AND_OP {isOR = 0;} InputLine {isOR = -1; exitCode = alwaysTrue || (nomalizeCode($1) && nomalizeCode($4)); $$ = exitCode;}
            | Chain SEMICOLON InputLine
            | Chain NEWLINE {printShellPrompt();} InputLine
            | NEWLINE {printShellPrompt();} InputLine {$$ = 0;}
            | {$$ = 0;}
            ;

Chain : Pipeline Redirections {$$ = execChain();}
        | BUILTIN {latestCMD = $1;} Options {isBuiltIn = true; $$ = execChain();}
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

bool nomalizeCode(int code) {
  return code == EXIT_FAILURE || code == CMD_NOT_FOUND;
}

int execChain() {
  int returnCode = 0;
  if (isOR == -1) { // first time
    returnCode = execCommand(latestCMD, isBuiltIn); 
  } else if (isOR && exitCode == EXIT_SUCCESS) {
    cleanUp();
    returnCode = EXIT_SUCCESS;
  } else if (isOR && nomalizeCode(exitCode)) {
    returnCode = execCommand(latestCMD, isBuiltIn); 
  } else if (!isOR && exitCode == EXIT_SUCCESS) {
    returnCode = execCommand(latestCMD, isBuiltIn); 
  } else if (!isOR && nomalizeCode(exitCode)) {
    cleanUp();
    returnCode = EXIT_FAILURE;
  }
  resetFlags();
  return returnCode;
}

void resetFlags() {
  isBuiltIn = false;
  alwaysTrue = false;
  isOR = -1;
}


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
  if (!silent) {
    showErrorLine();
    printToken(yychar);
    printf(").\n");
  }
  printf("Error: invalid syntax!\n");
  printShellPrompt();
  yyparse();
}


int main(int argc, char *argv[]) {
  if (argc > 3) {
    return EXIT_FAILURE;
  }
  
  FILE *f = stdin;
  if (argc == 2) {
    if (strcmp(argv[1], "-s") == 0) {
      silent = true;
    } else if (strcmp(argv[1], "-v") == 0) {
      silent = false;
    } else if (strcmp(argv[1], "-e") == 0) {
      experimental = true;
    }
  }

  printShellPrompt();

  initLexer(f);
  yyparse();
  finalizeLexer();
  return EXIT_SUCCESS;
}
