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
#include <nel/misc/dynloadlib.h>
#include <nel/net/module_manager.h>

// Project includes
#include "info_flags.h"
#include "pipeline_workspace.h"
#include "pipeline_project.h"
#include "database_status.h"
#include "pipeline_interface_impl.h"
#include "pipeline_process_impl.h"
#include "../plugin_library/process_info.h"

// using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;

namespace PIPELINE {

std::string g_DatabaseDirectory;
std::string g_PipelineDirectory;
bool g_IsExiting = false;
bool g_IsMaster = false;

NLGEORGES::UFormLoader *g_FormLoader = NULL;
CPipelineWorkspace *g_PipelineWorkspace = NULL;
CDatabaseStatus *g_DatabaseStatus = NULL;

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

extern void module_pipeline_master_forceLink();
extern void module_pipeline_slave_forceLink();

// ******************************************************************

namespace {

#define PIPELINE_LONG_SERVICE_NAME "pipeline_service"
#define PIPELINE_SHORT_SERVICE_NAME "PLS"

#ifdef NL_DEBUG
#define PIPELINE_SERVICE_DIRECTORY "R:\\build\\dev\\bin\\Release"
#else
#define PIPELINE_SERVICE_DIRECTORY ""
#endif

/// Enum
enum EState
{
	STATE_IDLE, 
	STATE_RELOAD_SHEETS, 
	STATE_DATABASE_STATUS, 
	STATE_RUNNABLE_TASK, 
	STATE_BUSY_TEST, 
	STATE_DIRECT_CODE, 
};

/// Data
CInfoFlags *s_InfoFlags = NULL;
CTaskManager *s_TaskManager = NULL;
CPipelineInterfaceImpl *s_PipelineInterfaceImpl = NULL;
CPipelineProcessImpl *s_PipelineProcessImpl = NULL;

EState s_State = STATE_IDLE;
std::string s_StateRunnableTaskName = "";
CMutex s_StateMutex;

std::vector<NLMISC::CLibrary *> s_LoadedLibraries;

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

	nlassert(s_State != STATE_IDLE);
	
	s_TaskManager->addTask(task);
	
	return true;
}

} /* anonymous namespace */

bool isServiceStateIdle()
{
	return (s_State == STATE_IDLE);
}

bool tryRunnableTask(const std::string &stateName, IRunnable *task)
{
	// copy paste from above.

	bool result = false;
	s_StateMutex.enter();
	result = (s_State == STATE_IDLE);
	if (result)
	{
		s_State = STATE_RUNNABLE_TASK;
		s_StateRunnableTaskName = stateName;
	}
	s_StateMutex.leave();
	if (!result) return false;

	nlassert(s_State == STATE_RUNNABLE_TASK);
	
	s_TaskManager->addTask(task);
	
	return true;
}

namespace {

void endedRunnableTask(EState state)
{
	nlassert(s_State == state);

	s_StateMutex.enter();
	s_State = STATE_IDLE;
	s_StateMutex.leave();
}

} /* anonymous namespace */

void endedRunnableTask()
{
	endedRunnableTask(STATE_RUNNABLE_TASK);
}

bool tryDirectTask(const std::string &stateName)
{
	bool result = false;
	s_StateMutex.enter();
	result = (s_State == STATE_IDLE);
	if (result)
	{
		s_State = STATE_DIRECT_CODE;
		s_StateRunnableTaskName = stateName;
	}
	s_StateMutex.leave();
	if (!result) return false;

	nlassert(s_State == STATE_DIRECT_CODE);
	
	return true;
}

void endedDirectTask()
{
	endedRunnableTask(STATE_DIRECT_CODE);
}

// ******************************************************************

