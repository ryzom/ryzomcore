/**
 * \file process_info.h
 * \brief IProcessInfo
 * \date 2012-03-03 10:14GMT
 * \author Jan Boon (Kaetemi)
 * IProcessInfo
 * A process info is executed by the PLS MASTER to prepare a process handler to be run.
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

#ifndef PIPELINE_PROCESS_INFO_H
#define PIPELINE_PROCESS_INFO_H
#include <nel/misc/types_nl.h>

// STL includes
#include <vector>
#include <string>

// NeL includes
#include <nel/misc/class_registry.h>

// Project includes
#include "pipeline_process.h"

namespace PIPELINE {

/**
 * \brief IProcessInfo
 * \date 2012-03-03 10:14GMT
 * \author Jan Boon (Kaetemi)
 * IProcessInfo
 */
class IProcessInfo : public NLMISC::IClassable
{
protected:
	IPipelineProcess *m_PipelineProcess;

public:
	IProcessInfo() : m_PipelineProcess(IPipelineProcess::getInstance()) { }
	virtual ~IProcessInfo() { }	
	
	void setPipelineProcess(IPipelineProcess *pipelineProcess) { m_PipelineProcess = pipelineProcess; }
	
	/// Dependency information used to store the initial state of files on which the process depends.
	/// A process handler is not allowed to depend on any files it does not list here.
	
	/// Must return all directories on which the process handler depends, these are NOT RECURSIVE.
	virtual void getDependentDirectories(std::vector<std::string> &result) = 0;	
	/// Must return all files on which the process handler depends, ONLY if these are not in dependent directories.
	virtual void getDependentFiles(std::vector<std::string> &result) = 0;

}; /* class IProcessInfo */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PROCESS_INFO_H */

/* end of file */
