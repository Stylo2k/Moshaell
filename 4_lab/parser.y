%{
  #include "common.h"
  #include <glob.h>
  #include <getopt.h>

  void resetFlags();
  
  extern char *yytext;
  void yyerror(char *msg);

  int isOR = -1;
  bool isBuiltIn = false;
  extern WordState state;

  int execChain();
  char* lastBuiltin;

  bool silent;
  bool experimental;

  bool latestCMDPiped = false;
  int execPipeline();

  int currentRedirIndex = 0;
  bool backGround = false;


  const struct option longOpts[] = {
    {"silent", no_argument, NULL, 's'},
    {"verbose", no_argument, NULL, 'v'},
    {"experimental", no_argument, NULL, 'e'},
    {"code", required_argument, NULL, 'c'},
    {"file", required_argument, NULL, 'f'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
  };

  const char * shortOpts = "svehf:c:";
%}


%union {
  int ival;     
  double dval;  
  char* str;
}

%token EXECUTABLE OPTION FILENAME AMP AND_OP OR_OP SEMICOLON BUILTIN GT LT PIPE_OP NEWLINE GLOB NR_GT NR_LT GT_GT

%type <str> EXECUTABLE OPTION BUILTIN FILENAME Command GLOB
%type <ival> InputLine Chain Pipeline NR_GT NR_LT

%start program

%%

program : {printShellPrompt();} InputLine;

InputLine :   Chain     AMP AmpEnd {backGround = true; execChain();resetFlags();}        InputLine 
            | Chain OR_OP {execChain();resetFlags(); isOR = 1;} InputLine  {isOR = -1;setAlwaysTrue(false);}
            | Chain AND_OP {execChain();resetFlags(); isOR = 0;} InputLine {isOR = -1;setAlwaysTrue(false);}
            | Chain End {execChain();resetFlags();} InputLine                {isOR = -1; setAlwaysTrue(false); $$ = $1;}
            | Chain {execChain();resetFlags();}                              {isOR = -1; setAlwaysTrue(false); $$ = $1;}
            |                                    {isOR = -1; setAlwaysTrue(false); $$ = 0;}
            ;

AmpEnd :
        End 
        |
        ;

End :       SEMICOLON
            | NEWLINE {printShellPrompt();}
            | SEMICOLON NEWLINE
            ;

Chain :     Pipeline Redirections
            | BUILTIN {lastBuiltin = $1;} Options
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
                                            if (fd == -1 || fd1 == -1) {
                                              printf("Error: cannot open file for reading\n");
                                            } else {
                                              configureInput(getFirstCommand(), fd1); 
                                              configureOutput(getLastCommand(), fd);
                                            }
                                          }
                                          if($2) free($2);
                                          if($4) free($4);
                                        }
              | GT FILENAME             {
                                          int fd = open($2, O_TRUNC|O_CREAT|O_WRONLY, 0666);
                                          if (fd == -1) {
                                            printf("Error: input and output files cannot be equal!\n");
                                            resetPipeline();
                                          } else {
                                            configureOutput(getLastCommand(), fd);
                                          }
                                          if($2) free($2);
                                        }
              | GT_GT FILENAME          {
                                          int fd = open($2, O_APPEND|O_CREAT|O_WRONLY, 0666);
                                          if (fd == -1) {
                                            printf("Error: cannot open file %s for writing\n", $2);
                                          } else {
                                            configureOutput(getLastCommand(), fd);
                                          }
                                          if($2) free($2);
                                        }
              | LT FILENAME             {
                                          int fd = open($2, O_RDONLY);
                                          if (fd == -1) {
                                            printf("Error: cannot open file %s for reading\n", $2);
                                          } else {
                                            configureInput(getLastCommand(), fd);
                                          }
                                          if($2) free($2);
                                        }
              | NR_GT FILENAME      {
                                          int fd = open($2, O_TRUNC|O_CREAT|O_WRONLY, 0666);
                                          if (fd == -1) {
                                            printf("Error: cannot open file %s for writing\n", $2);
                                          } else {
                                            if ($1 == 1) {
                                              configureOutput(getLastCommand(), fd);
                                            } else if ($1 == 2) {
                                              configureError(getLastCommand(), fd);
                                            } else {
                                              printf("Error: invalid file descriptor %d\n", $1);
                                            }
                                          }
                                          if($2) free($2);
                                        }
              | NR_GT FILENAME NR_GT FILENAME 
                                              {
                                                int fd = open($2, O_TRUNC|O_CREAT|O_WRONLY, 0666);
                                                int fd1 = open($4, O_TRUNC|O_CREAT|O_WRONLY, 0666);
                                                if (fd == -1 || fd1 == -1) {
                                                  printf("Error: cannot open file for writing\n");
                                                } else {
                                                  if ($1 == 1) {
                                                    configureOutput(getLastCommand(), fd);
                                                  } else if ($1 == 2) {
                                                    configureError(getLastCommand(), fd);
                                                  } else {
                                                    printf("Error: invalid file descriptor %d\n", $1);
                                                  }
                                                  if ($3 == 1) {
                                                    configureOutput(getLastCommand(), fd1);
                                                  } else if ($3 == 2) {
                                                    configureError(getLastCommand(), fd1);
                                                  } else {
                                                    printf("Error: invalid file descriptor %d\n", $3);
                                                  }
                                                }
                                                if($2) free($2);
                                                if($4) free($4);
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
            Pipeline 
                                        {
                                          $$ = $4;
                                        }
            | Command 
                                        {
                                          addCommandToPipelineWithArgs($1, getOptions(), getNumberOfOptions());
                                          cleanUp();
                                          currentRedirIndex++;
                                        }
          ;

Command: EXECUTABLE {
                    }
         Options 
                    {
                      $$ = $1;
                    };

Options: OPTION { 
                    addOption($1);
                } 
        Options
        |
        GLOB { 
                  // char **found;
                  // glob_t gstruct;
                  // int err;
                  // err = glob($1, GLOB_NOCHECK, NULL, &gstruct);
                  
                  // if(err) {
                  //     if( err != GLOB_NOMATCH ) {
                  //       fprintf(stderr,"Some kinda glob error\n");
                  //       exit(1);
                  //     }
                  // }
                  
                  // found = gstruct.gl_pathv;
                  // while(*found) {
                  //   addOption(strdup(*found));
                  //   found++;
                  // }
                  // globfree(&gstruct);
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
  while ((command = getCommandAt(index))) {
    exitCode = execCommand(command, backGround);
    printf("just executed %s\n", command->name);
    index++;
  }
  return exitCode;
}

int execChain() {
  int exitCode = getExitCode();

  if (lastBuiltin) {
    isBuiltIn = true;
    addBuiltInToPipelineWithArgs(lastBuiltin, getOptions(), getNumberOfOptions());
  } else if (latestCMDPiped) {
    exitCode = execCommands(getCommandAt(0), backGround);
    resetPipeline();
    return exitCode;
  }

  if (isOR == -1) { // first time
    exitCode = execPipeline();
    resetPipeline();
    return exitCode;
  }

  if(isOR && successExitCode(exitCode)) {
    resetPipeline();
    return EXIT_SUCCESS;
  }

  if (isOR && failureExitCode(exitCode)) {
    exitCode = execPipeline();
    resetPipeline();
    return exitCode;
  }
  
  if (!isOR && successExitCode(exitCode)) {
    exitCode = execPipeline();
    resetPipeline();
    return exitCode;
  }
  
  if (!isOR && failureExitCode(exitCode)) {
    resetPipeline();
    return EXIT_FAILURE;
  }

  return EXIT_FAILURE;
}

void resetFlags() {
  isBuiltIn = false;
  isOR = -1;
  currentRedirIndex = 0;
  // state = COMMAND_STATE;
  latestCMDPiped = false;
  backGround = false;
  lastBuiltin = NULL;
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
    printToken(yychar);
  }
  printf("Error: invalid syntax!\n");
  cleanUp();
  resetPipeline();
  resetFlags();
  state = COMMAND_STATE;
  printShellPrompt();
  yyparse();
}

