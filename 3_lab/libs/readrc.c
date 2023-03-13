#include "../common.h"


static FILE* rcFile = NULL;

typedef enum COLORS {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
} COLOR;

static char* PROMPT_START = "╰─Σ";
static char* NAME_START = "╭─";
static COLOR NAME_COLOR = RED;
static COLOR HOST_NAME_COLOR = RED;
static COLOR PATH_COLOR = MAGENTA;
static COLOR PROMPT_START_COLOR = RED;


void openConfigFile() {
    //first look locally for the .sheeshrc file
    rcFile = fopen(".sheeshrc", "r");
    if (!rcFile) {
        //if it doesn't exist, look in the home directory
        char* home = getenv("HOME");
        char* path = calloc(strlen(home) + 10, sizeof(char));
        strcpy(path, home);
        strcat(path, "/.sheeshrc");
        rcFile = fopen(path, "r");
        free(path);
    }
    if (!rcFile) {
        DEBUG("No .sheeshrc file found\n");
        return;
    }
}

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

void closeConfigFile() {
    if (rcFile) {
        fclose(rcFile);
    }
}


char* stripLine(char* line) {
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
    closeConfigFile();
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
