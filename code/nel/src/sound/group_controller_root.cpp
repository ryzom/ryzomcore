/**
 * \file group_controller_root.cpp
 * \brief CGroupControllerRoot
 * \date 2012-04-10 09:44GMT
 * \author Jan Boon (Kaetemi)
 * CGroupControllerRoot
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE.
 * RYZOM CORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * RYZOM CORE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "stdsound.h"
#include <nel/sound/group_controller_root.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/algo.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

CGroupControllerRoot::CGroupControllerRoot() : CGroupController(NULL)
{
	
}

CGroupControllerRoot::~CGroupControllerRoot()
{
	
}

void CGroupControllerRoot::calculateFinalGain()
{
	m_FinalGain = calculateTotalGain();
}

void CGroupController::increaseSources()
{
	++m_NbSourcesInclChild;

	// Update source gain when this controller was inactive before.
	if (m_NbSourcesInclChild == 1)
		updateSourceGain();
}

void CGroupController::decreaseSources()
{
	--m_NbSourcesInclChild;
}

CGroupController *CGroupControllerRoot::getGroupController(const std::string &path)
{
	std::vector<std::string> pathNodes;
	NLMISC::splitString(NLMISC::toLower(path), "/", pathNodes);
	CGroupController *active = this;
	for (std::vector<std::string>::iterator it(pathNodes.begin()), end(pathNodes.end()); it != end; ++it)
	{
		if (!(*it).empty())
		{
			std::map<std::string, CGroupController *>::iterator found = active->m_Children.find(*it);
			if (found == active->m_Children.end())
			{
				active = new CGroupController(active);
				active->m_Parent->m_Children[*it] = active;
			}
			else
			{
				active = (*found).second;
			}
		}
	}
	return active;
}

} /* namespace NLSOUND */

/* end of file */
