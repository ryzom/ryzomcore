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
#include <nel/misc/path.h>

// Project includes
#include "pipeline_service.h"
#include "pipeline_project.h"
#include "database_status.h"

using namespace std;
using namespace NLMISC;

namespace PIPELINE {

// If this returns false, the status is not sane, and the build must error; or the file does not exist and must error or warn.
// Gets the file status as it was known at the beginning of the build or when this file was first known.
bool CPipelineProcessImpl::getDependencyFileStatusCached(CFileStatus &fileStatus, const std::string &filePath) // not by macro path :)
{
	std::map<std::string, CFileStatus>::iterator statusIt = m_FileStatusInputCache.find(filePath);
	if (statusIt == m_FileStatusInputCache.end())
	{
		/*nldebug("File status '%s' not requested before, ensure this is not a dependency", filePath.c_str());
		if (g_DatabaseStatus->getFileStatus(fileStatus, filePath))
		{
			m_FileStatusCache[filePath] = fileStatus;
			return true;
		}
		else*/ return false;
	}
	fileStatus = statusIt->second; // copy
	return true;
}

// If this returns false, the status is not sane, and the build must error; or the file does not exist and must error or warn.
bool CPipelineProcessImpl::getDependencyFileStatusLatest(CFileStatus &fileStatus, const std::string &filePath)
{
	return g_DatabaseStatus->getFileStatus(fileStatus, filePath);
}

bool CPipelineProcessImpl::isFileDependency(const std::string &path)
{
	if (m_ListDependentFiles.find(path) == m_ListDependentFiles.end()) // check if it's directly in the dependent files
	{
		// Not found!
		std::string pathParent = path.substr(0, path.find_last_of('/') + 1);
		if (m_ListDependentDirectories.find(pathParent) == m_ListDependentDirectories.end()) // check if it's inside one of the dependent directories
		{
			// Still not found!
			return false;
		}
	}
	return true;
} // ok

bool CPipelineProcessImpl::isDirectoryDependency(const std::string &path)
{
	return m_ListDependentDirectories.find(path) != m_ListDependentDirectories.end();
} // ok

bool CPipelineProcessImpl::hasInputDirectoryBeenModified(const std::string &inputDirectory)
{
	// Check if any files added/changed/removed are part of this directory (slow).
	for (std::set<std::string>::const_iterator itr = m_ListInputAdded.begin(), endr = m_ListInputAdded.end(); itr != endr; ++itr)
	{
		const std::string &pathr = *itr;
		if ((pathr.size() > inputDirectory.size())
			&& (pathr.substr(0, inputDirectory.size()) == inputDirectory) // inside the path
			&& (pathr.substr(inputDirectory.size(), pathr.size() - inputDirectory.size())).find('/') == std::string::npos) // not in a further subdirectory 
		{
			nldebug("Found added '%s' in dependency directory '%s'", pathr.c_str(), inputDirectory.c_str());
			return true;
		}
	}
	for (std::set<std::string>::const_iterator itr = m_ListInputChanged.begin(), endr = m_ListInputChanged.end(); itr != endr; ++itr)
	{
		const std::string &pathr = *itr;
		if ((pathr.size() > inputDirectory.size())
			&& (pathr.substr(0, inputDirectory.size()) == inputDirectory) // inside the path
			&& (pathr.substr(inputDirectory.size(), pathr.size() - inputDirectory.size())).find('/') == std::string::npos) // not in a further subdirectory 
		{
			nldebug("Found changed '%s' in dependency directory '%s'", pathr.c_str(), inputDirectory.c_str());
			return true;
		}
	}
	for (std::set<std::string>::const_iterator itr = m_ListInputRemoved.begin(), endr = m_ListInputRemoved.end(); itr != endr; ++itr)
	{
		const std::string &pathr = *itr;
		if ((pathr.size() > inputDirectory.size())
			&& (pathr.substr(0, inputDirectory.size()) == inputDirectory) // inside the path
			&& (pathr.substr(inputDirectory.size(), pathr.size() - inputDirectory.size())).find('/') == std::string::npos) // not in a further subdirectory 
		{
			nldebug("Found removed '%s' in dependency directory '%s'", pathr.c_str(), inputDirectory.c_str());
			return true;
		}
	}
	// nldebug("Input directory '%s' not modified", inputDirectory.c_str());
	return false;
} // ok

bool CPipelineProcessImpl::hasInputFileBeenModified(const std::string &inputFile)
{
	return m_ListInputAdded.find(inputFile) != m_ListInputAdded.end()
		|| m_ListInputChanged.find(inputFile) != m_ListInputChanged.end()
		|| m_ListInputRemoved.find(inputFile) != m_ListInputRemoved.end();
} // ok

bool CPipelineProcessImpl::haveFilesBeenAddedInDirectorySince(const std::string &inputDirectory, const std::set<std::string> &excludeFiles, uint32 since)
{
	for (std::set<std::string>::const_iterator itr = m_ListInputAdded.begin(), endr = m_ListInputAdded.end(); itr != endr; ++itr)
	{
		const std::string &pathr = *itr;
		if (excludeFiles.find(pathr) == excludeFiles.end())
		{
			if ((pathr.size() > inputDirectory.size())
				&& (pathr.substr(0, inputDirectory.size()) == inputDirectory) // inside the path
				&& (pathr.substr(inputDirectory.size(), pathr.size() - inputDirectory.size())).find('/') == std::string::npos) // not in a further subdirectory 
			{
				CFileStatus status;
				if (!getDependencyFileStatusCached(status, pathr))
					nlerror("Cannot get cached status of known file");
				if (status.FirstSeen >= since) // or > ?
				{
					nldebug("Found newly added '%s' in dependency directory '%s'", pathr.c_str(), inputDirectory.c_str());
					return true;
				}
			}
		}
	}
	return false;
}

bool CPipelineProcessImpl::hasFileBeenAddedSince(const std::string &inputFile, uint32 since)
{
	if (m_ListInputAdded.find(inputFile) != m_ListInputAdded.end())
	{
		CFileStatus status;
		if (!getDependencyFileStatusCached(status, inputFile))
			nlerror("Cannot get cached status of known file");
		if (status.FirstSeen >= since) // or > ?
		{
			nldebug("Found newly added '%s' in dependency files", inputFile.c_str());
			return true;
		}
	}
	return false;
}

bool CPipelineProcessImpl::needsToBeRebuilt(const std::vector<std::string> &inputPaths)
{
	if (m_SubTaskResult != FINISH_SUCCESS)
		return false; // Cannot continue on previous failure.

	m_SubTaskResult = FINISH_NOT;

	// Sanity check
	for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		if (path[path.size() - 1] == '/') // isDirectory
		{
			m_SubTaskResult = FINISH_ERROR;
			m_SubTaskErrorMessage = std::string("Input file '") + path + "' cannot be a directory";
			return false; // Error, cannot rebuild.
		}
		if (!isFileDependency(path))
		{
			m_SubTaskResult = FINISH_ERROR;
			m_SubTaskErrorMessage = std::string("File '") + path + "' is not part of the dependencies";
			return false; // Error, cannot rebuild.
		}
	}

