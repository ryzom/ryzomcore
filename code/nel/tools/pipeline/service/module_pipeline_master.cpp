/**
 * \file module_pipeline_master.cpp
 * \brief CModulePipelineMaster
 * \date 2012-03-03 16:26GMT
 * \author Jan Boon (Kaetemi)
 * CModulePipelineMaster
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
#include "module_pipeline_master_itf.h"

// STL includes
#include <map>
#include <boost/thread/mutex.hpp>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/net/service.h>
#include <nel/misc/task_manager.h>
#include <nel/misc/tool_logger.h>

// Project includes
#include "info_flags.h"
#include "module_pipeline_slave_itf.h"
#include "pipeline_service.h"
#include "database_status.h"
#include "build_task_queue.h"
#include "pipeline_workspace.h"
#include "../plugin_library/pipeline_process.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace PIPELINE {

// temporary flags
#define PIPELINE_INFO_MASTER_RELOAD_SHEETS "M_RELOAD_SHEETS"
#define PIPELINE_INFO_MASTER_UPDATE_DATABASE_FOR_SLAVE "M_UPD_DB_FOR_S"
#define PIPELINE_INFO_ABORTING "M_ABORTING"

// permanent flags
#define PIPELINE_INFO_CODE_ERROR_UNMACRO "#CODE_ERROR_UNMACRO"
#define PIPELINE_INFO_SLAVE_REJECTED "#M_SLAVE_REJECT"
#define PIPELINE_INFO_SLAVE_ABORTED "#M_SLAVE_ABORTED"
#define PIPELINE_INFO_SLAVE_CRASHED "#M_SLAVE_CRASH"
#define PIPELINE_INFO_SLAVE_NOT_READY "#M_SLAVE_NOT_R"
#define PIPELINE_INFO_SLAVE_CB_GONE "#M_SLAVE_CB_GONE"

/**
 * \brief CModulePipelineMaster
 * \date 2012-03-03 16:26GMT
 * \author Jan Boon (Kaetemi)
 * CModulePipelineMaster
 */
class CModulePipelineMaster :
	public CModulePipelineMasterSkel,
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >
{
	struct CSlave
	{
	public:
		CSlave(CModulePipelineMaster *master, IModuleProxy *moduleProxy)
			: Master(master),
			Proxy(moduleProxy),
			ActiveTaskId(0),
			SheetsOk(true),
			SaneBehaviour(3),
			BuildReadyState(0),
			TimeOutStamp(0) { }
		CModulePipelineMaster *Master;
		CModulePipelineSlaveProxy Proxy;
		std::vector<std::string> Vector;
		uint32 ActiveTaskId;
		bool SheetsOk;
		std::vector<uint32> PluginsAvailable;
		sint SaneBehaviour;
		uint BuildReadyState;
		uint32 TimeOutStamp;

		~CSlave()
		{
			if (!SheetsOk)
			{
				CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_MASTER_RELOAD_SHEETS);
			}
		}

		bool canAcceptTask()
		{
			return SheetsOk && (ActiveTaskId == 0) && SaneBehaviour > 0 && BuildReadyState == 2 && TimeOutStamp < NLMISC::CTime::getSecondsSince1970();
		}
	};

protected:
	typedef std::map<IModuleProxy *, CSlave *> TSlaveMap;
	TSlaveMap m_Slaves;
	mutable boost::mutex m_SlavesMutex;
	CBuildTaskQueue m_BuildTaskQueue;
	bool m_BuildWorking;

	bool m_AbortRequested;

	NLMISC::CTaskManager *m_TaskManager; // Manages tasks requested by a slave.
	NLMISC::CSynchronized<std::list<IRunnable *> > m_WaitingCallbacks;

	// A runnable that's executed at next update state used as a callback.
	// Override runnable as normal as the callback function.
	class CDelayedCallback : public IRunnable
	{
	public:
		CDelayedCallback(CModulePipelineMaster *master) : Master(master)
		{
			NLMISC::CSynchronized<std::list<IRunnable *> >::CAccessor waitingCallbacks(&Master->m_WaitingCallbacks);
			waitingCallbacks.value().push_back(this);
		}
		inline CCallback<void> getCallback() { return CCallback<void>(this, &CDelayedCallback::callback); }
		CModulePipelineMaster *Master;

	private:
		void callback()
		{
			{
				Master->addUpdateTask(this);
			}
			{
				NLMISC::CSynchronized<std::list<IRunnable *> >::CAccessor waitingCallbacks(&Master->m_WaitingCallbacks);
				waitingCallbacks.value().remove(this);
			}
		}
	};

	// Runnable tasks that are ran on the next update cycle.
	// The IRunnable instance is deleted by the update cycle.
	NLMISC::CSynchronized<std::deque<IRunnable *> > m_UpdateTasks;

	// build command
	bool m_BypassErrors;
	bool m_VerifyOnly;

