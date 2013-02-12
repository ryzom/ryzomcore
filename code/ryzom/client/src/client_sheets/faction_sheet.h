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

#ifndef RY_FACTION_SHEET_H
#define RY_FACTION_SHEET_H

#include "entity_sheet.h"

/** Sheet of a faction
  *  NB : it doesn't derives from CEntitySheet, because its instances are aggragated in a CContinentSheet
  *
  * \author Jerome Vuarand
  * \author Nevrax France
  * \date 2005
  */

class CFactionSheet : public CEntitySheet
{
public:
	std::string Icon;

	// ctor
	CFactionSheet()
	{
		Type = FACTION;
	}

	/// Build the sheet from an external script.
	void build(const NLGEORGES::UFormElm &item);

	/// Serialize character sheet into binary data file.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

};






#endif
