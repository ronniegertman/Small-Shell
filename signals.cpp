// signals.c
#include "signals.h"

bool isSigInt = false;

// ctrl - z
// void handleSigStp(JobManager& jm) {
//     // check if this is the shell process
//     printf("smash: caught CTRL+Z\n");
//     ShellCommand shell = ShellCommand("smash", {}, false, 123, 0);
//     pid_t pid = getpid();
//     if(pid == showpid(shell)){
//         return;
//     }
//     jm.addJob(jm.fgcmd, pid, 3);
//     // SIGSTOP = 19
//     if(my_system_call(SYS_KILL, pid, 19) == -1) {
//         perror("smash error: kill failed");
//     }
//     jm.clearFgCmd();
//     printf("smash: process %d was stopped\n", pid);
// }

// ctrl - c
