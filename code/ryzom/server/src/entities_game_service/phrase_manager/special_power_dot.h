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


#ifndef RYZOM_SPECIAL_POWER_DOT_H
#define RYZOM_SPECIAL_POWER_DOT_H

#include "special_power.h"


/**
 * Specialized class for dot-like powers such as berserk
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerDoT : public CSpecialPower
{
public:
	/// Constructor
	CSpecialPowerDoT(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float durationInSeconds, float disableTimeInSeconds, POWERS::TPowerType powerType)
	:CSpecialPower()
	{
		_EffectFamily = EFFECT_FAMILIES::Unknown;
		_ParamValue = 0;		
		_Phrase = phrase;
		_Duration = NLMISC::TGameCycle(durationInSeconds / CTickEventHandler::getGameTimeStep());
		_DisablePowerTime = NLMISC::TGameCycle(disableTimeInSeconds / CTickEventHandler::getGameTimeStep());

		_PowerType = powerType;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerDoT> invalid data set row passed as actor");
		}
	}

	/// set the update frequency
	inline void setUpdateFrequency(float updateFrequencyInSeconds) { _UpdateFrequency = NLMISC::TGameCycle(updateFrequencyInSeconds / CTickEventHandler::getGameTimeStep()); }

	/// set the damage per update
	inline void setDamagePerUpdate(float damage) { _DamagePerUpdate = damage; }

	/// apply effects
	virtual void apply();

	/// set effect family
	inline void setEffectFamily(EFFECT_FAMILIES::TEffectFamily family) { _EffectFamily = family; }

	/// set param value
	inline void setParamValue(sint32 value) { _ParamValue = value; }

protected:
	/// effect duration
	NLMISC::TGameCycle		_Duration;

	/// update rate in ticks
	NLMISC::TGameCycle		_UpdateFrequency;

	/// damage per update
	float					_DamagePerUpdate;

	/// effect param value
	sint32					_ParamValue;

	/// effect family
	EFFECT_FAMILIES::TEffectFamily	_EffectFamily;
};

#endif // RYZOM_SPECIAL_POWER_DOT_H

/* End of special_power_dot.h */
