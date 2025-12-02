#include "jobs.h"
#include "my_system_call.h"
#include "shellcommand.h"
#include <vector>
#include <sstream>
#include <unistd.h>
#include <ctime>

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
	<< job->cmd.command << " ";
	for(const auto &argument: job->cmd.args){
			out << argument;
	}
	if(job->cmd.isBackground){ out << " &"; }
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
	removeJobByPid(job->pid);

	out << "done\n";
	printf("%s", out.str().c_str());
	return 0; // success
}

void JobManager::updateList(){
	// check each job if it is done
	std::vector<int> pidsToRemove;
	for(const auto& job : jobsList) {
		pid_t result = my_system_call(SYS_WAITPID ,job.pid, nullptr, WNOHANG);
		if (result == job.pid) {
			// job is done
			pidsToRemove.push_back(job.pid);
		}
	}
	// remove done jobs
	for(const auto pid : pidsToRemove){
		JobManager::removeJobByPid(pid);
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
