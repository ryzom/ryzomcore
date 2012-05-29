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



#ifndef RY_SHOP_TYPE_MANAGER_H
#define RY_SHOP_TYPE_MANAGER_H

#include "game_share/people.h"
#include "game_share/ecosystem.h"

#include "nel/misc/eval_num_expr.h"
#include "game_item_manager/game_item_manager.h"
#include "game_share/fame.h"
#include "../egs_variables.h"
#include "item_for_sale.h"
#include "shop_type.h"
#include "shop_type/merchant.h"

#include <map>
#include <vector>
#include <string>

#define INVALID_SHOP_INDEX 0x00ffffff
#define NO_CHAT	0x01ffffff
#define NUM_QUALITY 5
#define NUM_LEVEL 20


/**
 * Manage all shop type
 * Mount list of items/mps/bicks... selling by each shop type
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CShopTypeManager : public NLMISC::CEvalNumExpr
{
public:

	enum TShopTypeSelected { StaticShopSelected = 0, DynamicShopSelected, AllShopSelected };

	/// init / release class
	static void initShopBase();
	static void release();
	
	// mount all basic shop type
	static void mountShopBase();
	
	//mount item basic shop type
	static void mountItemShopBase( const std::string& type );

	// mount rm basic shop type
	static void mountRmShopBase( const std::string& family );

	// mount tp basic shop type
	static void mountTpShopBase( const std::string& type );

	// add a player item in proper dynamic shop unit
	static bool addItemInShopUnitDynamic( const TItemTradePtr& item );

	// remove a player item from proper dynamic shop unit
	static bool removeItemFromShopUnitDynamic( const TItemTradePtr& item );

	// return a dynamic shop unit pointer corresponding IItemTrade
	static IShopUnit * getDynamicShopUnit( const TItemTradePtr& item, uint32 idx = 0 );

	// return a dynamic shop unit pointer corresponding to selectors
	static IShopUnit * getDynamicShopUnit( IShopType::TShopType shopType, uint32 shop, uint32 p1, uint32 p2, uint32 p3 );
		
	// CShopTypeManager::convertQualityToQualityIndex return corresponding quality shop index
	static uint32 convertQualityToQualityIndex( uint32 quality );

	// CShopTypeManager::convertLevelToLevelIndex return corresponding level shop index
	static uint32 convertLevelToLevelIndex( uint32 level );

	// add shop base reference to tradeList
	static void addShopBase( uint32 shopCategoryNumber, CMerchant& merchant, std::vector< uint32 >& rmSelector, std::vector< uint32 >& originSelector, std::vector< uint32 >& qualitySelector, std::vector< uint32 >& levelSelector, std::vector< uint32 >& shopTypeSelector );
	
	// add one item shop base reference to tradeList
	static void addItemShopBase( uint32 shopCategoryNumber, uint32 origin, uint32 quality, uint32 level, TShopTypeSelected shopTypeSelected, CMerchant& merchant );

	// add one raw material shop base reference to tradeList
	static void addRmShopBase( uint32 shopCategoryNumber, uint32 ecosystem, uint32 quality, uint32 level, TShopTypeSelected shopTypeSelected, CMerchant& merchant );

	// add one teleport shop base reference to tradeList
	static void addTpShopBase( uint32 shopCategoryNumber, uint32 ecosystem, TShopTypeSelected shopTypeSelected, CMerchant& merchant );

	// Calculate item base price
	static uint32 computeBasePrice( CGameItemPtr itemPtr, uint16 level );
	static uint32 computeBasePrice( const NLMISC::CSheetId& sheet, uint16 level, float statEnergyModifier = 1.0f );

//	static uint32 CShopTypeManager::computePlayerBuyPrice( const NLMISC::CSheetId& sheet, uint16 level );
		
	// get category number to shop base index reference
	static const std::vector< uint32 >& getCategoryNumberToShopBaseIndex() { return _CategoryNumberToShopCategory; }

	// accessor to private members
	static inline uint32 getLevelStart() { return _LevelStart; }
	static inline uint32 getLevelEnd() { return _LevelEnd; }
	static inline uint32 getQualityStart() { return _QualityStart; }
	static inline uint32 getQualityEnd() { return _QualityEnd; }
	static inline uint32 getRmEcosystemStart() { return _RmEcosystemStart; }
	static inline uint32 getRmEcosystemEnd() { return _RmEcosystemEnd; }
	static inline uint32 getOriginStart() { return _OriginStart; }
	static inline uint32 getOriginEnd() { return _OriginEnd; }
	static inline uint32 getTpStart() { return _TpStart; }
	static inline uint32 getTpEnd() { return _TpEnd; }
	static inline uint32 getItemStart() { return _ItemStart; }
	static inline uint32 getItemEnd() { return _ItemEnd; }
	static inline uint32 getRmStart() { return _RmStart; }
	static inline uint32 getRmEnd() { return _RmEnd; }
	static inline uint32 getShopTypeStart() { return _ShopTypeStart; }
	static inline uint32 getShopTypeEnd() { return _ShopTypeEnd; }
	
	static inline const std::vector< std::string >& getCategoryName() { return _CategoryName; }
	
    /**
     * Eval Specialization for return Standard price based  value
     *
     * \param value is the value to parse.
     * \param result is the result to fill if the value has been succesfully parsed.
	 * \param userData is quality of evaluated item
     * \return UnknownValue if the value is not known, ValueError is the value evaluation failed or NoError if it has been parsed.
     */
	NLMISC::CEvalNumExpr::TReturnState evalValue (const char *value, double &result, uint32 userData);

	/// compute the sell price of a raw material
	static uint32 computeRawMaterialBasePrice( uint16 level )
	{
		return uint32( ItemPriceCoeff2 * level * level + ItemPriceCoeff1 * level  + ItemPriceCoeff0 );
	}

	/// return the player buy price factor due to fame
	static float getFamePriceFactor(sint32 fameValue);

	static bool isCosmeticItem( uint32 category )
	{
		return ( category == _HairMale || category == _HairColorMale || category == _TatooMale ||
				 category == _HairFemale || category == _HairColorFemale || category == _TatooFemale );
	}

private:
	static uint32 _DummyPrice;
	static std::vector< std::string >		_CategoryName;
	static std::vector< uint32 >			_CategoryNumberToShopCategory;
	static std::vector< IShopType * >		_ShopTypes;

	typedef std::map<std::string, std::vector<std::string> >	TAliasMap;
	static TAliasMap	_ShopAliases;

	static uint32 _ItemStart;
	static uint32 _ItemEnd;
	static uint32 _RmStart;
	static uint32 _RmEnd;
	static uint32 _TpStart;
	static uint32 _TpEnd;
	static uint32 _RmEcosystemStart;
	static uint32 _RmEcosystemEnd;
	static uint32 _OriginStart;
	static uint32 _OriginEnd;
	static uint32 _QualityStart;
	static uint32 _QualityEnd;
	static uint32 _LevelStart;
	static uint32 _LevelEnd;
	static uint32 _ShopTypeStart;
	static uint32 _ShopTypeEnd;

	static uint32 _HairMale;
	static uint32 _HairColorMale;
	static uint32 _TatooMale;
	static uint32 _HairFemale;
	static uint32 _HairColorFemale;
	static uint32 _TatooFemale;
	
	NLMISC_COMMAND_FRIEND(listShopContent);
};


#endif // RY_SHOP_TYPE_MANAGER_H

/* shop_type_manager.h */

