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



#ifndef RY_CHANGE_MOVE_SPEED_EFFECT_H
#define RY_CHANGE_MOVE_SPEED_EFFECT_H

//
#include "phrase_manager/s_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "entity_manager/entity_base.h"


/**
 * class for haste or snare effects
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CChangeMoveSpeedEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CChangeMoveSpeedEffect)
		
	///\ctor
	CChangeMoveSpeedEffect() 
	{}

	CChangeMoveSpeedEffect( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 effectValue, // in % of base speed >0 haste, <0 snare
						NLMISC::TGameCycle endDate,
						CEntityBase *affectedEntity
						)
		:	CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue, abs(effectValue), endDate),
			_AffectedEntity(affectedEntity)
	{
		_AffectedEntity = affectedEntity;
		if (_AffectedEntity)
			DEBUGLOG("EFFECT: create slow move effect (value %d) on entity %s", _Value, _AffectedEntity->getId().toString().c_str());
	}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

private:
	/// affected entity
	CEntityBaseRefPtr		_AffectedEntity;
};


#endif // RY_CHANGE_MOVE_SPEED_EFFECT_H

/* End of change_move_speed_effect.h */
