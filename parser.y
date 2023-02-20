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

  int quotesContext = 0;

  int execChain();

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
            | Chain OR_OP {isOR = 1;} InputLine  {isOR = -1;alwaysTrue = false;}
            | Chain AND_OP {isOR = 0;} InputLine {isOR = -1;alwaysTrue = false;}
            | Chain End InputLine {isOR = -1; alwaysTrue = false; $$ = $1;}
            | Chain {isOR = -1; alwaysTrue = false; $$ = $1;}
            | {isOR = -1; alwaysTrue = false; $$ = 0;}
            ;

End : SEMICOLON
      | NEWLINE
      | SEMICOLON NEWLINE
      ;

Chain :   Pipeline Redirections {$$ = execChain();}
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

bool failureExitCode(int code) {
  return code == EXIT_FAILURE || code == CMD_NOT_FOUND;
}

bool successExitCode(int code) {
  return code == EXIT_SUCCESS || alwaysTrue;
}

int execChain() {
  DEBUG("isOR: %d isBuiltIn: %d alwaysTrue: %d exitCode: %d latestCMD: %s\n", isOR, isBuiltIn, alwaysTrue, exitCode, latestCMD);
  int returnCode = 0;
  if (isOR == -1) { // first time
    returnCode = execCommand(latestCMD, isBuiltIn);
  } else if (isOR && successExitCode(exitCode)) {
    if (latestCMD) free(latestCMD);
    latestCMD = NULL;
    cleanUp();
    returnCode = EXIT_SUCCESS;
  } else if (isOR && failureExitCode(exitCode)) {
    returnCode = execCommand(latestCMD, isBuiltIn); 
  } else if (!isOR && successExitCode(exitCode)) {
    returnCode = execCommand(latestCMD, isBuiltIn); 
  } else if (!isOR && failureExitCode(exitCode)) {
    if (latestCMD) free(latestCMD);
    latestCMD = NULL;
    cleanUp();
    returnCode = EXIT_FAILURE;
  }
  resetFlags();
  return returnCode;
}

void resetFlags() {
  isBuiltIn = false;
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
  case EXECUTABLE   : printf("EXECUTABLE\n");
    break;
  case NEWLINE     : printf("NEWLINE\n");
    break;
  case OPTION: printf("OPTION<%s>\n", yytext);
    break;
  case FILENAME       : printf("FILENAME<%s>\n", yytext);
    break;
  case AMP     : printf("AMP\n");
    break;
  case AND_OP     : printf("AND_OP\n");
    break;
  case OR_OP : printf("OR_OP\n");
    break;
  case SEMICOLON: printf("SEMICOLON\n");
    break;
  case BUILTIN        : printf("BUILTIN\n");
    break;
  case GT   : printf("GT\n");
    break;
  case LT      : printf("LT\n");
    break;
  case PIPE_OP  : printf("PIPE_OP\n");
    break;
  }
}

void yyerror (char *msg) {
  if (!silent) {
    // showErrorLine();
    printToken(yychar);
    // printf(").\n");
  }
  printf("Error: invalid syntax!\n");
  exitCode = EXIT_FAILURE;
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

  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  printShellPrompt();

  initLexer(f);
  yyparse();
  finalizeLexer();
  return EXIT_SUCCESS;
}
