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



#ifndef RY_DAMAGE_SHIELD_EFFECT_H
#define RY_DAMAGE_SHIELD_EFFECT_H

//
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"



/**
 * class for damage shields
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CDamageShieldEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CDamageShieldEffect)

	///\ctor
	CDamageShieldEffect( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 damagePerHit, 
						NLMISC::TGameCycle endDate,
						float drainFactor
						)
		:	CSTimedEffect(creatorRowId, targetRowId, family, true, damagePerHit, damagePerHit, endDate),
			_DrainFactor(drainFactor)
	{
	}
	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect) { return false; }

	/// callback called when the effect is actually removed
	virtual void removed();

	/// get drain factor
	inline float getDrainFactor() const { return _DrainFactor; }

	/// set drain factor
	inline void setDrainFactor(float f) { _DrainFactor = f; }

private:
	/// factor of inflicted damage recovered by the effect creator
	float					_DrainFactor;

	CDamageShieldEffect() {}
};


#endif // RY_DAMAGE_SHIELD_EFFECT_H

/* End of damage_shield_effect.h */
