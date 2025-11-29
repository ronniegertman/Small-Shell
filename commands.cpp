//commands.c
#include "commands.h"
#include <unistd.h>
#include <string.h>
#include "my_system_call.h"
#include <vector>
#include <ctime>
#include <sstream>
#include <sys/stat.h>
// ShellCommand class constructor
ShellCommand::ShellCommand(std::string cmd, std::vector<std::string> arguments, bool bg, pid_t p, int n)
	: command(std::move(cmd)), args(std::move(arguments)), isBackground(bg), pid(p), nargs(n) {}

// Job class functions
Job::Job(const ShellCommand& command, int jobId, int p, int stat, bool isBg)
	: jobId(jobId), cmd(command), pid(p), status(stat), startTime(std::time(nullptr)), isBackground(isBg) {}

double Job::getElapsedTime() const {
	return difftime(std::time(nullptr), this->startTime);
}

// JobManager class functions
int JobManager::generateJobId() {
	for (int i = 0; i < jobsList.size(); i++) {
        if (jobsList[i].jobId != i+1) {
            return i+1;
        }
    }
    return jobsList.size() + 1;
}

int JobManager::addJob(const ShellCommand& cmd, int pid, int status, bool isBg) {
	// enter new job sorted by jobId
	Job newJob(cmd, this->generateJobId(), pid, status, isBg);
	auto it = jobsList.begin();
	while(it != jobsList.end() && it->jobId < newJob.jobId) {
		it++;
	}
	jobsList.insert(it, newJob);
	return newJob.jobId;
}

int JobManager::removeJobById(int jobId) {
	for(auto it = jobsList.begin(); it != jobsList.end(); it++) {
		if(it->jobId == jobId) {
			jobsList.erase(it);
			return 0; // success
		}
	}
	return -1; // not found
}

int JobManager::removeJobByPid(int pid) {
	for(auto it = jobsList.begin(); it != jobsList.end(); it++) {
		if(it->pid == pid) {
			jobsList.erase(it);
			return 0; // success
		}
	}
	return -1; // not found
}

std::string JobManager::printJobsList() {
    std::stringstream out;
    for (const auto& job : jobsList) {
        out << "[" << job.jobId << "] "
            << job.cmd.command;
		if(job.isBackground){
			out << " &";
		}
		out << ": "
		<< job.pid << " "
		<< "status=" << job.status << " "
		<< job.getElapsedTime() << " secs ";
		if (job.status == 3){
			out << "(stopped)";
		}
		out << "\n";
		}
    return out.str();
}

Job* JobManager::getJobById(int jobId){
	for(auto& job : jobsList) {
		if(job.jobId == jobId) {
			return &job;
		}
	}
	return nullptr;
}

bool JobManager::isEmpty(){
	return jobsList.empty();
}

int JobManager::size(){
	return jobsList.size();
}

int JobManager::getLastJobId(){
	if(jobsList.empty()){
		return 0;
	}
	return jobsList.back().jobId;
}

int JobManager::killJobById(int jobId){
	Job* job = getJobById(jobId);
	if(job == nullptr){
		return -1; // not found
	}
	std::stringstream out;
	out <<"[" << job->jobId << "] "
	<< job->cmd.command;
	if(job->isBackground){ out << " &"; }
	out << " - " << "sending SIGTERM... ";

	//SIGTERM = 15
	my_system_call(SYS_KILL, job->pid, 15);
	pid_t result;
	int status = 0;
	  for (int i = 0; i < 50; i++) {
        result = my_system_call(SYS_WAITPID ,job->pid, &status, WNOHANG);
        if (result == job->pid) {
            // child exited during wait
			out << "done\n";
            removeJobByPid(job->pid);
			printf("%s", out.str().c_str());
            return 0;
        }
        usleep(100 * 1000);  // 100ms
    }

	// sending SIGKILL
	out << "sending SIGKILL... ";
	my_system_call(SYS_KILL, job->pid, 9); // SIGKILL = 9
	my_system_call(SYS_WAITPID ,job->pid, &status, 0);
	out << "done\n";
	printf("%s", out.str().c_str());
	return 0; // success
}
// showpid
void showpid(ShellCommand& cmd)
{
	if(cmd.nargs != 0) {
		perrorSmash("showpid", "expected 0 arguments");
		return;
	}

	pid_t pid;
	if(cmd.isBackground) {
		pid_t fork_pid = fork();
		if(fork_pid == 0) {
			//child 
			pid = getppid();
			exit(0);
		}
		else {
			//parent
			my_system_call(SYS_WAITPID, fork_pid, NULL, 0);
		}
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
	if(cmd.isBackground){
		pid_t fork_pid = fork();
		if(fork_pid == 0) {
			//child 
			getcwd(cwd, sizeof(cwd));
			exit(0);
		}
		else {
			//parent
			my_system_call(SYS_WAITPID, fork_pid, NULL, 0);
		}

	} else {
		getcwd(cwd, sizeof(cwd));
	}
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
	int status = job->status;
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

void diff(ShellCommand* cmd){
	if(cmd->nargs != 2){
		perrorSmash("diff", "expected 2 arguments");
		return;
	}
	std::string file1 = cmd->args[0];
	std::string file2 = cmd->args[1];

	if ((!isDirectory(file1)  && !isRegularFile(file1)) || (!isDirectory(file2) && !isRegularFile(file2))){
		perrorSmash("diff", "expected valid paths for files");
		return;
	}

	if(isDirectory(file1) || isDirectory(file2)){
		perrorSmash("diff", "paths are not files");
		return;
	}	
	int f1 = (int)my_system_call(SYS_OPEN ,file1.c_str(), "r");
	 if (f1 < 0) {
        return;
    }
	int f2 = (int)my_system_call(SYS_OPEN ,file2.c_str(), "r");
	if(f2 < 0){
		return;
	}
	const int BUF_SIZE = 4096;
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];

	while(1){
		ssize_t r1 = my_system_call(SYS_READ, f1, buf1, BUF_SIZE);
		ssize_t r2 = my_system_call(SYS_READ, f2, buf2, BUF_SIZE);

        if (r1 < 0 || r2 < 0) {
            close(f1);
            close(f2);
            return;
        }

        if (r1 != r2) {
            close(f1);
            close(f2);
            return;
        }

        if (r1 == 0 && r2 == 0) {
            break;
        }

        // Compare buffers
        for (int i = 0; i < r1; ++i) {
            if (buf1[i] != buf2[i]) {
				printf("1\n");
                close(f1);
                close(f2);
                return;
            }
        }
    }

    close(f1);
    close(f2);
	printf("0\n");
}
