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



#ifndef RY_S_LINK_EFFECT_HOT_H
#define RY_S_LINK_EFFECT_HOT_H

#include "nel/misc/types_nl.h"


#include "nel/misc/types_nl.h"
#include "s_link_effect.h"

class CSLinkEffectHot : public CSLinkEffect
{
public:
	inline CSLinkEffectHot( const TDataSetRow & creatorRowId,
		const TDataSetRow & targetRowId,
		sint32 cost,
		SCORES::TScores energyCost,
		SKILLS::ESkills skill, 
		uint8 power,
		sint32 healHp, sint32 healSap, sint32 healSta)
		:CSLinkEffect ( creatorRowId,targetRowId,EFFECT_FAMILIES::Hot,cost,energyCost,skill,0,power),
		_HealHp(healHp),_HealSap(healSap),_HealSta(healSta)
	{
	}

	/**
	 * apply the effects of the... effect
	 * \return true if the effect must be removed
	 */
	bool update(uint32 & updateFlag);

protected:
	void applyOnScore( CEntityBase * caster, CEntityBase * target,SCORES::TScores scoreType, sint32 value );
	sint32 _HealHp;
	sint32 _HealSap;
	sint32 _HealSta;
};


#endif // RY_S_LINK_EFFECT_HOT_H

/* End of s_link_effect_hot.h */
