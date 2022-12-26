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


#ifndef RY_SHOP_TYPE_H
#define RY_SHOP_TYPE_H

#include "shop_unit.h"
#include "game_share/item_origin.h"
#include "game_share/ecosystem.h"

const uint32 NumQuality = 5;
const uint32 NumLevel = 20;
const uint32 NumTpType = 2;

class IShopType
{
	NL_INSTANCE_COUNTER_DECL(IShopType);
public:
	
	enum TShopType { item = 0, rm, tp };

	// return type of shop
	virtual TShopType getShopType() = 0;

	// get shop unit
	virtual IShopUnit * getShopUnit( uint32 selector1, uint32 selector2, uint32 selector3, IShopUnit::TShopUnitType shopUnitType ) = 0;
	
	// create a shop unit, or just return it if already exist
	virtual IShopUnit * createShopUnit( uint32 selector1, uint32 selector2, uint32 selector3, IShopUnit::TShopUnitType shopUnitType ) = 0;

private:
	//release shop content
	virtual void releaseShopType() = 0;
};


class CItemShopType : public IShopType
{
public:
	CItemShopType();	

	// destructor
	~CItemShopType() { releaseShopType(); }

	// return type of shop
	TShopType getShopType() { return _ShopType; }

	// get shop unit
	IShopUnit * getShopUnit( uint32 selector1, uint32 selector2, uint32 selector3, IShopUnit::TShopUnitType shopUnitType );
	
	// create a shop unit, or just return it if already exist
	IShopUnit * createShopUnit( uint32 selector1, uint32 selector2, uint32 selector3, IShopUnit::TShopUnitType shopUnitType );
	
private:
	//release shop content
	void releaseShopType();

	IShopUnit *				_ShopItemBaseContent[ ITEM_ORIGIN::NUM_ITEM_ORIGIN ] [ NumQuality ] [ NumLevel ] [ IShopUnit::NumShopUnitType ];
	IShopType::TShopType	_ShopType;
};


class CRMShopType : public IShopType
{
public:
	CRMShopType();

	// destructor
	~CRMShopType() { releaseShopType(); }

	// return type of shop
	TShopType getShopType() { return _ShopType; }
	
	// get shop unit
	IShopUnit * getShopUnit( uint32 selector1, uint32 selector2, uint32 selector3, IShopUnit::TShopUnitType shopUnitType );
	
	// create a shop unit, or just return it if already exist
	IShopUnit * createShopUnit( uint32 selector1, uint32 selector2, uint32 selector3, IShopUnit::TShopUnitType shopUnitType );
	
private:
	//release shop content
	void releaseShopType();
	
	IShopUnit *				_ShopRMBaseContent[ ECOSYSTEM::NUM_ECOSYSTEM ] [ NumQuality ] [ NumLevel ] [ IShopUnit::NumShopUnitType ];
	IShopType::TShopType	_ShopType;
};


class CTPShopType : public IShopType
{
public:
	CTPShopType();
	
	// destructor
	~CTPShopType() { releaseShopType(); }
	
	// return type of shop
	TShopType getShopType() { return _ShopType; }
	
	// get shop unit
	IShopUnit * getShopUnit( uint32 selector1, uint32 selector2, uint32 selector3, IShopUnit::TShopUnitType shopUnitType );
	
	// create a shop unit, or just return it if already exist
	IShopUnit * createShopUnit( uint32 selector1, uint32 selector2, uint32 selector3, IShopUnit::TShopUnitType shopUnitType );
	
private:
	//release shop content
	void releaseShopType();
	
	IShopUnit *				_ShopTPBaseContent[ ECOSYSTEM::NUM_ECOSYSTEM ] [ NumTpType ];
	IShopType::TShopType	_ShopType;
};

#endif
