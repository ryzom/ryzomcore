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

#ifndef RY_ITEMS_FOR_SALE_H
#define RY_ITEMS_FOR_SALE_H

#include <vector>

#include "game_share/persistent_data.h"
#include "game_share/continent.h"

#include "item_for_sale.h"

class CCharacter;

class CItemsForSale
{
	NL_INSTANCE_COUNTER_DECL(CItemsForSale);
public:

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);
	
	DECLARE_PERSISTENCE_METHODS
		
	// constructor
	CItemsForSale() { _Character = 0; _NextIdentifier = 0; }

	// set owner character
	void setOwnerCharacter( CCharacter * c ) { nlassert(c != 0); _Character = c ; }

	// add an item in dynamic item list for sell it in shop store
	bool addItemForSale( uint32 price, uint32 retirePrice, CGameItemPtr item, uint32 quantity );

	// remove an item (removed by character or selled to another or selling delai reached)
	NLMISC::CSmartPtr< IItemTrade > removeItem( uint32 identifier, uint32 quantity, bool sellOffline );

	// get const reference on content
	const std::vector< TItemTradePtr >& getContent() const { return _ItemsForSale; }

	// serial
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream );

	// check coherency between CDynamicItems and CItemsForSale of character, assume CItemsForSale is a reference
	void checkSellStore( NLMISC::CEntityId charId );

	// return nb item in shop store
	uint32 getNbItemInShopStore() const { return (uint32)_ItemsForSale.size(); }

	// returns the number of items for sale on the given continent
	uint32 getNbItemsForSaleOnContinent(CONTINENT::TContinent continent) const;

	// remove items for sale on the given continent and get them
	std::vector<TItemTradePtr> removeItemsOnContinent(CONTINENT::TContinent continent);

private:
	CCharacter *					_Character;
	std::vector< TItemTradePtr >	_ItemsForSale;
	uint32							_NextIdentifier;
};

#endif
