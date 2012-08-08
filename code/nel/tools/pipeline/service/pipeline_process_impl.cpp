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
#ifndef NL_OS_WINDOWS
#	include <sys/wait.h>
#endif

// NeL includes
#include <nel/misc/time_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/net/service.h>

// Project includes
#include "pipeline_service.h"
#include "pipeline_project.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CPipelineProcessImpl::CPipelineProcessImpl(CPipelineProject *activeProject) : m_ActiveProject(activeProject), m_SubTaskResult(FINISH_NOT), m_Aborting(false), m_StatsBuild(0), m_StatsSkip(0), m_StatsInvalid(0)
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
	/*std::vector<std::string> dummy;
	NLMISC::CPath::getPathContent(path, false, true, true, dummy);
	if (dummy.size())
	{
		nlwarning("Directory '%s' not empty", path.c_str());
	}
	else
	{*/
		NLMISC::CFile::deleteDirectory(path);
	/*}*/
}

std::string CPipelineProcessImpl::getConfig(const std::string &name)
{
	return NLNET::IService::getInstance()->ConfigFile.getVar(name).asString(0);
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

void CPipelineProcessImpl::runConsoleTool(const std::string &executablePath, const std::vector<std::string> &arguments)
{
#ifdef NL_OS_WINDOWS
	// FIXME: NOT SAFE FOR ARGUMENTS WITH SPACES
	std::stringstream ss;
	ss << executablePath;
	for (std::vector<std::string>::const_iterator it = arguments.begin(), end = arguments.end(); it != end; ++it)
		ss << " " << *it;
	nldebug("System: '%s'", ss.str().c_str());
	system(ss.str().c_str());
#else
	pid_t fork_id = fork();
	if (fork_id)
	{	
		int exitCode;
		while (wait(&exitCode) != fork_id) { /* ... */ }
	}
	else
	{
		std::vector<char *> argv;
		argv.reserve(arguments.size() + 1);
		argv.push_back(const_cast<char *>(executablePath.c_str()));
		for (std::vector<std::string>::const_iterator it = arguments.begin(), end = arguments.end(); it != end; ++it)
			argv.push_back(const_cast<char *>((*it).c_str()));
		argv.push_back(NULL);
		execv(argv[0], &argv[0]);
		exit(1); // failure
	}
#endif
}

} /* namespace PIPELINE */

/* end of file */
