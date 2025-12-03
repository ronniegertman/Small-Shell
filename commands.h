#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "shellcommand.h"
#include "jobs.h"


#define CMD_LENGTH_MAX 120
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100
#define MAX_ARGS 20

/*=============================================================================
* error handling - some useful macros and examples of error handling,
* feel free to not use any of this
=============================================================================*/
#define ERROR_EXIT(msg) \
    do { \
        fprintf(stderr, "%s: %d\n%s", __FILE__, __LINE__, msg); \
        exit(1); \
    } while(0);

static inline void* _validatedMalloc(size_t size)
{
    void* ptr = malloc(size);
    if(!ptr) ERROR_EXIT("malloc");
    return ptr;
}

// example usage:
// char* bufffer = MALLOC_VALIDATED(char, MAX_LINE_SIZE);
// which automatically includes error handling
#define MALLOC_VALIDATED(type, size) \
    ((type*)_validatedMalloc((size)))


/*=============================================================================
* error definitions
=============================================================================*/
typedef enum  {
	INVALID_COMMAND = 0,
	//feel free to add more values here or delete this
} ParsingError;

typedef enum {
	SMASH_SUCCESS = 0,
	SMASH_QUIT,
	SMASH_FAIL
	//feel free to add more values here or delete this
} CommandResult;

/*=============================================================================
* global functions
=============================================================================*/
int parseCommandExample(char* line);
void perrorSmash(const char* cmd, const char* msg);
pid_t showpid(ShellCommand& cmd);
int pwd(ShellCommand& cmd);
int cd(ShellCommand& cmd);
int jobs(ShellCommand& cmd, JobManager& jm);
int kill(ShellCommand& cmd, JobManager& jm);
int fg(ShellCommand& cmd, JobManager& jm);
int bg(ShellCommand& cmd, JobManager& jm);
int quit(ShellCommand& cmd, JobManager& jm);
int diff(ShellCommand& cmd);
bool isUnsignedInt(const std::string& s);

#endif //COMMANDS_H

// TODO: no need to change status in jobs list if not by calling cntrl z or cntrl c
// waitpid works only on child processes