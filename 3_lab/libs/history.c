#include "../common.h"

History* history = NULL;
static int historyIndex = 0;

/**
 * @brief Creates a new history
 * 
 * @param cleanPrevious whether to clean the previous history or not
 */
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
    history->numArgs = malloc(sizeof(int) * 100);
    history->numCommands = 0;
}

/**
 * @brief adds a command to the history
 * 
 * @param command the command to add
 * 
 */
void addToHistory(char* command) {
    if (!history) {
        newHistory(DONT_CLEAN_PREV);
    }

    // add the command to the history
    history->commands[history->numCommands] = calloc(strlen(command) + 1, sizeof(char));
    strcpy(history->commands[history->numCommands], command);
    
    int numArgs = getNumberOfOptions();

    history->args[history->numCommands] = calloc(numArgs, sizeof(char**));
    for (int i = 1; i < numArgs; i++) {
        char* arg = getArgAt(i);
        history->numArgs[history->numCommands] = numArgs;
        history->args[history->numCommands][i] = calloc(strlen(arg) + 1, sizeof(char));
        strcpy(history->args[history->numCommands][i], arg);
    }

    history->numCommands++;
    historyIndex = history->numCommands;
}

/**
 * @brief checks if there is any history
 * 
 * @return true if there is any history
 * @return false if there is no history
 */
bool anyHistory() {
    if (!history) {
        DEBUG("No history to check\n");
        return false;
    }
    return history->numCommands > 0;
}

/**
 * @brief Get the most recent command
 * 
 * @return char* the most recent command
 */
char* getMostRecent() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->commands[history->numCommands - 1];
}

/**
 * @brief gets the previous command in the history
 * 
 * @return char* the previous command
 */
char* getPrevHistory() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    if (historyIndex != 0) {
        historyIndex--;
    }
    if (historyIndex < 0) {
        historyIndex = 0;
    }
    return history->commands[historyIndex];
}

/**
 * @brief get the previous command's arguments
 * 
 * @return char** the previous command's arguments
 */
char** getPrevHistoryArgs() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->args[historyIndex];
}

/**
 * @brief get the number of arguments of the previous command
 * 
 * @return int the number of arguments of the previous command
 */
int getPreviousHistoryArgsCount() {
    if (!history) {
        DEBUG("No history to check\n");
        return 0;
    }
    return history->numArgs[historyIndex];
}

/**
 * @brief get the number of arguments of the next command
 * 
 * @return int the number of arguments of the next command
 */
int getNextHistoryArgsCount() {
    if (!history) {
        DEBUG("No history to check\n");
        return 0;
    }
    return history->numArgs[historyIndex];
}

/**
 * @brief get the next command in the history
 * 
 * @return char* the next command
 */
char* getNextHistory() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    if (historyIndex != history->numCommands - 1) {
        historyIndex++;
    }
    if (historyIndex >= history->numCommands) {
        historyIndex = history->numCommands - 1;
    }
    return history->commands[historyIndex];
}

/**
 * @brief get next command's arguments
 * 
 * @return char**  the next command's arguments
 */
char** getNextHistoryArgs() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }

    return history->args[historyIndex];
}

/**
 * @brief get the command at the given index
 * 
 * @param index the index of the command
 * @return char* the command at the given index
 */
char* getHistoryAt(int index) {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->commands[index];
}

/**
 * @brief get the arguments of the most recent command
 * 
 * @return char** the arguments of the most recent command
 */
char** getArgsOfMostRecent() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->args[history->numCommands - 1];
}

/**
 * @brief Get the Args Of History At the given index
 * 
 * @param index the index of the command
 * @return char** the arguments of the command at the given index
 */
char** getArgsOfHistoryAt(int index) {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->args[index];
}

/**
 * @brief Get the Number Of History Commands object
 * 
 * @return int the number of commands in the history
 */
int getNumberOfHistoryCommands() {
    if (!history) {
        return 0;
    }
    return history->numCommands;
}

/**
 * @brief prints the history
 * 
 */
void printSessionHistory() {
    if (!history) {
        DEBUG("No history to print\n");
        return;
    }
    for (int i = 0; i < history->numCommands; i++) {
        printf("%d: %s ", i, history->commands[i]);
        // print the args without the first one
        for (int j = 1; j < history->numArgs[i]; j++) {
            printf("%s ", history->args[i][j]);
        }
        printf("\n");
    }
}
