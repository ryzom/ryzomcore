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



#ifndef RY_BOUNCE_EFFECT_H
#define RY_BOUNCE_EFFECT_H

//
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"



/**
 * class for 'bounce' effects
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CBounceEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CBounceEffect)

	/// ctor
	CBounceEffect() : CSTimedEffect()
	{}

	///\ctor
	CBounceEffect( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 effectValue, 
						NLMISC::TGameCycle endDate
						)
		:	CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue,(uint8)0, endDate)
	{
		// range of the effect in meters = effecValue
	}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect) { return false; }

	/// callback called when the effect is actually removed
	virtual void removed();

	/// get entity on which the attack is redirected
	CEntityBase *getTargetForBounce(CEntityBase *actor) const;

private:
	/// check entity is a valid target
	bool isEntityValidTarget(CEntityBase *entity, CEntityBase *actor) const;

private:
	/// affected entity
	mutable CEntityBaseRefPtr _AffectedEntity;
};


#endif // RY_BOUNCE_EFFECT_H

/* End of bounce_effect.h */
