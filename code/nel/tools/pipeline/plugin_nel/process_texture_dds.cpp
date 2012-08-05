/**
 * \file process_texture_dds.cpp
 * \brief CProcessTextureDDS
 * \date 2012-08-04 12:50GMT
 * \author Jan Boon (Kaetemi)
 * CProcessTextureDDS
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
#include "process_texture_dds.h"

// STL includes
#include <sstream>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

void CProcessTextureDDS::buildDDS(const std::string &dependLog, const std::string &errorLog, const std::string &srcFile, const std::string &dstFile, const std::string &algorithm, bool createMipMap, uint reduceFactor, bool checkUserColor)
{
	nldebug("Build: DDS '%s'", dstFile.c_str());
	std::vector<std::string> arguments;
	arguments.push_back(srcFile);
	arguments.push_back("-o");
	arguments.push_back(dstFile);
	if (!algorithm.empty())
	{
		arguments.push_back("-a");
		arguments.push_back(algorithm);
	}
	if (createMipMap)
		arguments.push_back("-m");
	if (reduceFactor)
	{
		std::stringstream ss;
		ss << "-r" << reduceFactor;
		arguments.push_back(ss.str());
	}
	if (!checkUserColor)
		arguments.push_back("-nousercolor");
	arguments.push_back("-f");
	arguments.push_back("-d");
	arguments.push_back(dependLog);
	arguments.push_back("-e");
	arguments.push_back(errorLog);
	m_PipelineProcess->runConsoleTool(m_PipelineProcess->getConfig("ToolTga2Dds"), arguments);
}

void CProcessTextureDDS::build()
{
	nldebug("Build process plugin: CProcessTextureDDS");

	std::string tempDir = m_PipelineProcess->getTempDirectory();
	std::string dependLog = tempDir + "depend.log";
	std::string errorLog = tempDir + "error.log";

	{
		uint nb;
		if (m_PipelineProcess->getValueNb(nb, "Texture.DDS"))
		{
			for (uint i = 0; i < nb; ++i)
			{
				std::string root;
				{
					std::stringstream ss;
					ss << "Texture.DDS[" << i << "].";
					root = ss.str();
				}
				std::vector<std::string> srcDirectories;
				if (!m_PipelineProcess->getValues(srcDirectories, root + "SrcDirectories")) break;
				std::string dstDirectory;
				if (!m_PipelineProcess->getValue(dstDirectory, root + "DstDirectory")) break;
				std::string algorithm;
				if (!m_PipelineProcess->getValue(algorithm, root + "Algorithm")) algorithm = "";
				bool createMipMap;
				if (!m_PipelineProcess->getValue(createMipMap, root + "CreateMipMap")) createMipMap = false;
				uint reduceFactor;
				if (!m_PipelineProcess->getValue(reduceFactor, root + "ReduceFactor")) reduceFactor = 0;
				bool checkUserColor;
				if (!m_PipelineProcess->getValue(checkUserColor, root + "CheckUserColor")) checkUserColor = false;
				for (std::vector<std::string>::iterator it = srcDirectories.begin(), end = srcDirectories.end(); it != end; ++it)
				{
					std::vector<std::string> srcFiles;
					NLMISC::CPath::getPathContent(*it, false, false, true, srcFiles);
					for (std::vector<std::string>::iterator itf = srcFiles.begin(), endf = srcFiles.end(); itf != endf; ++itf)
					{
						const std::string &srcFile = *itf;
						std::string dstFile = dstDirectory + NLMISC::CFile::getFilenameWithoutExtension(NLMISC::CFile::getFilename(srcFile)) + ".dds";
						if (m_PipelineProcess->needsToBeRebuilt(srcFile, dstFile, checkUserColor)) // may have additional inputs if usercolor exists // todo: just always pass _usercolor in the srcFile here if checkUserColor, it's the clean way, but for now this is for debugging purposes done this way
						{
							m_PipelineProcess->makePaths(dstFile);
							buildDDS(dependLog, errorLog, srcFile, dstFile, algorithm, createMipMap, reduceFactor, checkUserColor);
							if (m_PipelineProcess->needsExit()) return;
							m_PipelineProcess->parseToolLog(dependLog, errorLog, false);
						}
						if (m_PipelineProcess->needsExit()) return;
					}
				}
			}
		}
	}
	
	m_PipelineProcess->deleteDirectoryIfEmpty(tempDir);
}

void CProcessTextureDDSInfo::getDependentDirectories(std::vector<std::string> &resultAppend)
{
	m_PipelineProcess->getValuesRecurse(resultAppend, "Texture.DDS[].SrcDirectories");
}

void CProcessTextureDDSInfo::getDependentFiles(std::vector<std::string> &resultAppend)
{
	
}

} /* namespace PIPELINE */

/* end of file */
