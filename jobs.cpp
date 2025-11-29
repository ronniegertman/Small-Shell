#include "jobs.h"
#include "my_system_call.h"
#include "shellcommand.h"
#include <vector>
#include <sstream>
#include <unistd.h>
#include <ctime>

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
