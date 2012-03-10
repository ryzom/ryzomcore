/**
 * \file pipeline_service.h
 * \brief CPipelineService
 * \date 2012-02-18 17:25GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineService
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

#ifndef PIPELINE_PIPELINE_SERVICE_H
#define PIPELINE_PIPELINE_SERVICE_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>

// NeL includes

// Project includes

namespace NLMISC {
	class IRunnable;
}

namespace NLGEORGES {
	class UFormLoader;
}

namespace PIPELINE {
	class CPipelineWorkspace;
	class CDatabaseStatus;
	class CPipelineInterfaceImpl;

#define PIPELINE_MASTER

extern bool g_IsExiting;
extern bool g_IsMaster;

extern std::string g_WorkDir;

#define PIPELINE_DIRECTORY_UNKNOWN_PREFIX "unknown."
#define PIPELINE_DIRECTORY_PREFIX_PROJECT "project."
#define PIPELINE_DIRECTORY_PREFIX_ROOT "root."
#define PIPELINE_DIRECTORY_PREFIX_SHEET "pipeline."

#define PIPELINE_DIRECTORY_TEMP_SUFFIX ".temp"

/// Unmacros a path, and standardizes it as well.
std::string unMacroPath(const std::string &path);

/// Macros a path, and standardizes it in advance.
std::string macroPath(const std::string &path);

bool tryRunnableTask(const std::string &stateName, NLMISC::IRunnable *task);
void endedRunnableTask();

bool tryDirectTask(const std::string &stateName);
void endedDirectTask();

bool tryBuildReady();
void endedBuildReady();

bool tryBuildReadyMaster();
void endedBuildReadyMaster();

bool tryBuildProcess(const std::string &stateName);
void endedBuildProcess();

/// Only use for informational purposes, service may not be idle after calling this. Not for thread safe usage.
bool isServiceStateIdle();

bool reloadSheets();

extern NLGEORGES::UFormLoader *g_FormLoader;
extern CPipelineWorkspace *g_PipelineWorkspace;
extern CDatabaseStatus *g_DatabaseStatus;
extern CPipelineInterfaceImpl *g_PipelineInterfaceImpl;

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_SERVICE_H */

/* end of file */
