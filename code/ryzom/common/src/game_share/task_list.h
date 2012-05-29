// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef NL_TASK_LIST_H
#define NL_TASK_LIST_H

#include <queue>
#include <algorithm>

#include  "nel/misc/smart_ptr.h"

// Task added to the task list
//
//
template <typename T>
class CTask : public NLMISC::CRefCount
{
public:
	explicit CTask(T t = 0):_Time(t){}
	virtual  ~CTask(){}
	T getTime() const { return _Time;}
	void setTime(T t){ _Time = t; }
	virtual void doOperation() {}
private:
	T _Time;
};


// Used to implement CTaskList
template <typename T>
class CTaskHolder
{
public:
	CTaskHolder(CTask<T>* task):_Task(task){}
	void operator()()const { _Task->doOperation(); }
	~CTaskHolder() { }
	T getTime() const { return _Task->getTime();}
	CTask<T>* getTask() const { return _Task.getPtr(); }


private:
	NLMISC::CSmartPtr< CTask<T> > _Task;
};
/*
template <typename T>
inline bool operator<(const CTaskHolder<T>& lh, const CTaskHolder<T>& rh)
{
	return lh.getTime() < rh.getTime();
}
*/




template<typename T>
class CTaskHolderLess
{
public:
  bool operator()(const CTaskHolder<T>& lh, const CTaskHolder<T>& rh) const
  {
	  return lh.getTime() < rh.getTime();
  }
};

//
// Manages a list of Task that must be executed at a specific time (NLMISC::Time or tick)
// It is not a scheduler (the priority of task can not be dynamicaly changed)
// If 2 task are added at the same time, it is garenty that the first task will be donne before the seconde one ( so internal sort are stable)
template <typename T>
class CTaskList
{


public:

	// execute all task that are waiting
	// now is the current time
	// eg CTimeInterface::gameCycle(), if T == tick  or NLMISC::CTime::getLocalTime()
	void execute(T now);

	// Add a task to a task list (each task now when it must be execute)
	// You must inherit your callback from CTask
	// Complexity is in similar to stable_sort (high)
	void addTask(CTask<T>* task) { _Tasks.push_back(task); _Clean = false; }
	void addTaskAt(T t, CTask<T>* task) {  task->setTime(t); _Tasks.push_back(task);_Clean = false; }
	uint32 getSize() const { return (uint32)_Tasks.size(); }
	CTask<T>* getTaskAt(uint32 index) const { nlassert( index < _Tasks.size()); return _Tasks[index].getTask();}
	void removeTaskAt(uint32 index){ nlassert( index < _Tasks.size()); _Tasks.erase(_Tasks.begin() + index); };


	// Waiting task are discared
	~CTaskList();
	CTaskList(){ _Clean = false; }

private:



private:
	std::deque< CTaskHolder<T> > _Tasks;
	bool _Clean;
};


// Implementation
template <typename T>
void CTaskList<T>::execute(T now)
{


	if (!_Tasks.empty() && !_Clean)
	{
		stable_sort(_Tasks.begin(), _Tasks.end(), CTaskHolderLess<T>() );
		_Clean = true;
	}
	while (!_Tasks.empty())
	{
		const CTaskHolder<T>& task = _Tasks.front();
		if (  task.getTime()  <= now)
		{
			task();
			_Tasks.pop_front();
		}
		else
		{
			break;
		}
	}
}

template <typename T>
CTaskList<T>::~CTaskList()
{
//	_Tasks.clear();
}


#endif // NL_TASK_LIST_H

