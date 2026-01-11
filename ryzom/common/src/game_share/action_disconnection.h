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



#ifndef NL_ACTION_DISCONNECTION_H
#define NL_ACTION_DISCONNECTION_H

#include "nel/misc/types_nl.h"

#include "action.h"


namespace CLFECOMMON {

/**
 * This action means the entity Id has left the game.
 * Note: No more data or processing than in CAction.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CActionDisconnection : public CActionImpulsion
{
public:

	static CAction *create () { return new CActionDisconnection(); }

protected:

	friend class CActionFactory;

	/// This method intialises the action with a default state
	virtual void reset()
	{
		AllowExceedingMaxSize = false;
	}
};

}

#endif // NL_ACTION_DISCONNECTION_H

/* End of action_disconnection.h */
