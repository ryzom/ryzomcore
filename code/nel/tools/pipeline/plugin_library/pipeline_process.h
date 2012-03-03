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

// NeL includes

// Project includes

namespace PIPELINE {

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
	
	/// Get a value from the currently active project configuration
	virtual bool getValue(std::string &result, const std::string &name) = 0;
	virtual bool getValues(std::vector<std::string> &resultAppend, const std::string &name) = 0;
	virtual bool getValueNb(uint &result, const std::string &name) = 0;
}; /* class IPipelineProcess */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_PROCESS_H */

/* end of file */
