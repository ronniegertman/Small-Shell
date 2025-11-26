//commands.c
#include "commands.h"
#include <unistd.h>
#include <string.h>
#include "my_system_call.h"
#include <vector>

// jobs vector
std::vector<ShellCommand> jobsList;

// last working directory


// showpid
void showpid(ShellCommand& cmd)
{
	if(cmd.nargs != 0) {
		perrorSmash("showpid", "expected 0 arguments");
		return;
	}

	pid_t pid;
	if(cmd.isBackground) {
		pid_t fork_pid = fork();
		if(fork_pid == 0) {
			//child 
			pid = getppid();
			exit(0);
		}
		else {
			//parent
			my_system_call(SYS_WAITPID, fork_pid, NULL, 0);
		}
	}else {
		pid = getpid();
	}

	printf("smash pid is %d\n",pid);
}

//pwd
void pwd(ShellCommand& cmd){
	if(cmd.nargs != 0) {
		perrorSmash("pwd", "expected 0 arguments");
		return;
	}
	char cwd[CMD_LENGTH_MAX]; //current working directory
	if(cmd.isBackground){
		pid_t fork_pid = fork();
		if(fork_pid == 0) {
			//child 
			getcwd(cwd, sizeof(cwd));
			exit(0);
		}
		else {
			//parent
			my_system_call(SYS_WAITPID, fork_pid, NULL, 0);
		}

	} else {
		getcwd(cwd, sizeof(cwd));
	}
	printf("%s\n", cwd);
	
}

//cd
char prev_dir[CMD_LENGTH_MAX] = "";

void cd(ShellCommand& cmd){
	if(cmd.nargs != 1) {
		perrorSmash("cd", "expected 1 arguments");
		return;
	}
	char cwd[CMD_LENGTH_MAX]; //current working directory
	getcwd(cwd, sizeof(cwd));

	std::string currentDir = cwd;
	std::string targetDir;
	if(cmd.args[1] == "-") {
		if(strlen(prev_dir) == 0) {
			perrorSmash("cd", "old pwd not set");
			return;
		}
		targetDir = std::string(prev_dir);
	} else if(cmd.args[1] == ".."){
		targetDir = currentDir.substr(0, currentDir.find_last_of('/'));
		if(targetDir.empty()) {
			targetDir = "/";
		}
	} else {
		targetDir = cmd.args[1];
	}	
	// performing the dir change:
	chdir(targetDir.c_str());
	oldPwd = currentDir;
}




//example function for printing errors from internal commands
void perrorSmash(const char* cmd, const char* msg)
{
    fprintf(stderr, "smash error:%s%s%s\n",
        cmd ? cmd : "",
        cmd ? ": " : "",
        msg);
}

//example function for parsing commands
// int parseCmdExample(char* line)
// {
// 	char* delimiters = " \t\n"; //parsing should be done by spaces, tabs or newlines
// 	char* cmd = strtok(line, delimiters); //read strtok documentation - parses string by delimiters
// 	if(!cmd)
// 		return INVALID_COMMAND; //this means no tokens were found, most like since command is invalid
	
// 	char* args[MAX_ARGS];
// 	int nargs = 0;
// 	args[0] = cmd; //first token before spaces/tabs/newlines should be command name
// 	for(int i = 1; i < MAX_ARGS; i++)
// 	{
// 		args[i] = strtok(NULL, delimiters); //first arg NULL -> keep tokenizing from previous call
// 		if(!args[i])
// 			break;
// 		nargs++;
// 	}
// 	/*
// 	At this point cmd contains the command string and the args array contains
// 	the arguments. You can return them via struct/class, for example in C:
// 		typedef struct {
// 			char* cmd;
// 			char* args[MAX_ARGS];
// 		} Command;
// 	Or maybe something more like this:
// 		typedef struct {
// 			bool bg;
// 			char** args;
// 			int nargs;
// 		} CmdArgs;
// 	*/
// }
