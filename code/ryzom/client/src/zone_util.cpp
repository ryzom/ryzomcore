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
#include "zone_util.h"
#include "nel/misc/path.h"
#include "nel/misc/vector_2f.h"

using namespace std;


bool getPosFromZoneName(const std::string &name,NLMISC::CVector2f &dest)
{

	if (name.empty())
	{
		nlwarning ("getPosFromZoneName(): empty name, can't getPosFromZoneName");
		return false;
	}

	static std::string zoneName;
	static string xStr, yStr;
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
	dest.x = 160.f * ((xStr[0] - 'A') * 26 + (xStr[1] - 'A'));
	sint nY;
	NLMISC::fromString(yStr, nY);
	dest.y = 160.f * -nY;
	return true;
}

bool getZonePosFromZoneName(const std::string &name, sint &x, sint &y)
{

	if (name.empty())
	{
		nlwarning ("getPosFromZoneName(): empty name, can't getPosFromZoneName");
		return false;
	}

	static std::string zoneName;
	static string xStr, yStr;
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
	x = (xStr[0] - 'A') * 26 + (xStr[1] - 'A');
	NLMISC::fromString(yStr, y);
	y = -y;
	return true;
}



