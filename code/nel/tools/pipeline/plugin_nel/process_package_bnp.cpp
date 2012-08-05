/**
 * \file process_package_bnp.cpp
 * \brief CProcessPackageBNP
 * \date 2012-08-04 18:54GMT
 * \author Jan Boon (Kaetemi)
 * CProcessPackageBNP
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "process_package_bnp.h"

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

void CProcessPackageBNP::buildPackageBNP(const std::string &dependLog, const std::string &errorLog, const std::vector<std::string> &srcDirectories, const std::string &dstFile)
{
	nldebug("Build: Package BNP '%s'", dstFile.c_str());
	std::vector<std::string> arguments;
	arguments.push_back("/p");
	arguments.push_back(dstFile);
	for (std::vector<std::string>::const_iterator it = srcDirectories.begin(), end = srcDirectories.end(); it != end; ++it)
	{
		arguments.push_back("-dir");
		arguments.push_back(*it);
	}
	arguments.push_back("-dependlog");
	arguments.push_back(dependLog);
	arguments.push_back("-errorlog");
	arguments.push_back(errorLog);
	arguments.push_back("-norecurse");
	m_PipelineProcess->runConsoleTool(m_PipelineProcess->getConfig("ToolBnpMake"), arguments);
}

void CProcessPackageBNP::build()
{
	// TODO: Divisions

	nldebug("Build process plugin: CProcessPackageBNP");

	std::string tempDir = m_PipelineProcess->getTempDirectory();
	std::string dependLog = tempDir + "depend.log";
	std::string errorLog = tempDir + "error.log";
	
	breakable
	{
		std::string root = "Package.BNP.";
		std::vector<std::string> srcDirectories;
		if (!m_PipelineProcess->getValues(srcDirectories, root + "SrcDirectories")) break;
		std::string dstFile;
		if (!m_PipelineProcess->getValue(dstFile, root + "DstFile")) break;

		if (m_PipelineProcess->needsToBeRebuilt(srcDirectories, dstFile, false))
		{
			m_PipelineProcess->makePaths(dstFile);
			buildPackageBNP(dependLog, errorLog, srcDirectories, dstFile);
			if (m_PipelineProcess->needsExit()) return;
			m_PipelineProcess->parseToolLog(dependLog, errorLog, false);
		}
		if (m_PipelineProcess->needsExit()) return;
	}
	
	m_PipelineProcess->deleteDirectoryIfEmpty(tempDir);
}

void CProcessPackageBNPInfo::getDependentDirectories(std::vector<std::string> &resultAppend)
{
	m_PipelineProcess->getValues(resultAppend, "Package.BNP.SrcDirectories");
}

void CProcessPackageBNPInfo::getDependentFiles(std::vector<std::string> &resultAppend)
{
	
}

} /* namespace PIPELINE */

/* end of file */
