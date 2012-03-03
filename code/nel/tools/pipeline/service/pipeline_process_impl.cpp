/**
 * \file pipeline_process_impl.cpp
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

#include <nel/misc/types_nl.h>
#include "pipeline_process_impl.h"

// STL includes
#include <sstream>

// NeL includes
#include <nel/misc/time_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>

// Project includes
#include "pipeline_service.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CPipelineProcessImpl::CPipelineProcessImpl()
{
	nlassert(getInstance() == NULL);
	NLMISC::INelContext::getInstance().setSingletonPointer("IPipelineProcess", this);
}

CPipelineProcessImpl::~CPipelineProcessImpl()
{
	NLMISC::INelContext::getInstance().releaseSingletonPointer("IPipelineProcess", this);
}

std::string CPipelineProcessImpl::getProjectValue(const std::string &name)
{
	return ""; // TODO
}

std::string CPipelineProcessImpl::getTempDir()
{
	// IF PROJECT blahblah TODO
	// ELSE

	{
		std::stringstream ss;
		ss << g_PipelineDirectory;
		ss << NLMISC::CTime::getSecondsSince1970();
		ss << ".";
		ss << rand();
		ss << PIPELINE_DIRECTORY_TEMP_SUFFIX;
		return ss.str();
	}
}

} /* namespace PIPELINE */

/* end of file */
