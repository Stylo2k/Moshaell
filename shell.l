%{
// Headers for use in this file
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "lib.h"
#include "state.h"
#include "parser.tab.h"

extern WordState state;
extern Opera operator;

extern char* commandPath;
extern char** commandArgs;
extern int exitCode;
extern int prev;
extern int wasQuotes;

int linenr = 1;
int colnr  = 1;

static char *inputbuffer;

%}

/**
 * Parsing in flex is done based on a series of regexes. Below, we list these regexes
 * in order, flex will try to match the input with these in order. As soon as any regex
 * matches, it will run the code associated with it. Because of this order, we should
 * put our more specific regexes at the top: put a match for "exit" before a general
 * string regex.
 *
 * Optionally, you can specify a "start condition" (you can see it as some sort of
 * context) for a rule. This causes the regex to only be tried when the parsing is
 * in this context. You can control the current context using BEGIN(). The default
 * start condition is "INITIAL". So below, as soon as we hit a " char, we enter the
 * "STRING" context that will enable us to match any character until the next ",
 * after which we return to the "INITIAL" context.
 *
 * You could decide to hook up Bison to this part as well, as can be seen in many
 * tutorials online. Then you would store/return tokens from the actions in these rules.
 * However, for the scope of this simple grammar for the shell, that is really not
 * necessary and a few state variables should suffice to get the functionality you need!
 */

/* Here we inform flex that we have two additional "start conditions", besides INITIAL */
%x string error

/* Here we inform flex to not "look ahead" in stdin beyond what is necessary, to prevent
 * issues with passing stdin to another executable. */
%option always-interactive

%%
    /* From here on, comments must be indented! */

    /* Here we start with the rules. The highest priority rules are those to accept any
     * symbol when we are reading a string (so in the STRING context). */
    
    /* The first rule is to match the closing " char */
<string>"\""        BEGIN(INITIAL); /* Return to normal parsing */

    /* The second rule takes the longest string of characters not being " */
<string>[^\"]+      {
                        /* Here we match any entire string. We should either make this
                         * the command to execute, or store this as an option, or it is
                         * a filename, depending on the current state! */

                        // Entire string value is available in yytext with length yyleng
                        // Make sure to copy it!
                        // printf("STRING: %s\n", yytext);
                        char* copy = malloc(strlen(yytext) + 1);
                        strcpy(copy, yytext);

                        yylval.str = copy;
                        fprintf(stderr, "OPTION: %s\n", yytext);
                        return OPTION;
                    }

    /* Here we have cases for error handling - when we have encountered an invalid syntax,
     * we want to discard the entire line so we continue until the next newline symbol. */
<error>\n           BEGIN(INITIAL); /* Return to normal parsing */
<error>.

    /* From here on, we have only "normal" rules for our parsing */

    /* Built-in commands */
"exit"              {
                        /* For built-in command names, make sure that we are not currently
                         * parsing options or some other place where we cannot reasonably
                         * come across an actual "exit" command. So only perform some action
                         * here if you are sure that it is safe and valid to do so, otherwise
                         * use REJECT to let flex continue to the next match (as a string,
                         * later on). */

                        char* copy = malloc(strlen(yytext) + 1);
                        strcpy(copy, yytext);
                        yylval.str = copy;
                        state = COMMAND_STATE;
                        return BUILTIN;
                    }

    /* Other grammar parts */
"\""                BEGIN(string); /* We start reading a string until the next " char */
"&&"                {
                        // printf("Performing &&\n");
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
                        // TODO: check exit code if non zero execute next command
                    }
;                   { 
                        fprintf(stderr, "Performing ;\n");
                        prev = 0;
                        //TODO: check if we should add an if
                        state = COMMAND_STATE;
                        return SEMICOLON;
                    }
\n                  {
                        fprintf(stderr, "Performing \\n\n");
                        prev = 0;
                        //TODO: check if we should add an if
                        state = COMMAND_STATE;
                        return NEWLINE;
                    }
[ \t]+              /* Ignore whitespace */
"cd"                {
                        
                        char* copy = malloc(strlen(yytext) + 1);
                        strcpy(copy, yytext);
                        yylval.str = copy;
                        
                        state = OPTION_STATE;
                        return BUILTIN;
                    }
"status"            {  
                      // TODO: this might be broken
                        char* copy = malloc(strlen(yytext) + 1);
                        strcpy(copy, yytext);
                        yylval.str = copy;

                        state = COMMAND_STATE;
                        return BUILTIN;
                    }
[^ ;\t\n\"\^|]+     {
                        /* Here we match any sequence of characters without whitespace as a
                         * "word" or so. We should either make this the command to execute,
                         * or store this as an option, or it is a filename, depending on the
                         * current state! */

                        // Entire string value is available in yytext with length yyleng
                        // Make sure to copy it!
                        if (state == COMMAND_STATE) {
                            fprintf(stderr, "COMMAND: %s\n", yytext);
                            state= OPTION_STATE;
                            
                            char *copy = malloc(strlen(yytext) + 1);
                            strcpy(copy, yytext);

                            yylval.str = copy;

                            return EXECUTABLE;
                        } else if (state == OPTION_STATE) {
                            fprintf(stderr, "OPTION: %s\n", yytext);
                            
                            char *copy = malloc(strlen(yytext) + 1);
                            strcpy(copy, yytext);

                            yylval.str = copy;
                            
                            return OPTION;
                        } else if (state == FILENAME_STATE) {
                            fprintf(stderr, "FILENAME: %s\n", yytext);
                            return FILENAME;
                        }
                    }
<<EOF>>             {
                        /* At EOF we should unconditionally terminate! */
                        yyterminate();
                    }
.                   {
                        /* Error: unknown character! (probably doesn't happen) */
                        fprintf(stdout, "Unrecognized character: %s\n", yytext );
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
  
  inputbuffer = calloc (length+1, sizeof(char));
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