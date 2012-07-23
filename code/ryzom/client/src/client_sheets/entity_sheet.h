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



#ifndef CL_ENTITY_SHEET_H
#define CL_ENTITY_SHEET_H

/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/sheet_id.h"


///////////
// USING //
///////////


///////////
// CLASS //
///////////
namespace NLGEORGES
{
	class UFormElm;
	class UFormLoader;
}

/**
 * Class to manage an entity sheet
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CEntitySheet
{
private:
	static std::vector<std::string> _Debug;

public:
	enum TType
	{
		CHAR = 0,
		FAUNA,
		FLORA,
		OBJECT,
		FX,
		BUILDING,
		ITEM,
		PLANT,
		MISSION,
		RACE_STATS,
		PACT,
		LIGHT_CYCLE,
		WEATHER_SETUP,
		CONTINENT,
		WORLD,
		WEATHER_FUNCTION_PARAMS,
		UNKNOWN,
		BOTCHAT,
		MISSION_ICON,
		SBRICK,
		SPHRASE,
		SKILLS_TREE,
		UNBLOCK_TITLES,
		SUCCESS_TABLE,
		AUTOMATON_LIST,
		ANIMATION_SET_LIST,
		SPELL, // obsolete
		SPELL_LIST, // obsolete
		CAST_FX, // obsolete
		EMOT,
		ANIMATION_FX,
		ID_TO_STRING_ARRAY,
		FORAGE_SOURCE,
		CREATURE_ATTACK,
		ANIMATION_FX_SET,
		ATTACK_LIST,
		SKY,
		TEXT_EMOT,
		OUTPOST,
		OUTPOST_SQUAD,
		OUTPOST_BUILDING,
		FACTION,
		TypeCount,
		UNKNOWN_SHEET_TYPE = TypeCount
	};

	/// Type of the sheet.
	TType				Type;
	/// Sheet Id.
	NLMISC::CSheetId	Id;

public:
	/// Add string to the debug stack.
	static void debug(const std::string &str);
	/// Flush the debug stack with a title parameter.
	static void flush(const std::string &title);

	/// Constructor
	CEntitySheet() { Type = UNKNOWN_SHEET_TYPE; }
	/// Destructor.
	virtual ~CEntitySheet() {}

	/// Build the entity from an external script.
	virtual void build(const NLGEORGES::UFormElm &item) = 0;

	/// Return the type of the sheet.
	inline TType type() const {return Type;}


	// TType enum/string conversion
	static const std::string &typeToString(TType e);
	static TType			  typeFromString(const std::string &s);


	/// Serialize character sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) = 0;
};


#endif // CL_ENTITY_SHEET_H

/* End of entity_sheet.h */
