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



#ifndef RYZOM_SPECIAL_POWER_MOD_FORAGE_SUCCESS_H
#define RYZOM_SPECIAL_POWER_MOD_FORAGE_SUCCESS_H

#include "special_power.h"


/**
 * Specialized class for power "ModForageSuccess"
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2006
 */
class CSpecialPowerModForageSuccess : public CSpecialPower
{
public:
	/// Default Constructor
	CSpecialPowerModForageSuccess() : CSpecialPower()
	{}

	/// Constructor
	CSpecialPowerModForageSuccess(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float durationInSeconds,  
		const std::string& ecosystem, float modifier1, float modifier2 );

	/// apply effects
	virtual void apply();


protected:
	/// ecosystem
	std::string	_Ecosystem;

	// score modifier 
	float _Modifier1;
	float _Modifier2;

	/// Duration in ticks
	NLMISC::TGameCycle	_Duration;
};

#endif // RYZOM_SPECIAL_POWER_MOD_FORAGE_SUCCESS_H

/* End of special_power_mod_forage_success.h */
