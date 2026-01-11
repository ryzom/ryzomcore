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



#ifndef RY_NOLINK_DOT_EFFECT_H
#define RY_NOLINK_DOT_EFFECT_H

// game share
#include "game_share/scores.h"
//
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"



/**
 * class for dot effects that are not links
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CNoLinkDOTEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CNoLinkDOTEffect)

	///\ctor
	CNoLinkDOTEffect ( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 effectValue, 
						NLMISC::TGameCycle endDate,
						NLMISC::TGameCycle cycleLenght,
						SCORES::TScores	affectedScore,
						float cycleDamage,
						DMGTYPE::EDamageType damageType
						)
		:	CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue, abs((sint32)cycleDamage), endDate),
			_CycleLength(cycleLenght),
			_CycleDamage(cycleDamage),
			_AffectedScore(affectedScore),
			_DamageType(damageType)
	{
		_RemainingDamage = 0;
		_EndsAtCasterDeath = false;
		_Stackable = false;
	}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

	/// if true effect ends at caster death 
	inline void endsAtCasterDeath(bool flag) { _EndsAtCasterDeath = flag; }

	/// set the stackable flag
	inline void stackable(bool flag) { _Stackable = flag; }

private:
	/// cycle lenght in ticks
	NLMISC::TGameCycle		_CycleLength;

	/// number of score points lost by target each cycle
	float					_CycleDamage;
	/// fraction of lost points when not integer (0-1)
	float					_RemainingDamage;

	/// affected score
	SCORES::TScores			_AffectedScore;

	/// damage type
	DMGTYPE::EDamageType	_DamageType;

	/// if true effect ends at caster death 
	bool					_EndsAtCasterDeath;

	/// if true DoT can stacks, otherwise they don't stack
	bool					_Stackable;

	// private ctor for use in NLMISC class registry
	CNoLinkDOTEffect() {}

};


#endif // RY_NOLINK_DOT_EFFECT_H

/* End of nolink_dot_effect.h */
