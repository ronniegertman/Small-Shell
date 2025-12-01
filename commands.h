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
void showpid(ShellCommand& cmd);
void pwd(ShellCommand& cmd);
bool isRegularFile(const std::string& path);
bool isDirectory(const std::string& path);
void cd(ShellCommand& cmd);
void jobs(ShellCommand& cmd, JobManager& jm);
void kill(ShellCommand& cmd, JobManager& jm);
void fg(ShellCommand& cmd, JobManager& jm);
void bg(ShellCommand& cmd, JobManager& jm);
void quit(ShellCommand& cmd, JobManager& jm);
void diff(ShellCommand& cmd);

#endif //COMMANDS_H