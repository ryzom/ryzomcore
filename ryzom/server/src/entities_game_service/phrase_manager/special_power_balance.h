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


#ifndef RYZOM_SPECIAL_POWER_BALANCE_H
#define RYZOM_SPECIAL_POWER_BALANCE_H

#include "special_power.h"


/**
 * Specialized class for balance powers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerBalance : public CSpecialPower
{
public:
	/// Constructor
	CSpecialPowerBalance(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float disableTimeInSeconds, float lossFactor, float range, POWERS::TPowerType powerType)
	:CSpecialPower()
	{
		_AffectedScore = SCORES::hit_points;
		_Phrase = phrase;
		_LossFactor = lossFactor;
		_Range = range;
		_DisablePowerTime = NLMISC::TGameCycle(disableTimeInSeconds / CTickEventHandler::getGameTimeStep());

		_PowerType = powerType;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerBalance> invalid data set row passed as actor");
		}
	}

	/// validate the power utilisation
	virtual bool validate(std::string &errorCode);
	
	/// set affected score
	inline void setAffectedScore( SCORES::TScores score ) { _AffectedScore = score; }

	/// apply effects
	virtual void apply();

protected:
	/// loss factor on total value (1 = 100%, 0.5 = 50%, 0.3 = 30%)
	float			_LossFactor;
	/// max Range in meters
	float			_Range;
	/// affected score (Hp, sap, sta)
	SCORES::TScores	_AffectedScore;
};

#endif // RYZOM_SPECIAL_POWER_BALANCE_H

/* End of special_power_balance.h */

