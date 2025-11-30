// signals.c
#include "signals.h"
#include "commands.h"
#include "jobs.h"
#include <unistd.h>
#include "my_system_call.h"

void handleSigint(JobManager& jm) {
    // check if this is the shell process
    printf("smash: caught CTRL+Z\n");
    ShellCommand cmd = ShellCommand("smash", {}, false, 123, 0);
    pid_t pid = getpid();
    if(pid == showpid(cmd)){
        return;
    }
    jm.addJob(cmd, pid, 3);
    // SIGSTOP = 19
    if(my_system_call(SYS_KILL, pid, 19) == -1) {
        perror("smash error: kill failed");
    }
    printf("smash: process %d was stopped\n", pid);
}

