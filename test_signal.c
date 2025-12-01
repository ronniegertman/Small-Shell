#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void sigint_handler(int sig) {
    printf("Process %d received SIGINT (Ctrl-C)\n", getpid());
}

int main() {
    printf("Parent PID: %d\n", getpid());

    // Install custom handler in parent
    signal(SIGINT, sigint_handler);

    pid_t pid = fork();

    if(pid < 0) {
        perror("fork failed");
        exit(1);
    }
    else if(pid == 0) {
        // CHILD PROCESS
        printf("Child PID: %d\n", getpid());

        // Infinite loop
        while(1) {
            pause(); // wait for signals
        }
    }
    else {
        // PARENT PROCESS
        // Infinite loop
        while(1) {
            pause(); // wait for signals
        }
    }

    return 0;
}

