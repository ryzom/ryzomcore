/**
 * \file pipeline_process.h
 * \brief IPipelineProcess
 * \date 2012-03-03 09:22GMT
 * \author Jan Boon (Kaetemi)
 * IPipelineProcess
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

#ifndef PIPELINE_PIPELINE_PROCESS_H
#define PIPELINE_PIPELINE_PROCESS_H
#include <nel/misc/types_nl.h>

// STL includes
#include <vector>
#include <string>

// NeL includes

// Project includes

namespace PIPELINE {

enum TProcessResult
{
	/// Invalid state.
	FINISH_NOT = 0,

	/// Successfully built.
	FINISH_SUCCESS = 1,

	/// Built successfully with warnings.
	FINISH_WARNING = 2,

	/// Build failed.
	FINISH_ERROR = 3,

	/// Task aborted by slave. For internal usage only.
	FINISH_ABORT = 4, 

	/// Task rejected by slave. For internal usage only.
	FINISH_REJECT = 5, 
};

/**
 * \brief IPipelineProcess
 * \date 2012-03-03 09:22GMT
 * \author Jan Boon (Kaetemi)
 * IPipelineProcess
 */
class IPipelineProcess
{
public:
	IPipelineProcess() { }
	virtual ~IPipelineProcess() { }
	
	static IPipelineProcess *getInstance();
	
	// ***************** PROCESS FUNCTIONS (EASILY EXPOSABLE TO SCRIPTS ETCETERA) *****************
	
	/// Gets the output directory for the project.
	virtual std::string getOutputDirectory() = 0;
	
	/// Get the temporary directory for the current process. The directory must be deleted when the process ends. May return random temporary directories if no process is running.
	virtual std::string getTempDirectory() = 0;
	
	/// Get a value from the currently active project configuration. If false, don't use, no need to write warnings to service log, already written, set exit state and exit if necessary
	virtual bool getValue(std::string &result, const std::string &name) = 0;
	virtual bool getValues(std::vector<std::string> &resultAppend, const std::string &name) = 0;
	virtual bool getValueNb(uint &result, const std::string &name) = 0;

	/// Find out if the plugin needs to rebuild. Input can be files or directories, output can only be files
	virtual bool needsToBeRebuilt(const std::vector<std::string> &inputPaths, const std::vector<std::string> &outputPaths) = 0;
	bool needsToBeRebuilt(const std::vector<std::string> &inputPaths, const std::string &outputPath) { std::vector<std::string> outputPaths; outputPaths.push_back(outputPath); return needsToBeRebuilt(inputPaths, outputPaths); }
	bool needsToBeRebuilt(const std::string &inputPath, const std::vector<std::string> &outputPaths) { std::vector<std::string> inputPaths; inputPaths.push_back(inputPath); return needsToBeRebuilt(inputPaths, outputPaths); }
	bool needsToBeRebuilt(const std::string &inputPath, const std::string &outputPath) { std::vector<std::string> inputPaths; inputPaths.push_back(inputPath); std::vector<std::string> outputPaths; outputPaths.push_back(outputPath); return needsToBeRebuilt(inputPaths, outputPaths); }
	/// Find out if the plugin needs to rebuild. Input can only be files. Must request the service to write an .output metafile during depend log parsing.
	virtual bool needsToBeRebuilt(const std::vector<std::string> &inputPaths) = 0;
	bool needsToBeRebuilt(const std::string &inputPath) { std::vector<std::string> inputPaths; inputPaths.push_back(inputPath); return needsToBeRebuilt(inputPaths); }

	/// Create directories for paths. Call for output paths
	virtual void makePaths(const std::vector<std::string> &outputPaths) = 0;
	void makePaths(const std::string &outputPath) { std::vector<std::string> outputPaths; outputPaths.push_back(outputPath); makePaths(outputPaths); }

	/// Parse the depend and error logs. Only set writeOutputMeta true if the output is not known in advance by the plugin, see needsToBeRebuilt
	virtual void parseToolLog(const std::string &dependLogFile, const std::string &errorLogFile, bool writeOutputMeta) = 0;

	/// Check if the plugin needs to exit, exit plugin immediately if true.
	virtual bool needsExit() = 0;
	/// Set the exit state, must exit the plugin immediately afterwards. Use for configuration mistakes, etc
	virtual void setExit(const TProcessResult exitLevel, const std::string &exitMessage) = 0;

}; /* class IPipelineProcess */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_PROCESS_H */

/* end of file */
