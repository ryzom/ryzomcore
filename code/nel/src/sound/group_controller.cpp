/**
 * \file group_controller.cpp
 * \brief CGroupController
 * \date 2012-04-10 09:29GMT
 * \author Jan Boon (Kaetemi)
 * CGroupController
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
#include <nel/sound/group_controller.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/sound/source_common.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

CGroupController::CGroupController(CGroupController *parent) : 
	m_Parent(parent), m_Gain(1.0f), m_NbSourcesInclChild(0)
{
	
}

CGroupController::~CGroupController()
{
	// If m_Sources is not empty, a crash is very likely.
	nlassert(m_Sources.empty());

	for (std::map<std::string, CGroupController *>::iterator it(m_Children.begin()), end(m_Children.end()); it != end; ++it)
	{
		delete it->second;
		it->second = NULL;
	}
	m_Parent = NULL;
}

void CGroupController::addSource(CSourceCommon *source)
{
	nlassert(this);

	m_Sources.insert(source);
	increaseSources();
}

void CGroupController::removeSource(CSourceCommon *source)
{
	decreaseSources();
	m_Sources.erase(source);
}

std::string CGroupController::getPath() // overridden by root
{
	for (std::map<std::string, CGroupController *>::iterator it(m_Parent->m_Children.begin()), end(m_Parent->m_Children.end()); it != end; ++it)
	{
		if (it->second == this)
		{
			const std::string &name = it->first;
			std::string returnPath = m_Parent->getPath() + ":" + name;
			return returnPath;
		}
	}
	nlerror("Group Controller not child of parent");
	return "";
}

void CGroupController::calculateFinalGain() // overridden by root
{
	m_FinalGain = calculateTotalGain() * m_Parent->getFinalGain();
}

void CGroupController::updateSourceGain()
{
	// Dont update source gain when this controller is inactive.
	if (m_NbSourcesInclChild)
	{
		calculateFinalGain();
		for (TSourceContainer::iterator it(m_Sources.begin()), end(m_Sources.end()); it != end; ++it)
			(*it)->updateFinalGain();
		for (std::map<std::string, CGroupController *>::iterator it(m_Children.begin()), end(m_Children.end()); it != end; ++it)
			(*it).second->updateSourceGain();
	}
}

void CGroupController::increaseSources() // overridden by root
{
	++m_NbSourcesInclChild;
	m_Parent->increaseSources();
	
	// Update source gain when this controller was inactive before but the parent was active before.
	// Thus, when this controller was the root of inactive controllers.
	if (m_NbSourcesInclChild == 1 && m_Parent->m_NbSourcesInclChild > 1)
		updateSourceGain();
}

void CGroupController::decreaseSources() // overridden by root
{
	--m_NbSourcesInclChild;
	m_Parent->decreaseSources();
}

} /* namespace NLSOUND */

/* end of file */
