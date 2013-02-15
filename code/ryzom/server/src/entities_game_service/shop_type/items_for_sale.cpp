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

#include "stdpch.h"

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "shop_type/shop_type_manager.h"
#include "shop_type/items_for_sale.h"
#include "dynamic_items.h"

using namespace NLMISC;
using namespace std;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


NL_INSTANCE_COUNTER_IMPL(CItemsForSale);

//-----------------------------------------------------------------------------
bool CItemsForSale::addItemForSale( uint32 price, uint32 retirePrice, CGameItemPtr item, uint32 quantity )
{
#ifdef NL_DEBUG
	nlassert( _Character != 0 );
	nlassert( item != NULL );
//	nlassert( item->parent() == 0 );
	nlassert( item->getInventory() == 0 );
#endif
	if( _Character->getCurrentContinent() == CONTINENT::UNKNOWN )
	{
		nlwarning("<CItemsForSale::addItemForSale> Character %s is on unknown continent", _Character->getId().toString().c_str() );
		return false;
	}

	if( _ItemsForSale.size() >= NBMaxItemYoursSellDisplay )
	{
		return false;
	}

	if( item != 0 )
	{
		// TODO : check if this 'detach' is needed
//		item->detachFromParent();

		CSmartPtr< IItemTrade > ptr = new CItemForSale();
		ptr->itemForSale( price, retirePrice, item, quantity, _Character->getId(), _Character->getCurrentContinent(), _NextIdentifier++ );
		if( CDynamicItems::getInstance()->addDynamicItemForSell( ptr ) )
		{
			_ItemsForSale.push_back( ptr );
			return true;
		}
		// for prevent delete item if it can' be solded
		ptr->setItemPtr( 0 );
	}
	return false;
}

