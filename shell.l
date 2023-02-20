%{
// Headers for use in this file
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "lib.h"
#include "state.h"
#include "parser.tab.h"

extern WordState state;
extern Opera operator;

extern char* commandPath;
extern char** commandArgs;
extern int exitCode;
extern int prev;
extern int quotesContext;

int linenr = 1;
int colnr  = 1;

static char *inputbuffer;

%}

%x string error

%option always-interactive

%%
<string>"\""        {
                      quotesContext = 2;
                      BEGIN(INITIAL);
                    }

<string>[^\"]+      {
                    quotesContext = 1;
                    char* copy = malloc(strlen(yytext) + 1);
                    strcpy(copy, yytext);

                    yylval.str = copy;
                    DEBUG("OPTION: %s\n", yytext);
                    return OPTION;
                    }
<error>\n           BEGIN(INITIAL); /* Return to normal parsing */
<error>.

"exit"              {
                        if (state == OPTION_STATE) {
                            DEBUG("OPTION: %s\n", yytext);
                            char *copy = malloc(strlen(yytext) + 1);
                            strcpy(copy, yytext);

                            yylval.str = copy;
                            return OPTION;
                        }
                        DEBUG("Performing exit\n");
                        char* copy = malloc(strlen(yytext) + 1);
                        strcpy(copy, yytext);
                        yylval.str = copy;
                        state = OPTION_STATE;
                        return BUILTIN;
                    }

    /* Other grammar parts */
"\""                BEGIN(string); /* We start reading a string until the next " char */
"&&"                {
                        operator = AND;
                        state = COMMAND_STATE;
                        return AND_OP;
                    }
"&"                 {
                        return AMP;
                        //TODO: add the gt lt
}

"||"                {
                        operator = OR;
                        state = COMMAND_STATE;
                        return OR_OP;
                    }
;                   { 
                        DEBUG("Performing ;\n");
                        state = COMMAND_STATE;
                        return SEMICOLON;
                    }
^\n                {
                        BEGIN(INITIAL);
                        printShellPrompt();
                    }
\n                  {
                        DEBUG("Performing \\n\n");
                        state = COMMAND_STATE;
                        return NEWLINE;
                    }
[ \t]+              {
                        addWhiteSpace();
                    }
"cd"                {
                        
                        char* copy = malloc(strlen(yytext) + 1);
                        strcpy(copy, yytext);
                        yylval.str = copy;
                        
                        state = OPTION_STATE;
                        return BUILTIN;
                    }
"status"            {  
                          if (state == OPTION_STATE) {
                            DEBUG("OPTION: %s\n", yytext);
                            char *copy = malloc(strlen(yytext) + 1);
                            strcpy(copy, yytext);

                            yylval.str = copy;
                            return OPTION;
                        }
                        char* copy = malloc(strlen(yytext) + 1);
                        strcpy(copy, yytext);
                        yylval.str = copy;

                        state = OPTION_STATE;
                        return BUILTIN;
                    }
[^ ;\t\n\"\^|]+     {
                        if (state == COMMAND_STATE) {
                            DEBUG("COMMAND: %s\n", yytext);
                            state= OPTION_STATE;
                            
                            char *copy = malloc(strlen(yytext) + 1);
                            strcpy(copy, yytext);

                            yylval.str = copy;

                            return EXECUTABLE;
                        } else if (state == OPTION_STATE) {
                            DEBUG("OPTION: %s\n", yytext);
                            
                            char *copy = malloc(strlen(yytext) + 1);
                            strcpy(copy, yytext);

                            yylval.str = copy;
                            
                            return OPTION;
                        } else if (state == FILENAME_STATE) {
                            DEBUG("FILENAME: %s\n", yytext);
                            return FILENAME;
                        }
                    }
<<EOF>>             {
                        /* At EOF we should unconditionally terminate! */
                        yyterminate();
                    }
.                   {
                        /* Error: unknown character! (probably doesn't happen) */
                        DEBUG("Unrecognized character: %s\n", yytext );
                        BEGIN(error);
                    }
%%


void initLexer(FILE *f) {
  int length;
  fseek (f, 0, SEEK_END);
  length = ftell (f);
  if (length < 0) {
    return;
  }
  fseek (f, 0, SEEK_SET);
  inputbuffer = malloc (length+1);
  inputbuffer[length]='\0';
  length = fread (inputbuffer, 1, length, f);
  fseek (f, 0, SEEK_SET);
  yyin = f;
}

void showErrorLine() {
  char *p = inputbuffer;
  int line = linenr;
  while ((*p != '\0') && (line > 1)) {
    line -= (*p == '\n');
    p++;
  }
  printf("line %3d: ", linenr);
  while ((*p != '\0') && (*p != '\n')) {
    putchar(*p != '\t' ? *p : ' ');
    p++;
  }
  putchar('\n');
  printf("---------");
  for (int i=colnr-strlen(yytext); i>0; i--) {
    putchar('-');
  }
  printf("^\n");
}
 
void finalizeLexer() {
  free(inputbuffer);
  yylex_destroy();
}