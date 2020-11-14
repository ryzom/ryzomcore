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



#include "stdpch.h"

#include "nel/misc/debug.h"
#include "nel/misc/hierarchical_timer.h"

#include "module_manager.h"

using namespace NLMISC;
using namespace std;

/*
 * Static variables
 */

uint									CModuleManager::_MaxModules = 0;

NLMISC::CMutex							*CModuleManager::_ModMutexes = NULL;

vector<CModuleManager*>					CModuleManager::_RegisteredManagers;


// Constructor
CModuleManager::CModuleManager(const char *name, bool independent)
{
	_Independent = independent;
	_Thread = NULL;
	_StopThread = false;
	_ThreadStopped = false;

	_StackName = (name != NULL) ? string(name) : "<unnamed>";

	_Id = (uint)_RegisteredManagers.size();
	_Cycle = 0;
	_CompleteCycle = false;
	_RegisteredManagers.push_back(this);
}

// Destructor
CModuleManager::~CModuleManager()
{
	if (_Thread != NULL)
	{
		nlwarning("FEMMAN: [%s] Execution is not finished yet. Brutal thread killing", _StackName.c_str());
		_Thread->terminate();
		delete _Thread;
	}
}



//

void	CModuleManager::init(uint maxModules)
{
	nlassert(_MaxModules == 0);
	nlassert(_ModMutexes == NULL);

	_MaxModules = maxModules;
	_ModMutexes = new CMutex[maxModules];
}

//

void	CModuleManager::release()
{
	_MaxModules = 0;
	delete [] _ModMutexes;
	_ModMutexes = NULL;
}

//

void	CModuleManager::startAll()
{
	// first reset cycle
	resetCycle();

	uint	i;

	// and start all managers at once
	for (i=0; i<_RegisteredManagers.size(); ++i)
		if (_RegisteredManagers[i]->_Independent)
			_RegisteredManagers[i]->start();
}

void	CModuleManager::stopAll(TTime timeout)
{
	uint	i;

	// send soft stop
	for (i=0; i<_RegisteredManagers.size(); ++i)
		_RegisteredManagers[i]->stop(false, 0);

	// wait for all managers to stop or timeout
	TTime	before = CTime::getLocalTime();
	while (!allStopped() && CTime::getLocalTime()-before < timeout)
		nlSleep(10);

	// and hard stop
	for (i=0; i<_RegisteredManagers.size(); ++i)
		_RegisteredManagers[i]->stop(true, 0);
}

void	CModuleManager::resetCycle()
{
	uint	i;

	// reset all managers cycle counter
	for (i=0; i<_RegisteredManagers.size(); ++i)
		_RegisteredManagers[i]->_Cycle = 0;
}

bool	CModuleManager::allReady()
{
	uint	i;

	// checks if all managers are at the same cycle
	for (i=0; i<_RegisteredManagers.size()-1; ++i)
		if (_RegisteredManagers[i]->_Cycle != _RegisteredManagers[i+1]->_Cycle)
			return false;

	return true;
}

bool	CModuleManager::allComplete()
{
	uint	i;

	// checks if all managers have set the stop flag
	for (i=0; i<_RegisteredManagers.size()-1; ++i)
		if (_RegisteredManagers[i]->_CompleteCycle != _RegisteredManagers[i+1]->_CompleteCycle)
			return false;

	return true;
}

bool	CModuleManager::allStopped()
{
	uint	i;

	// checks if all managers have set the stop flag
	for (i=0; i<_RegisteredManagers.size(); ++i)
		if (!_RegisteredManagers[i]->_ThreadStopped)
			return false;

	return true;
}

void	CModuleManager::resetManagers()
{
	// clear all registered managers
	_RegisteredManagers.clear();
}



//

void	CModuleManager::addModule(uint id, TModuleExecCallback cb)
{
	nlassert(id < _MaxModules);
	nlassert(cb != NULL);
	nldebug("FEMMAN: [%s] Added module %d (Cb=%p) to stack", _StackName.c_str(), id, (void *)cb);

	_ExecutionStack.push_back(CExecutionItem());

	_ExecutionStack.back().Type = Module;
	_ExecutionStack.back().Id = id;
	_ExecutionStack.back().Cb = cb;

	_ExecutedModules.push_back(id);
}

//

void	CModuleManager::addWait(uint id)
{
	nlassert(id < _MaxModules);
	nldebug("FEMMAN: [%s] Added wait %d to stack", _StackName.c_str(), id);

	_ExecutionStack.push_back(CExecutionItem());

	_ExecutionStack.back().Type = Wait;
	_ExecutionStack.back().Id = id;
	_ExecutionStack.back().Cb = NULL;
}

//

void	CModuleManager::start()
{
	_Thread = IThread::create(this);

	_StopThread = false;
	_ThreadStopped = false;

	_Thread->start();
	//nlinfo("FEMMAN: [%s] Start", _StackName.c_str());
}

//

