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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.
// Client
#include "entity_sheet.h"
//
#include "nel/misc/string_conversion.h"

using namespace NLMISC;

static const CStringConversion<CEntitySheet::TType>::CPair sheetTypeStringTableDef[] =
{
	{ "CHAR", CEntitySheet::CHAR },
	{ "FAUNA", CEntitySheet::FAUNA },
	{ "FLORA", CEntitySheet::FLORA },
	{ "OBJECT", CEntitySheet::OBJECT },
	{ "FX", CEntitySheet::FX },
	{ "BUILDING", CEntitySheet::BUILDING },
	{ "ITEM", CEntitySheet::ITEM },
	{ "PLANT", CEntitySheet::PLANT },
	{ "MISSION", CEntitySheet::MISSION },
	{ "RACE_STATS", CEntitySheet::RACE_STATS },
	{ "PACT", CEntitySheet::PACT },
	{ "LIGHT_CYCLE", CEntitySheet::LIGHT_CYCLE },
	{ "WEATHER_SETUP", CEntitySheet::WEATHER_SETUP },
	{ "CONTINENT", CEntitySheet::CONTINENT },
	{ "WORLD", CEntitySheet::WORLD },
	{ "WEATHER_FUNCTION_PARAMS", CEntitySheet::WEATHER_FUNCTION_PARAMS },
	{ "BOTCHAT", CEntitySheet::BOTCHAT },
	{ "MISSION_ICON", CEntitySheet::MISSION_ICON },
	{ "SBRICK", CEntitySheet::SBRICK },
	{ "SPHRASE", CEntitySheet::SPHRASE },
	{ "SKILLS_TREE", CEntitySheet::SKILLS_TREE },
	{ "UNBLOCK_TITLES", CEntitySheet::UNBLOCK_TITLES },
	{ "SUCCESS_TABLE", CEntitySheet::SUCCESS_TABLE },
	{ "AUTOMATON_LIST", CEntitySheet::AUTOMATON_LIST },
	{ "ANIMATION_SET_LIST", CEntitySheet::ANIMATION_SET_LIST },
	{ "SPELL", CEntitySheet::SPELL },
	{ "SPELL_LIST", CEntitySheet::SPELL_LIST },
	{ "CAST_FX", CEntitySheet::CAST_FX },
	{ "EMOT", CEntitySheet::EMOT },
	{ "ANIMATION_FX", CEntitySheet::ANIMATION_FX },
	{ "ID_TO_STRING_ARRAY", CEntitySheet::ID_TO_STRING_ARRAY },
	{ "FORAGE_SOURCE", CEntitySheet::FORAGE_SOURCE },
	{ "CREATURE_ATTACK", CEntitySheet::CREATURE_ATTACK },
	{ "ANIMATION_FX_SET", CEntitySheet::ANIMATION_FX_SET },
	{ "ATTACK_LIST", CEntitySheet::ATTACK_LIST },
	{ "SKY", CEntitySheet::SKY },
	{ "TEXT_EMOT", CEntitySheet::TEXT_EMOT },
	{ "OUTPOST", CEntitySheet::OUTPOST },
	{ "OUTPOST_SQUAD", CEntitySheet::OUTPOST_SQUAD },
	{ "FACTION",	 CEntitySheet::FACTION }
};
static CStringConversion<CEntitySheet::TType> sheetTypeStringTable(sheetTypeStringTableDef, sizeofarray(sheetTypeStringTableDef),  CEntitySheet::UNKNOWN_SHEET_TYPE);

std::vector<std::string> CEntitySheet::_Debug;

//-----------------------------------------------
// debug :
// Add string to the debug stack.
//-----------------------------------------------
void CEntitySheet::debug(const std::string &str)
{
	// Add the string to the debug stack.
	_Debug.push_back(str);
}// debug //

//-----------------------------------------------
// flush :
// Flush the debug stack with a title parameter.
//-----------------------------------------------
void CEntitySheet::flush(const std::string &/* title */)
{
	// If debug stack is not empty
	// temp
	/*
	if(!_Debug.empty())
	{
		nlwarning("Bug in the form '%s':", title.c_str());
		for(uint i=0; i<_Debug.size(); ++i)
			nlwarning("  %s", _Debug[i]);

		// Empty line separator
		nlwarning("");

		// Clean the stack.
		_Debug.clear();
	}
	*/
}// flush //

// ***************************************************************************
const std::string	&CEntitySheet::typeToString(TType e)
{
	return sheetTypeStringTable.toString(e);
}

// ***************************************************************************
CEntitySheet::TType	CEntitySheet::typeFromString(const std::string &s)
{
	return sheetTypeStringTable.fromString(s);
}
