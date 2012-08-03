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
#include <nel/misc/path.h>

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
		NLMISC::CFile::createDirectoryTree(ss.str());
		return ss.str();
	}
	else
	{
		return m_ActiveProject->getTempDirectory();
	}
}

void CPipelineProcessImpl::deleteDirectoryIfEmpty(const std::string &path)
{
	std::vector<std::string> dummy;
	NLMISC::CPath::getPathContent(path, false, true, true, dummy);
	if (dummy.size())
	{
		nlwarning("Directory '%s' not empty", path.c_str());
	}
	else
	{
		NLMISC::CFile::deleteDirectory(path);
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

void CPipelineProcessImpl::makePaths(const std::vector<std::string> &outputPaths)
{
	for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		if (path[path.size() - 1] == '/') // isDirectory
		{
			NLMISC::CFile::createDirectoryTree(path);
		}
		else
		{
			NLMISC::CFile::createDirectoryTree(NLMISC::CFile::getPath(path));
		}
	}
}

} /* namespace PIPELINE */

/* end of file */
