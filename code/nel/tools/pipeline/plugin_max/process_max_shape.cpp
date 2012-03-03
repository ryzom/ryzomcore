/**
 * \file process_max_shape.cpp
 * \brief CProcessMaxShape
 * \date 2012-02-25 10:45GMT
 * \author Jan Boon (Kaetemi)
 * CProcessMaxShape
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
#include "process_max_shape.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/task_manager.h>

// Project includes
#include "../plugin_library/pipeline_interface.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CProcessMaxShape::CProcessMaxShape()
{
	
}

CProcessMaxShape::~CProcessMaxShape()
{
	
}

namespace {

class CMaxExportShapeCommand : public NLMISC::IRunnable
{
public:
	NLMISC::CLog *Log;

	virtual void getName(std::string &result) const 
	{ result = "CMaxExportShapeCommand"; }

	virtual void run()
	{
		std::string tempDirectory = PIPELINE::IPipelineInterface::getInstance()->getTempDir();

		PIPELINE::IPipelineInterface::getInstance()->endedRunnableTask();
	}
};
CMaxExportShapeCommand s_MaxExportShapeCommand;

} /* anonymous namespace */

} /* namespace PIPELINE */

NLMISC_CATEGORISED_COMMAND(max, maxExportShape, "Export shapes from a .max file manually.", "<filePath> <outDirectory>")
{
	if(args.size() != 2) return false;
	PIPELINE::s_MaxExportShapeCommand.Log = &log;
	if (!PIPELINE::IPipelineInterface::getInstance()->tryRunnableTask("COMMAND_MAX_EXPORT_SHAPE", &PIPELINE::s_MaxExportShapeCommand))
	{
		log.displayNL("Busy.");
		return false;
	}
	return true;
}

/* end of file */
