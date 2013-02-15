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

#ifndef RYZOM_SOLDED_ITEMS_H
#define RYZOM_SOLDED_ITEMS_H

// Misc
#include "nel/misc/entity_id.h"
//
#include "item_for_sale.h"

class CCharacter;

/**
 * CSoldedItems
 * Keep list of solded item when owner are offline
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 */
class CSoldedItems
{
	NL_INSTANCE_COUNTER_DECL(CSoldedItems);
public:

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);
	
//	DECLARE_PERSISTENCE_METHODS
		
	struct TSoldedItem
	{
		DECLARE_PERSISTENCE_METHODS
			
		TItemTradePtr Item;
		uint32 Quantity;
	};

	typedef std::vector< TSoldedItem > TMapElem;
	typedef std::map< NLMISC::CEntityId, TMapElem > TItemSolded;

	// get instance
	static CSoldedItems * getInstance();
	
	// an item are solded
	void soldedItem( TItemTradePtr item, uint32 quantity, CCharacter * character );

	// character log
	void characterLogon( CCharacter * character );

	// save
	void save() {}

private:
	// constructor
	CSoldedItems() {}
	
	static CSoldedItems *	_Instance;
	TItemSolded				_SoldedItem;
};

#endif // RYZOM_SOLDED_ITEMS_H

/* solded_items.h */
