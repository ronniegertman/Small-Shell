#ifndef SHELLPROMPT_H
#define SHELLPROMPT_H

#include "commands.h"
#include "shellcommand.h"
#include <sstream>



class ShellPrompt{
	public:
		ShellCommand shellcmd;
		bool isPromptDone;
		std::stringstream leftover;

    //constractor
    ShellPrompt();
};




#endif