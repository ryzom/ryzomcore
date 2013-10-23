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



#ifndef RYZOM_CONTINENT_H
#define RYZOM_CONTINENT_H

#include "nel/misc/types_nl.h"

namespace CONTINENT
{
	enum TContinent
	{
		FYROS = 0,
		ZORAI,
		TRYKER,
		MATIS,
		BAGNE,
		NEXUS,
		ROUTE_GOUFFRE,
		SOURCES,
		TERRE,
		FYROS_ISLAND,
		FYROS_NEWBIE,
		TRYKER_ISLAND,
		TRYKER_NEWBIE,
		ZORAI_ISLAND,
		MATIS_ISLAND,
		ZORAI_NEWBIE,
		MATIS_NEWBIE,
		TESTROOM,
		INDOORS,
		NEWBIELAND,
		R2_ROOTS,
		R2_DESERT,
		R2_LAKES,
		R2_FOREST,
		R2_JUNGLE,
		CORRUPTED_MOOR,
		KITINIERE,

		UNKNOWN,
//		NB_RESPAWN_POINT_TYPE = UNKNOWN,
		NB_CONTINENTS = UNKNOWN
	};


	/**
	 * get continent type corresponding to input string
	 * \param str the input string
	 * \return the TContinent associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	TContinent toContinent(const std::string &str);

	/**
	 * get the continent type string corresponding to enum
	 * \param nature the TContinent value
	 * \return nature as a string (or UNKNOWN)
	 */
	const std::string& toString(TContinent continent);

	// A small wrapper around the continent enum to allow it
	// to be used as a serialisable map key.
	struct TContinentId
	{

		TContinentId()
			: _Continent(CONTINENT::UNKNOWN)
		{}

		TContinentId(CONTINENT::TContinent cont)
			: _Continent(cont)
		{}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Continent);
		}

		bool operator < (const TContinentId &other) const
		{
			return _Continent < other._Continent;
		}

		bool operator == (const TContinentId &other) const
		{
			return _Continent == other._Continent;
		}

	private:
		CONTINENT::TContinent _Continent;
	};

	// A container to host respawn points counters
	typedef std::map < TContinentId, uint32 >	TRespawnPointCounters;


}; // namespace CONTINENT

#endif // RYZOM_CONTINENT_H
/* End of continent.h */
