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



#ifndef RY_CHARACTERISTICS_H
#define RY_CHARACTERISTICS_H

#include "nel/misc/types_nl.h"

#include <string.h>

namespace CHARACTERISTICS
{
	enum TCharacteristics
	{
		constitution = 0, //HP max
		metabolism, //Hp Regen

		intelligence, //Sap Max
		wisdom,	//Sap regen

		strength, //Stamina Max
		well_balanced, //Stamina regen

		dexterity, //Focus Max
		will, //Focus regen

		NUM_CHARACTERISTICS,
		Unknown = NUM_CHARACTERISTICS
	};

	/**
	 * get the right characteristic enum from the input string
	 * \param str the input string
	 * \return the ECharacteristics associated to this string (unknown if the string cannot be interpreted)
	 */
	TCharacteristics toCharacteristic( const std::string &str );

	/**
	 * get the code associated to his characteristic
	 * \param c is the enum number of characteristic
	 * \return associated code
	 */
	TCharacteristics getCharacteristicFromCode( const std::string &code );

	/**
	 * get the characteristic associated to a code
	 * \param code the code to find
	 * \return associated enum value
	 */
	const std::string &getCharacteristicCode( TCharacteristics c );

	/**
	 * get the right characteristic string from the gived enum
	 * \param c is the enum number of characteristic
	 * \return the string associated to this enum number (Unknown if enum number not exist)
	 */
	const std::string& toString( TCharacteristics c );
	const std::string& toString( uint c );

}; // CHARACTERISTICS

#endif // RY_CHARACTERISTICS_H
/* End of charateristics.h */
