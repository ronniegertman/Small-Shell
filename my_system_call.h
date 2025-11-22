#ifndef MY_SYSTEM_CALL_H
#define MY_SYSTEM_CALL_H

//syscall numbers 
#define SYS_FORK     1
#define SYS_EXECVP   2
#define SYS_WAITPID  3
#define SYS_SIGNAL   4
#define SYS_KILL     5
#define SYS_PIPE     6
#define SYS_READ     7
#define SYS_WRITE    8
#define SYS_OPEN     9
#define SYS_CLOSE    10

/*
 * @General wrapper for invoking system calls by number.
 *
 * This function provides a unified interface for calling various system calls
 * using their assigned numeric identifiers. It allows invoking different
 * system-level operations without exposing the low-level implementation details.
 *
 * @param syscall_number The numeric ID representing the system call to execute.
 * @param ... Optional arguments depending on the specific system call type.
 *
 * @return The result of the system call matching the given number from the defined list.  
 *         The output is identical to that of invoking the corresponding system call directly  
 *         (see each call’s behavior in its respective man page).
 */long my_system_call(int syscall_number, ...);

#endif
