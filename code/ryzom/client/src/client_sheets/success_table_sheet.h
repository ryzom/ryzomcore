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



#ifndef NL_SUCCESS_TABLE_SHEET_H
#define NL_SUCCESS_TABLE_SHEET_H

#include "entity_sheet.h"


// ***************************************************************************
/**
 * <Class description>
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CSuccessTableSheet : public CEntitySheet
{
public:


	/// The Difficulty Table
	struct CSuccessEntry
	{
		sint32	RelativeLevel;
		sint32	SuccessProbability;
		sint32	PartialSuccessProbability;
		void	serial(NLMISC::IStream &f)
		{
			f.serialVersion(0);
			f.serial(RelativeLevel);
			f.serial(SuccessProbability);
			f.serial(PartialSuccessProbability);
		}
	};
	std::vector<CSuccessEntry>		SuccessTable;


public:

	/// Constructor
	CSuccessTableSheet()
	{
		Type = SUCCESS_TABLE;
	}

	/// destructor
	virtual ~CSuccessTableSheet() {}

	virtual void build(const NLGEORGES::UFormElm &root);

	/// serialize
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialCont(SuccessTable);
	}


};


#endif // NL_SUCCESS_TABLE_SHEET_H

/* End of success_table_sheet.h */
