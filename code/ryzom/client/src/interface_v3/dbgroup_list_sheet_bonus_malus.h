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



#ifndef NL_DBGROUP_LIST_SHEET_BONUS_MALUS_H
#define NL_DBGROUP_LIST_SHEET_BONUS_MALUS_H

#include "nel/misc/types_nl.h"
#include "dbgroup_list_sheet.h"


// ***************************************************************************
/**
 * Special list_sheet that display some disalbe bitmap if needed according to DB
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CDBGroupListSheetBonusMalus : public CDBGroupListSheet
{
public:

	/// Constructor
	CDBGroupListSheetBonusMalus(const TCtorParam &param);

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

	virtual void draw ();

private:
	sint32							_TextId;

	std::vector<NLMISC::CCDBNodeLeaf*>		_DisableStates;
};


#endif // NL_DBGROUP_LIST_SHEET_BONUS_MALUS_H

/* End of dbgroup_list_sheet_bonus_malus.h */
