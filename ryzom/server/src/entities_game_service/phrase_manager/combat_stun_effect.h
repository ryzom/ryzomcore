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



#ifndef RY_COMBAT_STUN_EFFECT_H
#define RY_COMBAT_STUN_EFFECT_H

#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatStunEffect : public CSEffect
{
public:
	///\ctor
	CCombatStunEffect( const TDataSetRow & creatorRowId, const TDataSetRow & targetRowId, EFFECT_FAMILIES::TEffectFamily family, sint32 effectValue, uint32 endDate)
		:CSEffect(creatorRowId, targetRowId, family, effectValue,0), _StunEndDate(endDate)
	{
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

	/// set stunned entity
	inline void stunnedEntity(CEntityBase *entity) { _StunnedEntity = entity; }

	/// get stunned entity
	inline CEntityBase *stunnedEntity() { return _StunnedEntity; }

private:
	/// effect end date in ticks
	NLMISC::TGameCycle		_StunEndDate;

	/// affected entity
	CEntityBase				*_StunnedEntity;
};


#endif // RY_COMBAT_STUN_EFFECT_H

/* End of combat_stun_effect.h */
