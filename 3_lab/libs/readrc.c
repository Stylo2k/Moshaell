#include "../common.h"


static FILE* rcFile = NULL;
static char* rcFilePath = NULL;

typedef struct Alias {
    char* name;
    char* command;
    struct Alias* next;
} Alias;

static Alias* aliasList = NULL;


// supported colors
typedef enum COLORS {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
} COLOR;

/**
 * some default values for the prompt
 */
static char* PROMPT_START = "╰─Σ";
static char* NAME_START = "╭─";
static COLOR NAME_COLOR = RED;
static COLOR HOST_NAME_COLOR = RED;
static COLOR PATH_COLOR = MAGENTA;
static COLOR PROMPT_START_COLOR = RED;

/**
 * @brief <b>opens</b> the config file
 *  this function will look in the current directory for a .sheeshrc file
 *  if it doesn't find one, it will look in the home directory
 */
void openConfigFile() {
    //first look locally for the .sheeshrc file
    if (!rcFilePath) {
        rcFilePath = strdup(".sheeshrc");
    }
    rcFile = fopen(rcFilePath, "r");
    if (!rcFile) {
        //if it doesn't exist, look in the home directory
        char* home = getenv("HOME");
        char* path = calloc(strlen(home) + 10, sizeof(char));
        strcpy(path, home);
        strcat(path, "/");
        strcat(path, rcFilePath);
        rcFile = fopen(path, "r");
        rcFilePath = strdup(path);
        free(path);
    }
    if (!rcFile) {
        DEBUG("No .sheeshrc file found\n");
        return;
    }
}

/**
 * @brief color printing function
 * 
 * @param color the color to print in
 */
void toColor(COLOR color) {
    switch(color) {
        case RED:
            printf("\033[0;31m");
            break;
        case GREEN:
            printf("\033[0;32m");
            break;
        case YELLOW:
            printf("\033[0;33m");
            break;
        case BLUE:
            printf("\033[0;34m");
            break;
        case MAGENTA:
            printf("\033[0;35m");
            break;
        case CYAN:
            printf("\033[0;36m");
            break;
        case WHITE:
            printf("\033[0;37m");
            break;
        default:
            printf("\033[0;37m");
            break;

    }
}

/**
 * @brief convert a char to a color since we dont want the user to write the unicode
 * 
 * @param char_ the char to convert
 * @return COLOR the color
 */
COLOR charToColor(char* char_) {
    char color = char_[0];
    // convert the color to int
    int colorInt = color - '0';
    switch(colorInt) {
        case 0:
            return RED;
        case 1:
            return GREEN;
        case 2:
            return YELLOW;
        case 3:
            return BLUE;
        case 4:
            return MAGENTA;
        case 5:
            return CYAN;
        case 6:
            return WHITE;
        default:
            return WHITE;
    }
}

/**
 * @brief closes the config file
 * 
 */
void closeConfigFile() {
    if (rcFile) {
        fclose(rcFile);
    }
}

/**
 * @brief strips the line from any garbage the user could have put in
 * 
 * @param line the line to strip
 * @return char* the stripped line
 */
char* stripLine(char* line) {
    if (!line) {
        return NULL;
    }
    if (strlen(line) == 1) {
        return line;
    }

    // if there is a newline at the end, remove it
    if (line[strlen(line) - 1] == '\n') {
        line[strlen(line) - 1] = '\0';
    }
    while(isspace(line[0])) {
        line++;
    }
    if (line[0] == '"') {
        line++;
        line[strlen(line) - 1] = '\0';
    }
    // remove the leading whitespace
    while (isspace(line[0])) {
        line++;
    }
    // remove the trailing whitespace
    int i = strlen(line) - 1;
    while (isspace(line[i])) {
        line[i] = '\0';
        i--;
    }
    if (line[strlen(line) - 1] == '"') {
        line[strlen(line) - 1] = '\0';
    }
    return line;
}

/**
 * @brief looks up a string in the config file STRING=VALUE
 * 
 * @param string the string to look up
 * @return char* the value of the string
 */
char* lookUpInConfigFile(char* string) {
    if (!rcFile) {
        return NULL;
    }
    fseek(rcFile, 0, SEEK_SET);
    // look up the string in the config file with case insensitively
    // if it exists, return the value
    // otherwise return NULL
    // also ignore lines that start with a #
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, rcFile)) != -1) {
        if (line[0] == '#') {
            continue;
        }
        // I LOVEEEEEEEE INDENTATION
        char* key = strtok(line, "=");
        if (key) {
            key = stripLine(key);
            if (strcasecmp(key, string) == 0) {
                char* value = strtok(NULL, "=");
                if (value) {
                    value = stripLine(value);
                    return value;
                }
            }
        }
    }
    return NULL;
}

