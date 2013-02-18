// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.



#ifndef NL_GROUP_CAREER_H
#define NL_GROUP_CAREER_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_container.h"


// ***************************************************************************
/** A Group that do not parse the title
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CGroupCareer : public CGroupContainer
{
public:
	// By default a GroupCareer don't save its "Active" state since depends on server.
	CGroupCareer(const TCtorParam &param)
		: CGroupContainer(param)
	{
		_ActiveSavable= false;
	}

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
};

// ***************************************************************************
/** A Group that do not parse the title
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CGroupJob : public CGroupContainer
{
public:
	// By default a GroupJob don't save its "Active" state since depends on server.
	CGroupJob(const TCtorParam &param)
		: CGroupContainer(param)
	{
		_ActiveSavable= false;
	}

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
};


#endif // NL_GROUP_CAREER_H

/* End of group_career.h */
