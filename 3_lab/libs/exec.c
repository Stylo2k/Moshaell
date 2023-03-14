#include <stdbool.h>

static int exitCode = 0;
static bool alwaysTrue = false;

#include "../common.h"

char* builtin[] = {
    "status",
    "exit",
    "history",
    "cd",
    NULL
};

void executeBuiltIn(char* name) {
    if (!name) {
        return;
    }

    if (strcmp(name, "history") == 0) {
        printSessionHistory();
    } else if (experimental) {
        addToHistory(name);
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

    // TODO: fix cd -
    if (strcmp(name, "cd") == 0) {
        if (noOptions()) {
            printf("Error: cd requires folder to navigate to!\n");
            exitCode = 2;
        } else {
            addBinPathToOptions(name);
            exitCode = chdir(getArgAt(1));
            // if the directory does not exist, print an error
            if (exitCode == -1) {
                printf("Error: cd directory not found!\n");
                exitCode = 2;
            }
        }
    }
    cleanUp();
}

bool doesBinaryExist(char* name) {
    if (!name) {
        return false;
    }
    if (strcmp(name, "") == 0) {
        return false;
    }

    for(int i = 0; builtin[i] != NULL; i++) {
        if (strcmp(name, builtin[i]) == 0) {
            return true;
        }
    }
    
    char* localName = malloc(strlen(name) + 1);
    strcpy(localName, name);


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
            if (cmdPath) free(cmdPath);
            break;
        }
        if (cmdPath) free(cmdPath);
        path = strtok(NULL, ":");
    }
    
    if (path == NULL) {
        return false;
    }

    if (fullPath) free(fullPath);

    if (localName) free(localName);
    return true;
}

char* findBinary(char* name) {
    if (!name) {
        return NULL;
    }

    if (strcmp(name, "") == 0) {
        return NULL;
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
            if (fullPath) free(fullPath);
            addBinPathToOptions(cmdPath);
            return cmdPath;
        }
        if (cmdPath) free(cmdPath);
        path = strtok(NULL, ":");
    }
    
    if (!builtin && path == NULL) {
        printf("Error: command not found!\n");
        // if the command is not found, we should just ignore it
        exitCode = 127;
    }

    if (fullPath) free(fullPath);

    return NULL;
}

int execCommands(Command* commands) {
    if (!commands) {
        return 0;
    }

    int prev_pipe, pfds[2];
    prev_pipe = STDIN_FILENO;

    int n = getCommandCount();
    
    pid_t* pids = malloc(sizeof(pid_t) * n);
    
    for (int i = 0; i < n; i++) {
        if (pipe(pfds) == -1) {perror("pipe");exit(1);}

        Command* command = getCommandAt(i);

        int commandIn = command->in;
        int commandOut = command->out;

        pid_t pid = fork();
        
        if (pid == 0) {
            if (i != n -1) {
                // command input has been set to something other than stdin
                if (commandIn != STDIN_FILENO) {
                    dup2(commandIn, STDIN_FILENO);
                    close(commandIn);
                } else if (prev_pipe != STDIN_FILENO) {
                    // Redirect previous pipe to stdin
                    dup2(prev_pipe, STDIN_FILENO);
                    close(prev_pipe);
                }
                
                // command output has been set to something other than stdout
                if (commandOut != STDOUT_FILENO) {
                    dup2(commandOut, STDOUT_FILENO);
                    close(commandOut);
                } else {
                    // Redirect stdout to current pipe
                    dup2(pfds[1], STDOUT_FILENO);
                    close(pfds[1]);
                }

            } else {
                // last command
                if (commandIn != STDIN_FILENO) {
                    dup2(commandIn, STDIN_FILENO);
                    close(commandIn);
                } else if (prev_pipe != STDIN_FILENO) {
                    // Get stdin from last pipe
                    dup2(prev_pipe, STDIN_FILENO);
                    close(prev_pipe);
                }

                if (commandOut != STDOUT_FILENO) {
                    dup2(commandOut, STDOUT_FILENO);
                    close(commandOut);
                }
            }
            execv(command->path, command->args);
            exit(1);
        } else {
            //  close the file descriptors
            if (commandIn != STDIN_FILENO) close(commandIn);
            if (commandOut != STDOUT_FILENO) close(commandOut);

            // save the pid
            pids[i] = pid;

            // Close read end of previous pipe (not needed in the parent)
            if(prev_pipe != STDIN_FILENO) close(prev_pipe);
            // Close write end of current pipe (not needed in the parent)
            close(pfds[1]);
            // Save read end of current pipe to use in next iteration
            prev_pipe = pfds[0];
        }
    }

    // wait for all the children to finish
    // and set the exitCode to the last child's exit code
    for (int i = 0; i < n; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)) {
            exitCode = WEXITSTATUS(status);
        }
    }

    if(pids) free(pids);
    return exitCode;
}

/**
 * @brief Execute the command 
 * 
 * @return int the exit code of the command
 */
int execCommand(Command* command) {
    bool builtIn = command->isBuiltIn;
    char* commandName = command->name;

    if (builtIn) {
        DEBUG("Executing built-in command %s\n", commandName);
        executeBuiltIn(commandName);
        cleanUp();
        return exitCode;
    }
    alwaysTrue = false;

    
    if (experimental) {
        addToHistory(commandName);
    }
 
    char* commandPath = command->path;
    if (!commandPath) {    
        // command not found, return 127
        cleanUp();
        printf("Error: command not found!\n");
        exitCode = 127;
        return 127;
    }
    

    int commandIn = command->in;
    int commandOut = command->out;

    int pipefd[2];
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }


    pid_t pid = fork();
    if (pid == 0) {
        // if command in is not stdin, redirect it
        if (commandIn != STDIN_FILENO) {
            dup2(commandIn, STDIN_FILENO);
            close(commandIn);
        }

        // if command out is not stdout, redirect it
        if (commandOut != STDOUT_FILENO) {
            dup2(commandOut, STDOUT_FILENO);
            close(commandOut);
        }

        // this will be performed by the child process
        // so execute the command
        DEBUG("Exec %s\n", commandPath);

        execv(commandPath, command->args);

    } else {
        // this will be performed by the parent process
        // so tell it to wait for the child process to finish
        int status;
        wait(&status);
        exitCode = WEXITSTATUS(status);
        cleanUp();
        
        return exitCode;
    }
    return 0;
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
