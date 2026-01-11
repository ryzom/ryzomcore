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



#ifndef RYZOM_SPECIAL_POWER_CHG_CHARAC_H
#define RYZOM_SPECIAL_POWER_CHG_CHARAC_H

#include "special_power.h"


/**
 * Specialized class for power "ChgCharac"
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2005
 */
class CSpecialPowerChgCharac : public CSpecialPower
{
public:
	/// Default Constructor
	CSpecialPowerChgCharac() : CSpecialPower()
	{}

	/// Constructor
	CSpecialPowerChgCharac(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float durationInSeconds,  
		POWERS::TPowerType powerType, const std::string& charac, float modifier1, float modifier2 )
	{
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerChgCharac> invalid data set row passed as actor");
		}

		_Phrase = phrase;
		
		_DisablePowerTime = 0;
		_Duration = NLMISC::TGameCycle(durationInSeconds / CTickEventHandler::getGameTimeStep());		

		_PowerType = powerType;
		_AffectedCharac = charac;
		_Modifier1 = modifier1;
		_Modifier2 = modifier2;
	}

	/// apply effects
	virtual void apply();


protected:
	/// affected characteristic
	std::string	_AffectedCharac;

	// score modifier 
	float _Modifier1;
	float _Modifier2;

	/// Duration in ticks
	NLMISC::TGameCycle	_Duration;
};

#endif // RYZOM_SPECIAL_POWER_CHG_CHARAC_H

/* End of special_power_chg_charac.h */
