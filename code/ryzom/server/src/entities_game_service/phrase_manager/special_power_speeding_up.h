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



#ifndef RYZOM_SPECIAL_POWER_SPEEDING_UP_H
#define RYZOM_SPECIAL_POWER_SPEEDING_UP_H

#include "special_power.h"


/**
 * Specialized class for power "SpeedingUp"
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerSpeedingUp : public CSpecialPower
{
public:
	/// Default Constructor
	CSpecialPowerSpeedingUp() : CSpecialPower()
	{}

	/// Constructor
	CSpecialPowerSpeedingUp(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, uint8 speedMod, float durationInSeconds, float disableTimeInSeconds) 
		: _SpeedMod(speedMod)
	{
		_Phrase = phrase;
		
		_DisablePowerTime = NLMISC::TGameCycle(disableTimeInSeconds / CTickEventHandler::getGameTimeStep());
		_Duration = NLMISC::TGameCycle(durationInSeconds / CTickEventHandler::getGameTimeStep());		

		_PowerType = POWERS::SpeedingUp;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerSpeedingUp> invalid data set row passed as actor");
		}
	}

	/// apply effects
	virtual void apply();


protected:
	/// speed modifier in % (max 100%)
	uint8				_SpeedMod;

	/// Duration in ticks
	NLMISC::TGameCycle	_Duration;
};

#endif // RYZOM_SPECIAL_POWER_SPEEDING_UP_H

/* End of special_power_speeding_up.h */