namespace {

void initSheets()
{
	std::string leveldesignDfnDirectory = IService::getInstance()->ConfigFile.getVar("LeveldesignDfnDirectory").asString();
	std::string leveldesignPipelineDirectory = IService::getInstance()->ConfigFile.getVar("LeveldesignPipelineDirectory").asString();

	nlinfo("Adding 'LeveldesignDfnDirectory' to search path (%s)", leveldesignDfnDirectory.c_str());
	CPath::addSearchPath(leveldesignDfnDirectory, true, false);

	nlinfo("Adding 'LeveldesignPipelineDirectory' to search path (%s)", leveldesignPipelineDirectory.c_str());
	CPath::addSearchPath(leveldesignPipelineDirectory, true, false);
	
	g_FormLoader = UFormLoader::createLoader();
	
	g_PipelineWorkspace = new CPipelineWorkspace(g_FormLoader, IService::getInstance()->ConfigFile.getVar("WorkspaceSheet").asString());
}

void releaseSheets()
{
	delete g_PipelineWorkspace;
	g_PipelineWorkspace = NULL;

	UFormLoader::releaseLoader(g_FormLoader);
	g_FormLoader = NULL;

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
		
		endedRunnableTask(STATE_RELOAD_SHEETS);
	}
};
CReloadSheets s_ReloadSheets;

} /* anonymous namespace */

bool reloadSheets()
{
	return tryStateTask(STATE_RELOAD_SHEETS, &s_ReloadSheets);
}

namespace {

// ******************************************************************

class CUpdateDatabaseStatus : public IRunnable
{
	virtual void getName(std::string &result) const 
	{ result = "CUpdateDatabaseStatus"; }
	
	void databaseStatusUpdated()
	{
		endedRunnableTask(STATE_DATABASE_STATUS);
	}

	virtual void run()
	{
		g_DatabaseStatus->updateDatabaseStatus(CCallback<void>(this, &CUpdateDatabaseStatus::databaseStatusUpdated));
	}
};
CUpdateDatabaseStatus s_UpdateDatabaseStatus;

bool updateDatabaseStatus()
{
	return tryStateTask(STATE_DATABASE_STATUS, &s_UpdateDatabaseStatus);
}

// ******************************************************************

// ******************************************************************

class CBusyTestStatus : public IRunnable
{
	virtual void getName(std::string &result) const 
	{ result = "CBusyTestStatus"; }

	virtual void run()
	{
		nlSleep(20000);
		endedRunnableTask(STATE_BUSY_TEST);
	}
};
CBusyTestStatus s_BusyTestStatus;

bool busyTestStatus()
{
	return tryStateTask(STATE_BUSY_TEST, &s_BusyTestStatus);
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
		module_pipeline_master_forceLink();
		module_pipeline_slave_forceLink();
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
		s_InfoFlags = new CInfoFlags();

		s_InfoFlags->addFlag("INIT");

		g_DatabaseDirectory = CPath::standardizePath(ConfigFile.getVar("DatabaseDirectory").asString(), true);
		if (!CFile::isDirectory(g_DatabaseDirectory)) nlwarning("'DatabaseDirectory' does not exist! (%s)", g_DatabaseDirectory.c_str());
		ConfigFile.getVar("DatabaseDirectory").setAsString(g_DatabaseDirectory);
		g_PipelineDirectory = CPath::standardizePath(ConfigFile.getVar("PipelineDirectory").asString(), true);
		if (!CFile::isDirectory(g_PipelineDirectory)) nlwarning("'PipelineDirectory' does not exist! (%s)", g_PipelineDirectory.c_str());
		ConfigFile.getVar("PipelineDirectory").setAsString(g_PipelineDirectory);

		s_TaskManager = new CTaskManager();
		
		initSheets();

		g_DatabaseStatus = new CDatabaseStatus();

		s_PipelineInterfaceImpl = new CPipelineInterfaceImpl();
		s_PipelineProcessImpl = new CPipelineProcessImpl(NULL); // Create a singleton impl for global usage without running project for test purposes.

		// Load libraries
		const CConfigFile::CVar &usedPlugins = ConfigFile.getVar("UsedPlugins");
		s_LoadedLibraries.reserve(usedPlugins.size());
		for (uint i = 0; i < usedPlugins.size(); ++i)
		{
			CLibrary *library = new CLibrary();
			if (library->loadLibrary(usedPlugins.asString(i), true, true, true))
			{
				s_LoadedLibraries.push_back(library);
			}
			else delete library;
		}

		s_InfoFlags->removeFlag("INIT");
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

		s_InfoFlags->addFlag("RELEASE");

		NLNET::IModuleManager::releaseInstance();

		while (NLMISC::CAsyncFileManager::getInstance().getNumWaitingTasks() > 0)
		{
			nlSleep(10);
		}
		NLMISC::CAsyncFileManager::terminate();

		for (std::vector<NLMISC::CLibrary *>::iterator it = s_LoadedLibraries.begin(), end = s_LoadedLibraries.end(); it != end; ++it)
		{
			(*it)->freeLibrary();
			delete (*it);
		}
		s_LoadedLibraries.clear();

		CClassRegistry::release();

		delete s_PipelineProcessImpl;
		s_PipelineProcessImpl = NULL;

		delete s_PipelineInterfaceImpl;
		s_PipelineInterfaceImpl = NULL;

		delete g_DatabaseStatus;
		g_DatabaseStatus = NULL;

		releaseSheets();

		delete s_TaskManager;
		s_TaskManager = NULL;

		s_InfoFlags->removeFlag("RELEASE");

		delete s_InfoFlags;
		s_InfoFlags = NULL;
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
			case PIPELINE::STATE_RUNNABLE_TASK:
				*pointer = "RT: " + PIPELINE::s_StateRunnableTaskName;
				break;
			case PIPELINE::STATE_DIRECT_CODE:
				*pointer = "DC: " + PIPELINE::s_StateRunnableTaskName;
				break;
			case PIPELINE::STATE_BUSY_TEST:
				*pointer = "BUSY_TEST";
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
		log.displayNL("I'm afraid I cannot do this, my friend.");
		return false;
	}
	return true;
}