public:
	CModulePipelineMaster() : m_BuildWorking(false), m_AbortRequested(false), m_TaskManager(NULL), m_WaitingCallbacks("WaitingCallbacks"), m_UpdateTasks("UpdateTasks"), m_MasterLog(NULL)
	{
		g_IsMaster = true;
		m_TaskManager = new NLMISC::CTaskManager();
	}

	virtual ~CModulePipelineMaster()
	{
		nldebug("START ~CModulePipelineMaster");
		if (m_BuildWorking)
			this->abort();

		g_IsMaster = false;

		nldebug("Wait for tasks on the master");
		while (m_TaskManager->getNumWaitingTasks() > 0)
			nlSleep(10);
		delete m_TaskManager;
		m_TaskManager = NULL;

		// Wait for waiting callbacks to be called
		for (; ; )
		{
			{
				NLMISC::CSynchronized<std::list<IRunnable *> >::CAccessor waitingCallbacks(&m_WaitingCallbacks);
				if (waitingCallbacks.value().size() == 0)
					break;
			}

			nlwarning("Waiting for callbacks on the master to be called...");
			nlSleep(1000);
		}

		// Ensure the remaining update tasks are handled
		handleUpdateTasks();

		m_SlavesMutex.lock();

		for (TSlaveMap::iterator it = m_Slaves.begin(), end = m_Slaves.end(); it != end; ++it)
			delete it->second;
		m_Slaves.clear();

		m_SlavesMutex.unlock();
		nldebug("END ~CModulePipelineMaster");
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	virtual bool initModule(const TParsedCommandLine &initInfo)
	{
		CModuleBase::initModule(initInfo);
		CModulePipelineMasterSkel::init(this);
		if (PIPELINE::tryDirectTask("MASTER_INIT_SHEETS"))
		{
			updateSheetsDatabaseStatus();
		}
		else
		{
			nlerror("Cannot initialize master, the service is busy. This should never happen");
		}
		return true;
	}

	static void dummyCallback() { }
	static void dummyFileStatusCallback(const std::string &/*filePath*/, const CFileStatus &/*fileStatus*/, bool /*success*/) { }

	void updateSheetsDatabaseStatus()
	{
		std::vector<std::string> sheetPaths;
		sheetPaths.push_back(NLNET::IService::getInstance()->ConfigFile.getVar("WorkspaceSheetDirectory").asString());
		g_DatabaseStatus->updateDatabaseStatus(dummyCallback, dummyFileStatusCallback, sheetPaths, true, true); // relatively fast so no need to thread
		PIPELINE::endedDirectTask();
	}
	
	void updateSheetsDatabaseStatus(const CCallback<void> &callback)
	{
		std::vector<std::string> sheetPaths;
		// sheetPaths.push_back(NLNET::IService::getInstance()->ConfigFile.getVar("WorkspaceDfnDirectory").asString()); // not really necessary to check the dfn's
		sheetPaths.push_back(NLNET::IService::getInstance()->ConfigFile.getVar("WorkspaceSheetDirectory").asString());
		g_DatabaseStatus->updateDatabaseStatus(callback, dummyFileStatusCallback, sheetPaths, false, true);
	}

	void cbMasterInitSheets()
	{
		PIPELINE::endedDirectTask();
	}

	//std::vector<IModuleProxy *> m_ModuleUpDelay;

	virtual void onModuleUp(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineSlave")
		{
			nlinfo("Slave UP (%s)", moduleProxy->getModuleName().c_str());

			nlassert(m_Slaves.find(moduleProxy) == m_Slaves.end());
			/*
			if (m_AbortRequested)
			{
				nlinfo("Add to slave delay list");
				m_ModuleUpDelay.push_back(moduleProxy);
			}
			else*/
			{
				m_SlavesMutex.lock();

				CSlave *slave = new CSlave(this, moduleProxy);
				m_Slaves[moduleProxy] = slave;

				m_SlavesMutex.unlock();

				slave->Proxy.submitToMaster(this);
			}
		}
	}

	virtual void onModuleDown(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineSlave")
		{
			nlinfo("Slave DOWN (%s)", moduleProxy->getModuleName().c_str());

			/*std::vector<IModuleProxy *>::iterator findDelay = std::find(m_ModuleUpDelay.begin(), m_ModuleUpDelay.end(), moduleProxy);
			if (findDelay != m_ModuleUpDelay.end())
			{
				nlinfo("Remove from slave delay list");
				m_ModuleUpDelay.erase(findDelay);
			}
			else*/
			{
				nlassert(m_Slaves.find(moduleProxy) != m_Slaves.end());

				m_SlavesMutex.lock();

				TSlaveMap::iterator slaveIt = m_Slaves.find(moduleProxy);
				CSlave *slave = slaveIt->second;

				if (slave->ActiveTaskId)
				{
					// if it goes down while busy on a task it crashed (or was poorly stopped by user...)
					CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_SLAVE_CRASHED);
					// ... TODO ...
					slaveAbortedBuildTask(moduleProxy); // see if this works
				}

				m_Slaves.erase(slaveIt);
				delete slave;
				// nldebug("Now %i slaves remaining", m_Slaves.size());

				m_SlavesMutex.unlock();
			}
		}
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	// Add a task to be run at the next update
	// The IRunnable will be deleted
	void addUpdateTask(IRunnable *runnable)
	{
		NLMISC::CSynchronized<std::deque<IRunnable *> >::CAccessor updateTasks(&m_UpdateTasks);
		updateTasks.value().push_back(runnable);
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	void handleUpdateTasks()
	{
		uint maxRuns;
		IRunnable *currentRunnable;
		{
			NLMISC::CSynchronized<std::deque<IRunnable *> >::CAccessor updateTasks(&m_UpdateTasks);
			if (updateTasks.value().size() == 0)
				return;
			maxRuns = updateTasks.value().size();
			currentRunnable = updateTasks.value().front();
		}
		for (uint i = 0; i < maxRuns; ++i)
		{
			currentRunnable->run();
			{
				NLMISC::CSynchronized<std::deque<IRunnable *> >::CAccessor updateTasks(&m_UpdateTasks);
				updateTasks.value().pop_front();
				if (updateTasks.value().size() == 0)
					break;
				currentRunnable = updateTasks.value().front();
			}
		}
	}

	virtual void onModuleUpdate()
	{
		// handle update tasks (like network responses from callbacks)
		handleUpdateTasks();

		// if state build, iterate trough all slaves to see if any is free, and check if there's any waiting tasks
		if (m_BuildWorking)
		{
			m_SlavesMutex.lock();
			// nldebug("Iterating %i slaves", m_Slaves.size());
			// iterate trough all slaves to tell them the enter build_ready state.
			for (TSlaveMap::iterator it = m_Slaves.begin(), end = m_Slaves.end(); it != end; ++it)
			{
				if (it->second->BuildReadyState == 0 && it->second->SaneBehaviour > 0 && it->second->SheetsOk && it->second->TimeOutStamp < NLMISC::CTime::getSecondsSince1970())
				{
					it->second->BuildReadyState = 1;
					it->second->Proxy.enterBuildReadyState(this);
				}
				// wait for confirmation, set BuildReadyState = 2 in that callback!
			}
			m_SlavesMutex.unlock();

			uint nbBuildable, nbWorking;
			m_BuildTaskQueue.countRemainingBuildableTasksAndWorkingTasks(nbBuildable, nbWorking);
			if (nbBuildable > 0)
			{
				m_SlavesMutex.lock();
				// Iterate trough all slaves to see if any is free for a building task.
				for (TSlaveMap::iterator it = m_Slaves.begin(), end = m_Slaves.end(); it != end; ++it)
				{
					if (it->second->canAcceptTask())
					{
						CBuildTaskInfo *taskInfo = m_BuildTaskQueue.getTaskForSlave(it->second->PluginsAvailable);
						if (taskInfo != NULL)
						{
							it->second->ActiveTaskId = taskInfo->Id.Global;
							it->second->Proxy.startBuildTask(this, taskInfo->ProjectName, taskInfo->ProcessPluginId);
							// the slave may either reject; or not answer until it's finished with this task
							// Display information to user terminals
							{
								// TODO_TERMINAL_SYNC
								CProcessPluginInfo pluginInfo;
								g_PipelineWorkspace->getProcessPlugin(pluginInfo, taskInfo->ProcessPluginId);
								nlinfo("Dispatching task '%i' ('%s': '%s') to slave '%s'", taskInfo->Id.Global, taskInfo->ProjectName.c_str(), pluginInfo.Handler.c_str(), it->second->Proxy.getModuleProxy()->getModuleName().c_str());
								notifyTerminalTaskBegin(taskInfo->Id.Global);
							}
						}
					}
				}
				m_SlavesMutex.unlock();
			}
			else if (nbWorking == 0)
			{
				// done (or stuck)

				m_BuildWorking = false;

				m_SlavesMutex.lock();
				// Iterate trough all slaves to tell them to end build_ready state.
				for (TSlaveMap::iterator it = m_Slaves.begin(), end = m_Slaves.end(); it != end; ++it)
				{
					it->second->BuildReadyState = 0;
					it->second->Proxy.leaveBuildReadyState(this);
					nlassert(it->second->ActiveTaskId == 0);
				}
				m_SlavesMutex.unlock();

				if (m_AbortRequested)
				{
					m_AbortRequested = false;
					CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_ABORTING);
					// Go through delayed slave list
					/*nldebug("Handle delayed slave list");
					for (std::vector<IModuleProxy *>::iterator it = m_ModuleUpDelay.begin(), end = m_ModuleUpDelay.end(); it != end; ++it)
						onModuleUp(*it);*/
				}

				PIPELINE::endedBuildReadyMaster();

				notifyTerminalBuildEnd();

				nlinfo("#####################################");
				nlinfo("#####################################");
				nlinfo("! DONE !");
				nlinfo("#####################################");
				nlinfo("#####################################");
			}
		}
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	/// When the slave task finishes, with or without error
	virtual void slaveFinishedBuildTask(NLNET::IModuleProxy *sender, uint8 errorLevel, const std::string &errorMessage)
	{
		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'slaveFinishedBuildTask' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();

		switch ((TProcessResult)errorLevel)
		{
		case FINISH_WARNING:
		case FINISH_SUCCESS:
			m_BuildTaskQueue.successTask(slave->ActiveTaskId);
			break;
		case FINISH_ERROR:
			m_BuildTaskQueue.erroredTask(slave->ActiveTaskId);
			break;
		default:
			nlerror("Slave returned bad error level");
			break;
		}

		notifyTerminalTaskState(slave->ActiveTaskId, (TProcessResult)errorLevel, errorMessage);

		slave->ActiveTaskId = 0;
	}

	/// When the user aborts slave-side, when slave-side exits, etc (assume the master requested abort or the slave crashed)
	virtual void slaveAbortedBuildTask(NLNET::IModuleProxy *sender)
	{
		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'slaveAbortedBuildTask' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();

		m_BuildTaskQueue.abortedTask(slave->ActiveTaskId);
		notifyTerminalTaskState(slave->ActiveTaskId, FINISH_ABORT, "The task has been aborted");
		slave->ActiveTaskId = 0;
	}

	// in fact slaves are not allowed to refuse tasks, but they may do this if the user is toying around with the slave service
	virtual void slaveRefusedBuildTask(NLNET::IModuleProxy *sender)
	{
		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'slaveRefusedBuildTask' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();

		m_BuildTaskQueue.rejectedTask(slave->ActiveTaskId);
		notifyTerminalTaskState(slave->ActiveTaskId, FINISH_REJECT, "The slave service has rejected the task. This is a programming error");
		slave->ActiveTaskId = 0;
		--slave->SaneBehaviour;
		slave->TimeOutStamp = NLMISC::CTime::getSecondsSince1970() + 3; // timeout for 3 seconds on this slave
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_SLAVE_REJECTED);
	}

	virtual void slaveLoggedToolError(NLNET::IModuleProxy *sender, uint8 type, const std::string &macroPath, const std::string &time, const std::string &error)
	{
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'slaveLoggedToolError' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		
		if (!macroPath.empty())
		{
			CBuildTaskInfo *task = m_BuildTaskQueue.getTaskInfo(slave->ActiveTaskId);
			CProcessPluginInfo pluginInfo;
			g_PipelineWorkspace->getProcessPlugin(pluginInfo, task->ProcessPluginId);
			
			CFileError fe;
			fe.MasterTime = CTime::getSecondsSince1970();
			fe.Level = (TError)type;
			fe.Message = error;
			fe.Time = time;
			fe.Project = task->ProjectName;;
			fe.Plugin = pluginInfo.Handler;
			CMetadataStorage::appendError(fe, CMetadataStorage::getErrorPath(unMacroPath(macroPath)));
		}

		notifyTerminalTaskMessage(slave->ActiveTaskId, (TError)type, macroPath, time, error);
	}

	virtual void slaveReloadedSheets(NLNET::IModuleProxy *sender)
	{
		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'slaveReloadedSheets' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();
		slave->SheetsOk = true;
		CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_MASTER_RELOAD_SHEETS);
	}

	virtual void slaveBuildReadySuccess(NLNET::IModuleProxy *sender)
	{
		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'slaveBuildReadySuccess' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();
		slave->BuildReadyState = 2;
	}

	virtual void slaveBuildReadyFail(NLNET::IModuleProxy *sender)
	{
		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'slaveBuildReadyFail' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();
		slave->BuildReadyState = 0;
		// --slave->SaneBehaviour; // allow this behaviour.
		slave->TimeOutStamp = NLMISC::CTime::getSecondsSince1970() + 3; // timeout for 3 seconds on this slave
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_SLAVE_NOT_READY);
	}

	virtual void vectorPushString(NLNET::IModuleProxy *sender, const std::string &str)
	{
		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'vectorPushString' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();
		slave->Vector.push_back(str);
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	
	FILE *m_MasterLog;

	void notifyTerminalBuildStart()
	{
		nlinfo("");
		// TODO NOTIFY TERMINAL
		if (m_MasterLog)
		{
			fflush(m_MasterLog);
			fclose(m_MasterLog);
			m_MasterLog = NULL;
		}
		m_MasterLog = fopen(IService::getInstance()->ConfigFile.getVar("MasterLog").asString(0).c_str(), "at");
		fprintf(m_MasterLog, "[BUILD_START]\n");
		fflush(m_MasterLog);
	}

	// task info
	void notifyTerminalTaskInfo(uint32 taskId, const std::string &projectName, const std::string &processHandler)
	{
		nlinfo("taskId: %i, projectName: %s, processHandler: %s", taskId, projectName.c_str(), processHandler.c_str());
		// TODO NOTIFY TERMINAL
		fprintf(m_MasterLog, "[TASK_INFO] taskId: %i, projectName: %s, processHandler: %s\n", taskId, projectName.c_str(), processHandler.c_str());
		fflush(m_MasterLog);
	}

	// task begin
	void notifyTerminalTaskBegin(uint32 taskId)
	{
		nlinfo("taskId: %i", taskId);
		// TODO NOTIFY TERMINAL
		fprintf(m_MasterLog, "[TASK_BEGIN] taskId: %i\n", taskId);
		fflush(m_MasterLog);
	}

	// task issues
	void notifyTerminalTaskMessage(uint32 taskId, TError type, const std::string &macroPath, const std::string &time, const std::string &error)
	{
		nlinfo("taskId: %i, type: %i, macroPath: %s, time: %s, error: %s", taskId, (uint32)type, macroPath.c_str(), time.c_str(), error.c_str());
		// TODO NOTIFY TERMINAL (send type as uint8 as usual)
		fprintf(m_MasterLog, "[TASK_MESSAGE] taskId: %i, type: %i, macroPath: %s, time: %s, error: %s\n", taskId, (uint32)type, macroPath.c_str(), time.c_str(), error.c_str());
		fflush(m_MasterLog);
	}

	// task end
	void notifyTerminalTaskState(uint32 taskId, TProcessResult errorLevel, const std::string &errorMessage)
	{
		nlinfo("taskId: %i, errorLevel: %i, errorMessage: %s", taskId, (uint32)errorLevel, errorMessage.c_str());
		// TODO NOTIFY TERMINAL (send errorlevel as uint8 as usual)
		fprintf(m_MasterLog, "[TASK_STATE] taskId: %i, errorLevel: %i, errorMessage: %s\n", taskId, (uint32)errorLevel, errorMessage.c_str());
		fflush(m_MasterLog);
	}

	// build end (any remaining tasks are considered aborted)
	void notifyTerminalBuildEnd()
	{
		nlinfo("");
		// TODO NOTIFY TERMINAL
		fprintf(m_MasterLog, "[BUILD_END]\n");
		fflush(m_MasterLog);
		fclose(m_MasterLog);
		m_MasterLog = NULL;
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	class CUpdateDatabaseStatusSlaveCallback : public CDelayedCallback
	{
		class CSlaveFileCallback : public NLMISC::IRunnable
		{
		public:
			CSlaveFileCallback(CModulePipelineMaster *master, NLNET::IModuleProxy *slaveProxy, const std::string &filePath, const CFileStatus &fileStatus)
				: m_Master(master), m_SlaveProxy(slaveProxy), m_FilePath(filePath), m_FileStatus(fileStatus) { }
			virtual void run()
			{
				m_Master->m_SlavesMutex.lock();
				TSlaveMap::iterator slaveIt = m_Master->m_Slaves.find(m_SlaveProxy);
				if (slaveIt == m_Master->m_Slaves.end())
				{
					// nlwarning("Slave disconnected before callback could be delivered");
					m_Master->m_SlavesMutex.unlock();
					delete this;
					return;
				}
				CSlave *slave = slaveIt->second;
				m_Master->m_SlavesMutex.unlock();

				slave->Proxy.addFileStatusToCache(m_Master, macroPath(m_FilePath), m_FileStatus);

				delete this;
			}
		private:
			CModulePipelineMaster *m_Master;
			NLNET::IModuleProxy *m_SlaveProxy;
			std::string m_FilePath;
			CFileStatus m_FileStatus;
		};

	public:
		CUpdateDatabaseStatusSlaveCallback(CModulePipelineMaster *master, NLNET::IModuleProxy *slaveProxy) : CDelayedCallback(master), m_SlaveProxy(slaveProxy) { }

		void fileCallback(const std::string &filePath, const CFileStatus &fileStatus, bool success)
		{
			if (success)
			{
				Master->addUpdateTask(new CSlaveFileCallback(Master, m_SlaveProxy, filePath, fileStatus));
			}
		}
		
		/// All status updated for this task
		virtual void run() // this is sanely run from the update thread
		{
			Master->m_SlavesMutex.lock();
			TSlaveMap::iterator slaveIt = Master->m_Slaves.find(m_SlaveProxy);
			if (slaveIt == Master->m_Slaves.end())
			{
				nlwarning("Slave disconnected before callback could be delivered");
				CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_MASTER_UPDATE_DATABASE_FOR_SLAVE);
				CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_SLAVE_CB_GONE);
				Master->m_SlavesMutex.unlock();
				delete this;
				return;
			}
			CSlave *slave = slaveIt->second;
			Master->m_SlavesMutex.unlock();

			slave->Proxy.masterUpdatedDatabaseStatus(Master);
			CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_MASTER_UPDATE_DATABASE_FOR_SLAVE);

			delete this;
		}

	private:
		NLNET::IModuleProxy *m_SlaveProxy;
	};

	class CUpdateDatabaseStatusByVectorTask : public IRunnable
	{
	public:
		CUpdateDatabaseStatusByVectorTask(CModulePipelineMaster *master, NLNET::IModuleProxy *sender, std::vector<std::string> &slaveVector) : m_Master(master), m_Sender(sender)
		{
			std::swap(slaveVector, m_Vector);
		}
		virtual void run() // run from the master process task manager
		{
			CUpdateDatabaseStatusSlaveCallback *cb = new CUpdateDatabaseStatusSlaveCallback(m_Master, m_Sender); // deleted by update
			g_DatabaseStatus->updateDatabaseStatus(cb->getCallback(), TFileStatusCallback(cb, &CUpdateDatabaseStatusSlaveCallback::fileCallback), m_Vector, false, false);

			delete this;
		}
	private:
		CModulePipelineMaster *m_Master;
		NLNET::IModuleProxy *m_Sender;
		std::vector<std::string> m_Vector;
	};

	virtual void updateDatabaseStatusByVector(NLNET::IModuleProxy *sender)
	{
		// FIXME: THIS MUST BE DONE ON A SEPERATE THREAD, IT HANGS WHILE ITERATING
		// SWAP THE VECTOR CONTAINERS

		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'updateDatabaseStatusByVector' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_MASTER_UPDATE_DATABASE_FOR_SLAVE);

		bool ok = true;
		for (std::vector<std::string>::size_type i = 0; i < slave->Vector.size(); ++i)
		{
			// not real security, just an extra for catching coding errors
			std::string &str = slave->Vector[i];
			if ((str[0] != '[') || (str[1] != '$'))
			{
				CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_CODE_ERROR_UNMACRO); // permanent flag
				CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_MASTER_UPDATE_DATABASE_FOR_SLAVE);
				--slave->SaneBehaviour;
				slave->Proxy.abortBuildTask(this);
				ok = false;
				break;
			}
		}

		if (ok)
		{
			if (g_IsExiting)
			{
				nlwarning("Cannot add task while exiting");
			}
			else
			{
				CUpdateDatabaseStatusByVectorTask *slaveTask = new CUpdateDatabaseStatusByVectorTask(this, sender, slave->Vector); // slave->Vector is wiped due to swap
				m_TaskManager->addTask(slaveTask);
			}
		}
	}

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	void setAvailablePlugins(NLNET::IModuleProxy *sender, const std::vector<uint32> &pluginsAvailable)
	{
		//m_SlavesMutex.lock();
		TSlaveMap::iterator slaveIt = m_Slaves.find(sender);
		if (slaveIt == m_Slaves.end()) { nlerror("Received 'setAvailablePlugins' from unknown slave at '%s'", sender->getModuleName().c_str()); m_Slaves.erase(sender); /*m_SlavesMutex.unlock();*/ return; }
		CSlave *slave = slaveIt->second;
		//m_SlavesMutex.unlock();
		slave->PluginsAvailable = pluginsAvailable;
	}

	bool build(bool bypassEros, bool verifyOnly)
	{
		if (PIPELINE::tryBuildReadyMaster())
		{
			notifyTerminalBuildStart();
			m_BuildWorking = true;
			m_BypassErrors = bypassEros;
			m_VerifyOnly = verifyOnly;
			m_BuildTaskQueue.resetQueue();
			m_BuildTaskQueue.loadQueue(g_PipelineWorkspace, bypassEros);

			// notify terminal task info
			{
				std::vector<PIPELINE::CBuildTaskInfo *> tasks;
				m_BuildTaskQueue.listTaskQueueByMostDependents(tasks);
				for (std::vector<PIPELINE::CBuildTaskInfo *>::iterator it = tasks.begin(), end = tasks.end(); it != end; ++it)
				{
					PIPELINE::CBuildTaskInfo *task = *it;
					PIPELINE::CProcessPluginInfo pluginInfo;
					PIPELINE::g_PipelineWorkspace->getProcessPlugin(pluginInfo, task->ProcessPluginId);
					notifyTerminalTaskInfo(task->Id.Global, task->ProjectName, pluginInfo.Handler);
				}
			}

			return true;
		}
		return false;
	}

	bool abort()
	{
		if (m_BuildWorking)
		{
			if (!m_AbortRequested)
			{
				m_AbortRequested = true;
				CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_ABORTING);
			}

			// Yes, we effectively send the abort again to the slaves, it makes for a more epic abort button hammering experience.

			m_BuildTaskQueue.abortQueue();

			m_SlavesMutex.lock();
			// Abort all slaves.
			for (TSlaveMap::iterator it = m_Slaves.begin(), end = m_Slaves.end(); it != end; ++it)
			{
				if (it->second->ActiveTaskId != 0)
					it->second->Proxy.abortBuildTask(this);
			}
			m_SlavesMutex.unlock();

			return true;
		}
		else
		{
			return false;
		}
	}

