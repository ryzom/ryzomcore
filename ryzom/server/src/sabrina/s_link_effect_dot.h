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



#ifndef RY_S_LINK_EFFECT_DOT_H
#define RY_S_LINK_EFFECT_DOT_H

#include "nel/misc/types_nl.h"
#include "s_link_effect.h"

class CSLinkEffectDot : public CSLinkEffectOffensive
{
public:
	inline CSLinkEffectDot( 
		const TDataSetRow & creatorRowId,
		const TDataSetRow & targetRowId,
		sint32 cost,
		SCORES::TScores energyCost,
		SKILLS::ESkills skill, 
		DMGTYPE::EDamageType dmgType,
		uint8 power,
		sint32 dmgHp,sint32 dmgSap,sint32 dmgSta )
		:CSLinkEffectOffensive ( creatorRowId,targetRowId,EFFECT_FAMILIES::Dot,cost,energyCost, skill, 0,power),
		_DmgType( dmgType ),
		_DmgHp( dmgHp), _DmgSap( dmgSap), _DmgSta(dmgSta)
	{
	}

	DMGTYPE::EDamageType getDamageType(){ return _DmgType; }

	/**
	 * apply the effects of the... effect
	 * \return true if the effect must be removed
	 */
	bool update(uint32 & updateFlag);



protected:
	DMGTYPE::EDamageType	_DmgType;
	sint32 _DmgHp;
	sint32 _DmgSap;
	sint32 _DmgSta;
};


#endif // RY_S_LINK_EFFECT_DOT_H

/* End of s_link_effect_dot.h */
