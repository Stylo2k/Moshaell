#include "../common.h"


char* inputBuffer = NULL;
int MAX_BUFFER = 100;
int BUFFER_INDEX = 0;


#define ADD_TO_BUFFER 1
#define DONT_ADD_TO_BUFFER 0

static struct termios   save_termios;
static int              term_saved;

// shamelessly stolen from https://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
int tty_raw(int fd) {       /* RAW! mode */
    struct termios  buf;

    if (tcgetattr(fd, &save_termios) < 0) /* get the original state */
        return -1;

    buf = save_termios;

    buf.c_lflag &= ~(ECHO | ICANON);
                    /* echo off, canonical mode off, extended input
                       processing off, signal chars off */

    if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
        return -1;

    term_saved = 1;

    return 0;
}


int tty_reset(int fd) { /* set it to normal! */
    if (term_saved)
        if (tcsetattr(fd, TCSAFLUSH, &save_termios) < 0)
            return -1;

    return 0;
}

void printStartOfLine() {
    printf("\33[2K\r");
    // print the color from getPromptStartColor
    getPromptStartColor();
    printf("%s ", getPromptStart());
    // print the reset color
    resetColor();
}

/**
 * @brief Get the current working directory but in a nicer format cuz fuck getcwd
 *  so instead of the full path, we set the $HOME to ~
 * @return char* 
 */
char* getCwd_() {
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
    return cwd;
}

/**
 * @brief gets the user from the USER environment variable
 * 
 * @return char* obviously the user
 */
char* getUser() {
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
    return user;
}

/**
 * @brief prints the user in a nice format with the colors set from the rc file
 * 
 * @param user the user string
 */
void printUser(char* user) {
    getNameColor();
    printf("%s%s@", getNameStart(), user);
    resetColor();
}

/**
 * @brief prints the hostname in a nice format with the colors set from the rc file
 * 
 */
void printHostName() {
    // print the hostname
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    
    getHostNameColor();
    printf("%s:", hostname);
    resetColor();
}

/**
 * @brief prints the current working directory in a nice format with the colors set from the rc file
 * 
 * @param cwd 
 */
void printCwd(char* cwd) {
    getPathColor();
    printf("%s ", cwd);
    resetColor();
}

/**
 * @brief prints the prompt in a nice format with the colors set from the rc file
 * 
 */
void printPlainPrompt() {
    printf("\n");

    char* cwd = getCwd_();
    char* user = getUser();

    printUser(user);

    printHostName();

    printCwd(cwd);

    if(cwd) free(cwd);
    if(user) free(user);

    printf("\n");
    printStartOfLine();
}

/**
 * @brief makes a new input buffer for reading input
 * 
 */
void newInputBuffer() {
    inputBuffer = calloc(MAX_BUFFER, sizeof(char));
    BUFFER_INDEX = 0;
}

/**
 * @brief frees the input buffer
 * 
 */
void freeInputBuffer() {
    if (inputBuffer) {
        free(inputBuffer);
        inputBuffer = NULL;
        BUFFER_INDEX = 0;
        MAX_BUFFER = 100;
    }
}

/**
 * @brief adds a char to the input buffer
 * 
 * @param input the char to add
 */
void addInputToBuffer(char input) {
    if (!inputBuffer) {
        newInputBuffer();
    }
    if (BUFFER_INDEX >= MAX_BUFFER) {
        MAX_BUFFER *= 2;
        inputBuffer = realloc(inputBuffer, MAX_BUFFER);
    }
    inputBuffer[BUFFER_INDEX] = input;
    BUFFER_INDEX++;
}

//@experimental
// callback for when the user presses ctrl + c
// char : 4
void listenForCtrlD() {
    if (BUFFER_INDEX == 0) {
        cleanUp();
        exit(0);
    }
}

// @experimental
// Char : 12
void listenForCtrlL() {
    printf("\033[H\033[J");
    printPlainPrompt();
}

// @experimental
void listenForUpArrow() {
    char* history = getPrevHistory();
    char** args = getPrevHistoryArgs();
    int nrArgs = getPreviousHistoryArgsCount() - 1;
    if (history) {
        // clear single line
        printStartOfLine();
        // put history in the stdin stream using ungetc
        // also put the args in the stdin stream
        if (args) {
            for (int i = nrArgs; i >= 1; i--) {
                if(!args[i]) {
                    DEBUG("args[%d] is null\n", i);
                    break;
                }
                for (int j = strlen(args[i]) - 1; j >= 0; j--) {
                    if(!args[i][j]) break;
                    ungetc(args[i][j], stdin);
                }
                ungetc(' ', stdin);
            }
        } else {
            ungetc(' ', stdin);
        }
        // add the null terminator to the end of the history
        for (int i = strlen(history) - 1; i >= 0; i--) {
            ungetc(history[i], stdin);
        }
        // add the history to the buffer
        for (int i = 0; i < strlen(history); i++) {
            addInputToBuffer(history[i]);
        }
    }
}

