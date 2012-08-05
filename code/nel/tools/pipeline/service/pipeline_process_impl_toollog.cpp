/**
 * \file pipeline_process_impl.cpp
 * \brief CPipelineProcessImpl
 * \date 2012-08-03 18:22GMT
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
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/tool_logger.h>

// Project includes
#include "pipeline_service.h"
#include "pipeline_project.h"
#include "metadata_storage.h"
#include "database_status.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

namespace {
	
void readTabbedLine(std::vector<std::string> &tabbedLine, NLMISC::CIFile &file)
{
	char r;
	file.serial(r);
	char buffer[4096];
	ptrdiff_t idx = 0;
	while (r != '\n')
	{
		switch (r)
		{
		case '\r':
			break;
		case '\t':
			buffer[idx] = '\0';
			tabbedLine.push_back(std::string(buffer, idx));
			idx = 0;
			break;
		default:
			buffer[idx] = r;
			++idx;
			if (idx >= 4096)
				return; // bad
			break;
		}
		if (file.eof())
			break;
		file.serial(r);
	}
	buffer[idx] = '\0';
	tabbedLine.push_back(std::string(buffer, idx));
}

} /* anonymous namespace */

void CPipelineProcessImpl::parseToolLog(const std::string &dependLogFile, const std::string &errorLogFile, bool writeOutputMeta)
{
	m_SubTaskResult = FINISH_NOT;

	if (!NLMISC::CFile::fileExists(dependLogFile))
	{
		m_SubTaskErrorMessage = "Depend log does not exist";
		m_SubTaskResult = FINISH_ERROR;
		return;
	}

	if (!NLMISC::CFile::fileExists(errorLogFile))
	{
		m_SubTaskErrorMessage = "Error log does not exist";
		m_SubTaskResult = FINISH_ERROR;
		return;
	}

	// Parse error log
	{
		// ...
	}

	// Parse depend log
	{
		std::map<std::string, CFileDepend> metaDepends;
		std::map<std::string, CFileOutput> metaOutputs;
		std::map<std::string, CFileStatus> statusCache;
		
		NLMISC::CIFile file;
		file.open(dependLogFile, true);

		std::vector<std::string> tabbedLine;
		readTabbedLine(tabbedLine, file);
		uint line = 0;

		if (tabbedLine.size() != 3 || tabbedLine[0] != "type" || tabbedLine[1] != "output_file" || tabbedLine[2] != "input_file")
		{
			m_SubTaskErrorMessage = "Bad depend file format";
			m_SubTaskResult = FINISH_ERROR;
			file.close();
			return;
		}
		
		for (; ; )
		{
			if (file.eof())
				break;
			tabbedLine.clear();
			readTabbedLine(tabbedLine, file);
			++line;
			if (tabbedLine.size() == 0)
				continue;
			if (tabbedLine.size() != 3)
			{
				std::stringstream ss;
				ss << "Bad line " << line << " in depend file";
				m_SubTaskErrorMessage = ss.str();
				m_SubTaskResult = FINISH_ERROR;
				file.close();
				return;
			}
			// Read line
			{
				TDepend type;
				if (tabbedLine[0] == "BUILD")
					type = BUILD;
				else if (tabbedLine[0] == "DIRECTORY")
					type = DIRECTORY;
				else if (tabbedLine[0] == "RUNTIME")
					type = RUNTIME;
				else
				{
					std::stringstream ss;
					ss << "Invalid type at line " << line << " in depend file";
					m_SubTaskErrorMessage = ss.str();
					m_SubTaskResult = FINISH_ERROR;
					file.close();
					return;
				}
				std::string outputFile = standardizePath(tabbedLine[1], false);
				// std::string outputFileMacro = macroPath(outputFile);
				std::string inputFile = standardizePath(tabbedLine[2], type == DIRECTORY);
				std::string inputFileMacro = macroPath(inputFile);
				std::map<std::string, CFileDepend>::iterator metaDependIt = metaDepends.find(outputFile);
				if (metaDependIt == metaDepends.end())
				{
					metaDepends[outputFile] = CFileDepend();
					metaDependIt = metaDepends.find(outputFile);
					if (outputFile != "*")
					{
						CFileStatus status;
						nldebug("Update status for output file '%s', calculate checksum", outputFile.c_str());
						g_DatabaseStatus->updateFileStatus(status, outputFile); // calculate the checksum of the output, this takes a while
						metaDependIt->second.CRC32 = status.CRC32;
						// nldebug("Checksum %i; %i", metaDependIt->second.CRC32, status.CRC32);
						m_FileStatusOutputCache[outputFile] = status;
					}
				}
				switch (type)
				{
				case BUILD:
					{
						CFileDepend::CDependency dependency;
						dependency.MacroPath = inputFileMacro;
						std::map<std::string, CFileStatus>::iterator statusIt = statusCache.find(inputFile);
						if (statusIt == statusCache.end())
						{
							if (!isFileDependency(inputFile))
							{
								m_SubTaskErrorMessage = std::string("Invalid dependency '") + inputFile + "'";
								m_SubTaskResult = FINISH_ERROR;
								file.close();
								return;
							}
							CFileStatus statusOriginal;
							if (!getDependencyFileStatusCached(statusOriginal, inputFile))
							{
								m_SubTaskErrorMessage = std::string("Cached status for '") + inputFile + "' does not exist, this may be a programming error";
								m_SubTaskResult = FINISH_ERROR;
								file.close();
								return;
							}
							CFileStatus status;
							if (!getDependencyFileStatusLatest(status, inputFile))
							{
								m_SubTaskErrorMessage = std::string("Invalid status for '") + inputFile + "', file may have changed during build";
								m_SubTaskResult = FINISH_ERROR;
								file.close();
								return;
							}
							if (statusOriginal.CRC32 != status.CRC32)
							{
								m_SubTaskErrorMessage = std::string("Status checksums changed for '") + inputFile + "', file has changed during build";
								m_SubTaskResult = FINISH_ERROR;
								file.close();
								return;
							}
							statusCache[inputFile] = status;
							statusIt = statusCache.find(inputFile);
						}
						dependency.CRC32 = statusIt->second.CRC32;
						metaDependIt->second.Dependencies.push_back(dependency);
					}
					if (writeOutputMeta)
					{
						// TODO ## OUTPUT META ##
					}
					break;
				case DIRECTORY:
					metaDependIt->second.DirectoryDependencies.push_back(inputFileMacro);
					break;
				case RUNTIME:
					metaDependIt->second.RuntimeDependencies.push_back(inputFileMacro);
					break;
				}
			}
		}

		file.close();

		// Allow wildcard in output parameter in depend log, use when input includes other files to depend all the outputs on the included files as well
		std::map<std::string, CFileDepend>::iterator wildcard_it = metaDepends.find("*");
		if (wildcard_it != metaDepends.end())
		{
			for (std::map<std::string, CFileDepend>::iterator it = metaDepends.begin(), end = metaDepends.end(); it != end; ++it)
			{
				if (it->first != "*")
				{
					for (std::vector<CFileDepend::CDependency>::iterator sub_it = wildcard_it->second.Dependencies.begin(), sub_end = wildcard_it->second.Dependencies.end(); sub_it != sub_end; ++sub_it)
						it->second.Dependencies.push_back(*sub_it);
					for (std::vector<std::string>::iterator sub_it = wildcard_it->second.DirectoryDependencies.begin(), sub_end = wildcard_it->second.DirectoryDependencies.end(); sub_it != sub_end; ++sub_it)
						it->second.DirectoryDependencies.push_back(*sub_it);
					for (std::vector<std::string>::iterator sub_it = wildcard_it->second.RuntimeDependencies.begin(), sub_end = wildcard_it->second.RuntimeDependencies.end(); sub_it != sub_end; ++sub_it)
						it->second.RuntimeDependencies.push_back(*sub_it);
				}
			}
			metaDepends.erase(wildcard_it);
		}

		// Write depend meta files
		for (std::map<std::string, CFileDepend>::iterator it = metaDepends.begin(), end = metaDepends.end(); it != end; ++it)
		{
			const CFileDepend &depend = it->second;
			CMetadataStorage::writeDepend(depend, CMetadataStorage::getDependPath(it->first));
		}

		// Write output meta files
		if (writeOutputMeta)
		{
			// TODO ## OUTPUT META ##
		}
	}
	
	NLMISC::CFile::deleteFile(dependLogFile);
	
	// m_SubTaskErrorMessage = "Log parsing not implemented, goodbye";
	// m_SubTaskResult = FINISH_ERROR;
	m_SubTaskResult = FINISH_SUCCESS;
}

} /* namespace PIPELINE */

/* end of file */
