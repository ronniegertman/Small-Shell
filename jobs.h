#ifndef JOBS_H
#define JOBS_H
/*=============================================================================*/
#include "shellcommand.h"
#include <string>
#include <vector>
/*=============================================================================*/
class Job {
	public:
		unsigned int jobId;
		int status;
		ShellCommand cmd;
		pid_t pid;
		time_t startTime;
		// REMOVE BACKGROUND IT IS IN SHELL COMMAND
		Job(const ShellCommand& command, unsigned int jobId, pid_t pid, int status);
		double getElapsedTime() const;
};

class JobManager{
	std::vector<Job> jobsList;
	bool fgactive = false;
	public:
		ShellCommand fgcmd; // foreground command
		JobManager();
		int generateJobId(); // job vector will be sorted
		int addJob(const ShellCommand& cmd, int pid, int status);
		int removeJobById(unsigned int jobId);
		int removeJobByPid(pid_t pid);
		std::string printJobsList();
		Job* getJobById(unsigned int jobId);
		bool isEmpty();
		int size();
		int getLastJobId();
		int killJobById(int jobId);
		void updateList();
		void updateFgCmd(ShellCommand& cmd);
		void clearFgCmd();
};
/*=============================================================================*/
#endif //JOBS_H
