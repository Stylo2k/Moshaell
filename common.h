#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdbool.h>
#include <termios.h>
#include <ctype.h>

#include "lib.h"
#include "lexer.h"
#include "types.h"
#include "parser.tab.h"

typedef enum yylval_type {
    YYVAL_INT,
    YYVAL_STRING,
    YYVAL_DOUBLE
} YylvalType;


static bool silent = true;
static bool experimental = false;

#endif
