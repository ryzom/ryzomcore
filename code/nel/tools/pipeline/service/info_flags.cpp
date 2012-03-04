/**
 * \file info_flags.cpp
 * \brief CInfoFlags
 * \date 2012-03-04 10:46GMT
 * \author Jan Boon (Kaetemi)
 * CInfoFlags
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
#include "info_flags.h"

// STL includes
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "info_flags.h"

using namespace std;
// using namespace NLMISC;

NLMISC::CVariable<std::string> InfoFlags("pipeline", "infoFlags", "READ ONLY", "<DISABLED>");

namespace PIPELINE {

CInfoFlags::CInfoFlags()
{
	updateInfoFlags();
}

CInfoFlags::~CInfoFlags()
{
	InfoFlags = "<DISABLED>";
}

void CInfoFlags::addFlag(const std::string &flagName)
{
	nldebug("addFlag: %s", flagName.c_str());
	if (m_FlagMap.find(flagName) != m_FlagMap.end())
	{
		++m_FlagMap[flagName];
	}
	else
	{
		m_FlagMap[flagName] = 1;
	}
	updateInfoFlags();
}

void CInfoFlags::removeFlag(const std::string &flagName)
{
	nldebug("removeFlag: %s", flagName.c_str());
	std::map<std::string, uint>::iterator it = m_FlagMap.find(flagName);
	if (it != m_FlagMap.end())
	{
		if (it->second == 1)
			m_FlagMap.erase(it);
		else
			--it->second;
	}
	updateInfoFlags();
}

void CInfoFlags::updateInfoFlags()
{
	if (m_FlagMap.empty())
	{
		InfoFlags = "<NONE>";
	}
	else
	{
		std::stringstream ss;
		for (std::map<std::string, uint>::iterator it = m_FlagMap.begin(), end = m_FlagMap.end(); it != end; ++it)
		{
			ss << it->first;
			if (it ->second > 1)
				ss << " (" << it->second << ")";
			ss << ", ";
		}
		std::string s = ss.str();
		InfoFlags = s.substr(0, s.size() - 2);
	}
}

} /* namespace PIPELINE */

/* end of file */
