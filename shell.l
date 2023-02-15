%{
// Headers for use in this file
#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void execCommand();
void addOption(char* option);
void findBinary(char* yytext);
void printShellPrompt();
void cleanUp();

WordState state;
Opera operator = NONE;

char* commandPath = NULL;
char** commandArgs = NULL;
int exitCode = 0;
int prev = 0;


//////////// Here you can put some helper functions and code, but make sure to properly
//////////// separate your code in logical "entities" in different files! This helps
//////////// us grade your code as well.

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
                        printf("STRING: %s\n", yytext);

                        //////////// Put your code here!
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

                        //////////// Put your code here!

                        REJECT;
                    }

    /* Other grammar parts */
"\""                BEGIN(string); /* We start reading a string until the next " char */
"&&"                {
                        printf("Performing &&\n");
                        operator = AND;
                        state = COMMAND_STATE;
                        if(prev==0){
                            execCommand();
                            prev = (exitCode==0);
                        } else{
                            cleanUp();
                        }
                        //////////// Put your code here!
                    }
"||"                {
                        printf("Performing ||\n");
                        operator = OR;
                        state = COMMAND_STATE;
                        if(prev == 0){
                            execCommand();
                            prev = (exitCode==0);
                        }
                        else{
                            cleanUp();
                        }
                        // TODO: check exit code if non zero execute next command
                        //////////// Put your code here!
                    }
;                   { 
                        fprintf(stderr, "Performing ;\n");
                        prev = 0;
                        //TODO: check if we should add an if
                        state = COMMAND_STATE;
                        execCommand();
                        printShellPrompt();
                        //////////// Put your code here!
                    }
\n                  {
                        fprintf(stderr, "Performing \\n\n");
                        prev = 0;
                        //TODO: check if we should add an if
                        state = COMMAND_STATE;
                        execCommand();
                        printShellPrompt();
                    }
[ \t]+              /* Ignore whitespace */
"status\n"            {
        printf("The most recent exit code is: %d.\n", exitCode);
        state = COMMAND_STATE;
        printShellPrompt();
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
                            findBinary(yytext);
                        } else if (state == OPTION_STATE) {
                            fprintf(stderr, "OPTION: %s\n", yytext);
                            addOption(yytext);
                        } else if (state == FILENAME_STATE) {
                            fprintf(stderr, "FILENAME: %s\n", yytext);
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

/* All code after the second pair of %% is just plain C where you typically
 * write your main function and such. */

void state_init() {
    // Initialize state
    state = COMMAND_STATE;
}


void cleanUp() {
    free(commandPath);
    commandPath = NULL;
    int i = 0;
    while (commandArgs[i] != NULL) {
        free(commandArgs[i]);
        i++;
    }
    free(commandArgs);
    commandArgs = NULL;
}

void printShellPrompt() {
    char* cwd = getcwd(NULL, 0);
    printf("%s$ ", cwd);
    free(cwd);
}

void findBinary(char* yytext) {
    // verify that the command is valid, by looking it up in the
    // PATH environment variable
    char* origFullPath = getenv( "PATH" );
    
    // we need to make a copy of the original string, since its a pointer
    char* fullPath = malloc(strlen(origFullPath) + 1);
    strcpy(fullPath, origFullPath);

    char* path = strtok(fullPath, ":");
    struct stat st;
    while (path != NULL) {
        char* fullPath = malloc(strlen(path) + strlen(yytext) + 2);
        strcpy(fullPath, path);
        strcat(fullPath, "/");
        strcat(fullPath, yytext);
        if (stat(fullPath, &st) == 0) {
            fprintf(stderr, "Command found at %s\n", fullPath);
            
            if (commandPath == NULL) {
                commandPath = malloc(strlen(fullPath) + strlen(yytext) + 2);
                strcpy(commandPath, fullPath);
                if (commandArgs == NULL) {
                    commandArgs = malloc(sizeof(char**)*2);
                    commandArgs[0] = malloc(strlen(fullPath) + 1);
                    strcpy(commandArgs[0], fullPath);
                    commandArgs[1] = NULL;
                } 
            }

            break;
        }
        path = strtok(NULL, ":");
    }
    
    if (path == NULL) {
        printf("Command not found\n");
        // if the command is not found, we should just ignore it
        //state = COMMAND_STATE; //!!!!!!*&^%&%$^%$%&^
    } else {
        state = OPTION_STATE;
    }
}

void addOption(char* option) {
    // start from 1, since 0 is the command
    int i = 1;
    while (commandArgs[i] != NULL) {
        i++;
    }
    commandArgs = realloc(commandArgs, sizeof(char**) * (i + 2));
    commandArgs[i] = malloc(strlen(option) + 1);
    strcpy(commandArgs[i], option);
    commandArgs[i + 1] = NULL;
}


void execCommand() {
    // if(operator==AND && exitCode!=0){
    //     operator = NONE;
    //     cleanUp();
    //     printShellPrompt();
    //     return;
    // }
    // if (operator==OR && exitCode == 0) {
    //     operator = NONE;
    //     cleanUp();
    //     printShellPrompt();
    //     return;
    // }

    // check if the command is "cd" 
    if (strcmp(commandArgs[0], "/usr/bin/cd") == 0) {
        if (commandArgs[1] == NULL) {
            chdir(getenv("HOME"));
        } else {
            chdir(commandArgs[1]);
        }
        cleanUp();
        printShellPrompt();
        return;
    }

    int link[2];

    if (pipe(link) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        // this will be performed by the child process
        // so execute the command
        fprintf(stderr, "Exec %s\n", commandPath);
        execv(commandPath, commandArgs);
    } else {
        close(link[1]);
        char buffer[1024];
        int bytesRead = read(link[0], buffer, 1024);
        buffer[bytesRead] = '\0';
        printf("%s\n\n", buffer);
        // this will be performed by the parent process
        // so tell it to wait for the child process to finish
        int status;
        wait(&status);
        exitCode = WEXITSTATUS(status);
        cleanUp();
        //printShellPrompt();
    }
}

int main() {
    // Initialize program
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // Set up some state
    state_init();
    
    printShellPrompt();
    // Start parsing process
    yylex();

    // Cleanup
    fclose(yyin);
    yylex_destroy();

    return EXIT_SUCCESS;
}
