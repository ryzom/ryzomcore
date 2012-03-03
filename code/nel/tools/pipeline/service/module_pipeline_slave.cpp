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
#include "module_pipeline_master_itf.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace PIPELINE {

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
protected:
	CModulePipelineMasterProxy *m_Master;
	
public:
	CModulePipelineSlave() : m_Master(NULL)
	{
		
	}

	virtual ~CModulePipelineSlave()
	{
		
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
			nlassert(m_Master == NULL);
			
			m_Master = new CModulePipelineMasterProxy(moduleProxy);

			nlinfo("Master UP (%s)", moduleProxy->getModuleName().c_str());
		}
	}
	
	virtual void onModuleDown(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "ModulePipelineMaster")
		{
			nlassert(m_Master->getModuleProxy() == moduleProxy);

			delete m_Master;
			m_Master = NULL;

			nlinfo("Slave DOWN (%s)", moduleProxy->getModuleName().c_str());
		}
	}

	virtual void startBuildTask(NLNET::IModuleProxy *sender, uint32 taskId, const std::string &projectName, const std::string &processHandler)
	{
		//this->queueModuleTask
		CModulePipelineMasterProxy master(sender);
		master.slaveRefusedBuildTask(this, taskId);
	}

	virtual void masterUpdatedDatabaseStatus(NLNET::IModuleProxy *sender)
	{
		
	}
	
}; /* class CModulePipelineSlave */

void module_pipeline_slave_forceLink() { }
NLNET_REGISTER_MODULE_FACTORY(CModulePipelineSlave, "ModulePipelineSlave");

} /* namespace PIPELINE */

/* end of file */
