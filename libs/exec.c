#include <stdbool.h>

static int exitCode = 0;
static bool alwaysTrue = false;

#include "../common.h"



void executeBuiltIn(char* name) {
    if (!name) {
        return;
    }
    if (strcmp(name, "status") == 0) {
        printf("The most recent exit code is: %d\n", exitCode);
        alwaysTrue = true;
    }
    
    if (strcmp(name, "exit") == 0) {
        cleanUp();
        finalizeLexer();
        free(name);
        exit(0);
    }

    if (strcmp(name, "cd") == 0) {
        if (noOptions()) {
            exitCode = chdir(getenv("HOME"));
        } else {
            addBinPathToOptions(name);
            exitCode = chdir(getArgAt(1));
            // if the directory does not exist, print an error
            if (exitCode == -1) {
                printf("cd: no such file or directory: %s\n", getArgAt(1));
            }
        }
    }
    if(name) {
        free(name); 
    }
    cleanUp();
}


#include "../common.h"

void findBinary(char* name) {
    if (!name) {
        return;
    }

    if (strcmp(name, "") == 0) {
        return;
    }

    // verify that the command is valid, by looking it up in the
    // PATH environment variable
    char* origFullPath = getenv( "PATH" );
    
    // we need to make a copy of the original string, since its a pointer
    // get the cwd
    char* fullPath = malloc(strlen(origFullPath) + 4);
    strcpy(fullPath, origFullPath);
    // add the current directory to the path
    strcat(fullPath, ":");
    strcat(fullPath, ".");

    char* path = strtok(fullPath, ":");
    struct stat st;
    while (path != NULL) {
        char* cmdPath = malloc(strlen(path) + strlen(name) + 2);
        if (name[0] == '~' || name[0] == '.' || name[0] == '/') {
            // absolute path so just use the name
            strcpy(cmdPath, name);
        } else {
            strcpy(cmdPath, path);
            strcat(cmdPath, "/");
            strcat(cmdPath, name);
        }

        if (stat(cmdPath, &st) == 0) {
            DEBUG("Command found at %s\n", cmdPath);
            addBinPathToOptions(cmdPath);
            if (cmdPath) free(cmdPath);
            break;
        }
        if (cmdPath) free(cmdPath);
        path = strtok(NULL, ":");
    }
    
    if (path == NULL) {
        printf("Error: command not found!\n");
        // if the command is not found, we should just ignore it
        exitCode = 127;
    }

    if (fullPath) free(fullPath);
}

/**
 * @brief Execute the command 
 * 
 * @return int the exit code of the command
 */
#include "../common.h"

int execCommand(char* command, bool builtIn) {
    if (builtIn) {
        DEBUG("Executing built-in command %s\n", command);
        executeBuiltIn(command);
        cleanUp();
        return exitCode;
    }
    alwaysTrue = false;
    findBinary(command);
    // command not found, return 127
    if (noCommand()) {
        if(command) {
            free(command);
        }
        cleanUp();
        return 127;
    }

    if (experimental) addToHistory(command);

    char* commandPath = getBinPath();

    pid_t pid = fork();
    if (pid == 0) {
        // this will be performed by the child process
        // so execute the command
        DEBUG("Exec %s\n", commandPath);
        execv(commandPath, getArgs());
    } else {
        // this will be performed by the parent process
        // so tell it to wait for the child process to finish
        int status;
        wait(&status);
        exitCode = WEXITSTATUS(status);
        cleanUp();
        
        if (command) free(command);
        return exitCode;
    }
}

bool isAlwaysTrue() {
    return alwaysTrue;
}

void setAlwaysTrue(bool value) {
    alwaysTrue = value;
}

void setExitCode(int code) {
    exitCode = code;
}

int getExitCode() {
    return exitCode;
}