void	CModuleManager::runOnce()
{
	_StopThread = false;
	_ThreadStopped = false;

	//nlinfo("FEMMAN: [%s] Running one time", _StackName.c_str());

	// step cycle
	stepCycle();

	// wait for all managers to sync on the same cycle before entering mutexes
	waitAllReady();

	// lock mutexes
	enterMutexes();

	completeCycle();

	// and wait for all managers to finish entering mutexes
	waitAllComplete();

	//H_BEFORE(MMExecuteStack);
	executeStack();
	//H_AFTER(MMExecuteStack);

	_ThreadStopped = true;
}

//

void	CModuleManager::stop(bool blockingMode, TTime timeout)
{
	// non independent modules (called by main thread) are always stopped at this point, no need to force stop
	if (!_Independent)
	{
		_ThreadStopped = true;
		return;
	}


	if (!blockingMode)
	{
		// if soft stop, just send stop message and leave
		nlinfo("FEMMAN: [%s] soft stop", _StackName.c_str());
		_StopThread = true;
		return;
	}

	if (!_StopThread)
	{
		// if not yet called stop, send message stop
		nlinfo("FEMMAN: [%s] hard stop", _StackName.c_str());
		_StopThread = true;

		// wait for stop or timeout
		TTime	before = CTime::getLocalTime();
		while (!_ThreadStopped && CTime::getLocalTime()-before < timeout)
			nlSleep(10);
	}

	if ( _Thread )
	{
		// if timeout, terminate thread
		if (!_ThreadStopped)
		{
			nlwarning("FEMMAN: [%s] Can't stop. Brutal thread killing", _StackName.c_str());
			_Thread->terminate();
		}

		delete _Thread;
		_Thread = NULL;
	}
}


//

void	CModuleManager::run()
{
	nldebug("FEMMAN: [%s] attached thread loop start", _StackName.c_str());

	while (true)
	{
		// step cycle
		stepCycle();

		// wait for all managers to sync on the same cycle before entering mutexes
		waitAllReady();

		// lock mutexes
		enterMutexes();

		completeCycle();

		// and wait for all managers to finish entering mutexes
		waitAllComplete();

		//H_BEFORE(MMExecuteStack);
		executeStack();
		//H_AFTER(MMExecuteStack);

		// if stop sent, just leave
		if (_StopThread)
			break;
	}

	nldebug("FEMMAN: [%s] attached thread loop end", _StackName.c_str());

	_ThreadStopped = true;
}

//

void	CModuleManager::executeStack()
{
	// for each item,
	//   if a module, 
	//      calls associated callback
	//      leaves mutex
	//   else
	//      enters mutex
	//      leaves mutex

	//nldebug("FEMMAN: [%s] execute stack", _StackName.c_str());

	uint	i;

	for (i=0; i<_ExecutionStack.size(); ++i)
	{
		CExecutionItem	&item = _ExecutionStack[i];

		if (item.Type == Module)
		{
			//nldebug("FEMMAN: [%s] execute module %d at %p", _StackName.c_str(), item.Id, item.Cb);
			//H_BEFORE(MMCall);
			item.Cb();
			//H_AFTER(MMCall);
			_ModMutexes[item.Id].leave();
		}
		else if (item.Type == Wait)
		{
			//nldebug("FEMMAN: [%s] wait for module %d to finish", _StackName.c_str(), item.Id);
			//H_BEFORE(MMWait);
			//TTime t = CTime::getLocalTime();
			_ModMutexes[item.Id].enter();
			//nlinfo( "Waited %u ms", (uint32)(CTime::getLocalTime()-t) );
			_ModMutexes[item.Id].leave();
			//H_AFTER(MMWait);
		}
		else
		{
			nlwarning("FEMMAN: Unexpected ExecutionItem type (%d) at item %d of the execution stack %s", item.Type, i, _StackName.c_str());
			uint	j;
			for (j=0; j<_ExecutionStack.size(); ++j)
				nlwarning("FEMMAN: > %d [%s] Id=%d Cb=%p", j, (item.Type == Module) ? "MOD" : (item.Type == Wait) ? "WAIT" : "ERR", item.Id, (void *)item.Cb);
			nlerror("FEMMAN: Error in execution stack %s", _StackName.c_str());
		}
	}

	//nldebug("FEMMAN: [%s] stack executed", _StackName.c_str());
}

void	CModuleManager::enterMutexes()
{
	// enters all controlled mutexes

	//nldebug("FEMMAN: [%s] Entering all controlled mutexes", _StackName.c_str());

	uint	i;

	for (i=0; i<_ExecutedModules.size(); ++i)
	{
		//nldebug("FEMMAN: [%s] Entering mutex %d", _StackName.c_str(), _ExecutedModules[i]);
		//TTime t = CTime::getLocalTime();
		_ModMutexes[_ExecutedModules[i]].enter();
		//nlinfo( "Waited %u ms", (uint32)(CTime::getLocalTime()-t) );
	}
	//nldebug("FEMMAN: [%s] All controlled mutexes successfully entered", _StackName.c_str());
}


