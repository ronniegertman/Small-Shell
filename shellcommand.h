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

#endif /* SHELLCOMMAND_H */
/*=============================================================================*/