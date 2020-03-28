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
#include "guild_grade.h"

namespace EGSPD
{

static const struct { const char* Name; CGuildGrade::TGuildGrade Value; } TGuildGradeConvert[] =
{
	{ "Leader", CGuildGrade::Leader },
	{ "HighOfficer", CGuildGrade::HighOfficer },
	{ "Officer", CGuildGrade::Officer },
	{ "Member", CGuildGrade::Member }
};
/* -----------------------------------------
* Static Implementation of CGuildGrade
* ----------------------------------------- */
void							CGuildGrade::init()
{
	_StrTable.clear();
	_ValueMap.clear();
	_StrTable.resize(4);
	uint	i;
	for (i=0; i<4; ++i)
	{
		_StrTable[TGuildGradeConvert[i].Value] = TGuildGradeConvert[i].Name;
		_ValueMap[NLMISC::toLower(std::string(TGuildGradeConvert[i].Name))] = TGuildGradeConvert[i].Value;
	}
	_Initialised = true;
}
bool							CGuildGrade::_Initialised = false;
std::string						CGuildGrade::_UnknownString = "Unknown";
std::vector<std::string>		CGuildGrade::_StrTable;
std::map<std::string, CGuildGrade::TGuildGrade>	CGuildGrade::_ValueMap;
// End of static implementation of CGuildGrade


} // End of EGSPD
