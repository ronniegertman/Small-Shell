//commands.c
#include "commands.h"
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "my_system_call.h"
#include "jobs.h"
#include "shellcommand.h"
#include "signals.h"
#include <vector>
#include <ctime>
#include <sstream>

// showpid
pid_t showpid(ShellCommand& cmd)
{
	if(cmd.nargs != 0) {
		perrorSmash("showpid", "expected 0 arguments");
		return -1;
	}
	// pid syscalls never fail
	pid_t pid;
	if(cmd.isBackground) {
		pid = getppid();
	}else {
		pid = getpid();
	}

	printf("smash pid is %d\n",pid);
	return pid;
}

//pwd
int pwd(ShellCommand& cmd){
	if(cmd.nargs != 0) {
		perrorSmash("pwd", "expected 0 arguments");
		return -1;
	}
	char cwd[CMD_LENGTH_MAX]; //current working directory
	if(getcwd(cwd, sizeof(cwd)) == NULL){
		perror("smash error: pwd failed");
		return -1;
	}
	printf("%s\n", cwd);
	return 0;
	
}


char prev_dir[CMD_LENGTH_MAX] = "";

int cd(ShellCommand& cmd){
	if(cmd.nargs != 1) {
		perrorSmash("cd", "expected 1 arguments");
		return -1;
	}
	char cwd[CMD_LENGTH_MAX]; //current working directory
	if(getcwd(cwd, sizeof(cwd)) == NULL){
		perror("smash error: cd failed");
		return -1;
	}

	std::string currentDir = cwd;
	std::string targetDir;
	//check if address ilegal
	if(cmd.args[0] == "-") {
		if(strlen(prev_dir) == 0) {
			perrorSmash("cd", "old pwd not set");
			return -1;
		}
		targetDir = std::string(prev_dir);
		printf("%s\n", targetDir.c_str());
	} else if(cmd.args[0] == ".."){
		targetDir = currentDir.substr(0, currentDir.find_last_of('/'));
		if(targetDir.empty()) {
			targetDir = "/";
		}
	} else {
		targetDir = cmd.args[0];
	}	
	

	// performing the dir change:
	if(chdir(targetDir.c_str()) == -1){
		if(errno == ENOENT){
			perrorSmash("cd", "target directory does not exist");
			return -1;
		}
		else if(errno == ENOTDIR){
			std::string err = targetDir + ": not a directory";
			perrorSmash("cd", err.c_str());
			return -1;
		}
		else{
			perror("smash error: chdir failed");
			return -1;
		}

	}
	// updating prev_dir
	strcpy(prev_dir, cwd);
	return 0;
}

//jobs
// PRINT SRGUMENTS LIST
int jobs(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs !=0){
		perrorSmash("jobs", "expected 0 arguments");
		return -1;
	}
  printf("%s", jm.printJobsList().c_str());
	return 0;
}

bool isUnsignedInt(const std::string& s) {
    if (s.empty()) return false;

    for (char c : s) {
        if (!isdigit(c)) return false;
    }

    return true;
}

