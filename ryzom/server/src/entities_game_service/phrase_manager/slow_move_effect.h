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



#ifndef RY_SLOW_MOVE_EFFECT_H
#define RY_SLOW_MOVE_EFFECT_H

//
#include "phrase_manager/s_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"


/**
 * class for slow move effects
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSlowMoveEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CSlowMoveEffect)

	///\ctor
	CSlowMoveEffect() 
	{_SlowedEntity = NULL; _FirstUpdate = true;}

	CSlowMoveEffect( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 effectValue, // should be -100..0, -20 means -20%
						NLMISC::TGameCycle endDate,
						CEntityBase *slowedEntity
						)
		:	CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue, (uint8)abs(effectValue), endDate),
			_FirstUpdate(true), _SlowedEntity(slowedEntity)
	{
		if (slowedEntity)
			DEBUGLOG("EFFECT: create slow move effect (value %d) on entity %s", _Value, _SlowedEntity->getId().toString().c_str());
	}

	/**
	 *  return true if it is time to update the effect. It modifies the next update of the effect
	 */
	virtual bool isTimeToUpdate();

	/**
	 * apply the effects of the... effect
	 * \param updateFlag is a flag telling which effect type has been already processed for an entity. An effect shoud set to 1 the bit corresponding to its effect family
	 * \return true if the effect ends and must be removed
	 */
	virtual bool update( uint32 & updateFlag );

	/// callback called when the effect is actually removed
	virtual void removed();

private:
	/// affected entity
	CEntityBase				*_SlowedEntity;

	/// flag, true if first update
	bool					_FirstUpdate;
};


#endif // RY_SLOW_MOVE_EFFECT_H

/* End of slow_move_effect.h */
