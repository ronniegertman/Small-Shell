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




int main(int argc, char* argv[])
{
	char _cmd[CMD_LENGTH_MAX];
	while(1) {
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);
		//execute command
		std::string cmdStr(_cmd);
		std::vector<std::string> tokens = tokenize_words(cmdStr);
		
		for (auto token : tokens) {
			printf("Token: '%s',\t", token.c_str());
		}
		printf("\n");
		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
	}

	return 0;
}
