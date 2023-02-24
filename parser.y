%{
  #include "common.h"
  
  void resetFlags();
  
  extern char *yytext;
  void yyerror(char *msg);

  int isOR = -1;
  bool isBuiltIn = false;
  char* latestCMD;
  extern WordState state;
  int execChain();

  bool silent;
  bool experimental;

%}


%union {
  int ival;     
  double dval;  
  char* str;
}

%token EXECUTABLE OPTION FILENAME AMP AND_OP OR_OP SEMICOLON BUILTIN GT LT PIPE_OP NEWLINE

%type <str> EXECUTABLE OPTION BUILTIN
%type <ival> InputLine Chain

%start program

%%

program : {printShellPrompt();} InputLine;

InputLine :   Chain     AMP        InputLine 
            | Chain OR_OP {isOR = 1;} InputLine  {isOR = -1;setAlwaysTrue(false);}
            | Chain AND_OP {isOR = 0;} InputLine {isOR = -1;setAlwaysTrue(false);}
            | Chain End InputLine {isOR = -1; setAlwaysTrue(false); $$ = $1;}
            | Chain {isOR = -1; setAlwaysTrue(false); $$ = $1;}
            | {isOR = -1; setAlwaysTrue(false); $$ = 0;}
            ;

End :       SEMICOLON
            | NEWLINE {printShellPrompt();}
            | SEMICOLON NEWLINE
            ;

Chain :     Pipeline Redirections                         {
                                                            $$ = execChain();
                                                            resetFlags();
                                                          }
            | BUILTIN                                     {
                                                            latestCMD = $1;
                                                          }
            Options                                       {
                                                            isBuiltIn = true;
                                                            $$ = execChain();
                                                            resetFlags();
                                                          }
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

Command: EXECUTABLE {
                      latestCMD = $1;
                    }
         Options;

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
  return code == EXIT_SUCCESS || isAlwaysTrue();
}

int execChain() {
  int exitCode = getExitCode();
  bool alwaysTrue = isAlwaysTrue();

  DEBUG("isOR: %d isBuiltIn: %d alwaysTrue: %d exitCode: %d latestCMD: %s\n", isOR, isBuiltIn, alwaysTrue, exitCode, latestCMD);

  if (isOR == -1) { // first time
    return execCommand(latestCMD, isBuiltIn);
  }

  if(isOR && successExitCode(exitCode)) {
    if (latestCMD) free(latestCMD);
    latestCMD = NULL;
    cleanUp();
    return EXIT_SUCCESS;
  }

  if (isOR && failureExitCode(exitCode)) {
    return execCommand(latestCMD, isBuiltIn); 
  }
  
  if (!isOR && successExitCode(exitCode)) {
    return execCommand(latestCMD, isBuiltIn); 
  }
  
  if (!isOR && failureExitCode(exitCode)) {
    if (latestCMD) free(latestCMD);
    latestCMD = NULL;
    cleanUp();
    return EXIT_FAILURE;
  }

  return EXIT_FAILURE;
}

void resetFlags() {
  isBuiltIn = false;
  isOR = -1;
  state = COMMAND_STATE;
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
  setExitCode(EXIT_FAILURE);
  printShellPrompt();
  yyparse();
}


int main(int argc, char *argv[]) {
  if (argc > 3) {
    return EXIT_FAILURE;
  }
  
  silent = true;
  experimental = false;

  FILE *f = stdin;
  if (argc == 2) {
    if (strcmp(argv[1], "-s") == 0) {
      silent = true;
    } else if (strcmp(argv[1], "-v") == 0) {
      printf("Running in verbose mode\n");
      silent = false;
    } else if (strcmp(argv[1], "-e") == 0) {
      printf("Running in experimental mode\n");
      experimental = true;
    }
  }

  if(argc == 3) {
    if (strcmp(argv[1], "-s") == 0 && strcmp(argv[2], "-e") == 0) {
      printf("Running in silent and experimental mode\n");
      silent = true;
      experimental = true;
    } else if (strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "-e") == 0) {
      printf("Running in verbose and experimental mode\n");
      silent = false;
      experimental = true;
    } else {
      return EXIT_FAILURE;
    }
  }

  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  initLexer(f);
  yyparse();
  finalizeLexer();
  return EXIT_SUCCESS;
}
