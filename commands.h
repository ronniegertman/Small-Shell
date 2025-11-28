#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>


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

class ShellCommand {
	public:
		std::string command;
		std::vector<std::string> args;
		bool isBackground;
		pid_t pid;
		int nargs;

		ShellCommand(std::string cmd, std::vector<std::string> arguments, bool bg, pid_t p, int n);
};

class Job {
	public:
		int jobId;
		int status;
		ShellCommand cmd;
		int pid;
		time_t startTime;
		Job(const ShellCommand& command, int jobId, int pid, int status);
		double getElapsedTime() const;
};

class JobManager{
	std::vector<Job> jobsList;
	public:
		int generateJobId(); // job vector will be sorted
		int addJob(const ShellCommand& cmd, int pid, int status);
		int removeJobById(int jobId);
		int removeJobByPid(int pid);
		std::string printJobsList();
};


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

#endif //COMMANDS_H