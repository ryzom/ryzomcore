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

//-----------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"
#include "nel/net/service.h"

// game share
#include "game_share/ryzom_version.h"
#include "game_share/tick_event_handler.h"
#include "game_share/singleton_registry.h"
#include "server_share/handy_commands.h"
#include "game_share/utils.h"

// local
#include "service_main.h"
#include "patchman_tester.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//-----------------------------------------------------------------------------
// methods CServiceClass 
//-----------------------------------------------------------------------------

void CServiceClass::init()
{
	setVersion (RYZOM_VERSION);
	_ExitRequested= false;

//	// run self tests before we begin
//	nlinfo("*************************************************************************");
//	nlinfo("* Starting init-time self tests");
//	nlinfo("*************************************************************************");
//	uint32 errorCount= CSelfTestRegistry::getInstance().runTests(*WarningLog,*InfoLog,*DebugLog);
//	nlinfo("*************************************************************************");
//	nlinfo("* Finished init-time self tests: %d errors",errorCount);
//	nlinfo("*************************************************************************");
//	BOMB_IF(errorCount!= 0,"Self test failed",_ExitRequested=true)
}

bool CServiceClass::update()
{
	// let the test system do its thing...
	PATCHMAN::CPatchmanTester::getInstance().trigger("update");

	return !_ExitRequested;
}

void CServiceClass::release()
{
}


//-----------------------------------------------------------------------------
// methods CTaskScheduler
//-----------------------------------------------------------------------------

CTaskScheduler& CTaskScheduler::getInstance()
{
	static CTaskScheduler* instance= NULL;
	if (instance== NULL)
	{
		instance= new CTaskScheduler;
	}
	return *instance;
}

void CTaskScheduler::sceduleTask(IScheduledTask* task,NLMISC::TTime delay)
{
	// add a new entry to the end of our vector and take a refference to it
	STask& theTask= vectAppend(_Tasks);

	// store away the task ptr nad execution time in the new entry we just created
	theTask.TaskPtr= task;
	theTask.ExecutionTime= NLMISC::CTime::getLocalTime()+ delay;
}

void CTaskScheduler::update()
{
	// lookup the current time
	NLMISC::TTime timeNow= NLMISC::CTime::getLocalTime();

	// check to see if we've broken our max sheduled tasks record...
	if (_MaxTasks < _Tasks.size())
	{
		_MaxTasks = (uint32)_Tasks.size();
		nldebug("New scheduled task record: %u",_MaxTasks);
	}

	// iterate over all scheduled tasks (we go backwards to simplify deletion of executed tasks as we go)
	for (uint32 i=(uint32)_Tasks.size();i--;)
	{
		// get a refference to the next task
		STask& theTask= _Tasks[i];

		// see if the task's time is up
		if (theTask.ExecutionTime <= timeNow)
		{
			// time to execute...
			theTask.TaskPtr->execute();

			// replace the task with the back task in the scheduler and pop the back task
			theTask= _Tasks.back();
			_Tasks.pop_back();
		}
	}
}

CTaskScheduler::CTaskScheduler()
{
	_MaxTasks= 0;
}


//-----------------------------------------------------------------------------
// methods ISelfTestClass::
//-----------------------------------------------------------------------------

void ISelfTestClass::init(NLMISC::CLog& errorLog, NLMISC::CLog& progressLog, NLMISC::CLog& verboseLog)
{
	_ErrorLog	 = &errorLog;
	_ProgressLog = &progressLog;
	_VerboseLog	 = &verboseLog;
	_ErrorCount	 = 0;
}

void ISelfTestClass::errorLog(const char* fmt,...) const
{
	const uint32 stringLen= 1024;
	char cstring[stringLen];
	va_list args;
	va_start (args, fmt);
	int res = vsnprintf (cstring, stringLen-1, fmt, args);
	if (res == -1 || res == stringLen-1)
	{
		cstring[stringLen-1] = '\0';
	}
	va_end (args);

	_ErrorLog->displayNL("%s",cstring);
}

void ISelfTestClass::progressLog(const char* fmt,...) const
{
	const uint32 stringLen= 1024;
	char cstring[stringLen];
	va_list args;
	va_start (args, fmt);
	int res = vsnprintf (cstring, stringLen-1, fmt, args);
	if (res == -1 || res == stringLen-1)
	{
		cstring[stringLen-1] = '\0';
	}
	va_end (args);

	_ProgressLog->displayNL("%s",cstring);
}

void ISelfTestClass::verboseLog(const char* fmt,...) const
{
	const uint32 stringLen= 1024;
	char cstring[stringLen];
	va_list args;
	va_start (args, fmt);
	int res = vsnprintf (cstring, stringLen-1, fmt, args);
	if (res == -1 || res == stringLen-1)
	{
		cstring[stringLen-1] = '\0';
	}
	va_end (args);

	_VerboseLog->displayNL("%s",cstring);
}

void ISelfTestClass::addError() const
{
	++_ErrorCount;
}

uint32 ISelfTestClass::getErrorCount() const
{
	return _ErrorCount;
}


//-----------------------------------------------------------------------------
// methods CSelfTestRegistry
//-----------------------------------------------------------------------------

CSelfTestRegistry& CSelfTestRegistry::getInstance()
{
	static CSelfTestRegistry* selfTestRegistry= NULL;
	if (selfTestRegistry== NULL)
	{
		selfTestRegistry= new CSelfTestRegistry;
	}
	return *selfTestRegistry;
}

void CSelfTestRegistry::registerTestObject(ISelfTestClass* testObject,const NLMISC::CSString& description)
{
	_Tests[testObject]= description;
}

uint32 CSelfTestRegistry::runTests(NLMISC::CLog& errorLog, NLMISC::CLog& progressLog, NLMISC::CLog& verboseLog)
{
	uint32 result=0;
	for (TTests::iterator it= _Tests.begin(); it!=_Tests.end(); ++it)
	{
		progressLog.displayNL("SELF_TEST: Test '%s'",(*it).second.c_str());

		// init the test and setup logs
		#ifdef NL_DEBUG
			(*it).first->init(*WarningLog,*InfoLog,*DebugLog);
		#else
			// mute the verbose log in non-debug mode
			NLMISC::CLog dummyLog;
			(*it).first->init(*WarningLog,*InfoLog,dummyLog);
		#endif

		// run the test
		(*it).first->runTest();
		uint32 errors= (*it).first->getErrorCount();
		result+= errors;
		if (errors!=0)
		{
			errorLog.displayNL("SELF_TEST: Tests '%s' failed: %d errors",(*it).second.c_str(),errors);
		}
	}
	return result;
}

CSelfTestRegistry::CSelfTestRegistry()
{
}


//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------

NLNET_SERVICE_MAIN( CServiceClass, "PATCHMAN", "patchman_service", 0, EmptyCallbackArray, "", "" );

