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

#include "pd_string_mapper.h"

/*
 * Unknown String
 */
std::string		CPDStringMapper::_UnknownString = "Unknown";


/*
 * Constructor
 */
CPDStringMapper::CPDStringMapper()
{
}


/*
 * Set Mapping
 */
void	CPDStringMapper::setMapping(const std::string& str, uint32 id)
{
	std::string				lowMapStr = NLMISC::toLower(str);
	TStringMap::iterator	its;

	TIdMap::iterator		iti = _IdMap.find(id);
	if (iti != _IdMap.end())
	{
		its = (*iti).second;;
		if ((*its).first != lowMapStr || (*its).second != id)
		{
			nlwarning("CPDStringMapper::setMapping(): failed to map '%s' to '%d', id is already mapped to a different string", str.c_str(), id);
			return;
		}

		return;
	}

	its = _StringMap.find(lowMapStr);

	if (its != _StringMap.end())
	{
		if ((*its).second != id)
		{
			nlwarning("CPDStringMapper::setMapping(): failed to map '%s' to '%d', string is already mapped to a different id", str.c_str(), id);
			return;
		}

		return;
	}

	its = _StringMap.insert(std::make_pair<std::string, uint32>(lowMapStr, id)).first;
	_IdMap[id] = its;
}

/*
 * Serial Mapper
 */
void	CPDStringMapper::serial(NLMISC::IStream& f)
{
	f.serialCheck(NELID("PDSM');

	uint	version = f.serialVersion(0);

	f.serialCont(_StringMap);

	if (f.isReading())
	{
		buildIdMap();
	}
}

/*
 * Rebuild Id Mapping
 */
void	CPDStringMapper::buildIdMap()
{
	_IdMap.clear();

	TStringMap::iterator	it;
	for (it=_StringMap.begin(); it!=_StringMap.end(); ++it)
	{
		const std::string&	str = (*it).first;
		uint32				id = (*it).second;

		_IdMap[id] = it;
	}
}