//-----------------------------------------------------------------------------
NLMISC::CSmartPtr< IItemTrade > CItemsForSale::removeItem( uint32 identifier, uint32 quantity, bool sellOffline )
{
	for( uint32 index = 0; index < _ItemsForSale.size(); ++index )
	{
		if( _ItemsForSale[ index ]->getIdentifier() == identifier )
		{
			if( sellOffline )
				_ItemsForSale[ index ]->removeQuantity( quantity );
			if( _ItemsForSale[ index ]->getQuantity() == 0 )
			{
				NLMISC::CSmartPtr< IItemTrade > itemTrade = _ItemsForSale[ index ];
				_ItemsForSale[ index ] = _ItemsForSale.back();
				_ItemsForSale.pop_back();
				return itemTrade;
			}
			else
			{
				return _ItemsForSale[ index ];
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void CItemsForSale::serial(NLMISC::IStream &f) throw(NLMISC::EStream )
{
	// YOYO: we should not be here. OLD serial was very buggy.
	// AlainS ensure me this code is no more called (no write, use PDS instead, and old version
	// on read don't have ItemsForSale)
	nlstop;
}

//-----------------------------------------------------------------------------
void CItemsForSale::checkSellStore( CEntityId charId )
{
	std::vector< TItemTradePtr > itemsForSaleOfCharacter;
	std::vector< TItemTradePtr > itemsOfCharacterMustBeRemoved;
	CDynamicItems::getInstance()->getItemsOfCharacter( charId, itemsForSaleOfCharacter );

	uint32 inSell = 0;
	// check for missing item in sell store
	for( uint32 i = 0; i < _ItemsForSale.size(); ++i )
	{
		bool mustBeAdded = true;
		for( uint32 j = 0; j < itemsForSaleOfCharacter.size(); ++j )
		{
			if( _ItemsForSale[ i ]->getIdentifier() == itemsForSaleOfCharacter[ j ]->getIdentifier() )
			{
				++inSell;
				mustBeAdded = false;
				
				IItemTrade * loadedItemTrade = _ItemsForSale[ i ];
				_ItemsForSale[ i ] = itemsForSaleOfCharacter[ j ];
				continue;
			}
		}
		if( mustBeAdded )
		{
			if( _ItemsForSale[ i ]->getQuantity() > 0 )
			{
				// item missing
				CDynamicItems::getInstance()->addDynamicItemForSell( _ItemsForSale[ i ] );
				++inSell;
			}
			else
			{
				// item have zero quantity due to a previous bug, must be removed from player sell
				itemsOfCharacterMustBeRemoved.push_back( _ItemsForSale[ i ] );
			}
		}
	}

	// remove item for sell with zero quantity (due to previous bug when item reach maximum sale date with character is offline)
	for( uint32 i = 0; i < itemsOfCharacterMustBeRemoved.size(); ++i )
	{
		removeItem( itemsOfCharacterMustBeRemoved[ i ]->getIdentifier(), 0, false );
	}
	itemsOfCharacterMustBeRemoved.clear();

	// if item in sell for character is the same than CDynamicItems, all is synchronized
	if( inSell == itemsForSaleOfCharacter.size() )
		return;

	// not synchronized, some item of character are in sell in CDynamicItems but not in character, remove it
	for( uint32 i = 0; i < itemsForSaleOfCharacter.size(); ++i )
	{
		bool mustBeRemoved = true;
		for( uint32 j = 0; j < _ItemsForSale.size(); ++j )
		{
			if( _ItemsForSale[ j ]->getIdentifier() == itemsForSaleOfCharacter[ i ]->getIdentifier() )
			{
				mustBeRemoved = false;
				continue;
			}
		}
		if( mustBeRemoved )
		{
			// remove item from sell store
			CDynamicItems::getInstance()->removeDynamicItemForSell( itemsForSaleOfCharacter[ i ] );
		}		
	}

	for( uint32 i = 0; i < _Character->getItemInShop().getContent().size(); ++i )
	{
		CCharacter *pChar = PlayerManager.getChar(charId);
		if (pChar != NULL)
		{
			const TItemTradePtr pITP = pChar->getItemInShop().getContent()[i];
			if (pITP->getOwner() != charId)
			{
				nlwarning("save corrupted : split or merge account badly done !");
			}
		}
	}
}

//-----------------------------------------------------------------------------
uint32 CItemsForSale::getNbItemsForSaleOnContinent(CONTINENT::TContinent continent) const
{
	if (continent == CONTINENT::UNKNOWN)
		return 0;

	uint32 nbItems = 0;
	for (uint i = 0; i < _ItemsForSale.size(); i++)
	{
		TItemTradePtr ptr = _ItemsForSale[i];
		if ( ptr.isNull() )
			continue;

		if (ptr->getContinent() == continent)
			nbItems++;
	}

	return nbItems;
}

//-----------------------------------------------------------------------------
std::vector<TItemTradePtr> CItemsForSale::removeItemsOnContinent(CONTINENT::TContinent continent)
{
	vector<TItemTradePtr> removedItems;

	if (continent == CONTINENT::UNKNOWN)
		return removedItems;

	uint i = 0;
	while (i < _ItemsForSale.size())
	{
		TItemTradePtr ptr = _ItemsForSale[i];
		if ( ptr.isNull() )
			continue;

		if (ptr->getContinent() == continent)
		{
			removedItems.push_back( _ItemsForSale[i] );
			_ItemsForSale[i] = _ItemsForSale.back();
			_ItemsForSale.pop_back();
		}
		else
		{
			i++;
		}
	}

	return removedItems;
}


//-----------------------------------------------------------------------------
// Commands
//-----------------------------------------------------------------------------

NLMISC_COMMAND(nbItemsForSale, "get the number of items for sale of a player", "<eid> <continent = all/FYROS/FYROS_NEWBIE/...>")
{
	if (args.size() != 2)
		return false;

	CEntityId eid(args[0]);
	CCharacter * c = PlayerManager.getChar( eid );
	if ( !c )
	{
		log.displayNL("Unknown player '%s' (%s)", eid.toString().c_str(), args[0].c_str());
		return false;
	}

	const string & contName = args[1];
	uint32 nbItems;
	if (contName == "all")
	{
		for (uint i = 0; i < CONTINENT::NB_CONTINENTS; i++)
		{
			CONTINENT::TContinent continent = CONTINENT::TContinent(i);
			nbItems = c->getItemInShop().getNbItemsForSaleOnContinent(continent);
			if (nbItems == 0)
				continue;

			log.displayNL("Player %s: %u items for sale on continent %s",
				eid.toString().c_str(), nbItems, CONTINENT::toString(continent).c_str()
				);
		}

		nbItems = c->getItemInShop().getNbItemInShopStore();
		log.displayNL("Player %s: total of %u items for sale", eid.toString().c_str(), nbItems);
	}
	else
	{
		nbItems = c->getItemInShop().getNbItemsForSaleOnContinent( CONTINENT::toContinent(contName) );
		log.displayNL("Player %s: %u items for sale on continent %s", eid.toString().c_str(), nbItems, contName.c_str());
	}

	return true;
}

//-----------------------------------------------------------------------------
NLMISC_COMMAND(deleteItemsForSale, "delete items for sale of a player on the given continent", "<eid> <continent = FYROS/FYROS_NEWBIE/...>")
{
	if (args.size() != 2)
		return false;

	CEntityId eid(args[0]);
	CCharacter * c = PlayerManager.getChar( eid );
	if ( !c )
	{
		log.displayNL("Unknown player '%s' (%s)", eid.toString().c_str(), args[0].c_str());
		return false;
	}

	const string & contName = args[1];
	uint32 nbItems;

	CONTINENT::TContinent continent = CONTINENT::toContinent(contName);
	nbItems = c->getItemInShop().getNbItemsForSaleOnContinent(continent);
	if (nbItems == 0)
	{
		log.displayNL("Player %s: no item for sale on continent %s", eid.toString().c_str(), contName.c_str());
		return true;
	}

	vector<TItemTradePtr> removedItems = const_cast<CItemsForSale &>( c->getItemInShop() ).removeItemsOnContinent(continent);
	for (uint i = 0; i < removedItems.size(); i++)
	{
		TItemTradePtr & item = removedItems[i];
		log.displayNL("Player %s: deleted '%s': quantity=%u, quality=%u, level=%u, wanted price=%u, retire price=%u",
			eid.toString().c_str(),
			item->getSheetId().toString().c_str(),
			item->getQuantity(),
			item->getQuality(),
			item->getLevel(),
			item->getPriceInfo().getAmount(),
			item->getRetirePrice()
			);
		CDynamicItems::getInstance()->removeDynamicItemForSell( item );
	}

	log.displayNL("Player %s: deleted %u items for sale on continent %s", eid.toString().c_str(), nbItems, contName.c_str());

	return true;
}


//-----------------------------------------------------------------------------
// Persistent data for CItemsForSale
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CItemsForSale

#define PERSISTENT_DATA\
	PROP( uint32, _NextIdentifier)\
	LSTRUCT_VECT(\
		_ItemsForSale,\
		VECT_LOGIC(_ItemsForSale),\
		((CItemForSale*)&*(_ItemsForSale[ i ]))->store(pdr),\
		{\
			_ItemsForSale.push_back(new CItemForSale); \
			((CItemForSale*)&*_ItemsForSale.back())->apply(pdr);\
		}\
	)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