NLMISC_COMMAND(updateDatabaseStatus, "Updates the entire database status. This also happens on the fly during build.", "")
{
	if(args.size() != 0) return false;
	if (!PIPELINE::updateDatabaseStatus())
	{
		log.displayNL("I'm afraid I cannot do this, my friend.");
		return false;
	}
	return true;
}

NLMISC_COMMAND(busyTestState, "Keeps the service busy for twenty seconds.", "")
{
	if(args.size() != 0) return false;
	if (!PIPELINE::busyTestStatus())
	{
		log.displayNL("Already busy");
		return false;
	}
	return true;
}

NLMISC_COMMAND(dumpTaskManager, "Dumps the task manager.", "")
{
	if(args.size() != 0) return false;
	std::vector<std::string> output;
	PIPELINE::s_TaskManager->dump(output);
	for (std::vector<std::string>::iterator it = output.begin(), end = output.end(); it != end; ++it)
		log.displayNL("DUMP: %s", (*it).c_str());
	return true;
}

NLMISC_COMMAND(dumpAsyncFileManager, "Dumps the async file manager.", "")
{
	if(args.size() != 0) return false;
	std::vector<std::string> output;
	NLMISC::CAsyncFileManager::getInstance().dump(output);
	for (std::vector<std::string>::iterator it = output.begin(), end = output.end(); it != end; ++it)
		log.displayNL("DUMP: %s", (*it).c_str());
	return true;
}

