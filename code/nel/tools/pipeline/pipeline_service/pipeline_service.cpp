/**
 * \file pipeline_service.cpp
 * \brief CPipelineService
 * \date 2012-02-18 17:25GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineService
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
#include "pipeline_service.h"

// STL includes
#include <stdio.h>
#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/bit_mem_stream.h>
#include <nel/misc/sheet_id.h>
#include <nel/net/service.h>
#include <nel/georges/u_form_loader.h>
#include <nel/misc/mutex.h>
#include <nel/misc/task_manager.h>
#include <nel/misc/async_file_manager.h>
#include <nel/misc/algo.h>

// Project includes
#include "pipeline_workspace.h"
#include "database_status.h"

// using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;

namespace PIPELINE {

bool g_IsMaster = false;
std::string g_DatabaseDirectory;
std::string g_PipelineDirectory;
bool g_IsExiting = false;

std::string unMacroPath(const std::string &path)
{
	std::string result = path;
	strFindReplace(result, PIPELINE_MACRO_DATABASE_DIRECTORY, g_DatabaseDirectory);
	strFindReplace(result, PIPELINE_MACRO_PIPELINE_DIRECTORY, g_PipelineDirectory);
	return CPath::standardizePath(result, false);
}

std::string macroPath(const std::string &path)
{
	std::string result = CPath::standardizePath(path, false);
	strFindReplace(result, g_DatabaseDirectory, PIPELINE_MACRO_DATABASE_DIRECTORY "/");
	strFindReplace(result, g_PipelineDirectory, PIPELINE_MACRO_PIPELINE_DIRECTORY "/");
	return result;
}

// ******************************************************************

namespace {

#define PIPELINE_LONG_SERVICE_NAME "pipeline_service"
#ifdef PIPELINE_MASTER
#define PIPELINE_SHORT_SERVICE_NAME "PLSM"
#else
#define PIPELINE_SHORT_SERVICE_NAME "PLSS"
#endif

/// Enum
enum EState
{
	STATE_IDLE, 
	STATE_RELOAD_SHEETS, 
	STATE_DATABASE_STATUS, 
};

/// Data
UFormLoader *s_FormLoader = NULL;
CPipelineWorkspace *s_PipelineWorkspace = NULL;
CTaskManager *s_TaskManager = NULL;
CDatabaseStatus *s_DatabaseStatus = NULL;

EState s_State = STATE_IDLE;
CMutex s_StateMutex;

// ******************************************************************

/// Service wants to broadcast all users trough pipeline
void cbNull(CMessage & /* msgin */, const std::string & /* serviceName */, TServiceId /* sid */) // pipeline_server
{
	// null
}

/// Callbacks from shard
TUnifiedCallbackItem s_ShardCallbacks[] = // pipeline_server
{
	{ "N", cbNull }, 
};

bool tryStateTask(EState state, IRunnable *task)
{
	bool result = false;
	s_StateMutex.enter();
	result = (s_State == STATE_IDLE);
	if (result)
	{
		s_State = state;
	}
	s_StateMutex.leave();
	if (!result) return false;
	
	s_TaskManager->addTask(task);
	
	return true;
}

// ******************************************************************

void initSheets()
{
	std::string leveldesignDfnDirectory = IService::getInstance()->ConfigFile.getVar("LeveldesignDfnDirectory").asString();
	std::string leveldesignPipelineDirectory = IService::getInstance()->ConfigFile.getVar("LeveldesignPipelineDirectory").asString();

	nlinfo("Adding 'LeveldesignDfnDirectory' to search path (%s)", leveldesignDfnDirectory.c_str());
	CPath::addSearchPath(leveldesignDfnDirectory, true, false);

	nlinfo("Adding 'LeveldesignPipelineDirectory' to search path (%s)", leveldesignPipelineDirectory.c_str());
	CPath::addSearchPath(leveldesignPipelineDirectory, true, false);
	
	s_FormLoader = UFormLoader::createLoader();
	
	s_PipelineWorkspace = new CPipelineWorkspace(s_FormLoader, IService::getInstance()->ConfigFile.getVar("WorkspaceSheet").asString());
}

void releaseSheets()
{
	delete s_PipelineWorkspace;
	s_PipelineWorkspace = NULL;

	UFormLoader::releaseLoader(s_FormLoader);
	s_FormLoader = NULL;

	CPath::releaseInstance();
}

class CReloadSheets : public IRunnable
{
	virtual void getName(std::string &result) const 
	{ result = "CReloadSheets"; }
	
	virtual void run()
	{
		releaseSheets();
		initSheets();
		
		s_StateMutex.enter();
		s_State = STATE_IDLE;
		s_StateMutex.leave();
	}
};
CReloadSheets s_ReloadSheets;

bool reloadSheets()
{
	return tryStateTask(STATE_RELOAD_SHEETS, &s_ReloadSheets);
}

// ******************************************************************

