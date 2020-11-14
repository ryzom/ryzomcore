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
#include "sp_type.h"

namespace EGSPD
{

static const struct { const char* Name; CSPType::TSPType Value; } TSPTypeConvert[] =
{
	{ "Fight", CSPType::Fight },
	{ "Magic", CSPType::Magic },
	{ "Craft", CSPType::Craft },
	{ "Harvest", CSPType::Harvest },
};
/* -----------------------------------------
* Static Implementation of CSPType
* ----------------------------------------- */
void							CSPType::init()
{
	_StrTable.clear();
	_ValueMap.clear();
	_StrTable.resize(4);
	uint	i;
	for (i=0; i<4; ++i)
	{
		_StrTable[TSPTypeConvert[i].Value] = TSPTypeConvert[i].Name;
		_ValueMap[NLMISC::toLower(std::string(TSPTypeConvert[i].Name))] = TSPTypeConvert[i].Value;
	}
	_Initialised = true;
}
bool							CSPType::_Initialised = false;
std::string						CSPType::_UnknownString = "Unknown";
std::vector<std::string>		CSPType::_StrTable;
std::map<std::string, CSPType::TSPType>	CSPType::_ValueMap;
// End of static implementation of CSPType


} // End of EGSPD