	// Find out if anything happened to the input files.
	bool inputFilesDifferent = false;
	for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		if (hasInputFileBeenModified(path))
		{
			// Found!
			nldebug("Found added/changed/removed input file '%s'", path.c_str());
			inputFilesDifferent = true;
			break;
		}
	}
	if (!inputFilesDifferent)
	{
		nldebug("No added/changed/removed input files in this request");
		if (m_ListOutputChanged.size() == 0 && m_ListOutputRemoved.size() == 0)
		{
			nldebug("No output files were tampered with since last successful build, rebuild not needed");
			m_SubTaskResult = FINISH_SUCCESS;
			return false; // No rebuild required.
		}
		else
		{
			nldebug("Output files may have changed, find out which output files are part of these input files");
			m_SubTaskResult = FINISH_SUCCESS;
			return needsToBeRebuildSubByOutput(inputPaths, false);
		}
	}
	else // input files have changed
	{
		// Find out if all the input files have a .output file
		bool allInputFilesHaveOutputFile = true;
		for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			std::string outputMetaPath = CMetadataStorage::getOutputPath(path, m_ActiveProject->getName(), m_ActivePlugin.Handler);
			if (!CFile::fileExists(outputMetaPath))
			{
				nldebug("Found an input file that has no .output meta file");
				allInputFilesHaveOutputFile = false;
				break;
			}
		}
		if (!allInputFilesHaveOutputFile)
		{
			nldebug("Not all input files have an .output files, rebuild");
			m_SubTaskResult = FINISH_SUCCESS;
			return true; // Rebuild
		}
		else
		{
			nldebug("Output files may or may not be up to date, find out more, after the break");
			m_SubTaskResult = FINISH_SUCCESS;
			return needsToBeRebuildSubByOutput(inputPaths, true);
		}
	}
	// not reachable
} // ok

