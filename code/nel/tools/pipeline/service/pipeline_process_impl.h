/**
 * \file pipeline_process_impl.h
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

#ifndef PIPELINE_PIPELINE_PROCESS_IMPL_H
#define PIPELINE_PIPELINE_PROCESS_IMPL_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/tool_logger.h>

// Project includes
#include "../plugin_library/pipeline_process.h"
#include "pipeline_workspace.h"
#include "metadata_storage.h"
#include "callback.h"

namespace PIPELINE {
	class CPipelineProject;

typedef CCallback<void, TError /* type */, const std::string & /* path */, const std::string & /* time */ , const std::string & /* error */ > TErrorLogCallback;

/**
 * \brief CPipelineProcessImpl
 * \date 2012-03-03 09:33GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineProcessImpl
 */
class CPipelineProcessImpl : public IPipelineProcess
{	
	friend class CModulePipelineSlave;

private:
	CPipelineProject *m_ActiveProject;

public:
	CPipelineProcessImpl(CPipelineProject *activeProject);
	virtual ~CPipelineProcessImpl();
	
	virtual std::string getOutputDirectory();
	virtual std::string getTempDirectory();
	virtual void deleteDirectoryIfEmpty(const std::string &path);

	virtual std::string getConfig(const std::string &name);

	virtual bool getValue(std::string &result, const std::string &name);
	virtual bool getValues(std::vector<std::string> &resultAppend, const std::string &name);
	virtual bool getValueNb(uint &result, const std::string &name);

	virtual bool needsToBeRebuilt(const std::vector<std::string> &inputPaths, const std::vector<std::string> &outputPaths, bool inputDepends);
	virtual bool needsToBeRebuilt(const std::vector<std::string> &inputPaths, bool inputDepends);

	virtual void makePaths(const std::vector<std::string> &outputPaths);

	virtual void parseToolLog(const std::string &dependLogFile, const std::string &errorLogFile, bool writeOutputMeta);

	virtual bool needsExit();
	virtual void setExit(const TProcessResult exitLevel, const std::string &exitMessage);

	virtual void runConsoleTool(const std::string &executablePath, const std::vector<std::string> &arguments);

private:
	CProcessPluginInfo m_ActivePlugin;

	CProcessResult m_ResultPreviousSuccess;
	std::map<std::string, CFileStatus> m_FileStatusInputCache;
	std::map<std::string, CFileStatus> m_FileStatusOutputCache;

	/// List of dependencies (files) that were removed, or never existed (in which case remove time is 0).
	std::map<std::string, CFileRemove> m_FileRemoveInputCache;

	/// Result of current process
	CProcessResult m_ResultCurrent;
	
	/// Result of the last subtask (usually in the taskmanager), check this after the task has completed to make sure all went fine.
	TProcessResult m_SubTaskResult;
	std::string m_SubTaskErrorMessage;
	
	std::set<std::string> m_ListInputAdded;
	std::set<std::string> m_ListInputChanged;
	std::set<std::string> m_ListInputRemoved;
	
	std::set<std::string> m_ListOutputChanged;
	std::set<std::string> m_ListOutputRemoved;

	std::set<std::string> m_ListDependentDirectories;
	std::set<std::string> m_ListDependentFiles;
	
	bool m_Aborting;
	TErrorLogCallback m_ErrorLogCallback;

	uint32 m_StatsBuild;
	uint32 m_StatsSkip;
	uint32 m_StatsInvalid;

private:
	bool getDependencyFileStatusCached(CFileStatus &fileStatus, const std::string &filePath);
	bool getDependencyFileStatusLatest(CFileStatus &fileStatus, const std::string &filePath);
	bool isFileDependency(const std::string &path);
	bool isDirectoryDependency(const std::string &path);
	bool hasInputDirectoryBeenModified(const std::string &inputDirectory);
	bool hasInputFileBeenModified(const std::string &inputFile);
	bool haveFilesBeenAddedInDirectorySince(const std::string &inputDirectory, const std::set<std::string> &excludeFiles, uint32 since);
	bool hasFileBeenAddedSince(const std::string &inputFile, uint32 since);
	bool needsToBeRebuildSubByOutput(const std::vector<std::string> &inputPaths, bool inputChanged, bool inputDepends);
	bool needsToBeRebuiltSub(const std::vector<std::string> &inputPaths, const std::vector<std::string> &outputPaths, bool inputModified, bool inputDepends);

}; /* class CPipelineProcessImpl */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_PROCESS_IMPL_H */

/* end of file */
