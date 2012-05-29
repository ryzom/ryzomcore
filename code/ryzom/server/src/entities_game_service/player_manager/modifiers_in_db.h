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


#ifndef RY_MODIFIERS_DB_H
#define RY_MODIFIERS_DB_H

// Game Share
#include "game_share/persistent_data.h"
//
#include "player_manager/cdb.h"
#include "player_manager/cdb_synchronised.h"

// struct for modifier written in DB
struct CModifierInDB
{
	NLMISC::TGameCycle	ActivationDate;
	NLMISC::CSheetId	SheetId;
	bool				Disabled;
	
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CModifierInDB();

	void clear();

	void init();

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};

/// struct for disabled modifiers
struct CModifiersInDB
{
	std::vector<CModifierInDB>	Bonus;
	std::vector<CModifierInDB>	Malus;
	
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CModifiersInDB();

	void clear();

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	// write disabled effects in DB, used only in initDatabase
	void writeInDatabase(CCDBSynchronised &database);

	void update(CCDBSynchronised &database);

	/// add an active effect
	sint8 addEffect(const NLMISC::CSheetId &sheetId, bool bonus, CCDBSynchronised &database);

	/// remove an effect
	void removeEffect(uint8 index, bool bonus, CCDBSynchronised &database);

	/// disable an effect
	void disableEffect(uint8 index, bool bonus, NLMISC::TGameCycle activationDate, CCDBSynchronised &database);
	
private:
	void _addBonus(const CModifierInDB& bonus);
	void _addMalus(const CModifierInDB& malus);
};


#endif //RY_MODIFIERS_DB_H


/* End of modifiers_in_db.h */
