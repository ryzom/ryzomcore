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



#ifndef RZ_GROUP_MODAL_GET_KEY_H
#define RZ_GROUP_MODAL_GET_KEY_H

#include "nel/misc/types_nl.h"
#include "../actions.h" // CLIENT
#include "nel/gui/group_modal.h"


// ***************************************************************************
/**
 * A group with special modal options and event key catching
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2003
 */
class CGroupModalGetKey : public CGroupModal
{
public:

	/// Constructor
	CGroupModalGetKey(const TCtorParam &param);

	virtual bool handleEvent (const NLGUI::CEventDescriptor &event);

	virtual void setActive(bool state);

public:
	CCombo		Combo;
	std::string Caller;
};


#endif // RZ_GROUP_MODAL_GET_KEY_H

/* End of group_modal_get_key.h */
