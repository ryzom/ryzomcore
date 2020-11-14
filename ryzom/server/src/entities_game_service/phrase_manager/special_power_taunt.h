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



#ifndef RYZOM_SPECIAL_POWER_TAUNT_H
#define RYZOM_SPECIAL_POWER_TAUNT_H

#include "special_power.h"


/**
 * Specialized class for power "taunt"
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerTaunt : public CSpecialPower
{
public:
	/// Default Constructor
	CSpecialPowerTaunt() : CSpecialPower()
	{}

	/// Constructor
	CSpecialPowerTaunt(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, uint16 power, float range, float disableTimeInSeconds) 
		: _TauntPower(power), _Range(range)
	{
		_Phrase = phrase;
		
		_DisablePowerTime = uint32(disableTimeInSeconds / CTickEventHandler::getGameTimeStep());

		_PowerType = POWERS::Taunt;
		
		if(TheDataset.isAccessible(actorRowId))
			_ActorRowId = actorRowId;
		else
		{
			nlwarning("<CSpecialPowerTaunt> invalid data set row passed as actor");
		}
	}

	/// validate the power utilisation
	virtual bool validate(std::string &errorCode);

	/// apply effects
	virtual void apply();

protected:
	/**
     *test if the taunt works on given entity
	 * \param entity the entity to taunt
	 * \return true if the taunt is successful, false otherwise
	 */
	bool testTauntSuccessOnEntity(const CEntityBase *entity);

protected:
	/// taunt power (cannot taunt creatures above this "level")
	uint16	_TauntPower;

	/// max range in meters
	float	_Range;
};

#endif // RYZOM_SPECIAL_POWER_TAUNT_H

/* End of special_power_taunt.h */
