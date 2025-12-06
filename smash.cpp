//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include "commands.h"
#include "shellprompt.h"
#include "my_system_call.h"
#include <string>
#include <vector>
#include <sstream>
#include <string.h>
#include <csignal>
#include <unistd.h>

#define SHOWPID     1
#define PWD         2
#define CD          3
#define JOBS        4
#define KILL        5
#define FG          6
#define BG          7
#define QUIT        8
#define DIFF        9
#define ALIAS       10
#define UNALIAS     11


/*=============================================================================
* classes/structs declarations
=============================================================================*/

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];
JobManager jm;
AliasedCmds aliasesList;
/*=============================================================================
* main function
=============================================================================*/
void handleSigStp() {
	pid_t fgpid;	
    // check if this is the shell process
    printf("smash: caught CTRL+Z\n");
	if(jm.fgactive == false){  // no other process but the shell is in fg
		return;
	}
	fgpid = jm.fgcmd.pid;
    // SIGSTOP = 19
    if(my_system_call(SYS_KILL, fgpid, 19) == -1) {
		jm.removeJobByPid(fgpid);
        perror("smash error: kill failed");
    }
    jm.clearFgCmd();
    printf("smash: process %d was stopped\n", fgpid);
}


void handleSigInt() {
	pid_t fgpid;

    // check if this is the shell process
    printf("smash: caught CTRL+C\n");
	if(jm.fgactive == false){  // no other process but the shell is in fg
		return;
	}
	fgpid = jm.fgcmd.pid;
    // SIGKILL = 9
    if(my_system_call(SYS_KILL, fgpid, 9) == -1) {
        perror("smash error: kill failed");
    }
    jm.clearFgCmd();
    printf("smash: process %d was killed\n", fgpid);
}


void parse_prompt(ShellPrompt &prompt) {
    std::vector<std::string> words;
    // Use an istringstream to handle the splitting
    std::string word;

    // The extraction operator (>>) reads tokens separated by whitespace.
    // It automatically handles and discards multiple spaces, and leading/trailing spaces.
	prompt.leftover >> prompt.shellcmd.command; // first word is the command
	if(aliasesList.isAlias(prompt.shellcmd.command)){
        std::string expandedAlias = aliasesList.getCmd(prompt.shellcmd.command);
		// Get the rest of the leftover line
		std::string restOfLine;
		std::getline(prompt.leftover, restOfLine);

		std::string newBuffer = expandedAlias + restOfLine;

		// Reset stream
		prompt.leftover.clear();
		prompt.leftover.str(newBuffer);
		prompt.shellcmd.command = "";
		return;
	}
	if (prompt.shellcmd.command == "alias") {
        // We assume the format: alias name="value..."
        std::string namePart;
        std::string valuePart;
        // 1. Read everything up to the first quote (e.g. " name=")
        // This consumes the first quote from the stream.
        if (std::getline(prompt.leftover, namePart, '"')) {
            // 2. Read everything up to the second quote (e.g. "echo 1 && echo 2")
            // This consumes the second quote from the stream.
            if (std::getline(prompt.leftover, valuePart, '"')) {
				// Trim leading whitespace from the name part
				// Because getline picked up the space left over by >>
				size_t startPos = namePart.find_first_not_of(" \t");
				if (startPos != std::string::npos) {
					namePart = namePart.substr(startPos);
				}
                // 3. Reconstruct the full argument: name="value"
                // Note: namePart usually has a leading space from the >> operator, which is fine
                std::string fullAliasArg = namePart + '"' + valuePart + '"';
                // 4. Store it
                prompt.shellcmd.args.push_back(fullAliasArg);
                prompt.shellcmd.nargs++;
            }
        }
    }

    while (prompt.leftover >> word) {
		// & will always come at the prompt's end
		if(word == "&"){
			prompt.shellcmd.isBackground = true;
			prompt.isPromptDone = true;
			return;
		}
		// && meaning the prompt is not done
		else if(word == "&&"){
			return;
		}
		else{
			prompt.shellcmd.args.push_back(word);
			prompt.shellcmd.nargs++;
		}
    }
	//prompt is done if reached here
	prompt.isPromptDone = true;
    return;
}

int inner_index(std::string &cmd){
	const char* innerCommands[11] = {"showpid","pwd","cd",
		"jobs","kill","fg","bg","quit","diff","alias","unalias"};
	for(int i=1;i<12;i++){
		if(cmd == innerCommands[i-1]){
			return i;
		}
	}
	return 0;
}

