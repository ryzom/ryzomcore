/**
 * \file build_task_queue.cpp
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

#include <nel/misc/types_nl.h>
#include "build_task_queue.h"

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes
#include "pipeline_workspace.h"
#include "pipeline_project.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CBuildTaskQueue::CBuildTaskQueue() : m_QueueId(0)
{
	
}

CBuildTaskQueue::~CBuildTaskQueue()
{
	resetQueue();
}

namespace {

// CTaskTemporaryInfo

} /* anonymous namespace */

void CBuildTaskQueue::loadQueue(CPipelineWorkspace *workspace, bool bypassDependencyError)
{
	m_Mutex.lock();
	nlassert(m_Tasks.empty());
	
	++m_QueueId;
	m_BypassDependencyError = bypassDependencyError;
	const std::map<std::string, CPipelineProject *> &projects = workspace->getProjects();

	std::map<std::string, std::map<uint32, CBuildTaskInfo *> > builtTaskByProjectAndPlugin;
	
	for (std::map<std::string, CPipelineProject *>::const_iterator pr_it = projects.begin(), pr_end = projects.end(); pr_it != pr_end; ++pr_it)
	{
		const std::string &projectName = pr_it->first;
		CPipelineProject *project = pr_it->second;
		builtTaskByProjectAndPlugin[projectName] = std::map<uint32, CBuildTaskInfo *>();
		std::map<uint32, CBuildTaskInfo *> &builtTaskByPlugin = builtTaskByProjectAndPlugin[projectName];
		
		std::vector<std::string> processesToRun;
		project->getValues(processesToRun, "Processes");

		for (std::vector<std::string>::iterator it = processesToRun.begin(), end = processesToRun.end(); it != end; ++it)
		{
			std::string &processName = (*it);
			std::vector<CProcessPluginInfo> processHandlers;
			workspace->getProcessPlugins(processHandlers, processName);

			for (std::vector<CProcessPluginInfo>::iterator h_it = processHandlers.begin(), h_end = processHandlers.end(); h_it != h_end; ++h_it)
			{
				uint32 processHandlerId = (*h_it).Id.Global;

				CBuildTaskInfo *info = new CBuildTaskInfo();
				info->Id.Sub.Queue = m_QueueId;
				info->Id.Sub.Task = m_Tasks.size();
				m_Tasks.push_back(info);
				info->ProjectName = projectName;
				info->ProcessPluginId = processHandlerId;
				// info->Dependencies
			}

			// TODO: PROCESS DEPENDENCIES
		}

		// TODO: PROJECT DEPENDENCIES
	}

	m_Mutex.unlock();
}

CBuildTaskInfo *CBuildTaskQueue::getTaskInfo(uint32 taskId)
{
	CBuildTaskId id;
	id.Global = taskId;
	if (id.Sub.Queue != m_QueueId)
	{
		nlwarning("TaskId of wrong QueueId '%i' in queue '%i'", (sint32)id.Sub.Queue, (sint32)m_QueueId);
		return NULL;
	}
	if (id.Sub.Task >= m_Tasks.size())
	{
		nlwarning("TaskId '%i' out of range '%i'", (sint32)id.Sub.Task, (sint32)m_Tasks.size());
		return NULL;
	}
	return m_Tasks[id.Sub.Task];
}

CBuildTaskInfo *CBuildTaskQueue::getTaskForSlave(const std::vector<uint32> &availablePlugins)
{
	m_Mutex.lock();
	std::vector<CBuildTaskInfo *> availableTasks;
	createBuildableTaskList(availableTasks, m_BypassDependencyError);
	sortBuildableTaskListByMostDependents(availableTasks);
	for (std::vector<CBuildTaskInfo *>::iterator it = availableTasks.begin(), end = availableTasks.end(); it != end; ++it)
	{
		CBuildTaskInfo *task = (*it);
		if (find(availablePlugins.begin(), availablePlugins.end(), task->ProcessPluginId) != availablePlugins.end())
		{
			task->State = TASK_WORKING;
			m_Mutex.unlock();
			return task;
		}
	}
	m_Mutex.unlock();
	return NULL; // no task available for slave.
}

void CBuildTaskQueue::abortedTask(uint32 taskId)
{
	m_Mutex.lock();
	CBuildTaskInfo *info = getTaskInfo(taskId);
	info->State = TASK_ABORTED;
	m_Mutex.unlock();
}

void CBuildTaskQueue::rejectedTask(uint32 taskId)
{
	m_Mutex.lock();
	CBuildTaskInfo *info = getTaskInfo(taskId);
	info->State = TASK_WAITING; // make available again
	m_Mutex.unlock();
}

void CBuildTaskQueue::erroredTask(uint32 taskId)
{
	m_Mutex.lock();
	CBuildTaskInfo *info = getTaskInfo(taskId);
	info->State = TASK_ERRORED;
	m_Mutex.unlock();
}

void CBuildTaskQueue::successTask(uint32 taskId)
{
	m_Mutex.lock();
	CBuildTaskInfo *info = getTaskInfo(taskId);
	info->State = TASK_SUCCESS;
	m_Mutex.unlock();
}

void CBuildTaskQueue::abortQueue()
{
	m_Mutex.lock();
	for (std::vector<CBuildTaskInfo *>::iterator it = m_Tasks.begin(), end = m_Tasks.end(); it != end; ++it)
	{
		if ((*it)->State == TASK_WAITING)
			(*it)->State = TASK_ABORTED;
	}
	m_Mutex.unlock();
}

