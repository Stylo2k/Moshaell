#include "../common.h"

Options* options = NULL;

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

int getNumberOfOptions() {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get number of options, but no options exist\n");
        return 0;
    }
    return options->numArgs;
}


void addBinPathToOptions(char* bin) {
    if (!options) {
        newOptions(DONT_CLEAN_PREV);
    }

    options->commandArgs[0] = malloc(strlen(bin) + 1);
    strcpy(options->commandArgs[0], bin);
}


bool noCommand() {
    return !options || !options->commandArgs || !options->commandArgs[0];
}

bool noOptions() {
    bool here = !options || !options->commandArgs;
    return here || !options->commandArgs[1];
}

char* getBinPath() {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get bin path, but no options exist\n");
        return NULL;
    }
    return options->commandArgs[0];
}


char** getArgs() {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get options, but no options exist\n");
        return NULL;
    }
    return options->commandArgs;
}


char* getArgAt(int index) {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get option at %d, but no options exist\n", index);
        return NULL;
    }
    if (index == 0) return getBinPath();
    return options->commandArgs[index];
}
