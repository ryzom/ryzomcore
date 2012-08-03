/**
 * \file pipeline_process_impl.cpp
 * \brief CPipelineProcessImpl
 * \date 2012-03-03 09:33GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineProcessImpl
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
#include "pipeline_process_impl.h"

// STL includes
#include <sstream>

// NeL includes
#include <nel/misc/time_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>

// Project includes
#include "pipeline_service.h"
#include "pipeline_project.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CPipelineProcessImpl::CPipelineProcessImpl(CPipelineProject *activeProject) : m_ActiveProject(activeProject), m_SubTaskResult(FINISH_NOT)
{
	if (activeProject == NULL)
	{
		nlassert(getInstance() == NULL);
		NLMISC::INelContext::getInstance().setSingletonPointer("IPipelineProcess", this);
	}
	m_ResultPreviousSuccess.clear();
	m_ResultCurrent.clear();
}

CPipelineProcessImpl::~CPipelineProcessImpl()
{
	if (m_ActiveProject == NULL)
	{
		NLMISC::INelContext::getInstance().releaseSingletonPointer("IPipelineProcess", this);
	}
}

std::string CPipelineProcessImpl::getOutputDirectory()
{
	if (m_ActiveProject == NULL)
	{
		nlwarning("(m_ActiveProject == NULL)");
		return getTempDirectory();
	}
	else
	{
		return m_ActiveProject->getOutputDirectory();
	}
}

std::string CPipelineProcessImpl::getTempDirectory()
{
	if (m_ActiveProject == NULL)
	{
		nlwarning("(m_ActiveProject == NULL)");
		std::stringstream ss;
		ss << g_WorkDir;
		ss << PIPELINE_DIRECTORY_UNKNOWN_PREFIX;
		ss << NLMISC::CTime::getSecondsSince1970();
		ss << ".";
		ss << rand();
		ss << PIPELINE_DIRECTORY_TEMP_SUFFIX;
		ss << "/";
		return ss.str();
	}
	else
	{
		return m_ActiveProject->getTempDirectory();
	}
}

bool CPipelineProcessImpl::getValue(std::string &result, const std::string &name)
{
	if (m_ActiveProject == NULL)
	{
		nlwarning("(m_ActiveProject == NULL)");
		return false;
	}
	else
	{
		return m_ActiveProject->getValue(result, name);
	}
}

bool CPipelineProcessImpl::getValues(std::vector<std::string> &resultAppend, const std::string &name)
{
	if (m_ActiveProject == NULL)
	{
		nlwarning("(m_ActiveProject == NULL)");
		return false;
	}
	else
	{
		return m_ActiveProject->getValues(resultAppend, name);
	}
}

bool CPipelineProcessImpl::getValueNb(uint &result, const std::string &name)
{
	if (m_ActiveProject == NULL)
	{
		nlwarning("(m_ActiveProject == NULL)");
		return false;
	}
	else
	{
		return m_ActiveProject->getValueNb(result, name);
	}
}

} /* namespace PIPELINE */

/* end of file */
