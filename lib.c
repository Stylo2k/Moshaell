#include "common.h"

#define BONUS 0

Options* options = NULL;
History* history = NULL;
static int exitCode = 0;
static bool alwaysTrue = false;

#define DONT_CLEAN_PREV 0
#define CLEAN_PREV 1

void DEBUG(const char *fmt, ...) {
    if (silent) {
        return;
    }
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    printf("%s", buffer);
}

void cleanUp() {
    if (!options) {
        DEBUG("no options to clean up\n");
        return;
    }
    if (!options->commandArgs) {
        free(options);
        DEBUG("no command args to clean up\n");
        return;
    }

    int i = 0;
    int max = options->numArgs;

    while (options && (i < max || options->commandArgs[i] != NULL)) {
        DEBUG("freeing %s\n", options->commandArgs[i]);
        if(options->commandArgs[i]){
            free(options->commandArgs[i]);
        }
        options->commandArgs[i] = NULL;
        i++;
    }

    free(options->commandArgs);
    options->commandArgs = NULL;
    free(options);
    options = NULL;
}

// @experimental
void listenForCtrlL() {
    int c;   
    static struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);          
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    // read the input, if its ctrl + l, clear the screen
    c = getchar();
    if (c == 12) {
        printf("\033[H\033[J");
        printShellPrompt();
    } else {
        ungetc(c, stdin);
    }
    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}

// @experimental
void listenForUpArrow() {
    int c;   
    static struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);          
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    // read the input, if its the up arrow, print the previous command
    // the up arrow characters could be prefixed with some other characters, we need to ignore those
    // ignore all characters until we get to the up arrow
    // print the previous command
    c = getchar();
    if (c == 27) {
        c = getchar();
        if (c == 91) {
            c = getchar();
            if (c == 65) {
                if (history->numCommands > 0) {
                    // remove all characters inside of the stdin
                    fflush(stdin);
                    printf("\033[2K\r");
                    printPlainPrompt();
                    // print the previous command
                    printf("%s", history->commands[history->numCommands - 1]);
                    // write it to the stdin using ungetc and write
                    for (int i = strlen(history->commands[history->numCommands - 1]) - 1; i >= 0; i--) {
                        ungetc(history->commands[history->numCommands - 1][i], stdin);
                    }
                    // update the history
                    history->numCommands--;
                }
            }
        }
    } else {
        ungetc(c, stdin);
    }

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);    
}

void printPlainPrompt() {
    char* cwd = getcwd(NULL, 0);
    // replace the home directory with a tilde
    char* home = getenv( "HOME" );
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        char* newCwd = malloc(strlen(cwd) + 2);
        strcpy(newCwd, "~");
        strcat(newCwd, cwd + strlen(home));
        free(cwd);
        cwd = newCwd;
    }
    // print the username
    char* userEnv = getenv( "USER" );
    char* user = NULL;
    if (userEnv) {
        user = malloc(strlen(userEnv) + 1);
        strcpy(user, userEnv);
    }

    if (!user) {
        // get it from whoami
        FILE* whoami = popen( "whoami" , "r");
        char buffer[1024];
        int bytesRead = fread(buffer, 1, 1024, whoami);
        buffer[bytesRead] = '\0';
        user = malloc(strlen(buffer) + 1);
        strcpy(user, buffer);
        // remove the newline
        user[strlen(user) - 1] = '\0';
        pclose(whoami);
    }
    printf("%s@", user);
    // print the hostname
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    printf("%s:", hostname);
    printf("%s$ ", cwd);
    if(cwd) free(cwd);
    if(user) free(user);
}


void printShellPrompt() {
    #if BONUS
        printPlainPrompt();
        if(experimental) {
            listenForCtrlL();
            listenForUpArrow();
        }
    #endif
}

void newOptions(bool cleanPrevious) {
    if (cleanPrevious && options) {
        cleanUp();
    }
    if (cleanPrevious && !options) {
        DEBUG("Trying to clean the previous, but no prev exists\n");
    }
    options = calloc(sizeof(Options), 1);
    options->commandArgs = malloc(sizeof(char**)*2);
    options->commandArgs[0] = NULL;
    options->numArgs = 1;
    options->commandArgs[1] = NULL;
}

void addOption(char* option) {
    if (!options) {
        newOptions(DONT_CLEAN_PREV);
    }

    int i = options->numArgs;
    options->commandArgs = realloc(options->commandArgs, sizeof(char**) * (i + 2));
    if (!options->commandArgs[i]) {
        options->commandArgs[i] = malloc(strlen(option) + 1);
        strcpy(options->commandArgs[i], option);
        options->commandArgs[i + 1] = NULL;
    } else {
        //realloc the old one to include the new option
        options->commandArgs[i] = realloc(options->commandArgs[i], strlen(options->commandArgs[i]) + strlen(option) + 2);
        strcat(options->commandArgs[i], option);
        options->commandArgs[i + 1] = NULL;
    }

    options->numArgs++;

    if (option) {
        free(option);
    }
}

void newHistory(bool cleanPrevious) {
    if (cleanPrevious && history) {
        cleanUp();
    }
    if (cleanPrevious && !history) {
        DEBUG("Trying to clean the previous, but no prev exists\n");
    }
    history = malloc(sizeof(char*) * 100);
    history->commands = malloc(sizeof(char**) * 100);
    history->args = malloc(sizeof(char***) * 100);
    history->numCommands = 0;
}

void addToHistory(char* command) {
    if (!history) {
        newHistory(DONT_CLEAN_PREV);
    }

    // add the command to the history
    history->commands[history->numCommands] = malloc(strlen(command) + 1);
    strcpy(history->commands[history->numCommands], command);
    history->args[history->numCommands] = malloc(sizeof(char**) * options->numArgs);
    for (int i = 0; i < options->numArgs; i++) {
        history->args[history->numCommands][i] = malloc(strlen(options->commandArgs[i]) + 1);
        strcpy(history->args[history->numCommands][i], options->commandArgs[i]);
    }

    history->numCommands++;
}

/**
 * @brief Execute the command 
 * 
 * @return int the exit code of the command
 */
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
    if (!options || !options->commandArgs || !options->commandArgs[0]) {
        if(command) {
            free(command);
        }
        cleanUp();
        return 127;
    }

    if (experimental) addToHistory(command);

    char* commandPath = options->commandArgs[0];

    pid_t pid = fork();
    if (pid == 0) {
        // this will be performed by the child process
        // so execute the command
        DEBUG("Exec %s\n", commandPath);
        execv(commandPath, options->commandArgs);
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

void setExitCode(int code) {
    exitCode = code;
}

int getExitCode() {
    return exitCode;
}

bool isAlwaysTrue() {
    return alwaysTrue;
}

void setAlwaysTrue(bool value) {
    alwaysTrue = value;
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
            if (!options) {
                newOptions(DONT_CLEAN_PREV);
            }
            options->commandArgs[0] = malloc(strlen(cmdPath) + 1);
            strcpy(options->commandArgs[0], cmdPath);

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
        if (!options || !options->commandArgs || options->commandArgs[1] == NULL) {
            exitCode = chdir(getenv("HOME"));
        } else {
            options->commandArgs[0] = malloc(strlen(name) + 1);
            strcpy(options->commandArgs[0], name);
            exitCode = chdir(options->commandArgs[1]);
            // if the directory does not exist, print an error
            if (exitCode == -1) {
                printf("cd: no such file or directory: %s\n", options->commandArgs[1]);
            }
        }
    }
    if(name) {
        free(name); 
    }
    cleanUp();
}


void clearShell() {
    printf("\033[2J\033[1;1H");
}