void printHelpMenu(char **argv) {
  printf("Welcome to the most sophisticated shell the couse Operating Systems has ever seen 😎!\n\n");
  printf("Usage: %s [-s] [-v] [-e] [-f file] [-c command] [-h]\n", argv[0]);
  printf("  -s, --silent\t\tSilent mode\n");
  printf("  -v, --verbose\t\tVerbose mode\n");
  printf("  -e, --experimental\tExperimental mode\n");
  printf("  -f, --file\t\tRead from file\n");
  printf("  -c, --code\t\tRead from code between quotes\n");
  printf("  -h, --help\t\tPrint this help message\n");
}

int main(int argc, char *argv[]) {  
  silent = true;
  experimental = false;

  FILE *f = stdin;
  int opt;
  while ((opt = getopt_long(argc, argv, shortOpts, longOpts, NULL)) != -1) {
      switch (opt) {
      case 's':
        silent = true;
        break;
      case 'v':
        silent = false;
        break;
      case 'e':
        experimental = true;
        break;
      case 'f':
        // f to be set to file
        f = fopen(optarg, "r");
        break;
      case 'c':
        // get the code between quotes and set f to it
        // f = fmemopen(optarg, strlen(optarg), "r");
        break;
      case 'h':
        printHelpMenu(argv);
        return EXIT_SUCCESS;
      default:
        printHelpMenu(argv);
        return EXIT_FAILURE;
      }
    }

    for (int i = optind; i < argc; i++) {
      if (argv[i][0] != '-' && argv[i][1] != '-') {
          // f to be set to file
          f = fopen(argv[i], "r");
      }
    }


  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  if (experimental) {
    readConfigFile();
  }

  struct sigaction sa;
  sa.sa_sigaction = handleSigInt;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_SIGINFO; 
  sigaction(SIGINT, &sa, NULL);

  // handle sigchld
  struct sigaction sa2;
  sa2.sa_sigaction = handleSigChld;
  sigemptyset(&sa2.sa_mask);
  sa2.sa_flags = SA_RESTART | SA_SIGINFO;
  sigaction(SIGCHLD, &sa2, NULL);


  initLexer(f);
  yyparse();
  finalizeLexer();
  return EXIT_SUCCESS;
}