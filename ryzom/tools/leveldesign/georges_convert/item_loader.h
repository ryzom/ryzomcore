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

#ifndef NLGEORGES_ITEM_LOADER_H
#define NLGEORGES_ITEM_LOADER_H

#include "item.h"

namespace NLOLDGEORGES
{

class CItemLoader  
{
//	std::map< CStringEx, CFormFile* >;

public:
	CItemLoader();
	virtual ~CItemLoader();

	void LoadItem( CItem& _item, const CStringEx& _sxfilename );
	void LoadItem( CItem& _item, const CStringEx& _sxfilename, const CStringEx& _sxdate ); 
	void SaveItem( CItem& _item, const CStringEx& _sxfilename );
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_LOADER_H
