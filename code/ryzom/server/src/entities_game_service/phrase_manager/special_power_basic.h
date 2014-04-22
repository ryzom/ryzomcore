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



#ifndef RYZOM_SPECIAL_POWER_BASIC_H
#define RYZOM_SPECIAL_POWER_BASIC_H

#include "special_power.h"

#include "game_share/persistent_data.h"

/**
 * Specialized class for basic powers such as invulnerability
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerBasic : public CSpecialPower
{
public:
	/// Constructor
	CSpecialPowerBasic(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float durationInSeconds, float disableTimeInSeconds, POWERS::TPowerType powerType)
	:CSpecialPower()
	{
		_ParamValue = 0;
		_EffectFamily = EFFECT_FAMILIES::Unknown;
		
		_Phrase = phrase;
		_Duration = NLMISC::TGameCycle(durationInSeconds / CTickEventHandler::getGameTimeStep());;		
		_DisablePowerTime = NLMISC::TGameCycle(disableTimeInSeconds / CTickEventHandler::getGameTimeStep());

		_PowerType = powerType;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerBasic> invalid data set row passed as actor");
		}
	}

	/// apply effects
	virtual void apply();

	/// set param value
	inline void setParamValue(sint32 value) { _ParamValue = value; }

	/// set effect family
	inline void setEffectFamily(EFFECT_FAMILIES::TEffectFamily family) { _EffectFamily = family; }

protected:
	/// effect duration
	NLMISC::TGameCycle		_Duration;

	/// effect family
	EFFECT_FAMILIES::TEffectFamily	_EffectFamily;

	/// param value
	sint32					_ParamValue;
};

#endif // RYZOM_SPECIAL_POWER_BASIC_H

/* End of special_power_basic.h */
