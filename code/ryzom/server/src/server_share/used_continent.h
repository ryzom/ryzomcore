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


#ifndef USED_CONTINENT_H
#define USED_CONTINENT_H

#include "nel/misc/types_nl.h"
#include "nel/net/service.h"
#include "game_share/continent.h"

#include <algorithm>


/** Singleton class to handle the list of continent used and instance number.
 *	This class also handle the physical continent name translation table
*/
class CUsedContinent
{
public:

	struct TContinentInfo
	{
		std::string				ContinentName;
		CONTINENT::TContinent	ContinentEnum;
		uint32					ContinentInstance;
	};

	typedef std::vector<TContinentInfo>		TUsedContinentCont;


	static CUsedContinent	&instance()
	{
		if (_Instance == 0)
			_Instance = new CUsedContinent;

		return *_Instance;
	}

	const TUsedContinentCont &getContinents() const
	{
		return _Continents;
	}

	/* Return true if the continent is referenced in the used continent list.
	*/
	bool	isContinentUsed(const std::string &continentName) const;

	/** Return the static instance number associated with a continent name.
	*	If the continent name is unknow, return ~0
	*/
	uint32	getInstanceForContinent(const std::string &continentName) const;

	/** Return the static instance number associated with a continent enum value
	 *	If the continent name is unknow, return ~0
	 */
	uint32	getInstanceForContinent(CONTINENT::TContinent continentEnum) const;

	/** Return the continent name associated with a static instance number.
	*	If the static instance number is unknow, return an empty string.
	*/
	const std::string &getContinentForInstance(uint32 instanceNumber) const;

	/** Return the name of the physical continent associated to the given 
	 *  logical continent name.
	 *	If there is no translation for the logical name, then the logical name
	 *	is returned as physical name.
	 */
	std::string getPhysicalContinentName(const std::string &locicalName) const;
private:

	// functor to search a continent from name
	struct TFindContinent : public std::unary_function<TContinentInfo, bool>
	{ 
		bool operator ()(const TContinentInfo &ci) const
		{
			return ci.ContinentName == ContinentName;
		}

		TFindContinent(const std::string &continentName)
			:	ContinentName(continentName)
		{}
		const std::string &ContinentName;
	};

	// functor to search a continent from enum Value
	struct TFindContinentFromEnum : public std::unary_function<TContinentInfo, bool>
	{
		bool operator ()(const TContinentInfo &ci) const
		{
			return ci.ContinentEnum == ContinentEnum;
		}
		
		TFindContinentFromEnum(CONTINENT::TContinent continentEnum)
			:	ContinentEnum(continentEnum)
		{}

		CONTINENT::TContinent ContinentEnum;
	};

	// functor to search an instance number
	struct TFindInstance : public std::unary_function<TContinentInfo, bool>
	{ 
		bool operator ()(const TContinentInfo &ci) const
		{
			return ci.ContinentInstance == InstanceNumber;
		}

		TFindInstance(uint32 instanceNumber)
			:	InstanceNumber(instanceNumber)
		{}
		const uint32 InstanceNumber;
	};

	// the singleton instance
	static CUsedContinent	*_Instance;
	/// private constructor
	CUsedContinent();
	/// The continents
	TUsedContinentCont		_Continents;

	/// Physical continent name translation table
	std::map<std::string, std::string>	_PhysicalNames;
};

#endif // USED_CONTINENT_H
