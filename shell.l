%{
#include "common.h"

extern WordState state;

int linenr = 1;
int colnr  = 1;
static char *inputbuffer;

WordState state = COMMAND_STATE;
void setYYlval(void *obj, YylvalType type);
%}

%x string error

%option always-interactive

%%
<string>"\""        {
                      BEGIN(INITIAL);
                    }

<string>[^\"]+      {
                    setYYlval(yytext, YYVAL_STRING);
                    DEBUG("OPTION: %s\n", yytext);
                    return OPTION;
                    }
<error>\n           BEGIN(INITIAL); /* Return to normal parsing */
<error>.
"history"           {
                        DEBUG("Performing history\n");
                        setYYlval(yytext, YYVAL_STRING);
                        // state = OPTION_STATE;
                        return BUILTIN;
                    }
"exit"              {
                        if (state == OPTION_STATE) {
                            DEBUG("OPTION: %s\n", yytext);
                            setYYlval(yytext, YYVAL_STRING);
                            return OPTION;
                        }
                        DEBUG("Performing exit\n");
                        setYYlval(yytext, YYVAL_STRING);
                        state = OPTION_STATE;
                        return BUILTIN;
                    }

    /* Other grammar parts */
"\""                BEGIN(string); /* We start reading a string until the next " char */
"&&"                {
                        state = COMMAND_STATE;
                        return AND_OP;
                    }
"&"                 {
                        return AMP;
                        //TODO: add the gt lt
}

"||"                {
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

                    }
"cd"                {
                     // TODO: this has to change, we are better off by just calling the builtin from here   
                      setYYlval(yytext, YYVAL_STRING);
                      state = OPTION_STATE;
                      return BUILTIN;
                    }
"status"            {  
                      if (state == OPTION_STATE) {
                        DEBUG("OPTION: %s\n", yytext);
                        setYYlval(yytext, YYVAL_STRING);
                        return OPTION;
                      }
                      setYYlval(yytext, YYVAL_STRING);
                      state = OPTION_STATE;
                      return BUILTIN;
                    }
[^ ;\t\n\"\^|]+     {
                        if (state == COMMAND_STATE) {
                          // clear yytext from all unicode characters                          
                            DEBUG("COMMAND: %s\n", yytext);
                            state= OPTION_STATE;
                            setYYlval(yytext, YYVAL_STRING);
                            return EXECUTABLE;
                        } else if (state == OPTION_STATE) {
                            DEBUG("OPTION: %s\n", yytext);
                            setYYlval(yytext, YYVAL_STRING);
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

/**
  * This function is called by the lexer when it has found a token.
  * It sets the yylval variable to the correct value. 
  * @param obj The object to set the yylval to
  * @param type The type of the object
  */
void setYYlval(void *obj, YylvalType type) {
  // obj is generic pointer, so we need to cast it to the correct type
  
  if(type == YYVAL_STRING) {
    char* strObj = (char*) obj;
    char *str = malloc(strlen(strObj) + 1);
    strcpy(str, strObj);
    yylval.str = str;
  } else if (type == YYVAL_DOUBLE) {
    yylval.dval = *((double *) obj);
  } else if (type == YYVAL_INT) {
    yylval.ival = *((int *) obj);
  } else {
    fprintf(stderr, "Unknown YylvalType: %d\n", type);
    exit(1);
  }
}

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
  yyterminate();
}