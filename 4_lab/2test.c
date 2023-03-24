#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


void handleSig(int sig, siginfo_t *siginfo, void *context) {
    printf("Caught signal %d\n", sig);
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = handleSig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO; 
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGTTIN, &sa, NULL);
    sigaction(SIGTTOU, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    signal(SIGHUP, SIG_IGN);

    while (1) {
        // printf("I'm alive\n");
        sleep(1);
    }
}