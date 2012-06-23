/**
 * \file module_pipeline_slave.cpp
 * \brief CModulePipelineSlave
 * \date 2012-03-03 16:26GMT
 * \author Jan Boon (Kaetemi)
 * CModulePipelineSlave
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
#include "module_pipeline_slave_itf.h"

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes
#include "info_flags.h"
#include "module_pipeline_master_itf.h"
#include "pipeline_service.h"
#include "../plugin_library/process_info.h"
#include "pipeline_workspace.h"
#include "pipeline_process_impl.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace PIPELINE {

#define PIPELINE_INFO_SLAVE_RELOAD_SHEETS "SLAVE_RELOAD_SHEETS"
#define PIPELINE_ERROR_SHEETS_CRC32_FAILED "Failed sheets CRC32. Sheets were modified inbetween launching services. This causes newly loaded services to be out of sync. Not allowed. Reload the sheets from the master service, and restart this slave service"

enum TRequestState
{
	REQUEST_NONE, 
	REQUEST_MADE, 
	REQUEST_WORKING, 
};

/**
 * \brief CModulePipelineSlave
 * \date 2012-03-03 16:26GMT
 * \author Jan Boon (Kaetemi)
 * CModulePipelineSlave
 */
class CModulePipelineSlave :
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CModulePipelineSlaveSkel
{
public:
	CModulePipelineMasterProxy *m_Master;
	bool m_TestCommand;
	TRequestState m_ReloadSheetsState;
	bool m_BuildReadyState;
	
public:
	CModulePipelineSlave() : m_Master(NULL), m_TestCommand(false), m_ReloadSheetsState(REQUEST_NONE), m_BuildReadyState(false)
	{
		
	}

	virtual ~CModulePipelineSlave()
	{
		// TODO: IF MASTER STILL CONNECTED, NOTIFY INSANITY
		// TODO: ABORT RUNNING BUILD PROCESS
		abortBuildTask(NULL);
		leaveBuildReadyState(NULL);
	}

	virtual bool initModule(const TParsedCommandLine &initInfo)
	{
		CModuleBase::initModule(initInfo);
		CModulePipelineSlaveSkel::init(this);
		return true;
	}

	virtual void onModuleUp(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineMaster")
		{
			nlinfo("Master UP (%s)", moduleProxy->getModuleName().c_str());

			nlassert(m_Master == NULL);
			
			m_Master = new CModulePipelineMasterProxy(moduleProxy);
		}
	}

	/// This is here instead of inside onModuleUp because we don't know the order of onModuleUp on local systems. This is only called once per master up like onModuleUp.
	virtual void submitToMaster(NLNET::IModuleProxy *sender)
	{
		// TODO: AUTHENTICATE OR GATEWAY SECURITY?
		CModulePipelineMasterProxy master(sender);
		if (!g_PipelineWorkspace->loadCRC32())
			nlerror(PIPELINE_ERROR_SHEETS_CRC32_FAILED);
		sendMasterAvailablePlugins(&master);
	}
	
	virtual void onModuleDown(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineMaster")
		{
			nlinfo("Master DOWN (%s)", moduleProxy->getModuleName().c_str());

			nlassert(m_Master->getModuleProxy() == moduleProxy);

			// TODO: ABORT RUNNING BUILD PROCESS
			abortBuildTask(NULL);
			leaveBuildReadyState(NULL); // leave state if building

			delete m_Master;
			m_Master = NULL;
		}
	}

	virtual void onModuleUpdate()
	{
		if (m_ReloadSheetsState == REQUEST_MADE)
		{
			if (PIPELINE::reloadSheets())
			{
				m_ReloadSheetsState = REQUEST_WORKING;
			}
		}
		else if (m_ReloadSheetsState == REQUEST_WORKING)
		{
			if (PIPELINE::isServiceStateIdle())
			{
				m_ReloadSheetsState = REQUEST_NONE;
				if (!g_PipelineWorkspace->loadCRC32())
					nlerror(PIPELINE_ERROR_SHEETS_CRC32_FAILED);
				sendMasterAvailablePlugins(m_Master);
				m_Master->slaveReloadedSheets(this);
				CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_SLAVE_RELOAD_SHEETS);
			}
		}
	}

	virtual void startBuildTask(NLNET::IModuleProxy *sender, const std::string &projectName, const uint32 &pluginId)
	{
		// TODO

		//this->queueModuleTask
		CModulePipelineMasterProxy master(sender);
		master.slaveRefusedBuildTask(this); // NO MORE TASK ID
	}

	virtual void abortBuildTask(NLNET::IModuleProxy *sender)
	{
		// TODO
	}

	virtual void masterUpdatedDatabaseStatus(NLNET::IModuleProxy *sender)
	{
		if (m_TestCommand)
		{
			endedRunnableTask();
		}
		else
		{
			nlwarning("NOT_IMPLEMENTED");
		}
	}

	virtual void reloadSheets(NLNET::IModuleProxy *sender)
	{
		CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_SLAVE_RELOAD_SHEETS);
		if (PIPELINE::reloadSheets()) m_ReloadSheetsState = REQUEST_WORKING;
		else m_ReloadSheetsState = REQUEST_MADE;
	}
	
	virtual void enterBuildReadyState(NLNET::IModuleProxy *sender)
	{
		if (!m_BuildReadyState && PIPELINE::tryBuildReady())
		{
			m_BuildReadyState = true;
			m_Master->slaveBuildReadySuccess(this);
		}
		else
		{
			m_Master->slaveBuildReadyFail(this);
		}
	}
	
	virtual void leaveBuildReadyState(NLNET::IModuleProxy *sender)
	{
		if (m_BuildReadyState)
		{
			m_BuildReadyState = false;
			PIPELINE::endedBuildReady();
		}
	}

	void sendMasterAvailablePlugins(CModulePipelineMasterProxy *master)
	{
		std::vector<uint32> availablePlugins;
		g_PipelineWorkspace->listAvailablePlugins(availablePlugins);
		master->setAvailablePlugins(this, availablePlugins);
	}
	
