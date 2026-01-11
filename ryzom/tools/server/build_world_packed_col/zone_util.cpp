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

#include "std_header.h"
//
#include "zone_util.h"
//
#include "nel/misc/path.h"

//**********************************************************************************************
bool getZonePos(const std::string &name, sint &destX, sint &destY)
{
	if (name.empty())
	{
		printf ("empty name\n");
		return false;
	}

	static std::string zoneName;
	static std::string xStr, yStr;
	xStr.clear();
	yStr.clear();
	zoneName = NLMISC::CFile::getFilenameWithoutExtension(name);
	uint32 i = 0;
	while (zoneName[i] != '_')
	{
		if (!::isdigit(zoneName[i])) return false;
		yStr += zoneName[i]; ++i;
		if (i == zoneName.size())
			return false;
	}	
	++i;
	while (i < zoneName.size())
	{
		if (!::isalpha(zoneName[i])) return false;
		xStr += (char) ::toupper(zoneName[i]); ++i;
	}
	if (xStr.size() != 2) return false;
	// compute min corner
	destX = ((xStr[0] - 'A') * 26 + (xStr[1] - 'A'));
	NLMISC::fromString(yStr, destY);
	destY = -destY;
	return true;
}

//**********************************************************************************************
std::string posToZoneName(float x, float y)
{
	std::string zoneName;
	sint zoneX = (sint) floorf(x / 160.f);
	sint zoneY = (sint) floorf(y / 160.f);
	zoneName += NLMISC::toString(- zoneY) + "_";
	zoneName += (char) ('A' + zoneX / 26);
	zoneName += (char) ('A' + zoneX % 26);
	return zoneName;
}

