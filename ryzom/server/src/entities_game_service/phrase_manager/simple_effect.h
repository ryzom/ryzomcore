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



#ifndef RY_SIMPLE_EFFECT_H
#define RY_SIMPLE_EFFECT_H

#include "phrase_manager/s_effect.h"


/**
 * Effect class for simple effects
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSimpleEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CSimpleEffect)

	CSimpleEffect() : CSTimedEffect()
	{
#ifdef NL_DEBUG
		_Applied =false;
#endif
	}
	
	CSimpleEffect( const TDataSetRow & creatorRowId, const TDataSetRow & targetRowId, EFFECT_FAMILIES::TEffectFamily family, sint32 effectValue, uint32 endDate, uint8 power )
		:CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue, power, endDate)
	{
#ifdef NL_DEBUG
		_Applied =false;
#endif
	}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

#ifdef NL_DEBUG
private:
	bool			_Applied;
#endif
};


#endif // RY_SIMPLE_EFFECT_H

/* End of combat_simple_effect.h */
