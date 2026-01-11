// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdmisc.h"

#include "nel/misc/task_manager.h"
#include "nel/misc/big_file.h"

using namespace std;

#define NLMISC_DONE_TASK_SIZE 20

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

/*
 * Constructor
 */
CTaskManager::CTaskManager() : _RunningTask (""), _TaskQueue (""), _DoneTaskQueue ("")
{
	_IsTaskRunning = false;
	_ThreadRunning = true;
	CSynchronized<string>::CAccessor currentTask(&_RunningTask);
	currentTask.value ().clear();
	_Thread = IThread::create(this);
	_Thread->start();
	_ChangePriorityCallback = NULL;
}

/*
 * Destructeur
 */
CTaskManager::~CTaskManager()
{
	_ThreadRunning = false;
	while(!_ThreadRunning)
		nlSleep(10);

	// There should be no remaining Tasks
	CSynchronized<std::list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	nlassert(acces.value().empty());
	_Thread->wait();
	delete _Thread;
	_Thread = NULL;

}

// Manage TaskQueue
void CTaskManager::run(void)
{
	IRunnable *runnableTask;
	float priorityTask = 0.f;
	while(_ThreadRunning)
	{
		{
			CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
			if(acces.value().empty())
			{
				runnableTask = NULL;
			}
			else
			{
				// Update task priorities
				changeTaskPriority ();

				// Get the best task
				list<CWaitingTask> &taskList = acces.value();
				list<CWaitingTask>::iterator ite = taskList.begin();
				list<CWaitingTask>::iterator bestIte = ite;
				while (ite != taskList.end())
				{
					if (ite->Priority < bestIte->Priority)
						bestIte = ite;

					// Next task;
					ite++;
				}

				_IsTaskRunning = true;
				runnableTask = bestIte->Task;
				priorityTask = bestIte->Priority;
				taskList.erase (bestIte);
			}
		}
		if(runnableTask)
		{
			{
				CSynchronized<string>::CAccessor currentTask(&_RunningTask);
				string temp;
				runnableTask->getName(temp);
				currentTask.value () = temp + " " + toString (priorityTask);
			}
			runnableTask->run();
			{
				CSynchronized<string>::CAccessor currentTask(&_RunningTask);
				CSynchronized<deque<string> >::CAccessor doneTask(&_DoneTaskQueue);
				doneTask.value().push_front (currentTask.value ());
				currentTask.value ().clear();
				if (doneTask.value().size () > NLMISC_DONE_TASK_SIZE)
					doneTask.value().resize (NLMISC_DONE_TASK_SIZE);
			}

			_IsTaskRunning = false;
		}
		else
		{
			sleepTask();
		}
	}
	CBigFile::getInstance().currentThreadFinished();
	_ThreadRunning = true;
}

// Add a task to TaskManager
void CTaskManager::addTask(IRunnable *r, float priority)
{
	CSynchronized<std::list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	acces.value().push_back(CWaitingTask(r, priority));
}

/// Delete a task, only if task is not running, return true if found and deleted
bool CTaskManager::deleteTask(IRunnable *r)
{
	CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	for(list<CWaitingTask>::iterator it = acces.value().begin(); it != acces.value().end(); it++)
	{
		if(it->Task == r)
		{
			acces.value().erase(it);
			return true;
		}
	}
	return false;
}

/// Task list size
uint CTaskManager::taskListSize(void)
{
	CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	return (uint)acces.value().size();
}


void	CTaskManager::waitCurrentTaskToComplete ()
{
	while (_IsTaskRunning)
		sleepTask();
}

// ***************************************************************************

void CTaskManager::dump (std::vector<std::string> &result)
{
	CSynchronized<string>::CAccessor accesCurrent(&_RunningTask);
	CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	CSynchronized<deque<string> >::CAccessor accesDone(&_DoneTaskQueue);

	const list<CWaitingTask> &taskList = acces.value();
	const deque<string> &taskDone = accesDone.value();
	const string &taskCurrent = accesCurrent.value();

	// Resize the destination array
	result.clear ();
	result.reserve (taskList.size () + taskDone.size () + 1);

	// Add the done strings
	deque<string>::const_reverse_iterator iteDone = taskDone.rbegin ();
	while (iteDone != taskDone.rend ())
	{
		result.push_back ("Done : " + *iteDone);

		// Next task
		iteDone++;
	}

	// Add the current string
	if (!taskCurrent.empty())
	{
		result.push_back ("Current : " + taskCurrent);
	}

	// Add the waiting strings
	list<CWaitingTask>::const_iterator ite = taskList.begin ();
	while (ite != taskList.end ())
	{
		string name;
		ite->Task->getName (name);
		result.push_back ("Waiting : " + name + " " + toString(ite->Priority));

		// Next task
		ite++;
	}
}

// ***************************************************************************
void CTaskManager::clearDump()
{
	CSynchronized<deque<string> >::CAccessor accesDone(&_DoneTaskQueue);
	accesDone.value().clear();
}

// ***************************************************************************

uint CTaskManager::getNumWaitingTasks()
{
	CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	return (uint)acces.value().size();
}

// ***************************************************************************

void CTaskManager::changeTaskPriority ()
{
	if (_ChangePriorityCallback)
	{
		CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
		list<CWaitingTask> &taskList = acces.value();

		list<CWaitingTask>::iterator ite = taskList.begin();
		while(ite != taskList.end())
		{
			// Get the new priority
			ite->Priority = _ChangePriorityCallback->getTaskPriority(*(ite->Task));

			// Next task
			ite++;
		}
	}
}

// ***************************************************************************

void CTaskManager::registerTaskPriorityCallback (IChangeTaskPriority *callback)
{
	_ChangePriorityCallback = callback;
}

// ***************************************************************************

} // NLMISC