protected:
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CModulePipelineSlave, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CModulePipelineSlave, testUpdateDatabaseStatus, "Test master request for database status update on dependencies", "<projectName> <processName>")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(testUpdateDatabaseStatus);

}; /* class CModulePipelineSlave */

//return PIPELINE::tryRunnableTask(stateName, task);

namespace {

class CTestUpdateDatabaseStatusCommand : public NLMISC::IRunnable
{
public:
	NLMISC::CLog *Log;
	std::string Project;
	std::string Process;
	CModulePipelineSlave *Slave;

	virtual void getName(std::string &result) const 
	{ result = "CTestUpdateDatabaseStatusCommand"; }

	virtual void run()
	{
		Slave->m_TestCommand = true;

		// std::string tempDirectory = PIPELINE::IPipelineProcess::getInstance()->getTempDirectory();
		std::vector<PIPELINE::CProcessPluginInfo> plugins;
		PIPELINE::g_PipelineWorkspace->getProcessPlugins(plugins, Process);
		PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(Project);
		if (project)
		{
			std::vector<std::string> result;
			PIPELINE::IPipelineProcess *pipelineProcess = new PIPELINE::CPipelineProcessImpl(project);
			for (std::vector<PIPELINE::CProcessPluginInfo>::iterator plugin_it = plugins.begin(), plugin_end = plugins.end(); plugin_it != plugin_end; ++plugin_it)
			{
				switch (plugin_it->InfoType)
				{
				case PIPELINE::PLUGIN_REGISTERED_CLASS:
					{
						PIPELINE::IProcessInfo *processInfo = static_cast<PIPELINE::IProcessInfo *>(NLMISC::CClassRegistry::create(plugin_it->Info));
						processInfo->setPipelineProcess(pipelineProcess);
						processInfo->getDependentDirectories(result);
						for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
							Slave->m_Master->vectorPushString(Slave, PIPELINE::macroPath(*it));
						result.clear();
						processInfo->getDependentFiles(result);
						for (std::vector<std::string>::iterator it = result.begin(), end = result.end(); it != end; ++it)
							Slave->m_Master->vectorPushString(Slave, PIPELINE::macroPath(*it));
						result.clear();
					}
					break;
				default:
					nlwarning("Not implemented");
					break;
				}
			}
		}
		else
		{
			Log->displayNL("Project '%s' does not exist", Project.c_str());
		}

		Slave->m_Master->updateDatabaseStatusByVector(Slave);
		
		delete this;
	}
};

} /* anonymous namespace */

NLMISC_CLASS_COMMAND_IMPL(CModulePipelineSlave, testUpdateDatabaseStatus)
{
	// EXAMPLE USAGE: slave.testUpdateDatabaseStatus common_interface Interface

	if (args.size() != 2) return false;
	
	PIPELINE::CPipelineProject *project = PIPELINE::g_PipelineWorkspace->getProject(args[0]);
	if (!project)
	{ 
		log.displayNL("Project '%s' does not exist", args[0].c_str());
		return false;
	}
	
	CTestUpdateDatabaseStatusCommand *runnableCommand = new CTestUpdateDatabaseStatusCommand();
	runnableCommand->Log = &log;
	runnableCommand->Project = args[0];
	runnableCommand->Process = args[1];
	runnableCommand->Slave = this;
	
	if (!tryRunnableTask("SLAVE_TEST_UPD_DB_STATUS", runnableCommand))
	{ log.displayNL("BUSY"); delete runnableCommand; return false; }
	return true;
}

void module_pipeline_slave_forceLink() { }
NLNET_REGISTER_MODULE_FACTORY(CModulePipelineSlave, "ModulePipelineSlave");

} /* namespace PIPELINE */

/* end of file */
