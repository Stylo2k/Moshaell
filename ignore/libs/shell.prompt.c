#include "../common.h"

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
                if (anyHistory()) {
                    // remove all characters inside of the stdin
                    fflush(stdin);
                    printf("\033[2K\r");
                    printPlainPrompt();
                    // print the previous command
                    char** mostRecentCmd = getMostRecent();
                    printf("%s", mostRecentCmd);
                    // write it to the stdin using ungetc and write
                    for (int i = strlen(mostRecentCmd) - 1; i >= 0; i--) {
                        ungetc(mostRecentCmd[i], stdin);
                    }
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

void listForBackspace() {
    int c;   
    static struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);          
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    // read the input, if its the backspace, delete the last character
    c = getchar();
    if (c == 127) {
        printf("\b \b");
    } else {
        ungetc(c, stdin);
    }

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}



void printShellPrompt() {
    if(experimental) {
        printPlainPrompt();
        listForBackspace();
        listenForCtrlL();
        listenForUpArrow();
    }
}

void clearShell() {
    printf("\033[2J\033[1;1H");
}