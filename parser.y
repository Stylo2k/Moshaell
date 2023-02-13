%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <stdbool.h>
  #include <math.h>
  #include "strtab.h"
  #include <string.h>
  #include <ctype.h>
  #include "libs/common.h"

  void yyerror(char *msg);    /* forward declaration */
  /* exported by the lexer (made with flex) */
  extern int yylex(void);
  extern char *yytext;
  extern void showErrorLine();
  extern void initLexer(FILE *f);
  extern void finalizeLexer();
%}

%token

// %left '+' '-'
// %left '*' '/' DIV MOD
// %left OR
// %left AND
// %left NOT

%union {
  int ival;     /* used for passing int values from lexer to parser */
  double dval;  /* used for passing double values from lexer to parser */
  /* add here anything you may need */
  /*....*/  
  char* str;
}

%token

%%





%%

void yyerror (char *msg) {
  showErrorLine();
  printToken(yychar);
  printf(").\n");
  exit(EXIT_SUCCESS);  /* EXIT_SUCCESS because we use Themis */
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

  initLexer(f);
  yyparse();
  finalizeLexer();
  return EXIT_SUCCESS;
}
