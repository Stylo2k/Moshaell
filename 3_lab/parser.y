%{
  #include "common.h"
  
  void resetFlags();
  
  extern char *yytext;
  void yyerror(char *msg);

  int isOR = -1;
  bool isBuiltIn = false;
  extern WordState state;
  int execChain();

  bool silent;
  bool experimental;

  bool latestCMDPiped = false;
  int execPipeline();

  int currentRedirIndex = 0;

  #define NOTPIPED false
  #define PIPED true

%}


%union {
  int ival;     
  double dval;  
  char* str;
}

%token EXECUTABLE OPTION FILENAME AMP AND_OP OR_OP SEMICOLON BUILTIN GT LT PIPE_OP NEWLINE

%type <str> EXECUTABLE OPTION BUILTIN FILENAME Command
%type <ival> InputLine Chain Pipeline

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

Chain :     Pipeline Redirections
                                                          {
                                                            int exitCode = 0;
                                                            if (latestCMDPiped) {
                                                              exitCode = execCommands(getCommand(0));
                                                            } else {
                                                              exitCode  = execChain();
                                                            }
                                                            resetPipeline();
                                                            resetFlags();
                                                            $$ = exitCode;
                                                          }
            | BUILTIN                                     {
                                                            
                                                          }
            Options                                       {
                                                            isBuiltIn = true;
                                                            addBuiltInToPipelineWithArgs($1, getOptions(), getNumberOfOptions());
                                                            $$ = execChain();
                                                            resetFlags();
                                                          }
            ;

Redirections :  LT FILENAME GT FILENAME {
                                          if(strcmp($2, $4) == 0) {
                                            printf("Error: input and output files cannot be equal!\n");
                                            resetPipeline();
                                          } else {
                                            int fd = open($2, O_RDONLY);
                                            int fd1 = open($4, O_TRUNC|O_CREAT|O_WRONLY, 0666);
                                            if (fd == -1 || fd1 == -1) {
                                              printf("Error: cannot open file for reading\n");
                                            } else {
                                              configureInput(getCommandAt(0), fd); 
                                              configureOutput(getCommandAt(currentRedirIndex - 1), fd1); 
                                            }
                                          }
                                          if($2) free($2);
                                          if($4) free($4);
                                        }
              | GT FILENAME LT FILENAME {
                                          if(strcmp($2, $4) == 0) {
                                            printf("Error: input and output files cannot be equal!\n");
                                            resetPipeline();
                                          } else {
                                            int fd = open($2, O_TRUNC|O_CREAT|O_WRONLY, 0666);
                                            int fd1 = open($4, O_RDONLY);
                                            configureInput(getCommandAt(0), fd1); 
                                            configureOutput(getCommandAt(currentRedirIndex - 1), fd);
                                          }
                                          if($2) free($2);
                                          if($4) free($4);
                                        }
              | GT FILENAME             {
                                          configureOutput(getCommandAt(currentRedirIndex - 1), open($2, O_TRUNC|O_CREAT|O_WRONLY, 0666));
                                          if($2) free($2);
                                        }
              | LT FILENAME             {
                                          int fd = open($2, O_RDONLY);
                                          if (fd == -1) {
                                            printf("Error: cannot open file %s for reading\n", $2);
                                          } else {
                                            configureInput(getCommandAt(currentRedirIndex - 1), fd);
                                          }
                                          if($2) free($2);
                                        }
              |
              ;

Pipeline :  Command PIPE_OP 
                                {
                                  latestCMDPiped = true;
                                  addCommandToPipelineWithArgs($1, getOptions(), getNumberOfOptions());
                                  cleanUp();
                                  currentRedirIndex++;
                                }
            Pipeline {$$ = $4;}
          | Command {
                      addCommandToPipelineWithArgs($1, getOptions(), getNumberOfOptions());
                      cleanUp();
                      currentRedirIndex++;
                    }
          ;

Command: EXECUTABLE {
                    }
         Options {$$ = $1;};

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

int execPipeline() {
  int index = 0;
  int exitCode = 0;
  Command* command = NULL;
  while ((command = getCommand(index))) {
    exitCode = execCommand(command);
    index++;
  }
  return exitCode;
}

int execChain() {
  int exitCode = getExitCode();
  bool alwaysTrue = isAlwaysTrue();

  DEBUG("isOR: %d isBuiltIn: %d alwaysTrue: %d exitCode: %d: %s\n", isOR, isBuiltIn, alwaysTrue, exitCode);

  if (isOR == -1) { // first time
    exitCode = execPipeline();
    resetPipeline();
    cleanUp();
    return exitCode;
  }

  if(isOR && successExitCode(exitCode)) {
    cleanUp();
    resetPipeline();
    return EXIT_SUCCESS;
  }

  if (isOR && failureExitCode(exitCode)) {
    exitCode = execPipeline();
    resetPipeline();
    cleanUp();
    return exitCode;
  }
  
  if (!isOR && successExitCode(exitCode)) {
    exitCode = execPipeline();
    resetPipeline();
    cleanUp();
    return exitCode;
  }
  
  if (!isOR && failureExitCode(exitCode)) {
    cleanUp();
    resetPipeline();
    return EXIT_FAILURE;
  }

  return EXIT_FAILURE;
}

void resetFlags() {
  isBuiltIn = false;
  isOR = -1;
  currentRedirIndex = 0;
  state = COMMAND_STATE;
  latestCMDPiped = false;
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
  cleanUp();
  resetPipeline();
  resetFlags();
  state = COMMAND_STATE;
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
  if (experimental) {
    readConfigFile();
  }

  initLexer(f);
  yyparse();
  finalizeLexer();
  return EXIT_SUCCESS;
}
