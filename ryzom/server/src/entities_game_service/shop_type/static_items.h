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

#ifndef RYZOM_STATIC_ITEM_H
#define RYZOM_STATIC_ITEM_H

#include "game_share/people.h"
#include "game_share/item_type.h"
#include "game_share/rm_family.h"


/**
 * CStaticItems
 * Mount list of items selling in game
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 */
class CStaticItems
{
public:
	struct CRMForOneItemPart
	{
		std::vector< NLMISC::CSheetId >	RawMaterial;
	};

	// build static item vector
	static void buildStaticItem();		

	// CStaticItems::buildOntItem use faber system for make an item
	static CGameItemPtr buildOnetItem( NLMISC::CSheetId sheet );

	// init raw material table used for build static item
	static void initRmTable();

	// select raw material for build static item
	static void selectRmForCraft( const NLMISC::CSheetId& craftPlan, std::vector< NLMISC::CSheetId >& Rm, std::vector< NLMISC::CSheetId >& RmFormula );

	// return StaticItem reference
	static inline const std::vector< CGameItemPtr > & getStaticItems() {return _StaticItems;}

private:
	static std::vector< CGameItemPtr >	_StaticItems;
	static std::vector< CRMForOneItemPart >	_RmForSystemCraft;
};

#endif // RYZOM_STATIC_ITEM_H

/* static_items.h */

