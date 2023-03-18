#include <stdbool.h>

static int exitCode = 0;
static bool alwaysTrue = false;

#include "../common.h"

typedef struct BGProcess {
    pid_t pid;
} BGProcess;

typedef struct BGProcesses {
    struct BGProcess* processes;
    int size;
    int index;
} BGProcesses;

static BGProcesses* bgProcesses = NULL;

typedef struct DirectoryStack {
    char** directory;
    int index;
    int size;
} DirectoryStack;


static DirectoryStack* directoryStack = NULL;

typedef void (*function_ptr)(char*);

typedef struct {
    char* name;
    function_ptr func;
} BuiltIn;

void statusFunc(char* name);
void historyFunc(char* name);
void exitFunc(char* name);
void changeDirectory(char* name);
void sourceFunc(char* name);
void aliasFunc(char* name);
void popDir(char* name);
void printDirStack(char* name);
void printBGProcesses(char* _);

BuiltIn builtin[] = {
    {"status", statusFunc},
    {"history", historyFunc},
    {"source", sourceFunc},
    {"alias", aliasFunc},
    {"exit", exitFunc},
    {"popd", popDir},
    {"jobs", printBGProcesses},
    {"dirstack", printDirStack},
    {"cd", changeDirectory}
};

void aliasFunc(char* name) {
    if (noOptions()) {
        printf("No alias specified!\n");
        return;
    }
    char* alias = getArgAt(1);
    if (noOptions()) {
        printf("No command specified!\n");
        return;
    }
    int i = 2;
    char* command = calloc(strlen(getArgAt(i)), sizeof(char));
    while (getArgAt(i)) {
        command = strcat(command, getArgAt(i));
        if (getArgAt(i+1)) {
            command = strcat(command, " ");
        }
        i++;
    }
    addAlias(alias, command);
}

void sourceFunc(char* name) {
    char* sourceFilePath = getRcFilePath();
    if (!experimental) {
        fprintf(stderr, "Error: source is only available in experimental mode!\n");
        return;
    }
    if (noOptions() && sourceFilePath) {
        printf("No source file specified!\nTaking %s as source file\n", sourceFilePath);
    }
    if (noOptions() && !sourceFilePath) {
        printf("No source file specified and nothing found in local or home dir!\n");
        return;
    }
    addBinPathToOptions(name);
    if (getArgAt(1)) {
        sourceFilePath = getArgAt(1);
        printf("Taking %s as source file\n", sourceFilePath);
    }
    readSpecificRcFile(sourceFilePath);
}

void statusFunc(char* _) {
    printf("The most recent exit code is: %d\n", exitCode);
    alwaysTrue = true;
}

void historyFunc(char* _) {
    printSessionHistory();
}

bool anyBGProcesses() {
    if (!bgProcesses) {
        return false;
    }
    // loop through all processes and check if they are still running
    for (int i = 0; i < bgProcesses->index; i++) {
        pid_t pid = bgProcesses->processes[i].pid;
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == 0) {
            return true;
        }
    }
    return false;
}

void exitFunc(char* name) {
    int finalExitCode = 0;
    if (getArgAt(1)) {
        finalExitCode = atoi(getArgAt(1));
    }
    if(anyBGProcesses()) {
        printf("Error: there are still background processes running!\n");
        return;
    }
    cleanUp();
    freeAliasList();
    freeHistory();
    resetPipeline();
    finalizeLexer();
    freeAtExit();
    exit(finalExitCode);
}

void assertDirectoryStack() {
    if (!directoryStack) {
        directoryStack = calloc(1, sizeof(DirectoryStack));
        directoryStack->directory = calloc(10, sizeof(char*));
        directoryStack->index = 0;
        directoryStack->size = 10;
    }
    if (directoryStack->size == directoryStack->index) {
        directoryStack->directory = realloc(directoryStack->directory, (directoryStack->size * 2) * sizeof(char*));
        directoryStack->size *= 2;
    }
}

void pushDirectory(char* directory) {
    assertDirectoryStack();
    directoryStack->directory[directoryStack->index] = calloc(strlen(directory) + 1, sizeof(char));
    strcpy(directoryStack->directory[directoryStack->index], directory);
    directoryStack->index++;
}

void printDirStack(char* _) {
    assertDirectoryStack();
    for (int i = 0; i < directoryStack->index; i++) {
        printf("%d: %s\n", i, directoryStack->directory[i]);
    }
}

char* popDirectory() {
    assertDirectoryStack();
    if (directoryStack->index == 0) {
        return NULL;
    }
    directoryStack->index--;
    char* directory = directoryStack->directory[directoryStack->index];
    directoryStack->directory[directoryStack->index] = NULL;
    return directory;
}


void popDir(char* _) {
    char* directory = popDirectory();
    // cd to the directory
    if (directory) {
        printf("Popping directory: %s\n", directory);
        exitCode = chdir(directory);
    } else {
        printf("Error: No directory to pop!\n");
        exitCode = 2;
    }
}

