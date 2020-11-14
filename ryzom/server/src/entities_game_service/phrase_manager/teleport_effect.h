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

#ifndef RY_TELEPORT_EFFECT_H
#define RY_TELEPORT_EFFECT_H


//
#include "entity_manager/entity_base.h"
#include "phrase_manager/s_effect.h"

class TDataSetRow;
class CStaticItem;

/**
 * class for teleport effects. Used when player uses a teleport
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CTeleportEffect : public CSTimedEffect
{
public:
	///\ctor
	CTeleportEffect( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 effectValue, 
						NLMISC::TGameCycle endDate,
						const CStaticItem & form
						);

	virtual bool update(CTimerEvent * event, bool applyEffect){ return false; }
	/// callback called when the effect is actually removed
	virtual void removed();

private:
	const CStaticItem & _Form;
};


#endif // RY_TELEPORT_EFFECT_H

/* End of teleport_effect.h */