NLMISC_COMMAND(listProcessPlugins, "List process plugins.", "<processName>")
{
	if(args.size() != 1) return false;
	std::vector<PIPELINE::CProcessPluginInfo> result;
	PIPELINE::g_PipelineWorkspace->getProcessPlugins(result, args[0]);
	for (std::vector<PIPELINE::CProcessPluginInfo>::iterator it = result.begin(), end = result.end(); it != end; ++it)
	{
		std::string statusHandler;
		switch (it->HandlerType)
		{
		case PIPELINE::PLUGIN_REGISTERED_CLASS:
			if (std::find(PIPELINE::s_PipelineInterfaceImpl->RegisteredClasses.begin(), PIPELINE::s_PipelineInterfaceImpl->RegisteredClasses.end(), it->Handler) != PIPELINE::s_PipelineInterfaceImpl->RegisteredClasses.end())
				statusHandler = "AVAILABLE";
			else
				statusHandler = "NOT FOUND";
			break;
		default:
			statusHandler = "UNKNOWN";
			break;
		}
		std::string statusInfo;
		switch (it->InfoType)
		{
		case PIPELINE::PLUGIN_REGISTERED_CLASS:if (std::find(PIPELINE::s_PipelineInterfaceImpl->RegisteredClasses.begin(), PIPELINE::s_PipelineInterfaceImpl->RegisteredClasses.end(), it->Info) != PIPELINE::s_PipelineInterfaceImpl->RegisteredClasses.end())
				statusInfo = "AVAILABLE";
			else
				statusInfo = "NOT FOUND";
			break;
		default:
			statusInfo = "UNKNOWN";
			break;
		}
		log.displayNL("LISTPP '%s': '%s' (%s), '%s' (%s)", args[0].c_str(), it->Handler.c_str(), statusHandler.c_str(), it->Info.c_str(), statusInfo.c_str());
	}
	return true;
}

NLMISC_COMMAND(showProjectMacro, "Show project macro.", "<projectName> <macroName>")
{
	if(args.size() != 2) return false;
	PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(args[0]);
	if (project)
	{
		std::string result;
		project->getMacro(result, args[1]);
		log.displayNL("MACRO: '%s', '%s': '%s'", args[0].c_str(), args[1].c_str(), result.c_str());
		return true;
	}
	else
	{
		log.displayNL("MACRO: Project '%s' does not exist", args[0].c_str());
		return false;
	}
}

NLMISC_COMMAND(showProjectValue, "Show project value.", "<projectName> <valueName>")
{
	if(args.size() != 2) return false;
	PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(args[0]);
	if (project)
	{
		std::string result;
		project->getValue(result, args[1]);
		log.displayNL("VALUE: '%s', '%s': '%s'", args[0].c_str(), args[1].c_str(), result.c_str());
		return true;
	}
	else
	{
		log.displayNL("VALUE: Project '%s' does not exist", args[0].c_str());
		return false;
	}
}

NLMISC_COMMAND(showDependencies, "Show dependencies.", "<projectName> <processName>")
{
	if(args.size() != 2) return false;
	std::vector<PIPELINE::CProcessPluginInfo> plugins;
	PIPELINE::g_PipelineWorkspace->getProcessPlugins(plugins, args[1]);
	PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(args[0]);
	if (project)
	{
		PIPELINE::IPipelineProcess *pipelineProcess = new PIPELINE::CPipelineProcessImpl(project);
		for (std::vector<PIPELINE::CProcessPluginInfo>::iterator plugin_it = plugins.begin(), plugin_end = plugins.end(); plugin_it != plugin_end; ++plugin_it)
		{
			switch (plugin_it->InfoType)
			{
			case PIPELINE::PLUGIN_REGISTERED_CLASS:
				{
					PIPELINE::IProcessInfo *processInfo = static_cast<PIPELINE::IProcessInfo *>(NLMISC::CClassRegistry::create(plugin_it->Info));
					log.displayNL("PROCESS_INFO: %s", plugin_it->Info.c_str());
					processInfo->setPipelineProcess(pipelineProcess);
					std::vector<std::string> result;
					processInfo->getDependentDirectories(result);
					for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
						log.displayNL("DIRECTORY: %s", it->c_str());
					result.clear();
					processInfo->getDependentFiles(result);
					for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
						log.displayNL("FILE: %s", it->c_str());
				}
				break;
			default:
				nlwarning("Not implemented.");
				break;
			}
		}
	}
	else
	{
		log.displayNL("Project '%s' does not exist", args[0].c_str());
		return false;
	}
	return true;
}

NLNET_SERVICE_MAIN(PIPELINE::CPipelineService, PIPELINE_SHORT_SERVICE_NAME, PIPELINE_LONG_SERVICE_NAME, 0, PIPELINE::s_ShardCallbacks, PIPELINE_SERVICE_DIRECTORY, PIPELINE_SERVICE_DIRECTORY)

/* end of file */
