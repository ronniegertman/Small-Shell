#include "jobs.h"
#include "my_system_call.h"
#include "shellcommand.h"
#include <cerrno>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <ctime>
#include "signal.h"
#include "signals.h"

// Job class functions
Job::Job(const ShellCommand& command, unsigned int jobId, pid_t pid, int stat) :
	jobId(jobId),
	status(stat),
	cmd(command),
	pid(pid),
	startTime(std::time(nullptr)) {}

double Job::getElapsedTime() const {
	return difftime(std::time(nullptr), this->startTime);
}

// initializing jm with "empty" foreground command
JobManager::JobManager() : fgactive(false), fgcmd("", {}, false, 0, 0) {}

// JobManager class functions
int JobManager::generateJobId() {
	for (size_t i = 0; i < jobsList.size(); i++) {
        if (jobsList[i].jobId != i) {
            return i;
        }
    }
    return jobsList.size();
}

int JobManager::addJob(const ShellCommand& cmd, int pid, int status) {
	// enter new job sorted by jobId
	JobManager::updateList();
	Job newJob(cmd, this->generateJobId(), pid, status);
	auto it = jobsList.begin();
	while(it != jobsList.end() && it->jobId < newJob.jobId) {
		it++;
	}
	jobsList.insert(it, newJob);
	return newJob.jobId;
}

int JobManager::removeJobById(unsigned int jobId) {
	for(auto it = jobsList.begin(); it != jobsList.end(); it++) {
		if(it->jobId == jobId) {
			jobsList.erase(it);
			return 0; // success
		}
	}
	return -1; // not found
}

int JobManager::removeJobByPid(pid_t pid) {
	for(auto it = jobsList.begin(); it != jobsList.end(); it++) {
		if(it->pid == pid) {
			jobsList.erase(it);
			return 0; // success
		}
	}
	return -1; // not found
}

std::string JobManager::printJobsList() {
	JobManager::updateList();
    std::stringstream out;
    for (const auto& job : jobsList) {
        out << "[" << job.jobId << "] "
            << job.cmd.command << " ";
		// print all the arguments
		for(const auto &argument: job.cmd.args){
			out << argument;
		}
		if(job.cmd.isBackground){
			out << " &";
		}
		out << ": "
		<< job.pid << " "

		<< job.getElapsedTime() << " secs ";
		if (job.status == 3){
			out << "(stopped)";
		}
		out << "\n";
		}
    return out.str();
}

Job* JobManager::getJobById(unsigned int jobId){
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
		return -1;
	}
	return jobsList.back().jobId;
}
int JobManager::getFirstJobId(){
	if(jobsList.empty()){
		return -1;
	}
	return jobsList.front().jobId;
}

int JobManager::killJobById(int jobId){
	Job* job = getJobById(jobId);
	if(job == nullptr){
		return -1; // not found
	}
	std::stringstream out;
	out <<"[" << job->jobId << "] "
	<< job->cmd.command << " ";
	for(const auto &argument: job->cmd.args){
			out << argument;
	}
	if(job->cmd.isBackground){ out << " &"; }
	out << " - " << "sending SIGTERM... ";
	printf("%s", out.str().c_str());
	fflush(stdout);
	out.str(""); // clear the stringstream
	//SIGTERM = 15
	if(my_system_call(SYS_KILL, job->pid, 15) == -1){
		perror("smash error: kill failed");
		return -1;
	}
  if (isSigInt) return -1;
  
	pid_t result;
	int status = 0;
	  for (int i = 0; i < 50; i++) {
        result = my_system_call(SYS_WAITPID ,job->pid, &status, WNOHANG);
		if(result == -1){
      if (errno == EINTR) return -1;
      perror("smash error: waitpid failed");
			return -1;
		}
        if (result == job->pid) {
            // child exited during wait
			printf("done\n");
            removeJobByPid(job->pid);
            return 0;
        }
        usleep(100 * 1000);  // 100ms
        if (isSigInt) return -1;
    }

	// sending SIGKILL
	out << "sending SIGKILL... ";
	if(my_system_call(SYS_KILL, job->pid, 9) == -1){ // SIGKILL = 9
		perror("smash error: kill failed");
		return -1;
	}
  if (isSigInt) return -1;
	if(my_system_call(SYS_WAITPID ,job->pid, &status, 0) == -1){
    if (errno == EINTR) return -1;
		perror("smash error: waitpid failed");
		return -1;
	}
	removeJobByPid(job->pid);

	out << "done\n";
	printf("%s", out.str().c_str());
	return 0; // success
}

void JobManager::updateList() {
    int status;
    for (auto it = jobsList.begin(); it != jobsList.end(); /* no increment here */) {
        Job& job = *it; // Get a reference to the current job
        pid_t result = my_system_call(SYS_WAITPID, job.pid, &status, WNOHANG | WUNTRACED | WCONTINUED);

        if (result == -1) {
            perror("smash error: waitpid failed");
            ++it;
            continue;
        }

        if (result == job.pid) {
            // Status changed or process finished
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                // Process finished -> Remove from list
                // erase() removes the element and returns the iterator to the *next* element
                it = jobsList.erase(it);
            } 
            else if (WIFSTOPPED(status)) {
                // Process stopped -> Update status
                job.status = 3; // 3 = STOPPED
                ++it;
            } 
            else if (WIFCONTINUED(status)) {
                job.status = 2; // 2 = BACKGROUND
                ++it;
            } 
            else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}


void JobManager::updateFgCmd(ShellCommand& cmd){
	this->fgactive = true;
	fgcmd = cmd;
}

void JobManager::clearFgCmd(){
	this->fgactive = false;
	fgcmd = ShellCommand("", {}, false, -1, 0);
}

int JobManager::getLastStoppedJobId(){
	if(jobsList.empty()){
		return -1;
	}
	int stoppedJobId = -1; // default value if no stopped jobs
	for(auto it = jobsList.rbegin(); it != jobsList.rend(); it++) {
		if(it->status == 3){ // stopped
			stoppedJobId = it->jobId;
		}
	}
	return stoppedJobId;
}
