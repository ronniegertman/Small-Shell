//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include "commands.h"
#include "my_system_call.h"
#include "signals.h"
#include <string>
#include <vector>
#include <sstream>
#include <string.h>

#define SHOWPID     1
#define PWD         2
#define CD          3
#define JOBS        4
#define KILL        5
#define FG          6
#define BG          7
#define QUIT        8
#define DIFF        9


/*=============================================================================
* classes/structs declarations
=============================================================================*/
class ShellPrompt{
	public:
		ShellCommand shellcmd;
		//bool isInnercmd;
		//int Innercmd; // i think now that we won't use this because
		// it will require 2 switch cases, 1 to save value
		// and 1 to call function
		bool isPromptDone;
		std::stringstream* leftover;

};
/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];

/*=============================================================================
* main function
=============================================================================*/
void parse_prompt(ShellPrompt &prompt) {
    std::vector<std::string> words;
    // Use an istringstream to handle the splitting
    std::string word;

    // The extraction operator (>>) reads tokens separated by whitespace.
    // It automatically handles and discards multiple spaces, and leading/trailing spaces.
	prompt.leftover >> prompt.shellcmd.command; // first word is the command
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
		}
    }
	//prompt is done if reached here
	prompt.isPromptDone = true;
    return;
}

int inner_index(std::string &cmd){
	const char* innerCommands[9] = {"showpid","pwd","cd",
		"jobs","fg","bg","kill","quit","diff"};
	for(int i=1;i<10;i++){
		if(cmd == innerCommands[i]){
			return true;
		}
	}
	return 0;
}

int call_inner(ShellCommand &cmd, int innercmd){
	switch(innercmd){
		case(SHOWPID):
			return showpid(cmd);
		case(PWD):
			return pwd(cmd);
		case(CD):
			return cd(cmd);
		case(JOBS):
			return jobs(cmd);
		case(KILL):
			return kill(cmd);
		case(FG):
			return fg(cmd);
		case(BG):
			return bg(cmd);	
		case(QUIT):
			return quit(cmd);
		case(DIFF):
			return diff(cmd);
	}
}

void args_vector_to_array(ShellCommand &cmd, char *argv){

}

//void just for now
void exe_command(ShellCommand &cmd){
	int pid;
	char* argv[MAX_ARGS];
	//returns index of inner command if inner
	//0 if outer
	int innercmdIndex = inner_index(cmd.command); 
	if(innercmdIndex > 0){
		if(cmd.isBackground == false){
			call_inner(cmd, innercmdIndex);
		}
		else{
			pid = my_system_call(SYS_FORK);
			if(pid == 0){
				call_inner(cmd,innercmdIndex);
			}
			else if(pid > 0){
				addJob(cmd,pid);
			}
		}
	}
	else if(innercmdIndex == 0){
		pid = my_system_call(SYS_FORK);
		if(pid == 0){
			args_vector_to_array(cmd,argv);
			my_system_call(SYS_EXECVP,cmd.command.c_str(),argv)
		}
		else if(pid > 0){
			if(cmd.isBackground == true)
			//finish it
		}
	}
}




int main(int argc, char* argv[])
{
	char _cmd[CMD_LENGTH_MAX];
	ShellPrompt shellPrompt; //object to handle each prompt
	while(1) {
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);
		shellPrompt.leftover << _cmd; //put prompt in ss
		shellPrompt.isPromptDone = false;
		//inner loop to handle &&
		while(shellPrompt.isPromptDone == false){
			parse_prompt(shellPrompt);
			exe_command(shellPrompt.shellcmd);
			shellPrompt.shellcmd.args.clear(); //maybe will do it in exe_cmd
			// but just so i wont forget
		}
		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
	}

	return 0;
}
