%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <stdbool.h>
  #include <math.h>
  #include <string.h>
  #include <ctype.h>

  void yyerror(char *msg);    /* forward declaration */
  /* exported by the lexer (made with flex) */
  extern int yylex(void);
  extern char *yytext;
  extern void showErrorLine();
  extern void initLexer(FILE *f);
  extern void finalizeLexer();
%}

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

%token EXECUTABLE OPTIONS FILENAME AMP AND_OP OR_OP SEMICOLON BUILTIN GT LT PIPE_OP NEWLINE
%start InputLine

%%
InputLine : Chain AMP InputLine 
            | BinOp NEWLINE InputLine
            | BinOp SEMICOLON InputLine 
            | 
            ;

BinOp : Chain
        | BinOp OR_OP BinOp
        | BinOp AND_OP BinOp
        ;

Chain : Pipeline Redirections
        | BUILTIN OPTIONS
        ;

Redirections : LT FILENAME GT FILENAME
              | GT FILENAME LT FILENAME
              | GT FILENAME
              | LT FILENAME
              |
              ;

Pipeline : Command PIPE_OP Pipeline
          | Command
          ;

Command: EXECUTABLE Options;

Options: OPTIONS | ;

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
  case OPTIONS: printf("OPTIONS<%s>", yytext);
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
