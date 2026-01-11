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


#ifndef RY_SHOP_UNIT_H
#define RY_SHOP_UNIT_H


#include "item_for_sale.h"

class IShopUnit
{
	NL_INSTANCE_COUNTER_DECL(IShopUnit);
public:

	enum TShopUnitType { StaticShop = 0, DynamicShop, NumShopUnitType };
	

	// release content of shop unit
	virtual void releaseShopUnit() = 0;

	// get shop unit type
	virtual TShopUnitType getShopUnitType() const = 0;	

	// add content
	virtual void addContent( const TItemTradePtr& item ) = 0;

	// remove content
	virtual bool removeContent( const TItemTradePtr& item, bool updateQuantity = true ) = 0;// for dynamic shop
	virtual bool removeContent( const TItemTradePtr& item, bool updateQuantity = true ) const = 0;// for dynamic shop
	
	// return trade list reference corresponding to item shop content
	virtual const TTradeList& getShopContent( CONTINENT::TContinent continent ) const = 0;

	// get cycle of last content change
	virtual NLMISC::TGameCycle getGameCycleContentChange() const = 0;
};


class CShopUnitStatic : public IShopUnit
{
public:
	CShopUnitStatic() { _ShopUnitType = IShopUnit::StaticShop; }

	// release content of shop unit
	void releaseShopUnit();

	// get shop unit type
	IShopUnit::TShopUnitType getShopUnitType() const { return _ShopUnitType; }
	
	// add content
	void addContent( const TItemTradePtr& item ); 
	
	// remove content not valid for static shop
	bool removeContent( const TItemTradePtr& item, bool updateQuantity = true ) { return false; }
	bool removeContent( const TItemTradePtr& item, bool updateQuantity = true ) const { return false; }
	
	// return trade list reference corresponding to item shop content
	const TTradeList& getShopContent( CONTINENT::TContinent ) const { return _ShopContent; }

	// get cycle of last content change
	NLMISC::TGameCycle getGameCycleContentChange() const { return 0; }

private:
	IShopUnit::TShopUnitType	_ShopUnitType;
	TTradeList		_ShopContent;
};


class CShopUnitDynamic : public IShopUnit
{
public:

typedef std::vector< TTradeList > TShopContentPerContinent;

	CShopUnitDynamic() { _ShopUnitType = IShopUnit::DynamicShop; _ShopContent.resize( CONTINENT::NB_CONTINENTS );  _CycleContentChange = 0; }
	
	// release content of shop unit
	void releaseShopUnit();
	
	// get shop unit type
	IShopUnit::TShopUnitType getShopUnitType() const { return _ShopUnitType; }	

	//addContent
	void addContent( const TItemTradePtr& item );

	// remove content
	bool removeContent( const TItemTradePtr& item, bool updateQuantity = true );
	bool removeContent( const TItemTradePtr& item, bool updateQuantity = true ) const { return (const_cast<CShopUnitDynamic*>(this))->removeContent( item, updateQuantity ); }
	
	// return trade list reference corresponding to item shop content
	const TTradeList& getShopContent( CONTINENT::TContinent continent ) const { nlassert( ((uint32) continent) < _ShopContent.size() ); return _ShopContent[ continent ]; }
	
	// get cycle of last content change
	NLMISC::TGameCycle getGameCycleContentChange() const { return _CycleContentChange; }

private:
	IShopUnit::TShopUnitType	_ShopUnitType;
	TShopContentPerContinent	_ShopContent;
	NLMISC::TGameCycle			_CycleContentChange;
};

#endif
