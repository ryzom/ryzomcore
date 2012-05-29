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



#ifndef NL_FORAGE_SOURCE_SHEET_H
#define NL_FORAGE_SOURCE_SHEET_H

#include "nel/misc/types_nl.h"
#include "entity_sheet.h"


/**
 * Class to manage the forage source sheet.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CForageSourceSheet : public CEntitySheet
{
public:

	/// Constructor
	CForageSourceSheet() { Type = CEntitySheet::FORAGE_SOURCE; }

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize character sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	std::string					FxFilename;
	std::string					FxSafeFilename;
	uint8						Knowledge;
	std::vector<std::string>	Icons;
};


#endif // NL_FORAGE_SOURCE_SHEET_H

/* End of forage_source_sheet.h */
