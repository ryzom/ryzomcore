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



#ifndef RYZOM_SPECIAL_POWER_SHIELDING_H
#define RYZOM_SPECIAL_POWER_SHIELDING_H

#include "special_power.h"


/**
 * Specialized class for power "shielding"
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerShielding : public CSpecialPower
{
public:
	/// Default Constructor
	CSpecialPowerShielding() : CSpecialPower()
	{
		init();
	}

	/// Constructor
	CSpecialPowerShielding(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float durationInSeconds, float disableTimeInSeconds)
	{
		init();
		_Phrase = phrase;
		_Duration = NLMISC::TGameCycle(durationInSeconds / CTickEventHandler::getGameTimeStep());;		
		_DisablePowerTime = NLMISC::TGameCycle(disableTimeInSeconds / CTickEventHandler::getGameTimeStep());

		_PowerType = POWERS::Shielding;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerShielding> invalid data set row passed as actor");
		}
	}

	/// validate the power utilisation
	virtual bool validate(std::string &errorCode);

	/// apply effects
	virtual void apply();

	// set protections
	inline void setNoShieldProtection(float factor, uint16 max) { _NoShieldFactor = factor; _NoShieldMaxProtection = max; }
	inline void setBucklerProtection(float factor, uint16 max) { _BucklerFactor = factor; _BucklerMaxProtection = max; }
	inline void setShieldProtection(float factor, uint16 max) { _ShieldFactor = factor; _ShieldMaxProtection = max; }

protected:
	inline void init()
	{
		_NoShieldFactor = 0.0f;
		_BucklerFactor = 0.0f;
		_ShieldFactor = 0.0f;
		
		_NoShieldMaxProtection = 0;
		_BucklerMaxProtection = 0;
		_ShieldMaxProtection = 0;

		_Duration = 0;
	}

	/// granted protections 
	float					_NoShieldFactor;
	float					_BucklerFactor;
	float					_ShieldFactor;
	
	uint16					_NoShieldMaxProtection;
	uint16					_BucklerMaxProtection;
	uint16					_ShieldMaxProtection;

	// effect duration
	NLMISC::TGameCycle		_Duration;
	
};

#endif // RYZOM_SPECIAL_POWER_SHIELDING_H

/* End of special_power_shielding.h */
