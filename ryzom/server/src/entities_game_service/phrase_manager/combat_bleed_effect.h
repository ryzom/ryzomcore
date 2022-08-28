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



#ifndef RY_COMBAT_BLEED_EFFECT_H
#define RY_COMBAT_BLEED_EFFECT_H

//
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"



/**
 * class for bleeding effects
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatBleedEffect : public CSEffect
{
public:
	///\ctor
	CCombatBleedEffect( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 effectValue, 
						NLMISC::TGameCycle endDate,
						NLMISC::TGameCycle cycleLenght,
						uint16 cycleDamage
						)
		:	CSEffect(creatorRowId, targetRowId, family, effectValue,0),
			_BleedEndDate(endDate),
			_CycleLength(cycleLenght),
			_CycleDamage(cycleDamage)
	{
		_NextCycleDate = CTickEventHandler::getGameCycle() + _CycleLength;
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

	/// set bleeding entity
	inline void bleedingEntity(CEntityBase *entity) { _BleedingEntity = entity; }

	/// get bleeding entity
	inline CEntityBase *bleedingEntity() { return _BleedingEntity; }

private:
	/// effect end date in ticks
	NLMISC::TGameCycle		_BleedEndDate;

	/// next cycle date (for hp loss)
	NLMISC::TGameCycle		_NextCycleDate;

	/// cycle lenght in ticks
	NLMISC::TGameCycle		_CycleLength;

	/// number of hp lost by target each cycle
	uint16					_CycleDamage;

	/// affected entity
	CEntityBase				*_BleedingEntity;
};


#endif // NL_COMBAT_STUN_EFFECT_H

/* End of combat_stun_effect.h */
