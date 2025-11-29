#include "shellcommand.h"
#include <string>
#include <vector>

// ShellCommand class constructor
ShellCommand::ShellCommand(std::string cmd, std::vector<std::string> arguments, bool bg, pid_t p, int n)
	: command(std::move(cmd)), args(std::move(arguments)), isBackground(bg), pid(p), nargs(n) {}
