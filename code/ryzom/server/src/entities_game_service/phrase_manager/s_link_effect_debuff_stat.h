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



#ifndef RY_S_LINK_EFFECT_DEBUFF_STAT_H
#define RY_S_LINK_EFFECT_DEBUFF_STAT_H

#include "phrase_manager/s_link_effect.h"
#include "entity_manager/entity_base.h"


/**
 * dbueff stat effect
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CSLinkEffectDebuffStat : public CSLinkEffectOffensive
{
public:
	inline CSLinkEffectDebuffStat( 
		const TDataSetRow & creatorRowId,
		const TDataSetRow & targetRowId,
		EFFECT_FAMILIES::TEffectFamily family,
		sint32 costPerUpdate,
		SCORES::TScores energyCost,
		SKILLS::ESkills skill,
		uint32 maxDistance,
		sint32 modifier,
		float multiplier,
		STAT_TYPES::TStatType type,
		uint stat,
		uint8 power,
		TReportAction& report )
		 :CSLinkEffectOffensive( creatorRowId,targetRowId,family,costPerUpdate,energyCost, skill, maxDistance, modifier, power, report ),
		 _Multiplier( multiplier ),
		_Type( type ),
		_Stat( stat )
	{
		_Applied = false;
	}

	/**
	 * apply the effects of the... effect
	 * \return true if effects ends
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

protected:
	float					_Multiplier;
	sint32			  		_Stat;
	STAT_TYPES::TStatType	_Type;
	bool					_Applied;
	
};


#endif // RY_S_LINK_EFFECT_DEBUFF_STAT_H

/* End of s_link_effect_debuff_stat.h */