//kill
int kill(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs !=2){
		perrorSmash("kill", "invalid arguments");
		return -1;
	}
	if (!isUnsignedInt(cmd.args[0]) || !isUnsignedInt(cmd.args[1])){
		perrorSmash("kill", "invalid arguments");
		return -1;
	}

	int sigNum = std::stoi(cmd.args[0]);
	int jobId = std::stoi(cmd.args[1]);

	Job* job = jm.getJobById(jobId);
	if(job == nullptr){
		std::string err = "job id " + cmd.args[1] +  " does not exist";
		perrorSmash("kill", err.c_str());
		return -1;
	}

	// send the signal
	int res = my_system_call(SYS_KILL, job->pid, sigNum);
	if(res == -1){
		perror("smash error: kill failed");
		return -1;
	}
  if (sigNum == 19) {
    job->status = 3;
  }
	return 0;
}
// fg
int fg(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs > 1){
		perrorSmash("fg", "invalid arguments");
		return -1;
	}
	if (jm.isEmpty()){
		perrorSmash("fg", "jobs list is empty");
		return -1;
	}
	if (cmd.nargs == 1 && !isUnsignedInt(cmd.args[0])){
		perrorSmash("fg", "invalid arguments");
		return -1;
	}

	int jobId = cmd.nargs == 0? jm.getLastJobId() : std::stoi(cmd.args[0]);
	Job* job = jm.getJobById(jobId);
	if(job == nullptr){
		std::stringstream err;
		err << "job id " << jobId << " does not exist";
		perrorSmash("fg", err.str().c_str());;
		return -1;
	}

	std::stringstream out;
	out <<"[" << job->jobId << "] "
	<< job->cmd.command << " ";
	for (const auto &argument: job->cmd.args){
		out << argument;
	}
	if(job->cmd.isBackground){ out << " &"; }
	out << " : "
	<< job->pid << std::endl;

	// check if stopped  
	if(job->status == 3){
		// SIGCONT = 18
		if(my_system_call(SYS_KILL, job->pid, 18) == -1){
			perror("smash error: kill failed");
			return -1;
		}
	}

	// bring job to foreground
	pid_t pid = job->pid;
	ShellCommand jobcmd = job->cmd;
	jm.updateFgCmd(job->cmd);
	jm.removeJobById(jobId);
	int status = 0;
	jm.updateFgCmd(jobcmd);
	if(my_system_call(SYS_WAITPID, pid, &status, WUNTRACED) == -1){
		perror("smash error: waitpid failed");
		return -1;
	}

	jm.clearFgCmd();

	if (WIFSTOPPED(status)) {
	    jm.addJob(jobcmd, pid, 3);
	}
	return 0;
}

int bg(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs > 1){
		perrorSmash("bg", "invalid arguments");
		return -1;
	}
	if(jm.isEmpty()){
		perrorSmash("bg", "there are no stopped jobs to resume");
		return -1;
	}
	if (cmd.nargs == 1 && !isUnsignedInt(cmd.args[0])){
		perrorSmash("bg", "invalid arguments");
		return -1;
	}
	int jobId = cmd.nargs == 0? jm.getLastStoppedJobId() : std::stoi(cmd.args[0]);
	if(cmd.nargs == 0 && jobId == -1){
		perrorSmash("bg", "there are no stopped jobs to resume");
		return -1;
	}
	Job* job = jm.getJobById(jobId);
	if(job == nullptr){
		std::stringstream err;
		err << "job id " << jobId << " does not exist";
		perrorSmash("bg", err.str().c_str());
		return -1;
	}

	// check if stopped  
	if(job->status != 3 && cmd.nargs == 1){
		std::stringstream err;
		err << "job id " << jobId << " is already in background";
		perrorSmash("bg", err.str().c_str());
		return -1;
	}

	std::stringstream out;
	out <<"[" << job->jobId << "] "
	<< job->cmd.command << " ";
	for(const auto &argument: job->cmd.args){
			out << argument;
	}
	if(job->cmd.isBackground){ out << " &"; }
	out << " : "
	<< job->pid << std::endl;

	// SIGCONT = 18
	if(my_system_call(SYS_KILL, job->pid, 18) == -1){
		perror("smash error: kill failed");
		return -1;
	}
	job->status = 2; // running
	return 0;
}

int quit(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs > 1){
		perrorSmash("quit", "expected 0 or 1 arguments");
		return -1;
	}
	if(cmd.nargs == 1 && cmd.args[0] != "kill"){
		perrorSmash("quit", "unexpected arguments");
		return -1;
	}
	if(cmd.nargs == 1 && cmd.args[0] == "kill"){
		while (!jm.isEmpty()) {
      if (isSigInt) {
        return -1;
      }
			int jobId = jm.getFirstJobId(); 
			if(jm.killJobById(jobId) == -1){
				perror("smash error: kill failed");
				return -1;
			}
		}
	}
	exit(0);
}



//example function for printing errors from internal commands
void perrorSmash(const char* cmd, const char* msg)
{
    fprintf(stderr, "smash error:%s%s%s\n",
        cmd ? cmd : "",
        cmd ? ": " : "",
        msg);
}

