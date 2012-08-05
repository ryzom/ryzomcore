/**
 * \file pipeline_process.cpp
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

#include <nel/misc/types_nl.h>
#include "pipeline_process.h"

// STL includes
#include <vector>
#include <sstream>

// NeL includes
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

IPipelineProcess *IPipelineProcess::getInstance()
{
	nlassert(NLMISC::INelContext::isContextInitialised());
	return static_cast<IPipelineProcess *>(NLMISC::INelContext::getInstance().getSingletonPointer("IPipelineProcess"));
}

bool IPipelineProcess::getValue(bool &result, const std::string &name)
{
	// It's true.
	std::string resultString;
	if (!getValue(resultString, name))
		return false;
	if (resultString == "true")
		result = true;
	if (resultString == "false")
		result = false;
	else
	{
		nlwarning("Value '%s' with result '%s' is not a boolean", name.c_str(), resultString.c_str());
		return false;
	}
	return true;
}

bool IPipelineProcess::getValue(uint &result, const std::string &name)
{
	std::string resultString;
	if (!getValue(resultString, name))
		return false;
	if (!NLMISC::fromString(resultString, result))
	{
		nlwarning("Value '%s' with result '%s' is not an integer", name.c_str(), resultString.c_str());
		return false;
	}
	return true;
}

bool IPipelineProcess::getValue(sint &result, const std::string &name)
{
	std::string resultString;
	if (!getValue(resultString, name))
		return false;
	if (!NLMISC::fromString(resultString, result))
	{
		nlwarning("Value '%s' with result '%s' is not an integer", name.c_str(), resultString.c_str());
		return false;
	}
	return true;
}

bool IPipelineProcess::getValuesRecurse(std::vector<std::string> &resultAppend, const std::string &name)
{
	std::string::size_type split = name.find("[]");
	if (split == std::string::npos) return getValues(resultAppend, name);
	else
	{
		std::string subname = name.substr(0, split);
		uint nb;
		if (!getValueNb(nb, subname)) return false;
		std::string begname = name.substr(0, split + 1);
		std::string endname = name.substr(split + 1);
		for (uint i = 0; i < nb; ++i)
		{
			std::stringstream ss;
			ss << begname << i << endname;
			if (!getValuesRecurse(resultAppend, ss.str())) return false;
		}
	}
	return true;
}

} /* namespace PIPELINE */

/* end of file */
