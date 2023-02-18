#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "state.h"

WordState state;
Opera operator = NONE;

char* commandPath = NULL;
char** commandArgs = NULL;
int exitCode = 0;
int prev = 0;


void cleanUp() {
    if(commandPath) free(commandPath);
    commandPath = NULL;
    if (!commandArgs) {
        return;
    }
    int i = 0;
    while (commandArgs[i]) {
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


void addOption(char* option) {
    if (commandArgs == NULL) {
        commandArgs = malloc(sizeof(char**)*2);
        commandArgs[0] = malloc(1);
        commandArgs[1] = NULL;
    }
    // start from 1, since 0 is the command
    int i = 1;
    while (commandArgs[i] != NULL) {
        i++;
    }
    commandArgs = realloc(commandArgs, sizeof(char**) * (i + 2));
    commandArgs[i] = malloc(strlen(option) + 1);
    strcpy(commandArgs[i], option);
    commandArgs[i + 1] = NULL;
    if (option) {
        free(option);
    }
}


/**
 * @brief Execute the command 
 * 
 * @return int the exit code of the command
 */
int execCommand() {
    int link[2];

    if (pipe(link) == -1) {
        perror("pipe");
        exit(1);
    }

    // command not found, return 127
    if (!commandPath) {
        return 127;
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
        // read the entire output of the command using a for loop
        char buffer[1024];
        int bytesRead = 0;
        while ((bytesRead = read(link[0], buffer, 1024)) > 0) {
            buffer[bytesRead] = '\0';
            printf("%s", buffer);
        }

        // this will be performed by the parent process
        // so tell it to wait for the child process to finish
        int status;
        wait(&status);
        exitCode = WEXITSTATUS(status);
        cleanUp();
        
        state = COMMAND_STATE;

        return exitCode;
    }
}

void findBinary(char* name) {
    if (!name) {
        return;
    }

    // verify that the command is valid, by looking it up in the
    // PATH environment variable
    char* origFullPath = getenv( "PATH" );
    
    // we need to make a copy of the original string, since its a pointer
    char* fullPath = malloc(strlen(origFullPath) + 1);
    strcpy(fullPath, origFullPath);

    char* path = strtok(fullPath, ":");
    struct stat st;
    while (path != NULL) {
        char* fullPath = malloc(strlen(path) + strlen(name) + 2);
        if (name[0] == '~' || name[0] == '.' || name[0] == '/') {
            // absolute path so just use the name
            strcpy(fullPath, name);
        } else {
            strcpy(fullPath, path);
            strcat(fullPath, "/");
            strcat(fullPath, name);
        }

        if (stat(fullPath, &st) == 0) {
            fprintf(stderr, "Command found at %s\n", fullPath);
            
            if (commandPath == NULL) {
                commandPath = malloc(strlen(fullPath) + strlen(name) + 2);
                strcpy(commandPath, fullPath);
                if (commandArgs == NULL) {
                    commandArgs = malloc(sizeof(char**)*2);
                    commandArgs[1] = NULL;
                } 
                commandArgs[0] = malloc(strlen(fullPath) + 1);
                strcpy(commandArgs[0], fullPath);
            }
            if (fullPath) {
                free(fullPath);
            }
            break;
        }
        if (fullPath) {
            free(fullPath);
        }
        path = strtok(NULL, ":");
    }
    
    if (path == NULL) {
        printf("Error: command not found!\n");
        // if the command is not found, we should just ignore it
        exitCode = 127;
        state = COMMAND_STATE;
    } else {
        state = OPTION_STATE;
    }
    if (name) {
        free(name);
    }
    if (fullPath) {
        free(fullPath);
    }
}

extern int alwaysTrue;
void executeBuiltIn(char* name) {
    if (!name) {
        return;
    }
    if (strcmp(name, "status") == 0) {
        printf("The most recent exit code is: %d.\n", exitCode);
        alwaysTrue = 1;
    }
    
    if (strcmp(name, "exit") == 0) {
        cleanUp();
        exit(0);
    }

    if (strcmp(name, "cd") == 0) {
        if (commandArgs[1] == NULL) {
            exitCode = chdir(getenv("HOME"));
        } else {
            exitCode = chdir(commandArgs[1]);
        }
    }
    if(name) {
        free(name); 
    }
    cleanUp();
}