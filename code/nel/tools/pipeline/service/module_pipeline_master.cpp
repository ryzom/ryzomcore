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

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace PIPELINE {

/**
 * \brief CModulePipelineMaster
 * \date 2012-03-03 16:26GMT
 * \author Jan Boon (Kaetemi)
 * CModulePipelineMaster
 */
class CModulePipelineMaster :
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CModulePipelineMasterSkel
{
protected:
	// pointers
	// ...
	
	// instances
	// ...
public:
	CModulePipelineMaster()
	{

	}

	virtual ~CModulePipelineMaster()
	{

	}

	virtual void slaveFinishedBuildTask(NLNET::IModuleProxy *sender, uint32 taskId)
	{

	}
}; /* class CModulePipelineMaster */

void module_pipeline_master_forceLink() { }
NLNET_REGISTER_MODULE_FACTORY(CModulePipelineMaster, "ModulePipelineMaster");

} /* namespace PIPELINE */

/* end of file */
