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

#ifndef RY_ITEM_FOR_SALE_H
#define RY_ITEM_FOR_SALE_H

#include "game_share/continent.h"
#include "game_share/pvp_clan.h"
#include "game_share/msg_ais_egs_gen.h"

#include "../egs_variables.h"
#include "game_item_manager/game_item.h"

class CItemForSale;
class CTradeBase;

class IItemTrade : public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(IItemTrade);
public:

	// virtual destructor
	virtual ~IItemTrade() {}

	// sheet id
	virtual NLMISC::CSheetId getSheetId() const = 0;
	virtual void setSheetId( const NLMISC::CSheetId& sheet ) = 0;
	
	// return price
//	virtual uint32 getPrice() const = 0;
//	virtual void setPrice( uint32 price ) = 0;
	virtual const RYMSG::TPriceInfo &getPriceInfo() const =0;
	virtual RYMSG::TPriceInfo &getPriceInfo() =0;
	virtual void setPriceInfo(const RYMSG::TPriceInfo &priceInfo) =0;
	
	// return retire price
	virtual uint32 getRetirePrice() const = 0;
	virtual void setRetirePrice( uint32 price ) = 0;

	// return faction type
//	virtual PVP_CLAN::TPVPClan getFactionType() const = 0;
//	virtual void setFactionType( PVP_CLAN::TPVPClan factionType ) = 0;

	// return faction point price
//	virtual uint32 getFactionPointPrice() const = 0;
//	virtual void setFactionPointPrice( uint32 FPPrice ) = 0;

	// return quality
	virtual uint32 getQuality() const = 0;
	virtual void setQuality( uint32 q) = 0;
	
	// return level
	virtual uint32 getLevel() const = 0;
	virtual void setLevel( uint32 l ) = 0;
	
	// return item ptr
	virtual CGameItemPtr getItemPtr() const = 0;
	virtual void setItemPtr( CGameItemPtr itemPtr ) = 0;
	
	// return start sale cycle
	virtual NLMISC::TGameCycle getStartSaleCycle() const = 0;
	virtual void setStartSaleCycle( NLMISC::TGameCycle cycle ) = 0;
	
	// return owner of item
	virtual NLMISC::CEntityId getOwner() const = 0;
	virtual void setOwner( const NLMISC::CEntityId& owner ) = 0;
	
	// return continent where item is selled
	virtual CONTINENT::TContinent getContinent() const = 0;
	virtual void setContinent( CONTINENT::TContinent continent ) = 0;

//	// serial
//	virtual void serial(NLMISC::IStream &f) = 0;

	// set item for sale
	virtual void itemForSale( uint32 price, uint32 retirePrice, CGameItemPtr item, uint32 quantity, const NLMISC::CEntityId& id, CONTINENT::TContinent continent, uint32 identifier ) = 0;
	
	// return true if item stay available
	virtual bool isAvailable( uint32 quantity ) const = 0;

	// set item stay available/non available
	virtual void setAvailable( bool a ) = 0;

	// return game cycle left for item in store 
	virtual NLMISC::TGameCycle getGameCycleLeft() const = 0;

	// get quantity
	virtual uint32 getQuantity() const = 0;

	// set quantity
	virtual uint32 addQuantity( uint32 q ) = 0;

	// remove quantity
	virtual void removeQuantity( uint32 q ) = 0;

	// get identifier
	virtual uint32 getIdentifier() const = 0;

	// copy 
	virtual void copy( IItemTrade * itt ) = 0;

	// allow same item in the shopping list
	virtual bool	allowSameItemInShopList() const =0;
};

// smart ptr for trade item
typedef NLMISC::CSmartPtr< IItemTrade >	TItemTradePtr;

// for dynamic item
class CItemForSale : public IItemTrade
{
	friend class CShopTypeManager;

public:
	
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);
	
	DECLARE_PERSISTENCE_METHODS
	
	// constructor
	CItemForSale();

	// virtual destructor
	virtual ~CItemForSale() NL_OVERRIDE;

	// sheet id
	NLMISC::CSheetId getSheetId() const NL_OVERRIDE { return _ItemPtr->getSheetId(); }
	void setSheetId( const NLMISC::CSheetId& ) NL_OVERRIDE {}
	
	// price