void CBuildTaskQueue::resetQueue()
{
	m_Mutex.lock();

	// count remaining and working first and assert its 0
	std::vector<CBuildTaskInfo *> availableTasks;
	createBuildableTaskList(availableTasks, m_BypassDependencyError);
	uint nb = availableTasks.size();
	for (std::vector<CBuildTaskInfo *>::iterator it = m_Tasks.begin(), end = m_Tasks.end(); it != end; ++it)
		if ((*it)->State == TASK_WORKING)
			++nb;
	nlassert(nb == 0);

	for (std::vector<CBuildTaskInfo *>::iterator it = m_Tasks.begin(), end = m_Tasks.end(); it != end; ++it)
		delete (*it);
	m_Tasks.clear();

	m_Mutex.unlock();
}

uint CBuildTaskQueue::countRemainingBuildableTasks()
{
	m_Mutex.lock();
	std::vector<CBuildTaskInfo *> availableTasks;
	createBuildableTaskList(availableTasks, m_BypassDependencyError);
	m_Mutex.unlock();
	return availableTasks.size();
}

uint CBuildTaskQueue::countWorkingTasks()
{
	m_Mutex.lock();
	uint nb = 0;
	for (std::vector<CBuildTaskInfo *>::iterator it = m_Tasks.begin(), end = m_Tasks.end(); it != end; ++it)
		if ((*it)->State == TASK_WORKING)
			++nb;
	m_Mutex.unlock();
	return nb;
}

uint CBuildTaskQueue::countRemainingBuildableTasksAndWorkingTasks()
{
	m_Mutex.lock();
	std::vector<CBuildTaskInfo *> availableTasks;
	createBuildableTaskList(availableTasks, m_BypassDependencyError);
	uint nb = availableTasks.size();
	for (std::vector<CBuildTaskInfo *>::iterator it = m_Tasks.begin(), end = m_Tasks.end(); it != end; ++it)
		if ((*it)->State == TASK_WORKING)
			++nb;
	m_Mutex.unlock();
	return nb;
}

void CBuildTaskQueue::listTaskQueueByMostDependents(std::vector<CBuildTaskInfo *> &result)
{
	result.clear();
	result.reserve(m_Tasks.size());
	/*copy(m_Tasks.begin(), m_Tasks.end(), result);
	sortBuildableTaskListByMostDependents(result);*/
}

void CBuildTaskQueue::countDependents(uint &dependentResult, CBuildTaskInfo *taskInfo)
{
	uint nb = 0;
	for (std::vector<uint16>::size_type i = 0; i < m_Tasks.size(); ++i)
		if (doesTaskDependOnTask(m_Tasks[i], taskInfo)) ++nb;
	dependentResult = nb;
}

void CBuildTaskQueue::flagDependents(std::vector<bool> &dependentResult, CBuildTaskInfo *taskInfo)
{
	dependentResult.resize(m_Tasks.size());
	for (std::vector<bool>::size_type i = 0; i < dependentResult.size(); ++i)
		dependentResult[i] = doesTaskDependOnTask(m_Tasks[i], taskInfo);
}

bool CBuildTaskQueue::doesTaskDependOnTask(CBuildTaskInfo *doesThisTask, CBuildTaskInfo *dependOnThisTask)
{
	for (std::vector<uint16>::size_type i = 0; i < doesThisTask->Dependencies.size(); ++i)
	{
		uint16 dependencyId = doesThisTask->Dependencies[i];
		if (dependencyId == dependOnThisTask->Id.Sub.Task)
			return true;
		if (doesTaskDependOnTask(m_Tasks[dependencyId], dependOnThisTask))
			return true;
	}
	return false;
}

void CBuildTaskQueue::createBuildableTaskList(std::vector<CBuildTaskInfo *> &result, bool bypassError)
{
	// makes a list of tasks where all dependencies are ready
	result.clear();
	result.reserve(m_Tasks.size());
	for (std::vector<CBuildTaskInfo *>::iterator it = m_Tasks.begin(), end = m_Tasks.end(); it != end; ++it)
	{
		if ((*it)->State == TASK_WAITING)
		{
			bool ok = true;
			for (std::vector<uint16>::iterator it2 = (*it)->Dependencies.begin(), end2 = (*it)->Dependencies.end(); it2 != end2; ++it2)
			{
				TBuildTaskState dependencyState = m_Tasks[*it2]->State;
				if (((dependencyState == TASK_ERRORED) && !bypassError)
					|| dependencyState == TASK_WAITING
					|| dependencyState == TASK_WORKING
					|| (dependencyState == TASK_ABORTED && !bypassError))
				{
					ok = false;
					break;
				}
			}
			if (ok)
			{
				result.push_back((*it));
			}
		}
	}
	// sortBuildableTaskListByMostDependents(result);
}

void CBuildTaskQueue::sortBuildableTaskListByMostDependents(std::vector<CBuildTaskInfo *> &result)
{
	// brings most urgent tasks on top
	std::vector<uint> dependentsCache;
	dependentsCache.resize(result.size());
	for (std::vector<uint>::size_type i = 0; i < dependentsCache.size(); ++i)
		countDependents(dependentsCache[i], result[i]);
	uint sc;
	do
	{
		sc = 0;
		for (std::vector<uint>::size_type i = 0; i < dependentsCache.size() - 1; ++i)
		{
			if (dependentsCache[i + 1] > dependentsCache[i])
			{
				swap(dependentsCache[i], dependentsCache[i + 1]);
				swap(result[i], result[i + 1]);
				++sc;
			}
		}
	} while (sc != 0);
}

} /* namespace PIPELINE */

/* end of file */
