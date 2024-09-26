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
#include <vector>

#include "shop_type/merchant.h"

#include "creature_manager/creature.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CMerchant);

//-----------------------------------------------------------------------------
CMerchant::~CMerchant()
{
	clearMerchantTradeList();
	_ExpliciteShopContent.releaseShopUnit();
}

//-----------------------------------------------------------------------------
void CMerchant::addShopUnit( const IShopUnit * shop )
{
	for( uint i = 0; i < _MerchantTradeList.size(); ++i )
	{
		if( _MerchantTradeList[ i ] == shop )
			return;
	}
	_MerchantTradeList.push_back( shop );
}

//-----------------------------------------------------------------------------
void CMerchant::clearMerchantTradeList()
{
	_MerchantTradeList.clear();
	_MerchantTradeList.push_back( &_ExpliciteShopContent );
}

//-----------------------------------------------------------------------------
void CMerchant::addExpliciteSellingItem( CSmartPtr< IItemTrade>& item )
{
	_ExpliciteShopContent.addContent( item );
}

//-----------------------------------------------------------------------------
const vector< const IShopUnit * >& CMerchant::getMerchantTradeList()
{
	if(_Creature)
		_Creature->updateMerchantTradeList();
	return _MerchantTradeList; 
}

//-----------------------------------------------------------------------------
bool CMerchant::sellPlayerItem() 
{ 
	return _Creature ? _Creature->sellPlayerItem() : false;
}