bool CPipelineProcessImpl::needsToBeRebuildSubByOutput(const std::vector<std::string> &inputPaths, bool inputChanged)
{
	if (m_SubTaskResult != FINISH_SUCCESS)
		return false; // Cannot continue on previous failure.
	
	m_SubTaskResult = FINISH_NOT;

	std::vector<std::string> outputPaths;

	bool needsFurtherInfo = false;

	// For each inputPath
	for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		// FIXME: What if an input file does not exist? Deleted long ago? Just print a warning (& notify terminal), keep calm, and carry on.
		nlassert(!CFile::isDirectory(path)); // All input are files! Coding error otherwise, because should already be checked.
		std::string metaOutputPath = CMetadataStorage::getOutputPath(path, m_ActiveProject->getName(), m_ActivePlugin.Handler);
		nlassert(CFile::fileExists(metaOutputPath)); // Coding error otherwise, already must have checked beforehand that they all exist.

		// Read the .output file
		CFileOutput metaOutput;
		CMetadataStorage::readOutput(metaOutput, metaOutputPath);
		std::vector<std::string> localOutputPaths;
		localOutputPaths.reserve(metaOutput.MacroPaths.size());
		for (std::vector<std::string>::const_iterator it = metaOutput.MacroPaths.begin(), end = metaOutput.MacroPaths.end(); it != end; ++it)
			localOutputPaths.push_back(unMacroPath(*it));

		// If inputChanged & hasInputFileBeenModified(path)
		if (inputChanged && hasInputFileBeenModified(path))
		{
			// If .output file was empty
			if (metaOutput.MacroPaths.empty())
			{
				// Require rebuild because we don't know if there's new output
				nldebug("Input file '%s' has output file with no output files, state not known, so rebuild", path.c_str());
				m_SubTaskResult = FINISH_SUCCESS;
				return true; // Rebuild.
			}
		}
		
		// Copy the .output file into the output paths
		for (std::vector<std::string>::const_iterator it = localOutputPaths.begin(), end = localOutputPaths.end(); it != end; ++it)
			outputPaths.push_back(*it);
		// End
	}

	// Output files may have already been built by a failed build,
	// or they might have been tampered with after they were built.
	nldebug("Need further information");
	m_SubTaskResult = FINISH_SUCCESS;
	return needsToBeRebuiltSub(inputPaths, outputPaths, inputChanged); // to skip input changed checks because these are only input files and no input folders
} // ok ? ?

