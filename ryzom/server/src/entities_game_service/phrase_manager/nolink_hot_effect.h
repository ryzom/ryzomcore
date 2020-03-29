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



#ifndef RY_NOLINK_HOT_EFFECT_H
#define RY_NOLINK_HOT_EFFECT_H

// game share
#include "game_share/scores.h"
//
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"



/**
 * class for hot effects that are not links
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CNoLinkHoTEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CNoLinkHoTEffect)

	///\ctor
	CNoLinkHoTEffect ( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 effectValue, 
						NLMISC::TGameCycle endDate,
						NLMISC::TGameCycle cycleLenght,
						SCORES::TScores	affectedScore,
						float cycleHeal
						)
		:	CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue, abs((sint32)cycleHeal), endDate),
			_CycleLength(cycleLenght),
			_CycleHeal(cycleHeal),
			_AffectedScore(affectedScore)
	{
		_TargetEntity = NULL;
		_RemainingHeal = 0;
	}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

private:
	/// cycle lenght in ticks
	NLMISC::TGameCycle		_CycleLength;

	/// number of score points healed each cycle
	float					_CycleHeal;
	/// fraction of healed points when not integer (0-1)
	float					_RemainingHeal;

	/// affected score
	SCORES::TScores			_AffectedScore;

	/// affected entity
	CEntityBaseRefPtr		_TargetEntity;

	// private ctor for use in NLMISC class registry
	CNoLinkHoTEffect() {}
};


#endif // RY_NOLINK_HOT_EFFECT_H

/* End of nolink_hot_effect.h */
