#include "../common.h"

Options* options = NULL;

/**
 * @brief Cleans up the options
 */
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

/**
 * @brief Creates a new options struct
 * 
 * @param cleanPrevious whether to clean the previous options or not
 */
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

/**
 * @brief Get the Options object
 * 
 * @return char** the options
 */
char** getOptions() {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get options, but no options exist\n");
        return NULL;
    }
    return options->commandArgs;
}

/**
 * @brief Add an option to the options
 * 
 * @param option the option to add
 */
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

/**
 * @brief Get the Number Of Options
 * 
 * @return int the number of options
 */
int getNumberOfOptions() {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get number of options, but no options exist\n");
        return 0;
    }
    return options->numArgs;
}

/**
 * @brief add the bin path to the options (first index)
 * 
 * @param bin the bin path
 */
void addBinPathToOptions(char* bin) {
    if (!options) {
        newOptions(DONT_CLEAN_PREV);
    }

    options->commandArgs[0] = malloc(strlen(bin) + 1);
    strcpy(options->commandArgs[0], bin);
}

/**
 * @brief check if there is no command
 * 
 * @return true if there is no command
 * @return false if there is a command
 */
bool noCommand() {
    return !options || !options->commandArgs || !options->commandArgs[0];
}

/**
 * @brief check if there are no options
 * 
 * @return true if there are no options
 * @return false if there are options
 */
bool noOptions() {
    bool here = !options || !options->commandArgs;
    return here || !options->commandArgs[1];
}

/**
 * @brief Get the Bin Path 
 * 
 * @return char* the bin path
 */
char* getBinPath() {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get bin path, but no options exist\n");
        return NULL;
    }
    return options->commandArgs[0];
}

/**
 * @brief Get the Args object entirely
 * 
 * @return char** th array of args
 */
char** getArgs() {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get options, but no options exist\n");
        return NULL;
    }
    return options->commandArgs;
}

/**
 * @brief Get the Arg At a given index
 * 
 * @param index the index of the arg to get
 * @return char* the arg at the given index
 */
char* getArgAt(int index) {
    if (!options || !options->commandArgs) {
        DEBUG("Trying to get option at %d, but no options exist\n", index);
        return NULL;
    }
    if (index == 0) return getBinPath();
    return options->commandArgs[index];
}
