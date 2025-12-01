#ifndef JOBS_H
#define JOBS_H
/*=============================================================================*/
#include "shellcommand.h"
#include <string>
#include <vector>
/*=============================================================================*/
class Job {
	public:
		int jobId;
		int status; // 2 - running bg, 3 - stopped
		ShellCommand cmd;
		int pid;
		time_t startTime;
		// REMOVE BACKGROUND IT IS IN SHELL COMMAND
		bool isBackground;
		Job(const ShellCommand& command, int jobId, int pid, int status, bool isBg);
		double getElapsedTime() const;
};

class JobManager{
	std::vector<Job> jobsList;
	public:
		int generateJobId(); // job vector will be sorted
		int addJob(const ShellCommand& cmd, int pid, int status, bool isBg);
		int removeJobById(int jobId);
		int removeJobByPid(int pid);
		std::string printJobsList();
		Job* getJobById(int jobId);
		bool isEmpty();
		int size();
		int getLastJobId();
		int killJobById(int jobId);
		void updateList();
};
/*=============================================================================*/
#endif //JOBS_H
