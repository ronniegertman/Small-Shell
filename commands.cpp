//commands.c
#include "commands.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "my_system_call.h"
#include "jobs.h"
#include "shellcommand.h"
#include <vector>
#include <ctime>
#include <sstream>
#include <sys/stat.h>

// showpid
void showpid(ShellCommand& cmd)
{
	if(cmd.nargs != 0) {
		perrorSmash("showpid", "expected 0 arguments");
		return;
	}

	pid_t pid;
	if(cmd.isBackground) {
		pid = getppid();
	}else {
		pid = getpid();
	}

	printf("smash pid is %d\n",pid);
}

//pwd
void pwd(ShellCommand& cmd){
	if(cmd.nargs != 0) {
		perrorSmash("pwd", "expected 0 arguments");
		return;
	}
	char cwd[CMD_LENGTH_MAX]; //current working directory
	getcwd(cwd, sizeof(cwd));
	printf("%s\n", cwd);
	
}

//cd
bool isRegularFile(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return false; // does not exist
    return S_ISREG(info.st_mode);                       // true if regular file
}

bool isDirectory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return false;
    return S_ISDIR(info.st_mode);
}


char prev_dir[CMD_LENGTH_MAX] = "";

void cd(ShellCommand& cmd){
	if(cmd.nargs != 1) {
		perrorSmash("cd", "expected 1 arguments");
		return;
	}
	char cwd[CMD_LENGTH_MAX]; //current working directory
	getcwd(cwd, sizeof(cwd));

	std::string currentDir = cwd;
	std::string targetDir;
	//check if address ilegal
	if(cmd.args[0] == "-") {
		if(strlen(prev_dir) == 0) {
			perrorSmash("cd", "old pwd not set");
			return;
		}
		targetDir = std::string(prev_dir);
	} else if(cmd.args[0] == ".."){
		targetDir = currentDir.substr(0, currentDir.find_last_of('/'));
		if(targetDir.empty()) {
			targetDir = "/";
		}
	} else {
		targetDir = cmd.args[0];
	}	
	// checking for valid directory
	if(isRegularFile(targetDir)) {
		perrorSmash("cd", "not a directory");
		return;
	}
	if(!isDirectory(targetDir)) {
		perrorSmash("cd", "target directory does not exist");
		return;
	}

	// performing the dir change:
	chdir(targetDir.c_str());
	// updating prev_dir
	strcpy(prev_dir, cwd);
}

//jobs
// PRINT SRGUMENTS LIST
void jobs(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs !=0){
		perrorSmash("jobs", "expected 0 arguments");
		return;
	}
	printf("%s", jm.printJobsList().c_str());
}

//kill
void kill(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs !=2){
		perrorSmash("kill", "invalid arguments");
		return;
	}
	int sigNum = std::stoi(cmd.args[0]);
	int jobId = std::stoi(cmd.args[1]);

	Job* job = jm.getJobById(jobId);
	if(job == nullptr){
		std::string err = "job id " + cmd.args[1] +  " does not exist";
		perrorSmash("kill", err.c_str());
	}

	// send the signal 
	int res = my_system_call(SYS_KILL, job->pid, sigNum);
	if(res == -1){
		perrorSmash("kill", "invalid arguments");
	}
}
// fg
void fg(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs > 1){
		perrorSmash("fg", "invalid arguments");
		return;
	}
	if (jm.isEmpty()){
		perrorSmash("fg", "jobs list is empty");
		return;
	}
	int jobId = cmd.nargs == 0? jm.getLastJobId() : std::stoi(cmd.args[0]);
	Job* job = jm.getJobById(jobId);
	if(job == nullptr){
		std::string err = "job id " + cmd.args[0] +  " does not exist";
		perrorSmash("fg", err.c_str());
		return;
	}

	std::stringstream out;
	out <<"[" << job->jobId << "] "
	<< job->cmd.command;
	if(job->isBackground){ out << " &"; }
	out << " : "
	<< job->pid << std::endl;

	// check if stopped  
	if(job->status == 3){
		// SIGCONT = 18
		my_system_call(SYS_KILL, job->pid, 18);
	}

	// bring job to foreground
	jm.removeJobById(jobId);
	int status = 0;
	my_system_call(SYS_WAITPID, job->pid, &status, WUNTRACED);
}

void bg(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs > 1){
		perrorSmash("bg", "invalid arguments");
		return;
	}
	int jobId = cmd.nargs == 0? jm.getLastJobId() : std::stoi(cmd.args[0]);
	Job* job = jm.getJobById(jobId);
	if(job == nullptr){
		std::string err = "job id " + cmd.args[0] +  " does not exist";
		perrorSmash("bg", err.c_str());
		return;
	}

	// check if stopped  
	if(job->status != 3){
		std::string err = "job id " + cmd.args[0] +  " is already in background";
		perrorSmash("bg", err.c_str());
		return;
	}
	std::stringstream out;
	out <<"[" << job->jobId << "] "
	<< job->cmd.command;
	if(job->isBackground){ out << " &"; }
	out << " : "
	<< job->pid << std::endl;

	// SIGCONT = 18
	my_system_call(SYS_KILL, job->pid, 18);
	job->status = 2; // running
}

void quit(ShellCommand& cmd, JobManager& jm){
	if(cmd.nargs > 1){
		perrorSmash("quit", "expected 0 or 1 arguments");
		return;
	}
	if(cmd.nargs == 1 && cmd.args[0] != "kill"){
		perrorSmash("quit", "unexpected arguments");
		return;
	}
	for (int i=0; i<jm.size(); i++){
		Job* job = jm.getJobById(i+1);
		if(job != nullptr){
			continue;
		}
		jm.killJobById(job->jobId);
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

void diff(ShellCommand& cmd){
	int f1=-1, f2=-1;
	const int BUF_SIZE = 4096;
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];
	ssize_t r1, r2;
	std::string file1, file2;

	if(cmd.nargs != 2){
		perrorSmash("diff", "expected 2 arguments");
		return;
	}
	file1 = cmd.args[0];
	file2 = cmd.args[1];

	if ((!isDirectory(file1)  && !isRegularFile(file1)) || (!isDirectory(file2) && !isRegularFile(file2))){
		perrorSmash("diff", "expected valid paths for files");
		return;
	}

	if(isDirectory(file1) || isDirectory(file2)){
		perrorSmash("diff", "paths are not files");
		return;
	}	
	f1 = (int)my_system_call(SYS_OPEN ,file1.c_str(), O_RDONLY);
	if (f1 < 0) { 
		printf("f1\n");
		goto l_cleanup; 
	}
	f2 = (int)my_system_call(SYS_OPEN ,file2.c_str(), O_RDONLY);
	if(f2 < 0){
		printf("f2\n");
		goto l_cleanup; 
	}

	while(1){
		r1 = my_system_call(SYS_READ, f1, buf1, BUF_SIZE);
		r2 = my_system_call(SYS_READ, f2, buf2, BUF_SIZE);

        if (r1 < 0 || r2 < 0) {
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
	if (f1 >= 0) my_system_call(SYS_CLOSE, f1);
	if (f2 >= 0) my_system_call(SYS_CLOSE, f2);
}
