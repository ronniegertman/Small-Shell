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

bool AliasedCmds::isAlias(std::string cmd) {
    for (const auto& alias : aliasedList) {
        // We compare against originalcmd because that's where we stored the name
        if (alias.aliasedcmd == cmd) {
            return true;
        }
    }
    return false;
}

    std::string AliasedCmds::getCmd(std::string cmd) {
    for (const auto& alias : aliasedList) {
        if (alias.aliasedcmd == cmd) {
            return alias.originalcmd;
        }
    }
    return "";
}

void AliasedCmds::printAll() {
    for (const auto& alias : aliasedList) {
        printf("%s='%s'\n", alias.aliasedcmd.c_str(), alias.originalcmd.c_str());
    }
}