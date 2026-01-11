// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_ZONE_UTILITY
#define NL_ZONE_UTILITY

#include "nel/misc/types_nl.h"
#include <vector>
#include <set>

std::string getDir (const std::string& path);
std::string getName (const std::string& path);
std::string getExt (const std::string& path);
bool getZoneCoordByName(const char * name, uint16& x, uint16& y);
void getLettersFromNum(uint16 num, std::string& code);
void getZoneNameByCoord(uint16 x, uint16 y, std::string& zoneName);
void getAdjacentZonesName(const std::string& zoneName, std::vector<std::string>& names);
uint16 createZoneId(std::string zoneName);

#endif // NL_ZONE_UTILITY
