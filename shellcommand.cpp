#include "shellcommand.h"
#include <string>
#include <vector>

// ShellCommand class constructor
ShellCommand::ShellCommand(std::string cmd, std::vector<std::string> arguments, bool bg, pid_t p, int n)
	: command(std::move(cmd)), args(std::move(arguments)), isBackground(bg), pid(p), nargs(n) {}

Alias::Alias(std::string originalcmd, std::string aliasedcmd) : originalcmd(std::move(originalcmd)), 
aliasedcmd(std::move(aliasedcmd)){}

void AliasedCmds::addAlias(std::string original, std::string aliased){
	Alias toadd(original, aliased);
	aliasedList.push_back(toadd);
}

void AliasedCmds::unAlias(std::string aliased){
	for(auto it = aliasedList.begin(); it != aliasedList.end(); it++) {
		if(it->aliasedcmd == aliased) {
			aliasedList.erase(it);
          	return;
		}
	}
}

std::string AliasedCmds::getRealCmd(std::string aliascmd){
    for (auto &a : aliasedList) {
        if (a.aliasedcmd == aliascmd) {

            // prevent infinite self-loop
            if (a.originalcmd == aliascmd)
                return a.originalcmd;

            std::string real = getRealCmd(a.originalcmd);
            if (real == "")
                return a.originalcmd;

            return real;
        }
    }
    return "";
}
