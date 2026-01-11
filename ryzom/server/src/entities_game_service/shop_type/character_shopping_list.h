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

#ifndef RYZOM_CHARACTER_SHOPPING_LIST_H
#define RYZOM_CHARACTER_SHOPPING_LIST_H

#include "game_share/item_type.h"
#include "game_share/rm_family.h"
#include "game_share/continent.h"

#include "shop_unit.h"
#include "shop_type/merchant.h"

class CCharacter;

/**
 * CCharacterShoppingList
 * Manage merchant item list consulted by character for trading
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 */
class CCharacterShoppingList : public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(CCharacterShoppingList);
public:

	struct TShopStruct
	{
		TItemTradePtr		ItemTrade;
		const IShopUnit		*ShopUnit;
		bool				Retirable;

		TShopStruct( TItemTradePtr i, const IShopUnit * s, bool retirable = false )
		{
			ItemTrade = i;
			ShopUnit = s;
			Retirable = retirable;
		}
	};

	// constructor
	CCharacterShoppingList( NLMISC::CSmartPtr<CMerchant>& merchant, CCharacter& character, float fameFactor );

	// destructor
	~CCharacterShoppingList();

	// mount shopping list with merchant shop
	void mountShoppingList( CONTINENT::TContinent continent );

	// init page update data
	void initPageToUpdate( uint32 nbSlotPerPage );

	// fill one trade page in character database
	void fillTradePage( uint16 session );

	// character buy an item from shopping list
	void buyItem( uint16 itemNumber, uint32 quantity );

	// character destroy it's own item from shopping list
	void destroyItem( uint16 itemNumber, uint32 quantity );
		
	// return item from current trade list
	const CGameItemPtr getItemIndexFromTradeList( uint32 idx );

	// return true if item idx corresponding to an item solded by npc
	bool isItemSoldedByNpc( uint32 idx ) const;

	// return true if item exist and fill sheet and level from trade list
	bool getItemSheetAndQualityFromTradeList( uint32 idx, NLMISC::CSheetId& sheet, uint32& level ) const;
		
	// return true if selled item match with current filters
	bool passThruFilter(TItemTradePtr itemTrade, bool dynamicShop );

	// check altar restriction if item is TP ticket
	bool checkAltarRestriction();

	// return sell price of item
	uint32 getSellPrice( const TItemTradePtr itemTrade, bool dynnamicShop );

private:
	// return true if item are succesfully added to bag
	bool addBoughtItemToBag( CGameItemPtr item, uint32 price, const CStaticItem * form );
	
	NLMISC::CSmartPtr< CMerchant >	_CurrentMerchant;
	CCharacter *					_Character;
	float							_FameFactor;
	std::vector< TShopStruct >		_CurrentTradeListNpc;
	std::vector< TShopStruct >		_CurrentTradeListPlayer;
	std::vector< TShopStruct >		_CurrentTradeListYours;

	uint32							_NbSlotPerPage;
	std::list<uint16>				_TradePagesToUpdate;
	
	
};

#endif // RYZOM_CHARACTER_SHOPPING_LIST_H

/* character_shopping_list.h */

