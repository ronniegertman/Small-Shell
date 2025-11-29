//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include "commands.h"
#include "signals.h"
#include <string>
#include <vector>
#include <sstream>
#include <string.h>
#include <unistd.h>

/*=============================================================================
* classes/structs declarations
=============================================================================*/

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];

/*=============================================================================
* main function
=============================================================================*/
std::vector<std::string> tokenize_words(const std::string& input) {
    std::vector<std::string> words;
    // Use an istringstream to handle the splitting
    std::stringstream ss(input);
    std::string word;

    // The extraction operator (>>) reads tokens separated by whitespace.
    // It automatically handles and discards multiple spaces, and leading/trailing spaces.
    while (ss >> word) {
        words.push_back(word);
    }

    return words;
}




// int main(int argc, char* argv[])
// {
// 	char _cmd[CMD_LENGTH_MAX];
// 	while(1) {
// 		printf("smash > ");
// 		fgets(_line, CMD_LENGTH_MAX, stdin);
// 		strcpy(_cmd, _line);
// 		//execute command
// 		std::string cmdStr(_cmd);
// 		std::vector<std::string> tokens = tokenize_words(cmdStr);
		
// 		printf("Tokens:\n");
// 		for (auto token : tokens) {
// 			printf("'%s'\n", token.c_str());
// 		}
// 		//initialize buffers for next command
// 		_line[0] = '\0';
// 		_cmd[0] = '\0';
// 	}

// 	return 0;
// }

int main(int argc, char* argv[]){
    ShellCommand c1("ls", {"-l"}, false, 1234, 1);
    ShellCommand c2("sleep", {"10"}, true, 1235, 1);
    JobManager jm;

    int jobId = jm.addJob(c1, c1.pid, 0, true);
    jm.addJob(c2, c2.pid, 0, true);
    printf("%s", jm.printJobsList().c_str());
    printf("\n");
    jm.removeJobByPid(c1.pid);
    printf("%s", jm.printJobsList().c_str());
    ShellCommand c3("echo", {"Hello, World!"}, false, 1236, 1);
    jm.addJob(c3, c3.pid, 3, false);
    printf("\n");
    printf("%s", jm.printJobsList().c_str());
    //ShellCommand otherbash = ShellCommand("bash", {}, true, 22461, 0);
    // JobManager jm;
    //jm.addJob(otherbash, otherbash.pid, 2, true);
    // ShellCommand stopcmd("kill", {"19","1"}, false, 123, 2);
    // kill(stopcmd, jm);
    // sleep(10);
    // ShellCommand fgcmd("fg", {}, false, 123, 0);
    // fg(fgcmd, jm);
	return 0;
}