int diff(ShellCommand& cmd){
	int f1=-1, f2=-1;
	const int BUF_SIZE = 4096;
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];
	ssize_t r1, r2;
	std::string file1, file2;

	if(cmd.nargs != 2){
		perrorSmash("diff", "expected 2 arguments");
		return -1;
	}
	file1 = cmd.args[0];
	file2 = cmd.args[1];
	int returnCode = 0;
	
	f1 = (int)my_system_call(SYS_OPEN ,file1.c_str(), O_RDONLY);
	if (f1 < 0) { 
    if (errno != EINTR) {
      perrorSmash("diff", "expected valid paths for files");
    }
		returnCode = -1;
		goto l_cleanup; 
	}
	f2 = (int)my_system_call(SYS_OPEN ,file2.c_str(), O_RDONLY);
	if(f2 < 0){
    if (errno != EINTR) {
      perrorSmash("diff", "expected valid paths for files");
    }
		returnCode = -1;
		goto l_cleanup; 
	}

	while(1){
		r1 = my_system_call(SYS_READ, f1, buf1, BUF_SIZE);
		r2 = my_system_call(SYS_READ, f2, buf2, BUF_SIZE);
    if (isSigInt || errno == EINTR) {
      returnCode = -1;
      goto l_cleanup;
    }

        if (r1 < 0 || r2 < 0) {
			if (errno == EISDIR) {
                // READ FAILED because it's a directory
                // Matches your original: (isDirectory || isDirectory)
                perrorSmash("diff", "paths are not files");
            } else {
                perror("smash error: read failed");
        	}
			returnCode = -1;
			goto l_cleanup;
		}

        if (r1 != r2) {
			printf("1\n");
            goto l_cleanup;
        }

        if (r1 == 0 && r2 == 0) {
            break;
        }

        for (int i = 0; i < r1; ++i) {
            if (buf1[i] != buf2[i]) {
				printf("1\n");
                goto l_cleanup;
            }
        }
	}
	printf("0\n");

	l_cleanup:
		if (f1 >= 0){
			if(my_system_call(SYS_CLOSE, f1) == -1){
				perror("smash error: close failed");
				returnCode = -1;
			}
		}
		if (f2 >= 0){
			if(my_system_call(SYS_CLOSE, f2) == -1){
				perror("smash error: close failed");
				returnCode = -1;
			}
		}
		return returnCode;
}

int alias(ShellCommand& cmd, AliasedCmds& aliasesList) {
    if (cmd.nargs == 0) {
		perrorSmash("alias", "expected at least 1 argument");
        return -1;
    }
    std::string firstArg = cmd.args[0];
    size_t eqPos = firstArg.find('='); 

    if (eqPos == std::string::npos) {
        perrorSmash("alias", "invalid alias format");
        return -1;
    }

    if (eqPos + 1 >= firstArg.length() || firstArg[eqPos + 1] != '"') {
        perrorSmash("alias", "invalid alias format");
        return -1;
    }

    std::string aliasName = firstArg.substr(0, eqPos);

    std::string cmdString = firstArg.substr(eqPos + 2);

    for (size_t i = 1; i < cmd.args.size(); ++i) {
        cmdString += " " + cmd.args[i];
    }

    if (!cmdString.empty() && cmdString.back() == '"') {
        cmdString.pop_back();
    } else {
        perrorSmash("alias", "invalid alias format");
        return -1;
    }

    if (aliasName.empty() || cmdString.empty()) {
        perrorSmash("alias", "invalid alias format");
        return -1;
    }

    aliasesList.addAlias(cmdString, aliasName);
    
    return 0;
}

int unalias(ShellCommand& cmd, AliasedCmds& aliasesList) {
	if (cmd.nargs != 1) {
		perrorSmash("unalias", "expected exactly 1 argument");
		return -1;
	}

	std::string aliasName = cmd.args[0];

	aliasesList.unAlias(aliasName);

	return 0;
}
