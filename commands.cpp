//commands.c
#include "commands.h"



// showpid
void showpid(shell_cmd& cmd)
{
	if(cmd.nargs != 0) {
		perrorSmash("showpid", "expected 0 arguments");
		return;
	}
	printf("smash pid is %d\n", getpid());
}

//pwd
void pwd(shell_cmd& cmd){
	if(cmd.nargs != 0) {
		perrorSmash("pwd", "expected 0 arguments");
		return;
	}
	char cwd[CMD_LENGTH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("%s\n", cwd);
	}
}

//cd
void cd(shell_cmd& cmd){
	if(cmd.nargs != 1) {
		perrorSmash("cd", "expected 1 argument");
		return;
	}
	if (chdir(cmd.args[1].c_str()) != 0) {
		perrorSmash("cd", "chdir failed");
	}
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
int parseCmdExample(char* line)
{
	char* delimiters = " \t\n"; //parsing should be done by spaces, tabs or newlines
	char* cmd = strtok(line, delimiters); //read strtok documentation - parses string by delimiters
	if(!cmd)
		return INVALID_COMMAND; //this means no tokens were found, most like since command is invalid
	
	char* args[MAX_ARGS];
	int nargs = 0;
	args[0] = cmd; //first token before spaces/tabs/newlines should be command name
	for(int i = 1; i < MAX_ARGS; i++)
	{
		args[i] = strtok(NULL, delimiters); //first arg NULL -> keep tokenizing from previous call
		if(!args[i])
			break;
		nargs++;
	}
	/*
	At this point cmd contains the command string and the args array contains
	the arguments. You can return them via struct/class, for example in C:
		typedef struct {
			char* cmd;
			char* args[MAX_ARGS];
		} Command;
	Or maybe something more like this:
		typedef struct {
			bool bg;
			char** args;
			int nargs;
		} CmdArgs;
	*/
}
