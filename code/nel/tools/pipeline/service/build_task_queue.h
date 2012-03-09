/**
 * \file build_task_queue.h
 * \brief CBuildTaskQueue
 * \date 2012-03-09 12:02GMT
 * \author Jan Boon (Kaetemi)
 * CBuildTaskQueue
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_BUILD_TASK_QUEUE_H
#define PIPELINE_BUILD_TASK_QUEUE_H
#include <nel/misc/types_nl.h>

// STL includes
#include <boost/thread/mutex.hpp>

// NeL includes

// Project includes

namespace PIPELINE {
	class CPipelineWorkspace;

enum TBuildTaskState
{
	TASK_WAITING, // return to waiting after reject!
	TASK_WORKING, // set state to working while building
	TASK_SUCCESS, // after successful build set state to success
	TASK_ABORTED, // after aborted by slave and master
	TASK_ERRORED, // after error set state to errored
};

struct CBuildTaskId
{
	union
	{
		struct 
		{
			uint16 Task;
			uint16 Queue;
		} Sub;
		uint32 Global;
	};
};

struct CBuildTaskInfo
{
	CBuildTaskInfo() :
		State(TASK_WAITING)
	{
		
	}

	CBuildTaskId Id;

	std::string ProjectName;
	uint32 ProcessPluginId;
	
	std::vector<uint16> Dependencies; // Tasks on which this task depends
	TBuildTaskState State;
};

/**
 * \brief CBuildTaskQueue
 * \date 2012-03-09 12:02GMT
 * \author Jan Boon (Kaetemi)
 * CBuildTaskQueue
 */
class CBuildTaskQueue
{
protected:
	uint16 m_QueueId;
	boost::mutex m_Mutex;

	std::vector<CBuildTaskInfo *> m_Tasks;
	bool m_BypassDependencyError;

public:
	CBuildTaskQueue();
	virtual ~CBuildTaskQueue();
	
	void loadQueue(CPipelineWorkspace *workspace, bool bypassDependencyError);
	CBuildTaskInfo *getTaskInfo(uint32 taskId);
	
	/// Gets task for slave, state is set to TASK_WORKING, returns NULL if none available
	CBuildTaskInfo *getTaskForSlave(const std::vector<uint32> &availablePlugins);
	void abortedTask(uint32 taskId);
	void rejectedTask(uint32 taskId);
	void erroredTask(uint32 taskId);
	void successTask(uint32 taskId);

	void abortQueue();
	void resetQueue();

	uint countRemainingBuildableTasks();
	uint countWorkingTasks();
	// when next are 0 the build should stop
	uint countRemainingBuildableTasksAndWorkingTasks();

	// informational listing for sending initial task listing to terminals
	void listTaskQueueByMostDependents(std::vector<CBuildTaskInfo *> &result);
	
private:
	// void countDependencies(uint &waitingResult, uint &failAbortResult, CBuildTaskInfo *taskInfo);
	
	/// Same as next but only counts depending tasks that are waiting.	
	void countWaitingDependents(uint &dependentResult, CBuildTaskInfo *taskInfo);
	void flagWaitingDependents(std::vector<bool> &dependentResult, CBuildTaskInfo *taskInfo);

	/// Recursively count the number of tasks that depend on this task.
	void countDependents(uint &dependentResult, CBuildTaskInfo *taskInfo);
	void flagDependents(std::vector<bool> &dependentResult, CBuildTaskInfo *taskInfo);
	
	bool doesTaskDependOnTask(CBuildTaskInfo *doesThisTask, CBuildTaskInfo *dependOnThisTask);
	
	void createBuildableTaskList(std::vector<CBuildTaskInfo *> &result, bool bypassError);
	void sortBuildableTaskListByMostDependents(std::vector<CBuildTaskInfo *> &result);
	void sortBuildableTaskListByMostWaitingDependents(std::vector<CBuildTaskInfo *> &result);

}; /* class CBuildTaskQueue */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_BUILD_TASK_QUEUE_H */

/* end of file */