void changeDirectory(char* name) {
    assertDirectoryStack();
    if (noOptions()) {
        printf("Error: cd requires folder to navigate to!\n");
        exitCode = 2;
    } else {
        addBinPathToOptions(name);
        pushDirectory(getcwd(NULL, 0));
        exitCode = chdir(getArgAt(1));
        // if the directory does not exist, print an error
        if (exitCode == -1) {
            printf("Error: cd directory not found!\n");
            exitCode = 2;
        }
    }
}


/**
 * @brief Executes a built in command
 * 
 * @param command the command to execute
 */
void executeBuiltIn(Command* command) {
    char* name = command->name;
    if (!name) {
        return;
    }

    if (experimental) {
        addToHistory(command);
    }

    for (int i = 0; i < sizeof(builtin) / sizeof(BuiltIn); i++) {
        if (strcmp(name, builtin[i].name) == 0) {
            builtin[i].func(name);
            return;
        }
    }

    cleanUp();
}

/**
 * @brief Checks if a binary exists
 * 
 * @param name  the name of the binary
 * @return true  if the binary exists
 * @return false  if the binary does not exist
 */
bool doesBinaryExist(char* name) {
    if (!name) {
        return false;
    }
    if (strcmp(name, "") == 0) {
        return false;
    }

    for (int i = 0; i < sizeof(builtin) / sizeof(BuiltIn); i++) {
        if (strcmp(name, builtin[i].name) == 0) {
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
        if (fullPath) free(fullPath);
        if (localName) free(localName);
        return false;
    }

    if (fullPath) free(fullPath);

    if (localName) free(localName);
    return true;
}

/**
 * @brief Finds the binary in the path
 * 
 * @param name the name of the binary
 * @return char*  the path to the binary
 */
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

    if (fullPath) free(fullPath);

    return NULL;
}

/**
 * @brief Executes a list of commands
 * 
 * @param commands the list of commands
 * @return int the exit code of the last command
 */
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
        int commandErr = command->err;

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

                //stderr 
                if (commandErr != STDERR_FILENO) {
                    dup2(commandErr, STDERR_FILENO);
                    close(commandErr);
                } else {
                    // Redirect stderr to current pipe
                    dup2(pfds[1], STDERR_FILENO);
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

                if (commandErr != STDERR_FILENO) {
                    dup2(commandErr, STDERR_FILENO);
                    close(commandErr);
                }

            }
            execv(command->path, command->args);
            exit(1);
        } else {
            //  close the file descriptors
            if (commandIn != STDIN_FILENO) close(commandIn);
            if (commandOut != STDOUT_FILENO) close(commandOut);
            if (commandErr != STDERR_FILENO) close(commandErr);

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

void assertBGProcesses() {
    if (!bgProcesses) {
        bgProcesses = calloc(1, sizeof(BGProcesses));
         bgProcesses->processes = calloc(1, sizeof(BGProcess));
        bgProcesses->index = 0;
        bgProcesses->size = 1;
    }
    if (bgProcesses->index == bgProcesses->size) {
        bgProcesses->size *= 2;
        bgProcesses->processes = realloc(bgProcesses->processes, bgProcesses->size * sizeof(BGProcess));
    }
}

void addBGProcess(pid_t pid) {
    assertBGProcesses();
    BGProcess* process = &bgProcesses->processes[bgProcesses->index];
    process->pid = pid;
    bgProcesses->index++;
}

void printBGProcesses(char* _) {
    if (!bgProcesses) return;
    // loop through all the processes and check if any of them have exited
    // if so remove them from the list
    for (int i = 0; i < bgProcesses->index; i++) {
        BGProcess* process = &bgProcesses->processes[i];
        int status;
        pid_t pid = waitpid(process->pid, &status, WNOHANG);
        if (pid == process->pid) {
            // process has exited
            // remove it from the list
            for (int j = i; j < bgProcesses->index - 1; j++) {
                bgProcesses->processes[j] = bgProcesses->processes[j + 1];
            }
            bgProcesses->index--;
        }
    }

    for (int i = 0; i < bgProcesses->index; i++) {
        BGProcess* process = &bgProcesses->processes[i];
        printf("[%d] %d\n", i + 1, process->pid);
    }
}


/**
 * @brief Execute the command 
 * 
 * @return int the exit code of the command
 */
int execCommand(Command* command, bool backGround) {
    bool builtIn = command->isBuiltIn;
    char* commandName = command->name;

    if (builtIn) {
        DEBUG("Executing built-in command %s\n", commandName);
        executeBuiltIn(command);
        cleanUp();
        return exitCode;
    }
    alwaysTrue = false;

    
    if (experimental) {
        addToHistory(command);
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
        if (backGround) {
            // redirect the stdin
            dup2(pipefd[0], STDIN_FILENO);
        }
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

        // stderr
        if (command->err != STDERR_FILENO) {
            dup2(command->err, STDERR_FILENO);
            close(command->err);
        }

        // this will be performed by the child process
        // so execute the command
        DEBUG("Exec %s\n", commandPath);

        execv(commandPath, command->args);

    } else if (!backGround) {
        // this will be performed by the parent process
        // so tell it to wait for the child process to finish
        int status;
        wait(&status);
        exitCode = WEXITSTATUS(status);
        cleanUp();
        
        return exitCode;
    } else if (backGround) {
        addBGProcess(pid);
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