//	uint32 getPrice() const { return _Price; }
//	void setPrice( uint32 p ) { _Price = p; }
	virtual const RYMSG::TPriceInfo &getPriceInfo() const NL_OVERRIDE {return _PriceInfo;};
	virtual RYMSG::TPriceInfo &getPriceInfo() NL_OVERRIDE {return _PriceInfo;};
	virtual void setPriceInfo(const RYMSG::TPriceInfo &priceInfo) NL_OVERRIDE {_PriceInfo=priceInfo;}
	
	// retire price
	uint32 getRetirePrice() const NL_OVERRIDE { return _RetirePrice; }
	void setRetirePrice( uint32 price ) NL_OVERRIDE { _RetirePrice = price; }

	// Faction : No need for faction point and type for player item in shop
	PVP_CLAN::TPVPClan getFactionType() const { return PVP_CLAN::None; }
	void setFactionType(PVP_CLAN::TPVPClan  factionType) {}
	uint32 getFactionPointPrice() const { return 0; }
	void setFactionPointPrice( uint32 FPPrice ) {}

	// quality
	uint32 getQuality() const NL_OVERRIDE { return 1; }
	void setQuality( uint32 ) NL_OVERRIDE {}
	
	// level
	uint32 getLevel() const NL_OVERRIDE { return _ItemPtr->quality(); }
	void setLevel( uint32 ) NL_OVERRIDE {}
	
	// item ptr
	// warning: this item will be deleted by CItemForSale destructor
	CGameItemPtr getItemPtr() const NL_OVERRIDE { return _ItemPtr; }
	void setItemPtr( CGameItemPtr itemPtr ) NL_OVERRIDE { _ItemPtr = itemPtr; }
	
	// start sale cycle
	NLMISC::TGameCycle getStartSaleCycle() const NL_OVERRIDE { return _StartSaleCycle; }
	void setStartSaleCycle( NLMISC::TGameCycle cycle ) NL_OVERRIDE { _StartSaleCycle = cycle; }
	
	// owner id of item
	NLMISC::CEntityId getOwner() const NL_OVERRIDE { return _Owner; }
	void setOwner( const NLMISC::CEntityId& id ) NL_OVERRIDE { _Owner = id; }
	
	// continent where item are selled
	CONTINENT::TContinent getContinent() const NL_OVERRIDE { return _Continent; }
	void setContinent( CONTINENT::TContinent continent ) NL_OVERRIDE { _Continent = continent; }
	
	// set item for sale
	void itemForSale( uint32 price, uint32 retirePrice, CGameItemPtr item, uint32 quantity, const NLMISC::CEntityId& id, CONTINENT::TContinent continent, uint32 identifier ) NL_OVERRIDE;

	// serial
//	void serial(NLMISC::IStream &f);

	// cast operator
//	const IItemTrade * operator = ( CItemForSale * i ) const { return (IItemTrade *) i; }
	
	// return true if item stay available
	bool isAvailable( uint32 quantity ) const NL_OVERRIDE { return _Available && _Quantity >= quantity; }
	
	// set item stay available/non available
	void setAvailable( bool a ) NL_OVERRIDE;

	// return game cycle left for item in store 
//	NLMISC::TGameCycle getGameCycleLeft() const {	return (NLMISC::TGameCycle) ( (sint32) std::max( (sint32)0, (sint32)( ((sint32) MaxGameCycleSaleStore) - ( ( CTickEventHandler::getGameCycle() - _StartSaleCycle ) + _ItemPtr->getTotalSaleCycle() ) ) ) ); }
	NLMISC::TGameCycle getGameCycleLeft() const NL_OVERRIDE;
	
	// get quantity
	uint32 getQuantity() const NL_OVERRIDE { return _Quantity; }

	// set quantity
	uint32 addQuantity( uint32 q ) NL_OVERRIDE { _Quantity += q; return _Quantity; }

	// remove quantity
	void removeQuantity( uint32 q ) NL_OVERRIDE { _Quantity -= std::min( _Quantity, q ); }

	// get identifier
	uint32 getIdentifier() const NL_OVERRIDE { return _Identifier; }
	
	// copy 
	void copy( IItemTrade * itt ) NL_OVERRIDE;

	// For resale item: always allow same item in shopping list
	virtual bool	allowSameItemInShopList() const NL_OVERRIDE {return true;}
	
private:
	// setted price
//	uint32					_Price;
	RYMSG::TPriceInfo		_PriceInfo;
	// retire price
	uint32					_RetirePrice;
	// game cycle when item are put for sale
	NLMISC::TGameCycle		_StartSaleCycle;
	// item ptr
	CGameItemPtr			_ItemPtr;
	// character owner of item
	NLMISC::CEntityId		_Owner;
	//Continent where item is selled
	CONTINENT::TContinent	_Continent;
	//true if item stay available, false is already selled
	bool					_Available;
	//nb item to sell
	uint32					_Quantity;
	// get identifier
	uint32					_Identifier;
};


// for static item
class CTradeBase : public IItemTrade
{
	friend class CShopTypeManager;
	
	NL_INSTANCE_COUNTER_DECL(CTradeBase);
public:

	CTradeBase() { _ItemPtr = 0; /*_FactionType = PVP_CLAN::None; _FactionPointPrice = 0;*/ _AllowSameItemInShopList= false;}

