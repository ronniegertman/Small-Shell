#ifndef SHELLCOMMAND_H
#define SHELLCOMMAND_H
/*=============================================================================*/
#include <string>
#include <vector>
/*=============================================================================*/
class ShellCommand {
	public:
		std::string command;
		std::vector<std::string> args;
		bool isBackground;
		pid_t pid;
		int nargs;

		ShellCommand(std::string cmd, std::vector<std::string> arguments, bool bg, pid_t p, int n);
};

class Alias {
	public:
	std::string originalcmd;
	std::string aliasedcmd;
	Alias(std::string originalcmd, std::string aliasedcmd);
};

class AliasedCmds {
	std::vector<Alias> aliasedList;
	public:
	void addAlias(std::string original, std::string aliased);
	void unAlias(std::string aliased);
	bool isAlias(std::string cmd);
    std::string getCmd(std::string cmd);
	std::string getRealCmd(std::string aliascmd);
	void printAll(); // for debugging 
};
#endif /* SHELLCOMMAND_H */
/*=============================================================================*/