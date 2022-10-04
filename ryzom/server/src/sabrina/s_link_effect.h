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



#ifndef RY_S_LINK_EFFECT_H
#define RY_S_LINK_EFFECT_H

#include "nel/misc/types_nl.h"
#include "s_effect.h"

class CSLinkEffect : public CSEffect
{
public:
	inline CSLinkEffect( 
		const TDataSetRow & creatorRowId, 
		const TDataSetRow & targetRowId,
		EFFECT_FAMILIES::TEffectFamily family, 
		sint32 cost,
		SCORES::TScores energyCost,
		SKILLS::ESkills skill,
		sint32 value,
		uint8 power)
		:CSEffect(creatorRowId,targetRowId,family,value,power),
		_CostPerUpdate(cost),
		_EnergyCost(energyCost),
		_Skill(skill),
		_NextUpdate(0)
	{
	
	}

	/**
	 *  return true if it is time to update the effect. It modifies the next update of the effect
	 */
	bool isTimeToUpdate();

	bool update(uint32 & updateFlag);
	
	virtual void removed();
	
	inline static double getUpdatePeriod(){return _UpdatePeriod;}
	
protected:
	NLMISC::TGameCycle		_NextUpdate;
	sint32					_CostPerUpdate;
	SCORES::TScores			_EnergyCost;
	SKILLS::ESkills			_Skill;

	static double			_UpdatePeriod;		
};

class CSLinkEffectOffensive : public CSLinkEffect
{
public:
	inline CSLinkEffectOffensive( const TDataSetRow & creatorRowId, 
		const TDataSetRow & targetRowId,
		EFFECT_FAMILIES::TEffectFamily family, 
		sint32 cost,
		SCORES::TScores energyCost,
		SKILLS::ESkills skill,
		sint32 value,
		uint8 power)
		:CSLinkEffect(creatorRowId,targetRowId,family,cost,energyCost,skill,value,power),_ResistFactor(0.0f)
	{
	}
	
	bool update(uint32 & updateFlag);
	
protected:
	float	_ResistFactor;
};

#endif // RY_S_LINK_EFFECT_H

/* End of s_link_effect.h */