class CUpdateDatabaseStatus : public IRunnable
{
	virtual void getName(std::string &result) const 
	{ result = "CUpdateDatabaseStatus"; }
	
	void databaseStatusUpdated()
	{
		s_StateMutex.enter();
		s_State = STATE_IDLE;
		s_StateMutex.leave();
	}

	virtual void run()
	{
		s_DatabaseStatus->updateDatabaseStatus(CCallback<void>(this, &CUpdateDatabaseStatus::databaseStatusUpdated));
	}
};
CUpdateDatabaseStatus s_UpdateDatabaseStatus;

bool updateDatabaseStatus()
{
	return tryStateTask(STATE_DATABASE_STATUS, &s_UpdateDatabaseStatus);
}

// ******************************************************************

/**
 * \brief CPipelineService
 * \date 2012-02-18 17:25GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineService
 */
class CPipelineService : public IService
{
private:

public:
	CPipelineService()
	{
		
	}
	
	virtual ~CPipelineService()
	{
		
	}
	
	/** Called before the displayer is created, no displayer or network connection are built.
	    Use this callback to check some args and perform some command line based stuff */
	virtual void commandStart() 
	{
		// setup the randomizer properly
		{
			// get a good seed value from cpu dependent ticks or just milliseconds local
			uint32 s = (uint32)(CTime::getPerformanceTime() & 0xFFFFFFFF);
			if (!s) s = (uint32)(CTime::getLocalTime() & 0xFFFFFFFF);
			// seed the randomizer
			srand(s);
		}
	}
	
	/// Initializes the service (must be called before the first call to update())
	virtual void init()
	{
		g_IsMaster = ConfigFile.getVar("IsMaster").asBool();
		g_DatabaseDirectory = CPath::standardizePath(ConfigFile.getVar("DatabaseDirectory").asString(), true);
		if (!CFile::isDirectory(g_DatabaseDirectory)) nlwarning("'DatabaseDirectory' does not exist! (%s)", g_DatabaseDirectory.c_str());
		g_PipelineDirectory = CPath::standardizePath(ConfigFile.getVar("PipelineDirectory").asString(), true);
		if (!CFile::isDirectory(g_PipelineDirectory)) nlwarning("'PipelineDirectory' does not exist! (%s)", g_PipelineDirectory.c_str());

		s_TaskManager = new CTaskManager();
		
		initSheets();

		s_DatabaseStatus = new CDatabaseStatus();
	}
	
	/// This function is called every "frame" (you must call init() before). It returns false if the service is stopped.
	virtual bool update()
	{
		return true;
	}
	
	/// Finalization. Release the service. For example, this function frees all allocations made in the init() function.
	virtual void release()
	{
		g_IsExiting = true;

		while (NLMISC::CAsyncFileManager::getInstance().getNumWaitingTasks() > 0)
		{
			nlSleep(10);
		}
		NLMISC::CAsyncFileManager::terminate();

		delete s_DatabaseStatus;
		s_DatabaseStatus = NULL;

		releaseSheets();

		delete s_TaskManager;
		s_TaskManager = NULL;
	}
	
}; /* class CPipelineService */

} /* anonymous namespace */

} /* namespace PIPELINE */

NLMISC_DYNVARIABLE(std::string, pipelineServiceState, "State of the pipeline service.")
{
	// we can only read the value
	if (get)
	{
		switch (PIPELINE::s_State)
		{
			case PIPELINE::STATE_IDLE:
				*pointer = "IDLE";
				break;
			case PIPELINE::STATE_RELOAD_SHEETS:
				*pointer = "RELOAD_SHEETS";
				break;
			case PIPELINE::STATE_DATABASE_STATUS:
				*pointer = "DATABASE_STATUS";
				break;
		}
	}
}

NLMISC_DYNVARIABLE(uint, asyncFileQueueCount, "Number of tasks remaining in the async file manager.")
{
	// we can only read the value
	if (get)
	{
		*pointer = NLMISC::CAsyncFileManager::getInstance().getNumWaitingTasks();
	}
}

NLMISC_COMMAND(reloadSheets, "Reload all sheets.", "")
{
	if(args.size() != 0) return false;
	if (!PIPELINE::reloadSheets())
	{
		nlinfo("I'm afraid I cannot do this, my friend.");
		return false;
	}
	return true;
}

NLMISC_COMMAND(updateDatabaseStatus, "Updates the entire database status. This also happens on the fly during build.", "")
{
	if(args.size() != 0) return false;
	if (!PIPELINE::updateDatabaseStatus())
	{
		nlinfo("I'm afraid I cannot do this, my friend.");
		return false;
	}
	return true;
}

NLNET_SERVICE_MAIN(PIPELINE::CPipelineService, PIPELINE_SHORT_SERVICE_NAME, PIPELINE_LONG_SERVICE_NAME, 0, PIPELINE::s_ShardCallbacks, "", "")

/* end of file */
