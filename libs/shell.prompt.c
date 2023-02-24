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
    printf("-> ");
}

void printPlainPrompt() {
    printf("\n");
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
    printf("%s ", cwd);
    if(cwd) free(cwd);
    if(user) free(user);

    printf("\n");
    printStartOfLine();
}

void newInputBuffer() {
    inputBuffer = calloc(MAX_BUFFER, sizeof(char));
    BUFFER_INDEX = 0;
}

void freeInputBuffer() {
    if (inputBuffer) {
        free(inputBuffer);
        inputBuffer = NULL;
        BUFFER_INDEX = 0;
        MAX_BUFFER = 100;
    }
}

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
                    fprintf(stderr, "args[%d] is null\n", i);
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
                    fprintf(stderr, "args[%d] is null\n", i);
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

// Char : 127
void listenForBackspace() {
    // remove the last character from the input buffer
    if(!removeLastCharFromBuffer()) {
        return;
    }
    // remove the last character from the screen
    printf("\b \b");
}

void newLineCallBack() {
    ungetc('\n', stdin);
    printf("\n");
}

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


void printShellPrompt() {
    if(experimental) {
        printPlainPrompt();
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
            if(listenForOneKey(12, listenForCtrlL)) {
                freeInputBuffer();
                continue;
            }
            if(listenForOneKey(4, listenForCtrlD)) {
                freeInputBuffer();
                break;
            }
            if(listenForOneKey(127, listenForBackspace)) {
                continue;
            }

            if(listenForOneKey(10, newLineCallBack)) {
                // read any remaining characters in the stdin
                // write the entire input buffer to the stdin using ungetc
                for (int i = BUFFER_INDEX - 1; i >= 0; i--) {
                    ungetc(inputBuffer[i], stdin);
                }
                freeInputBuffer();
                break;
            }

            // consume the stdin
            tty_raw(STDIN_FILENO);
            int c = getchar();
            tty_reset(STDIN_FILENO);
            // put it in the input buffer
            addInputToBuffer(c);
            printf("%c", c);
        }
    }
}
