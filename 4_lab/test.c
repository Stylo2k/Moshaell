#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main() {
    // get pid
    pid_t pid = getpid();
    
    kill(pid, SIGINT);

    sleep(2);
}