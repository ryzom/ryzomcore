/**
 * \file pipeline_interface_impl.cpp
 * \brief CPipelineInterfaceImpl
 * \date 2012-02-25 12:21GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineInterfaceImpl
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
#include "pipeline_interface_impl.h"

// STL includes
#include <sstream>

// NeL includes
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>
#include <nel/net/service.h>
#include <nel/misc/class_registry.h>

// Project includes
#include "pipeline_service.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CPipelineInterfaceImpl::CPipelineInterfaceImpl()
{
	nlassert(getInstance() == NULL);
	NLMISC::INelContext::getInstance().setSingletonPointer("IPipelineInterface", this);
}

CPipelineInterfaceImpl::~CPipelineInterfaceImpl()
{
	NLMISC::INelContext::getInstance().releaseSingletonPointer("IPipelineInterface", this);
}

NLMISC::CConfigFile &CPipelineInterfaceImpl::getConfigFile()
{
	return NLNET::IService::getInstance()->ConfigFile;
}

void CPipelineInterfaceImpl::registerClass(const std::string &className, NLMISC::IClassable* (*creator)(), const std::string &typeidCheck) throw(NLMISC::ERegistry)
{
	NLMISC::CClassRegistry::registerClass(className, creator, typeidCheck);
	RegisteredClasses.push_back(className);
	nldebug("Registered class '%s'", className.c_str());
}

bool CPipelineInterfaceImpl::tryRunnableTask(std::string stateName, NLMISC::IRunnable *task)
{
	return PIPELINE::tryRunnableTask(stateName, task);
}

void CPipelineInterfaceImpl::endedRunnableTask()
{
	PIPELINE::endedRunnableTask();
}

std::string CPipelineInterfaceImpl::getProjectValue(const std::string &name)
{
	return ""; // TODO
}

std::string CPipelineInterfaceImpl::getTempDir()
{
	// IF PROJECT blahblah TODO
	// ELSE

	{
		std::stringstream ss;
		ss << g_PipelineDirectory;
		ss << NLMISC::CTime::getSecondsSince1970();
		ss << ".";
		ss << rand();
		ss << PIPELINE_DIRECTORY_TEMP_SUFFIX;
		return ss.str();
	}
}

} /* namespace PIPELINE */

/* end of file */
