#include "../common.h"

History* history = NULL;

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
    
    int numArgs = getNumberOfOptions();

    history->args[history->numCommands] = malloc(sizeof(char**) * numArgs);
    for (int i = 0; i < numArgs; i++) {
        char* arg = getArgAt(i);
        history->args[history->numCommands][i] = malloc(strlen(arg) + 1);
        strcpy(history->args[history->numCommands][i], arg);
    }

    history->numCommands++;
}

bool anyHistory() {
    if (!history) {
        DEBUG("No history to check\n");
        return false;
    }
    return history->numCommands > 0;
}


char** getMostRecent() {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->commands[history->numCommands - 1];
}

char** getHistoryAt(int index) {
    if (!history) {
        DEBUG("No history to check\n");
        return NULL;
    }
    return history->commands[index];
}


int getNumberOfHistoryCommands() {
    if (!history) {
        return 0;
    }
    return history->numCommands;
}