int call_inner(ShellCommand &cmd, int innercmd){
	switch(innercmd){
		case(SHOWPID):
            if(showpid(cmd) == -1) return -1;
			else return 0; 
		case(PWD):
			return pwd(cmd);
		case(CD):
			return cd(cmd);
		case(JOBS):
			return jobs(cmd,jm);
		case(KILL):
			return kill(cmd,jm);
		case(FG):
			return fg(cmd,jm);
		case(BG):
			return bg(cmd,jm);	
		case(QUIT):
			return quit(cmd,jm);
		case(DIFF):
			return diff(cmd);
		case(ALIAS):
			return alias(cmd, aliasesList);
		case(UNALIAS):
			return unalias(cmd, aliasesList);
		default:
			printf("entered default in call_inner, need fixing\n");
			return -1;
	}
}

void args_vector_to_array(ShellCommand &cmd, char **argv) {
    // 1. argv[0] must be the command name itself
    // We use const_cast because c_str() returns 'const char*' 
    // but the execvp signature (and your array) expects 'char*'
    argv[0] = const_cast<char*>(cmd.command.c_str());

    // 2. Copy the arguments from the vector to the array starting at index 1
    for (size_t i = 0; i < cmd.args.size(); ++i) {
        argv[i + 1] = const_cast<char*>(cmd.args[i].c_str());
    }

    // 3. The array MUST be terminated by a NULL pointer for execvp to know where to stop
    argv[cmd.nargs + 1] = NULL;
}

//void just for now
int exe_command(ShellCommand &cmd){
	jm.updateList(); //update before any cmd
	pid_t pid;
	int status;
	char* argv[MAX_ARGS + 2]; // +2 for command itself 
	//and null terminator
	if(cmd.command == ""){ //good for alias handling
			return 0;
	}
	//returns index of inner command if inner
	//0 if outer
	int innercmdIndex = inner_index(cmd.command); 
	if(innercmdIndex > 0){ // inner command
		if(cmd.isBackground == false){
			cmd.pid = getpid();
			if(call_inner(cmd, innercmdIndex) == -1) return -1;
			else return 0;
		}
		else{
			pid = my_system_call(SYS_FORK);
			if(pid == 0){
				// child
				setpgrp();
				cmd.pid = getpid();
				if(call_inner(cmd, innercmdIndex) == -1) exit(1);
				else exit(0);
			}
			else if(pid > 0){
				jm.addJob(cmd,pid,2);
				return 0;
			}
			else{
				// fork failed
				perror("smash error: fork failed");
				exit(1);
			}
		}
	}
	else if(innercmdIndex == 0){ // outer command
		pid = my_system_call(SYS_FORK);
		if(pid == 0){
			//child
			setpgrp();
			cmd.pid = getpid();
			args_vector_to_array(cmd,argv);
			int exerr = my_system_call(SYS_EXECVP,cmd.command.c_str(),argv);
			if(exerr == -1){ // execvp failed
				if(errno == ENOENT){ // command not found
					perror("smash error: external: cannot find program");
					exit(1);
				}
				else{
					perror("smash error: external: invalid command");
					exit(1);
				}
			}
			exit(0);
		}
		else if(pid > 0){
			cmd.pid = pid;
			if(cmd.isBackground == true){
				jm.addJob(cmd,pid,2);
			}
			else{
				jm.updateFgCmd(cmd);
				if(my_system_call(SYS_WAITPID,pid,&status,WUNTRACED) == -1){
					perror("smash error: waitpid failed");
				}
				jm.clearFgCmd();
				if(WIFSTOPPED(status)){
					jm.addJob(cmd,pid,3); // stopped
				}
				else if(WIFEXITED(status)){
					// command failed in child
					int exit_code = WEXITSTATUS(status);
					return exit_code ? -1 : 0;
				}
			}
		}
		else{
			// fork failed
			perror("smash error: fork failed");
			exit(1);
			}
	}
	return 0;
}




int main(int argc, char* argv[])
{
	my_system_call(SYS_SIGNAL,SIGINT, (__sighandler_t)handleSigInt);
	my_system_call(SYS_SIGNAL,SIGTSTP, (__sighandler_t)handleSigStp);
	char _cmd[CMD_LENGTH_MAX];
	int execResult = 0;
	ShellPrompt shellPrompt; //object to handle each prompt
	while(1) {
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);
		shellPrompt.leftover  << _cmd; //put prompt in ss
		shellPrompt.isPromptDone = false;
		//inner loop to handle &&
		while(shellPrompt.isPromptDone == false){
			parse_prompt(shellPrompt);
			execResult = exe_command(shellPrompt.shellcmd);
			shellPrompt.shellcmd.isBackground = false;
			shellPrompt.shellcmd.command = "";
			shellPrompt.shellcmd.args.clear();
			shellPrompt.shellcmd.nargs = 0;
			if(execResult == -1){
				break;
			}
		}
		shellPrompt.leftover.clear(); //reset ss to get data
		shellPrompt.leftover.str(""); //Empty the text buffer
		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
		//aliasesList.printAll(); // for debugging
	}

	return 0;
}