	// virtual destructor
	virtual ~CTradeBase() NL_OVERRIDE;

	// sheet id
	NLMISC::CSheetId getSheetId() const NL_OVERRIDE { return _Sheet; }
	void setSheetId( const NLMISC::CSheetId& sheet ) NL_OVERRIDE { _Sheet = sheet; }

	// price
//	uint32 getPrice() const { return _Price; }
//	void setPrice( uint32 price ) { _Price = price; }
	virtual const RYMSG::TPriceInfo &getPriceInfo() const NL_OVERRIDE {return _PriceInfo;};
	virtual RYMSG::TPriceInfo &getPriceInfo() NL_OVERRIDE {return _PriceInfo;};
	virtual void setPriceInfo(const RYMSG::TPriceInfo &priceInfo) NL_OVERRIDE {_PriceInfo=priceInfo;}

	// return retire price
	uint32 getRetirePrice() const NL_OVERRIDE { return 0; }
	void setRetirePrice( uint32 ) NL_OVERRIDE {}

	// faction type
//	virtual PVP_CLAN::TPVPClan getFactionType() const { return _FactionType; }
//	virtual void setFactionType(PVP_CLAN::TPVPClan factionType) { _FactionType = factionType; }

	// faction point price
//	virtual uint32 getFactionPointPrice() const;
//	virtual void setFactionPointPrice( uint32 FPPrice ) { _FactionPointPrice = FPPrice; }

	// Special for named items: allow duplicate of items seen in the shopping list (even if same sheet/quality/level)
	void			setAllowSameItemInShopList(bool s) {_AllowSameItemInShopList= s;}
	virtual bool	allowSameItemInShopList() const NL_OVERRIDE {return _AllowSameItemInShopList;}

	// quality
	uint32 getQuality() const NL_OVERRIDE { return _Quality; }
	void setQuality( uint32 q ) NL_OVERRIDE { _Quality = q; }

	// level
	uint32 getLevel() const NL_OVERRIDE { return _Level; }
	void setLevel( uint32 l ) NL_OVERRIDE { _Level = l; }

	// item ptr
	// warning: this item won't be deleted by CTradeBase destructor
	CGameItemPtr getItemPtr() const NL_OVERRIDE { return _ItemPtr; }
	void setItemPtr( CGameItemPtr itemPtr ) NL_OVERRIDE { _ItemPtr = itemPtr; }

	// start sale cycle
	NLMISC::TGameCycle getStartSaleCycle() const NL_OVERRIDE { return 0; }
	void setStartSaleCycle( NLMISC::TGameCycle ) NL_OVERRIDE {}
	
	// owner of item
	NLMISC::CEntityId getOwner() const NL_OVERRIDE { return NLMISC::CEntityId::Unknown; }
	void setOwner( const NLMISC::CEntityId& ) NL_OVERRIDE {}
	
	// continent where item is selled
	CONTINENT::TContinent getContinent() const NL_OVERRIDE { return CONTINENT::UNKNOWN; }
	void setContinent( CONTINENT::TContinent ) NL_OVERRIDE {}
	
	// serial
	void serial(NLMISC::IStream &f) {}

	// set item for sale
	void itemForSale( uint32 price, uint32 retirePrice, CGameItemPtr item, uint32 quantity, const NLMISC::CEntityId& id, CONTINENT::TContinent continent, uint32 identifier ) NL_OVERRIDE {}

	// cast operator
//	const IItemTrade * operator = ( CTradeBase * i ) const { return (IItemTrade *) i; }

	// return true, static item stay always available
	bool isAvailable( uint32 ) const NL_OVERRIDE { return true; }

	// set item stay available/non available
	void setAvailable( bool ) NL_OVERRIDE {}

	// return game cycle left for item in store 
	NLMISC::TGameCycle getGameCycleLeft() const NL_OVERRIDE { return 0; }

	// get quantity
	uint32 getQuantity() const NL_OVERRIDE { return 1; }

	// set quantity
	uint32 addQuantity( uint32 ) NL_OVERRIDE { return 1; }

	// remove quantity
	void removeQuantity( uint32 ) NL_OVERRIDE {}
	
	// get identifier
	uint32 getIdentifier() const NL_OVERRIDE { return 0; }

	// copy 
	void copy( IItemTrade * itt ) NL_OVERRIDE;

private:
	RYMSG::TPriceInfo	_PriceInfo;
//	uint32				_Price;

//	PVP_CLAN::TPVPClan	_FactionType;
//	uint32				_FactionPointPrice;
	bool				_AllowSameItemInShopList;

	NLMISC::CSheetId	_Sheet;
	uint32				_Quality;
	uint32				_Level;
	CGameItemPtr		_ItemPtr;
};

typedef std::vector< TItemTradePtr > TTradeList;

#endif
