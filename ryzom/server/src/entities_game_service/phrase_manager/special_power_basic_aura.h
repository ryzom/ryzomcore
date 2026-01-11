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



#ifndef RYZOM_SPECIAL_POWER_BASIC_AURA_H
#define RYZOM_SPECIAL_POWER_BASIC_AURA_H

#include "special_power.h"


/**
 * Specialized class for auras
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerBasicAura : public CSpecialPowerAuras
{
public:
	/// Default Constructor
	CSpecialPowerBasicAura() : CSpecialPowerAuras()
	{		
	}

	/// Constructor
	CSpecialPowerBasicAura(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float durationInSeconds, float userDisableTimeInSeconds, float targetsDisableTimeInSeconds, POWERS::TPowerType powerType)
	:CSpecialPowerAuras()
	{
		_Phrase = phrase;
		_Duration = NLMISC::TGameCycle(durationInSeconds / CTickEventHandler::getGameTimeStep());;		
		_DisablePowerTime = NLMISC::TGameCycle(userDisableTimeInSeconds / CTickEventHandler::getGameTimeStep());
		_TargetsDisableAuraTime = NLMISC::TGameCycle(targetsDisableTimeInSeconds / CTickEventHandler::getGameTimeStep());

		_PowerType = powerType;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerBasicAura> invalid data set row passed as actor");
		}
	}

	/// apply effects
	virtual void apply();

	/// set families
	inline void setFamilies(EFFECT_FAMILIES::TEffectFamily rootEffectFamily, EFFECT_FAMILIES::TEffectFamily createdEffectFamily)
	{
		_RootEffectFamily = rootEffectFamily;
		_CreatedEffectFamily = createdEffectFamily;
	}

	/// set param value
	inline void setParamValue(sint32 value) { _ParamValue = value; }

protected:
	/// effect duration
	NLMISC::TGameCycle		_Duration;

	/// param value
	sint32					_ParamValue;

	/// root effect family (used for txt msg)
	EFFECT_FAMILIES::TEffectFamily	_RootEffectFamily;
	/// created effect family
	EFFECT_FAMILIES::TEffectFamily	_CreatedEffectFamily;	
	
};

#endif // RYZOM_SPECIAL_POWER_BASIC_AURA_H

/* End of special_power_basic_aura.h */