protected:
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CModulePipelineMaster, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CModulePipelineMaster, reloadSheets, "Reload sheets across all services", "")
		NLMISC_COMMAND_HANDLER_ADD(CModulePipelineMaster, build, "Build", "")
		NLMISC_COMMAND_HANDLER_ADD(CModulePipelineMaster, abort, "Abort", "")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(reloadSheets)
	{
		if (args.size() != 0) return false;

		if (PIPELINE::tryDirectTask("MASTER_RELOAD_SHEETS"))
		{
			updateSheetsDatabaseStatus(CCallback<void>(this, &CModulePipelineMaster::cbMasterSheetReloadUpdate));

			return true;
		}
		else
		{
			log.displayNL("Busy");
			return false;
		}
	}

	void cbMasterSheetReloadUpdate()
	{
		m_SlavesMutex.lock();

		for (TSlaveMap::iterator it = m_Slaves.begin(), end = m_Slaves.end(); it != end; ++it)
		{
			CSlave *slave = it->second;
			slave->SheetsOk = false;
			slave->PluginsAvailable.clear();
			slave->Proxy.reloadSheets(this);
			CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_MASTER_RELOAD_SHEETS);
		}

		m_SlavesMutex.unlock();

		PIPELINE::endedDirectTask();
	}

	NLMISC_CLASS_COMMAND_DECL(build)
	{
		bool bypassErrors = false;
		bool verifyOnly = false;
		std::vector<std::string> unknownVec;
		std::vector<std::string> &workingVec = unknownVec;

		for (std::vector<std::string>::const_iterator it = args.begin(), end = args.end(); it != end; ++it)
		{
			if ((*it)[0] == '-')
			{
				workingVec = unknownVec;
				if (((*it) == "--bypassErrors") || ((*it) == "-be"))
					bypassErrors = true;
				else if (((*it) == "--verifyOnly") || ((*it) == "-vo"))
					bypassErrors = true;
				else
					unknownVec.push_back(*it);
			}
			else
			{
				workingVec.push_back(*it);
			}
		}

		if (unknownVec.size() > 0)
		{
			for (std::vector<std::string>::iterator it = unknownVec.begin(), end = unknownVec.end(); it != end; ++it)
				log.displayNL("Unknown build parameter: %s", (*it).c_str());
			return false;
		}
		else
		{
			if (build(bypassErrors, verifyOnly))
			{
				return true;
			}
			else
			{
				log.displayNL("Bad command usage");
				return false;
			}
		}
	}

	NLMISC_CLASS_COMMAND_DECL(abort)
	{
		if (args.size() != 0) return false;
		if (this->abort())
		{
			return true;
		}
		else
		{
			log.displayNL("Bad command usage");
			return false;
		}
	}

}; /* class CModulePipelineMaster */

void module_pipeline_master_forceLink() { }
NLNET_REGISTER_MODULE_FACTORY(CModulePipelineMaster, "ModulePipelineMaster");

} /* namespace PIPELINE */

/* end of file */
