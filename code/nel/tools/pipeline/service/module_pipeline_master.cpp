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

// Project includes
#include "info_flags.h"
#include "module_pipeline_slave_itf.h"
#include "pipeline_service.h"
#include "database_status.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace PIPELINE {

#define PIPELINE_INFO_MASTER_RELOAD_SHEETS "MASTER_RELOAD_SHEETS"

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
			SheetsOk(true) { }
		CModulePipelineMaster *Master;
		CModulePipelineSlaveProxy Proxy;
		std::vector<std::string> Vector;
		uint32 ActiveTaskId;
		bool SheetsOk;

		~CSlave()
		{
			if (!SheetsOk)
			{
				CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_MASTER_RELOAD_SHEETS);
			}
		}

		void cbUpdateDatabaseStatus()
		{
			Proxy.masterUpdatedDatabaseStatus(Master);
		}

		bool canAcceptTask()
		{
			return SheetsOk && (ActiveTaskId == 0);
		}
	};
	
protected:
	typedef std::map<IModuleProxy *, CSlave *> TSlaveMap;
	TSlaveMap m_Slaves;
	mutable boost::mutex m_SlavesMutex;

public:
	CModulePipelineMaster()
	{
		g_IsMaster = true;
	}

	virtual ~CModulePipelineMaster()
	{
		g_IsMaster = false;

		m_SlavesMutex.lock();
		
		for (TSlaveMap::iterator it = m_Slaves.begin(), end = m_Slaves.end(); it != end; ++it)
			delete it->second;
		m_Slaves.clear();
		
		m_SlavesMutex.unlock();
	}

	virtual bool initModule(const TParsedCommandLine &initInfo)
	{
		CModuleBase::initModule(initInfo);
		CModulePipelineMasterSkel::init(this);
		return true;
	}

	virtual void onModuleUp(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineSlave")
		{
			nlinfo("Slave UP (%s)", moduleProxy->getModuleName().c_str());
			
			nlassert(m_Slaves.find(moduleProxy) == m_Slaves.end());
			
			m_SlavesMutex.lock();

			CSlave *slave = new CSlave(this, moduleProxy);
			m_Slaves[moduleProxy] = slave;

			m_SlavesMutex.unlock();
		}
	}
	
	virtual void onModuleDown(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineSlave")
		{
			nlinfo("Slave DOWN (%s)", moduleProxy->getModuleName().c_str());
			
			nlassert(m_Slaves.find(moduleProxy) != m_Slaves.end());
			
			m_SlavesMutex.lock();

			TSlaveMap::iterator slaveIt = m_Slaves.find(moduleProxy);
			CSlave *slave = slaveIt->second;

			if (slave->ActiveTaskId)
			{
				// ...
			}
			
			m_Slaves.erase(slaveIt);
			delete slave;
			
			m_SlavesMutex.unlock();
		}
	}

	virtual void onModuleUpdate()
	{
		// if state build, iterate trough all slaves to see if any is free, and check if there's any waiting tasks
	}

	virtual void slaveFinishedBuildTask(NLNET::IModuleProxy *sender, uint32 taskId, uint8 errorLevel)
	{
		// TODO
	}
	
	virtual void slaveRefusedBuildTask(NLNET::IModuleProxy *sender, uint32 taskId)
	{
		// TODO
	}

	virtual void slaveReloadedSheets(NLNET::IModuleProxy *sender)
	{
		CSlave *slave = m_Slaves[sender];
		slave->SheetsOk = true;
		CInfoFlags::getInstance()->removeFlag(PIPELINE_INFO_MASTER_RELOAD_SHEETS);
	}

	virtual void vectorPushString(NLNET::IModuleProxy *sender, const std::string &str)
	{
		CSlave *slave = m_Slaves[sender];
		slave->Vector.push_back(str);
	}
	
	virtual void updateDatabaseStatusByVector(NLNET::IModuleProxy *sender)
	{
		CSlave *slave = m_Slaves[sender];
		g_DatabaseStatus->updateDatabaseStatus(CCallback<void>(slave, &CSlave::cbUpdateDatabaseStatus), slave->Vector, false, false);
		slave->Vector.clear();
	}
	
protected:
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CModulePipelineMaster, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CModulePipelineMaster, reloadSheets, "Reload sheets across all services", "")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(reloadSheets)
	{
		if (args.size() != 0) return false;

		if (PIPELINE::tryDirectTask("MASTER_RELOAD_SHEETS"))
		{
			m_SlavesMutex.lock();
			
			for (TSlaveMap::iterator it = m_Slaves.begin(), end = m_Slaves.end(); it != end; ++it)
			{
				CSlave *slave = it->second;
				slave->SheetsOk = false;
				slave->Proxy.reloadSheets(this);
				CInfoFlags::getInstance()->addFlag(PIPELINE_INFO_MASTER_RELOAD_SHEETS);
			}
			
			m_SlavesMutex.unlock();

			PIPELINE::endedDirectTask();

			return true;
		}
		else
		{
			log.displayNL("Busy");
			return false;
		}
	}

}; /* class CModulePipelineMaster */

void module_pipeline_master_forceLink() { }
NLNET_REGISTER_MODULE_FACTORY(CModulePipelineMaster, "ModulePipelineMaster");

} /* namespace PIPELINE */

/* end of file */
