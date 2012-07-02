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



#ifndef RYZOM_SPECIAL_POWER_MOD_DEFENSE_H
#define RYZOM_SPECIAL_POWER_MOD_DEFENSE_H

#include "special_power.h"


/**
 * Specialized class for power "ModDefense"
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2006
 */
class CSpecialPowerModDefense : public CSpecialPower
{
public:
	/// Default Constructor
	CSpecialPowerModDefense() : CSpecialPower()
	{}

	/// Constructor
	CSpecialPowerModDefense(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float durationInSeconds,  
		const std::string& defenseMode, float modifier1, float modifier2 )
	{
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerModDefense> invalid data set row passed as actor");
		}

		_Phrase = phrase;
		
		_DisablePowerTime = 0;
		_Duration = NLMISC::TGameCycle(durationInSeconds / CTickEventHandler::getGameTimeStep());		

		_DefenseMode = defenseMode;
		_Modifier1 = modifier1;
		_Modifier2 = modifier2;

		if( NLMISC::toLower(_DefenseMode) == "dodge" )
			_PowerType = POWERS::ModDodgeSkill;
		else
		if( NLMISC::toLower(_DefenseMode) == "parry" )
			_PowerType = POWERS::ModParrySkill;
		else
			_PowerType = POWERS::ModDefenseSkill;
	}

	/// apply effects
	virtual void apply();


protected:
	/// Dodge or Parry ?
	std::string	_DefenseMode;

	// score modifier 
	float _Modifier1;
	float _Modifier2;

	/// Duration in ticks
	NLMISC::TGameCycle	_Duration;
};

#endif // RYZOM_SPECIAL_POWER_MOD_DEFENSE_H

/* End of special_power_mod_defense.h */
