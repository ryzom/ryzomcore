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



#ifndef CL_EMOT_LIST_SHEET_H
#define CL_EMOT_LIST_SHEET_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"

#include "entity_sheet.h"
#include "animation_set_list_sheet.h"

/////////////
// CLASSES //
/////////////
/**
 * List of emots to be used in the EAM
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date December 2003
 */
class CEmotListSheet : public CEntitySheet
{
public:

	std::vector<TAnimStateId> Emots;

public:
	/// Constructor
	CEmotListSheet();

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};


#endif // CL_EMOT_LIST_SHEET_H

/* End of emot_list_sheet.h */
