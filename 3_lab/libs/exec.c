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
    if(name) {
        free(name); 
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
    findBinary(commandName);
    // command not found, return 127
    if (noCommand()) {
        if(commandName) {
            free(commandName);
        }
        cleanUp();
        return 127;
    }

    
    if (experimental) {
        addToHistory(commandName);
    }
 
    char* commandPath = getBinPath();

    int cmdFileDescriptorIn = command->in;
    int cmdFileDescriptorOut = command->out;

    int pipefd[2];
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }


    pid_t pid = fork();
    if (pid == 0) {
        // read the input from the pipe
        if (cmdFileDescriptorIn != -1) {
            dup2(pipefd[0], STDIN_FILENO);
        }
        // write the output to the pipe
        if (cmdFileDescriptorOut != -1) {
            dup2(cmdFileDescriptorOut, STDOUT_FILENO);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        // this will be performed by the child process
        // so execute the command
        DEBUG("Exec %s\n", commandPath);
        execv(commandPath, command->args);

    } else {
        // write the cmdFileDescriptorIn to the pipe
        if (cmdFileDescriptorIn != STDIN_FILENO) {
            char buffer[1024];
            int bytesRead = 0;
            while ((bytesRead = read(cmdFileDescriptorIn, buffer, 1024)) > 0) {
                write(pipefd[1], buffer, bytesRead);
            }
            close(cmdFileDescriptorIn);
        }
        close(pipefd[1]);

        // write the output to the cmdFileDescriptorOut
        if (cmdFileDescriptorOut != STDOUT_FILENO) {
            char buffer[1024];
            int bytesRead = 0;
            while ((bytesRead = read(pipefd[0], buffer, 1024)) > 0) {
                write(cmdFileDescriptorOut, buffer, bytesRead);
            }
            close(cmdFileDescriptorOut);
        }
        close(pipefd[0]);

        // this will be performed by the parent process
        // so tell it to wait for the child process to finish
        int status;
        wait(&status);
        exitCode = WEXITSTATUS(status);
        cleanUp();
        
        // if (commandName) free(commandName);
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
