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

#define CMD_NOT_FOUND 127

#include "types.h"
#include "libs/libs.common.h"
#include "lexer.h"
#include "parser.tab.h"

typedef enum yylval_type {
    YYVAL_INT,
    YYVAL_STRING,
    YYVAL_DOUBLE
} YylvalType;


extern bool silent;
extern bool experimental;
extern bool alwaysTrue;
extern int exitCode;
extern bool alwaysTrue;

#endif
