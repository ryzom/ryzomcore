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

#include "shop_unit.h"

NL_INSTANCE_COUNTER_IMPL(IShopUnit);

//-----------------------------------------------------------------------------
void CShopUnitStatic::releaseShopUnit()
{
	_ShopContent.clear();
}

//-----------------------------------------------------------------------------
void CShopUnitStatic::addContent( const TItemTradePtr& item )
{
	if( dynamic_cast<CTradeBase *>( (IItemTrade*)item ) != 0 )
	{
		_ShopContent.push_back( item );
	}
}

//-----------------------------------------------------------------------------
void CShopUnitDynamic::releaseShopUnit()
{
	_ShopContent.clear();
}

//-----------------------------------------------------------------------------
void CShopUnitDynamic::addContent( const TItemTradePtr& item )
{
	if( dynamic_cast<CItemForSale *>( (IItemTrade*)item ) != 0 )
	{
		//todo: check if item are already in shop
		_ShopContent[ item->getContinent() ].push_back( item ); 
		_CycleContentChange = CTickEventHandler::getGameCycle();
		item->setAvailable( true );
	}
}

//-----------------------------------------------------------------------------
bool CShopUnitDynamic::removeContent( const TItemTradePtr& item, bool updateQuantity )
{
	if( dynamic_cast<CItemForSale *>( (IItemTrade*)item ) != 0 )
	{
		for( uint32 i = 0; i < _ShopContent[ item->getContinent() ].size(); ++i )
		{
			if( (IItemTrade*)item == (IItemTrade*)_ShopContent[ item->getContinent() ] [ i ] )
			{
				// RM are in mutlitple shop unit (one per item part, and we must update quantity one time par Rm (and not one time per item part)
				if( updateQuantity )
					item->setAvailable( false );
				if( item->isAvailable( 1 ) == false )
				{
					_ShopContent[ item->getContinent() ] [ i ] = _ShopContent[ item->getContinent() ].back();
					_ShopContent[ item->getContinent() ].pop_back();
					return true;
				}
				return false;
			}
		}
	}
	return true;
}