bool CPipelineProcessImpl::needsToBeRebuilt(const std::vector<std::string> &inputPaths, const std::vector<std::string> &outputPaths)
{
	if (m_SubTaskResult != FINISH_SUCCESS)
		return false; // Cannot continue on previous failure.
	
	m_SubTaskResult = FINISH_NOT;

	// Sanity check of all the input paths, 
	// they must be either files that are dependency files or inside dependency directories
	// or directories that are dependency directories.
	for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		if (path[path.size() - 1] == '/') // isDirectory
		{
			// Check if this directory is in the dependencies.
			if (!isDirectoryDependency(path))
			{
				m_SubTaskResult = FINISH_ERROR;
				m_SubTaskErrorMessage = std::string("Directory '") + path + "' is not part of the dependencies";
				return false; // Error, cannot rebuild.
			}
		}
		else // isFile
		{
			// Check if this file is in the dependencies.
			if (!isFileDependency(path))
			{
				m_SubTaskResult = FINISH_ERROR;
				m_SubTaskErrorMessage = std::string("File '") + path + "' is not part of the dependencies";
				return false; // Error, cannot rebuild.
			}
		}
	}
	
	// Check if any of the input has changed
	bool inputFilesDifferent = false;
	for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		if (path[path.size() - 1] == '/') // isDirectory
		{
			if (hasInputDirectoryBeenModified(path))
			{
				nldebug("Found modified input directory '%s'", path.c_str());
				inputFilesDifferent = true;
				break;
			}
		}
		else
		{
			if (hasInputFileBeenModified(path))
			{
				nldebug("Found modified input file '%s'", path.c_str());
				inputFilesDifferent = true;
				break;
			}
		}
	}

	if (inputFilesDifferent)
	{
		nldebug("Input files were modified, check if output files were already built");
		m_SubTaskResult = FINISH_SUCCESS;
		return needsToBeRebuiltSub(inputPaths, outputPaths, true);
	}
	else
	{
		nldebug("Input files were not modified");
		if (m_ListOutputChanged.size() == 0 && m_ListOutputRemoved.size() == 0)
		{
			nldebug("No output files were tampered with since last successful build, rebuild not needed");
			m_SubTaskResult = FINISH_SUCCESS;
			return false; // No rebuild required.
		}
		else
		{
			nldebug("Output files may have changed, find out more");
			m_SubTaskResult = FINISH_SUCCESS;
			return needsToBeRebuiltSub(inputPaths, outputPaths, false);
		}
	}
}