/**
 * @brief listens for the down arrow key
 * 
 */
void listenForDownArrow() {
    char* history = getNextHistory();
    char** args = getNextHistoryArgs();
    int nrArgs = getNextHistoryArgsCount() - 1;
    if (history) {
        printStartOfLine();
        // put history in the stdin stream using ungetc
        // also put the args in the stdin stream
        if (args) {
            for (int i = nrArgs; i >= 1; i--) {
                if(!args[i]) {
                    DEBUG("args[%d] is null\n", i);
                    break;
                }
                for (int j = strlen(args[i]) - 1; j >= 0; j--) {
                    if(!args[i][j]) break;
                    ungetc(args[i][j], stdin);
                }
                ungetc(' ', stdin);
            }
        } else {
            ungetc(' ', stdin);
        }
        for (int i = strlen(history) - 1; i >= 0; i--) {
            if(!history[i]) break;
            ungetc(history[i], stdin);
        }
    }
}

/**
 * @brief removes the last character from the input buffer
 * 
 * @return true if the last character was removed
 * @return false if the last character was not removed
 */
int removeLastCharFromBuffer() {
    bool removed = false;
    if(!inputBuffer) return removed;
    if(BUFFER_INDEX == 0) {
        inputBuffer[BUFFER_INDEX] = '\0';
        return removed;
    }
    if (BUFFER_INDEX > 0) {
        BUFFER_INDEX--;
        inputBuffer[BUFFER_INDEX] = '\0';
        removed = true;
    }
    return removed;
}


void listenForLeftArrow() {
    /**
     * ignore the left arrow key
     * TODO: move the cursor to the left
     */
}

void listenForRightArrow() {
    /**
     * ignore the right arrow key
     * TODO: move the cursor to the right
     */
}

// @experimental
// Char : 127
void listenForBackspace() {
    // remove the last character from the input buffer
    if(!removeLastCharFromBuffer()) {
        return;
    }
    // remove the last character from the screen
    printf("\b \b");
}

// @experimental
// Char : 10
void newLineCallBack() {
    ungetc('\n', stdin);
    printf("\n");
}

/**
 * @brief listens for ctrl + backspace
 * does not work 😬
 */
void listenForCtrlBackspace() {
    fprintf(stderr, "Ctrl + Backspace %d\n", BUFFER_INDEX);
    // remove the previous word from the input buffer
    // remove the previous word from the screen
    int removed = 0;
    while (BUFFER_INDEX > 0 && inputBuffer[BUFFER_INDEX - 1] != ' ') {
        removed += removeLastCharFromBuffer();
    }
    while (removed > 0) {
        printf("\b \b");
        removed--;
    }
}

/**
 * @brief listens for a single key
 * 
 * @param char_ the key to listen for
 * @param callback the callback function to call when the key is matched
 * @return true if the key is matched
 * @return false if the key is not matched
 */
bool listenForOneKey(int char_, void (*callback)()) {
    tty_raw(STDIN_FILENO);
    int c = getchar();
    if (c == char_) {
        tty_reset(STDIN_FILENO);
        (*callback)();
        return true;
    } else {
        ungetc(c, stdin);
    }
    tty_reset(STDIN_FILENO);
    return false;
}

/**
 * @brief listens for a sequence of keys
 * 
 * @param char_ the sequence of keys to listen for
 * @param size the size of the char_ array
 * @param callback the callback function to call when the sequence is matched
 * @param addToBuffer the sequence of keys will be added to the input buffer if this is true
 * @return true if the sequence of keys is matched
 * @return false if the sequence of keys is not matched
 */
