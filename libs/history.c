#include "../common.h"

History* history = NULL;
static int historyIndex = 0;

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

void addToHistory(char* command) {
    if (!history) {
        newHistory(DONT_CLEAN_PREV);
    }

    // add the command to the history
    history->commands[history->numCommands] = calloc(strlen(command) + 1, sizeof(char));
    strcpy(history->commands[history->numCommands], command);
    
    int numArgs = getNumberOfOptions();

    history->args[history->numCommands] = calloc(numArgs, sizeof(char**));
    for (int i = 0; i < numArgs; i++) {
        char* arg = getArgAt(i);
        history->numArgs[history->numCommands] = numArgs;
        history->args[history->numCommands][i] = calloc(strlen(arg) + 1, sizeof(char));
        strcpy(history->args[history->numCommands][i], arg);
    }

    history->numCommands++;
    historyIndex = history->numCommands;
}

bool anyHistory() {
    if (!history) {
        DEBUG("No history to check\n");
        return false;
    }
    return history->numCommands > 0;
}


char* getMostRecent() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->commands[history->numCommands - 1];
}


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

char** getPrevHistoryArgs() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->args[historyIndex];
}

int getPreviousHistoryArgsCount() {
    if (!history) {
        DEBUG("No history to check\n");
        return 0;
    }
    return history->numArgs[historyIndex];
}

int getNextHistoryArgsCount() {
    if (!history) {
        DEBUG("No history to check\n");
        return 0;
    }
    return history->numArgs[historyIndex];
}


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

char** getNextHistoryArgs() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }

    return history->args[historyIndex];
}


char* getHistoryAt(int index) {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->commands[index];
}


char** getArgsOfMostRecent() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->args[history->numCommands - 1];
}

char** getArgsOfHistoryAt(int index) {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->args[index];
}


int getNumberOfHistoryCommands() {
    if (!history) {
        return 0;
    }
    return history->numCommands;
}


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