bool CPipelineProcessImpl::needsToBeRebuiltSub(const std::vector<std::string> &inputPaths, const std::vector<std::string> &outputPaths, bool inputModified)
{
	if (m_SubTaskResult != FINISH_SUCCESS)
		return false; // Cannot continue on previous failure.
	
	m_SubTaskResult = FINISH_NOT;

	// Sanity check of all the output paths
	// These must all be files, no directories allowed
	for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		if (path[path.size() - 1] == '/') // isDirectory
		{
			m_SubTaskResult = FINISH_ERROR;
			m_SubTaskErrorMessage = std::string("Output file '") + path + "' cannot be a directory";
			return false; // Error, cannot rebuild.
		}
	}

	// Check if any of the output paths are part of the removed output files
	for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		if (m_ListOutputRemoved.find(*it) != m_ListOutputRemoved.end())
		{
			// If so, rebuild
			nldebug("Output file '%s' has been removed, rebuild", path.c_str());
			m_SubTaskResult = FINISH_SUCCESS;
			return true; // Rebuild.
		}
	}

	// Check if any of the output files don't exist
	for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
	{
		// If so, rebuild
		const std::string &path = *it;
		if (!CFile::isExists(path))
		{
			// If so, rebuild
			nldebug("Output file '%s' does not exist, rebuild", path.c_str());
			m_SubTaskResult = FINISH_SUCCESS;
			return true; // Rebuild.
		}
	}
	
	// Check if any of the output paths are part of the changed output files
	bool outputChanged = false; 
	for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		if (m_ListOutputChanged.find(*it) != m_ListOutputChanged.end())
		{
			nldebug("Output file '%s' has been changed", path.c_str());
			outputChanged = true;
			break;
		}
	}

	if (!outputChanged && !inputModified)
	{
		nlerror("Should never reach this, this must have been cought earlier normally");
	}

	std::set<std::string> inputsChecked;
	uint32 earliestBuildStart = 0xFFFFFFFF;
	
	// Check the .depend files of all the output files // also check that they exist :)
	for (std::vector<std::string>::const_iterator it = outputPaths.begin(), end = outputPaths.end(); it != end; ++it)
	{
		const std::string &path = *it;
		std::string metaDependPath = CMetadataStorage::getDependPath(path);
		CFileDepend metaDepend;
		if (!CMetadataStorage::readDepend(metaDepend, metaDependPath))
		{
			nlwarning("Depend file for existing output '%s' does not exist, this should not happen, rebuild", path.c_str());
			m_SubTaskResult = FINISH_SUCCESS;
			return true; // Rebuild.
		}
		else
		{
			if (metaDepend.BuildStart < earliestBuildStart)
				earliestBuildStart = metaDepend.BuildStart;
			if (outputChanged)
			{
				// Compare the output checksum with the status output checksum
				CFileStatus metaStatus;
				if (!g_DatabaseStatus->updateFileStatus(metaStatus, path))
				{
					// FIXME: Is it really necessary to recalculate the crc32 if the status file is broken for an output file? Useful though for some rare cases.
					m_SubTaskResult = FINISH_ERROR;
					m_SubTaskErrorMessage = std::string("Could not get status for output file '") + path + "', this should never happen at all, coding error";
					return false; // Error, cannot rebuild.
				}
				else
				{
					if (metaStatus.CRC32 != metaDepend.CRC32)
					{
						nlwarning("Status checksum %i for output file '%s' does match depend checksum %i, output file was modified, this should not happen, rebuild", metaStatus.CRC32, path.c_str(), metaDepend.CRC32);
						m_SubTaskResult = FINISH_SUCCESS;
						return true; // Rebuild.
					}
				}
			}
			if (inputModified)
			{
				// Compare the input checksums with the cached input checksums
				for (std::vector<CFileDepend::CDependency>::const_iterator itd = metaDepend.Dependencies.begin(), endd = metaDepend.Dependencies.end(); itd != endd; ++itd)
				{
					const CFileDepend::CDependency &dependency = *itd;
					std::string dependencyFile = unMacroPath(dependency.MacroPath);
					CFileStatus metaStatus;
					if (!getDependencyFileStatusCached(metaStatus, dependencyFile))
					{
						nlwarning("Output file '%s' depends on unknown file '%s', rebuild", path.c_str(), dependencyFile.c_str());
						m_SubTaskResult = FINISH_SUCCESS;
						return true; // Rebuild.
					}
					else
					{
						if (metaStatus.CRC32 != dependency.CRC32)
						{
							nldebug("Status checksum %i for input file '%s' does match depend checksum %i, input file was modified, rebuild", metaStatus.CRC32, dependencyFile.c_str(), dependency.CRC32);
							m_SubTaskResult = FINISH_SUCCESS;
							return true; // Rebuild.
						}
					}
					inputsChecked.insert(dependencyFile);
				}
			}
		}
	}

	// Find out if any files were added in dependency directories since last build start
	if (inputModified)
	{
		// TODO: ONLY CHECK INPUT PATHS GIVEN BY DEPEND META FILES
		for (std::vector<std::string>::const_iterator it = inputPaths.begin(), end = inputPaths.end(); it != end; ++it)
		{
			const std::string &path = *it;
			if (path[path.size() - 1] == '/') // isDirectory
			{
				if (haveFilesBeenAddedInDirectorySince(path, inputsChecked, earliestBuildStart))
				{
					nldebug("Found a file added after last build start in a dependency directory that is not known by the depend files, rebuild");
					m_SubTaskResult = FINISH_SUCCESS;
					return true; // Rebuild.
				}
			}
			else // isFile
			{
				if (inputsChecked.find(path) == inputsChecked.end())
				{
					if (hasFileBeenAddedSince(path, earliestBuildStart))
					{
						nldebug("Found a dependency file added after last build start that is not known by the depend files, rebuild");
						m_SubTaskResult = FINISH_SUCCESS;
						return true; // Rebuild.
					}
				}
			}
		}
	}

	// (if any checksum was different, require rebuild, if no checksums were different, no rebuild is needed)
	nldebug("No differences found, no rebuild needed");
	m_SubTaskResult = FINISH_SUCCESS;
	return false; // Rebuild not necessary.
}

/// Set the exit message, exit the plugin immediately afterwards.
void CPipelineProcessImpl::setExit(const TProcessResult exitLevel, const std::string &exitMessage)
{
	m_SubTaskResult = exitLevel;
	m_SubTaskErrorMessage = exitMessage;
}

/// Must verify this regularly to exit the plugin in case something went wrong.
bool CPipelineProcessImpl::needsExit()
{
	if (g_IsExiting || m_Aborting)
		return true;
	
	// TODO: Bypass error feature.
	if (m_SubTaskResult != FINISH_SUCCESS) // m_SubTaskResult can only be FINISH_SUCCESS or FINISH_ERROR (or FINISH_NOT in case of incomplete implementation)
		return true;
	return false;
}

} /* namespace PIPELINE */

/* end of file */