void assertAliasList() {
    if (!aliasList) {
        aliasList = calloc(1, sizeof(Alias));
    }
}

void addAlias(char* alias, char* command) {
    if (!alias || !command) {
        return;
    }
    assertAliasList();
    Alias* alias_ = aliasList;
    while (alias_->next) {
        // if you try to add an alias that already exists, just update it
        if (strcasecmp(alias_->name, alias) == 0) {
            free(alias_->command);
            alias_->command = strdup(command);
            return;
        }
        alias_ = alias_->next;
    }

    alias_->name = strdup(alias);
    alias_->command = strdup(command);
    alias_->next = calloc(1, sizeof(Alias));
}

char* getCommandFromAlias(char* alias) {
    if (!alias) {
        return NULL;
    }
    assertAliasList();
    Alias* alias_ = aliasList;
    while (alias_->next) {
        if (strcasecmp(alias_->name, alias) == 0) {
            return alias_->command;
        }
        alias_ = alias_->next;
    }
    return NULL;
}

void freeAliasList() {
    if (!aliasList) {
        return;
    }
    Alias* alias_ = aliasList;
    while (alias_->next) {
        Alias* next = alias_->next;
        free(alias_->name);
        free(alias_->command);
        free(alias_);
        alias_ = next;
    }
    free(alias_);
}

void assignAliases() {
    if (!rcFile) {
        return;
    }
    fseek(rcFile, 0, SEEK_SET);
    // look up every line that starts with alias and add it to the alias list
    // aliases are of the form : alias name="command"
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, rcFile)) != -1) {
        if (line[0] == '#') {
            continue;
        }
        // I LOVEEEEEEEE INDENTATION
        // alias name="command"
        // check whether the line starts with alias
        if (strncasecmp(line, "alias", 5) == 0) {
            // get the name
            char* name = strtok(line, "=");
            if (name) {
                name = stripLine(name);
                // get the command
                char* command = strtok(NULL, "=");
                if (command) {
                    command = stripLine(command);
                    // remove the leading alias
                    name += 5;
                    // remove the leading whitespace
                    while (isspace(name[0])) {
                        name++;
                    }
                    // remove the trailing whitespace
                    int i = strlen(name) - 1;
                    while (isspace(name[i])) {
                        name[i] = '\0';
                        i--;
                    }
                    // remove the leading whitespace
                    while (isspace(command[0])) {
                        command++;
                    }
                    // remove the trailing whitespace
                    i = strlen(command) - 1;
                    while (isspace(command[i])) {
                        command[i] = '\0';
                        i--;
                    }
                    addAlias(name, command);
                }
            }
        }
    }

}

/**
 * @brief reads the config file and sets the variables
 * 
 */
void readConfigFile() {
    openConfigFile();
    char* value = lookUpInConfigFile("prompt_start");
    if (value) {
        PROMPT_START = value;
    }
    /**
     * name_color
     * host_name_color
     * path_color
     * prompt_start_color
     */
    // get name color
    value = lookUpInConfigFile("name_color");
    if (value) {
        NAME_COLOR = charToColor(value);
    }
    // get host name color
    value = lookUpInConfigFile("host_name_color");
    if (value) {
        HOST_NAME_COLOR = charToColor(value);
    }
    // get path color
    value = lookUpInConfigFile("path_color");
    if (value) {
        PATH_COLOR = charToColor(value);
    }
    // get prompt start color
    value = lookUpInConfigFile("prompt_start_color");
    if (value) {
        PROMPT_START_COLOR = charToColor(value);
    }
    // get the NAME_START
    value = lookUpInConfigFile("name_start");
    if (value) {
        NAME_START = value;
    }

    assignAliases();

    closeConfigFile();
}

void readSpecificRcFile(char* path) {
    rcFilePath = strdup(path);
    readConfigFile();
}

char* getRcFilePath() {
    return rcFilePath;
}

void resetColor() {
    printf("\033[0m");
}

char* getPromptStart() {
    return PROMPT_START;
}

char* getNameStart() {
    return NAME_START;
}

void getNameColor() {
    toColor(NAME_COLOR);
}

void getHostNameColor() {
    toColor(HOST_NAME_COLOR);
}

void getPathColor() {
    toColor(PATH_COLOR);
}

void getPromptStartColor() {
    toColor(PROMPT_START_COLOR);
}
