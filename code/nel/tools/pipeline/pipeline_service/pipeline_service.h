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

namespace PIPELINE {

#if defined(PIPELINE_MASTER)
#	if defined(PIPELINE_SLAVE)
#		error Cannot define both PIPELINE_MASTER and PIPELINE_SLAVE at the same time.
#	endif
#elif defined (PIPELINE_SLAVE)
#else
#	error Must define either PIPELINE_MASTER or PIPELINE_SLAVE. Create 2 projects that output pipeline_service_master and pipeline_service_slave executables.
#endif

extern std::string g_DatabaseDirectory;
extern std::string g_PipelineDirectory;

#define PIPELINE_MACRO_DATABASE_DIRECTORY "{{DatabaseDirectory}}"
#define PIPELINE_MACRO_PIPELINE_DIRECTORY "{{PipelineDirectory}}"

/// Unmacros a path, and standardizes it as well.
std::string unMacroPath(const std::string &path);

/// Macros a path, and standardizes it in advance.
std::string macroPath(const std::string &path);

extern bool g_IsExiting;

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_SERVICE_H */

/* end of file */
