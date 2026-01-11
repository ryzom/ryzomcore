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



#ifndef RY_AURA_REGEN_EFFECT_H
#define RY_AURA_REGEN_EFFECT_H

#include "aura_effect.h"


/**
 * Effect class for aura affecting regen
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CRegenAuraEffect : public CAuraBaseEffect
{
public:
	///\ctor
	CRegenAuraEffect( const CAuraRootEffect &rootEffect, TDataSetRow targetRowId ) : CAuraBaseEffect(rootEffect, targetRowId)
	{
		_RegenModifier = float(rootEffect.getParamValue()) / 10.0f;

		switch(_PowerType)
		{
		case POWERS::LifeAura:
			_AffectedScore = SCORES::hit_points;
			break;
		case POWERS::StaminaAura:
			_AffectedScore = SCORES::stamina;
			break;
		case POWERS::SapAura:
			_AffectedScore = SCORES::sap;
			break;
		default:
			nlwarning("CRegenAuraEffect : cannot manage power type %s", POWERS::toString(_PowerType).c_str());
			_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
			_AffectedScore = SCORES::unknown;
		};
	}
	
	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

private:
	/// regen modifier
	float			_RegenModifier;

	/// affected score
	SCORES::TScores _AffectedScore;
};


#endif // RY_AURA_REGEN_EFFECT_H

/* End of aura_regen_effect.h */
