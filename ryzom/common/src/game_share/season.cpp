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
#include "season.h"

namespace EGSPD
{

static const struct { const char* Name; CSeason::TSeason Value; } TSeasonConvert[] =
{
	{ "Spring", CSeason::Spring },
	{ "Summer", CSeason::Summer },
	{ "Autumn", CSeason::Autumn },
	{ "Winter", CSeason::Winter },
	{ "Invalid", CSeason::Invalid },
};
/* -----------------------------------------
* Static Implementation of CSeason
* ----------------------------------------- */
void							CSeason::init()
{
	_StrTable.clear();
	_ValueMap.clear();
	_StrTable.resize(5);
	uint	i;
	for (i=0; i<5; ++i)
	{
		_StrTable[TSeasonConvert[i].Value] = TSeasonConvert[i].Name;
		_ValueMap[NLMISC::toLower(std::string(TSeasonConvert[i].Name))] = TSeasonConvert[i].Value;
	}
	_Initialised = true;
}
bool							CSeason::_Initialised = false;
std::string						CSeason::_UnknownString = "Unknown";
std::vector<std::string>		CSeason::_StrTable;
std::map<std::string, CSeason::TSeason>	CSeason::_ValueMap;
// End of static implementation of CSeason


} // End of EGSPD