bool listenForSeqKeys(int* char_, int size, void (*callback)(), bool addToBuffer) {
    // match the sequence of keys in the char_ array
    char* buffer = calloc(size, sizeof(char));
    tty_raw(STDIN_FILENO);
    for (int i = 0; i < size; i++) {
        int c = getchar();
        if (c != char_[i]) {
            // put the characters back in the stdin stream
            ungetc(c, stdin);
            for (int j = i - 1; j >= 0; j--) {
                ungetc(buffer[j], stdin);
            }
            tty_reset(STDIN_FILENO);
            free(buffer);
            return false;
        }
        buffer[i] = c;
    }
    tty_reset(STDIN_FILENO);
    if (addToBuffer) {
        // put the buffer here into the addInputToBuffer
        for (int i = 0; i < size; i++) {
            addInputToBuffer(buffer[i]);
        }
        free(buffer);
    }
    (*callback)();
    return true;
}

/**
 * @brief print the input buffer in green since the binary exists
 * 
 */
void rightBin() {
    printStartOfLine();
    printf("\033[0;32m");
    printf("%s", inputBuffer);
    printf("\033[0m");
}

/**
 * @brief print the input buffer in red since the binary does not exist
 * 
 */
void wrongBin() {
    printStartOfLine();
    printf("\033[0;31m");
    printf("%s", inputBuffer);
    printf("\033[0m");
}

void checkBin(bool* alreadyReadBin, int* cmdEndIndex) {
    if (BUFFER_INDEX == 0) return;
    bool binExists = doesBinaryExist(inputBuffer);
    binExists = binExists || getCommandFromAlias(inputBuffer);
    if(readingCommand() && binExists) {
        // remove the text then print the color green
        rightBin();
        (*cmdEndIndex) = BUFFER_INDEX;
    } else if(readingCommand() && !(*alreadyReadBin) && !binExists) {
        // remove the text and print the color red
        wrongBin();
    }
}

void listenForCtrlK() {
}
void listenForCtrlJ() {
}

void printShellPrompt() {
    if(experimental) {
        printPlainPrompt();
        bool alreadyReadBin = false;
        int cmdEndIndex = 0;
        while (true) {
            if(listenForSeqKeys((int[]){27, 91, 65}, 3, listenForUpArrow, ADD_TO_BUFFER)) {
                freeInputBuffer();
                continue;
            }
            if(listenForSeqKeys((int[]){27, 91, 66}, 3, listenForDownArrow, ADD_TO_BUFFER)) {
                freeInputBuffer();
                continue;
            }
            // listen to left arrow
            if(listenForSeqKeys((int[]){27, 91, 68}, 3, listenForLeftArrow, DONT_ADD_TO_BUFFER)) {
                continue;
            }
            // listen to right arrow
            if(listenForSeqKeys((int[]){27, 91, 67}, 3, listenForRightArrow, DONT_ADD_TO_BUFFER)) {
                continue;
            }
            // listen to ctrl + K
            if(listenForOneKey(11, listenForCtrlK)) {
                continue;
            }
            // listen to ctrl + L
            if(listenForOneKey(12, listenForCtrlL)) {
                freeInputBuffer();
                continue;
            }
            if(listenForOneKey(4, listenForCtrlD)) {
                freeInputBuffer();
                break;
            }
            if(listenForOneKey(127, listenForBackspace)) {
                if (BUFFER_INDEX <= cmdEndIndex) {
                    alreadyReadBin = false;
                }
                checkBin(&alreadyReadBin, &cmdEndIndex);
                continue;
            }

            if(listenForOneKey(10, newLineCallBack)) {
                char* aliased = getCommandFromAlias(inputBuffer);
                if (aliased) {
                    freeInputBuffer();
                    inputBuffer = aliased;
                    // read any remaining characters in the stdin
                    // write the entire input buffer to the stdin using ungetc
                    BUFFER_INDEX = strlen(inputBuffer);
                    for (int i = BUFFER_INDEX - 1; i >= 0; i--) {
                        ungetc(inputBuffer[i], stdin);
                    }
                    inputBuffer = NULL;
                } else {
                    for (int i = BUFFER_INDEX - 1; i >= 0; i--) {
                        ungetc(inputBuffer[i], stdin);
                    }
                    freeInputBuffer();
                }
                break;
            }

            // consume the stdin
            tty_raw(STDIN_FILENO);
            int c = getchar();
            tty_reset(STDIN_FILENO);
            // put it in the input buffer
            addInputToBuffer(c);
            printf("%c", c);

            // if the char is a space reset the alreadyReadBin
            if(c == ' ' || c == ';' || c == '|' || c == '>' || c == '<') {
                alreadyReadBin = true;
            }
            
            checkBin(&alreadyReadBin, &cmdEndIndex);
        }
    }
}
