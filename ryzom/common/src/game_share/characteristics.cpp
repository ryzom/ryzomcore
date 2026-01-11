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



#include "stdpch.h"

#include "nel/misc/i18n.h"
#include "nel/misc/string_conversion.h"

#include "characteristics.h"

using namespace std;
using namespace NLMISC;

namespace CHARACTERISTICS
{
	// The conversion table
	const CStringConversion<TCharacteristics>::CPair stringTable [] =
	{
		{ "Constitution", constitution },	//HP max
		{ "Metabolism", metabolism },		//Hp Regen
		{ "Intelligence", intelligence },	//Sap Max
		{ "Wisdom", wisdom },				//Sap regen
		{ "Strength", strength },			//Stamina Max
		{ "WellBalanced", well_balanced },	//Stamina regen
		{ "Dexterity", dexterity },			//Focus Max
		{ "Will", will },					//Focus regen

		{ "Unknown", Unknown },

	};
	CStringConversion<TCharacteristics> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]), Unknown );


	/**
	 * get the right characteristic enum from the input string
	 * \param str the input string
	 * \return the TCharacteristics associated to this string (unknown if the string cannot be interpreted)
	 */
	TCharacteristics toCharacteristic( const std::string &str )
	{
		return conversion.fromString(str);
	}

	/**
	 * get the right characteristic string from the gived enum
	 * \param c is the enum number of characteristic
	 * \return the string associated to this enum number (Unknown if enum number not exist)
	 */
	const std::string& toString( TCharacteristics c )
	{
		return conversion.toString(c);
	}
	const std::string& toString( uint c )
	{
		return conversion.toString((TCharacteristics)c);
	}

	// The code conversion table
	const CStringConversion<TCharacteristics>::CPair codeTable [] =
	{
		{ "c", constitution },
		{ "m", metabolism },
		{ "i", intelligence },
		{ "w", wisdom },
		{ "s", strength },
		{ "b", well_balanced },
		{ "d", dexterity },
		{ "l", will },
	};
	CStringConversion<TCharacteristics> codeCharac(codeTable, sizeof(codeTable) / sizeof(codeTable[0]), Unknown );

	const std::string &getCharacteristicCode( TCharacteristics c )
	{
		return codeCharac.toString(c);
	}

	TCharacteristics getCharacteristicFromCode( const std::string &code )
	{
		return codeCharac.fromString(code);
	}
}; // CHARACTERISTICS
