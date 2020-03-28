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

#ifndef NL_SKILL_CHANGE_CALLBACK_H
#define NL_SKILL_CHANGE_CALLBACK_H

#include "nel/misc/types_nl.h"


// ***************************************************************************
/**
 * Callbakc called when a value of a skill change
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class ISkillChangeCallback
{
public:
	/*	Called only if SKILL or BaseSKILL change (not progressbar)
	*/
	virtual	void	onSkillChange() =0;
};


#endif // NL_SKILL_CHANGE_CALLBACK_H

/* End of skill_change_callback.h */
