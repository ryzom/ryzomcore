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



#ifndef RY_REGEN_MODIFIER_EFFECT_H
#define RY_REGEN_MODIFIER_EFFECT_H

//
#include "game_share/scores.h"
//
#include "phrase_manager/s_effect.h"


/**
 * Effect class for simple effects
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CRegenModifierEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CRegenModifierEffect)

	///\ctor
	CRegenModifierEffect(	const TDataSetRow & creatorRowId,
							const TDataSetRow & targetRowId,
							EFFECT_FAMILIES::TEffectFamily family,
							uint32 endDate,
							uint8 power,
							float factor,
							SCORES::TScores score)
		:CSTimedEffect(creatorRowId, targetRowId, family, false, 0, power, endDate),
		_AffectedScore(score),
		_DebuffFactor(factor),
		_FirstUpdate(true)
	{
	}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

	/// set the effect name
	inline void effectName(const std::string &str) { _EffectName = str; }
	/// get the effect name
	inline const std::string &effectName() { return _EffectName; }

private:
	/// affected score
	SCORES::TScores			_AffectedScore;

	/// debuff factor
	float					_DebuffFactor;

	/// "name" of the effect, used for client chat messages
	std::string				_EffectName;

	/// flag, true if first update
	bool					_FirstUpdate;

	/// the value added to the regen modifier (<0)
	float					_RegenModifier;

	// private ctor for use in NLMISC class registry
	CRegenModifierEffect() {}
};


#endif // RY_REGEN_MODIFIER_EFFECT_H

/* End of regen_modifier_effect.h */
