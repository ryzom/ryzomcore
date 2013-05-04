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

#include "shop_type/shop_type_manager.h"
#include "shop_type/static_items.h"
#include "shop_type/merchant.h"

#include "game_share/teleport_types.h"


#include "game_item_manager/game_item_manager.h"
#include "nel/misc/path.h"
#include "nel/misc/config_file.h"
#include "nel/misc/sstring.h"

#include "egs_sheets/egs_sheets.h"

#include "creature_manager/creature.h"

std::vector< IShopType * >							CShopTypeManager::_ShopTypes;
std::vector< std::string >							CShopTypeManager::_CategoryName;
std::vector< uint32 >								CShopTypeManager::_CategoryNumberToShopCategory;
std::map<std::string, std::vector<std::string> >	CShopTypeManager::_ShopAliases;

uint32 CShopTypeManager::_DummyPrice;

uint32 CShopTypeManager::_ItemStart;
uint32 CShopTypeManager::_ItemEnd;
uint32 CShopTypeManager::_RmStart;
uint32 CShopTypeManager::_RmEnd;
uint32 CShopTypeManager::_TpStart;
uint32 CShopTypeManager::_TpEnd;
uint32 CShopTypeManager::_RmEcosystemStart;
uint32 CShopTypeManager::_RmEcosystemEnd;
uint32 CShopTypeManager::_OriginStart;
uint32 CShopTypeManager::_OriginEnd;
uint32 CShopTypeManager::_QualityStart;
uint32 CShopTypeManager::_QualityEnd;
uint32 CShopTypeManager::_LevelStart;
uint32 CShopTypeManager::_LevelEnd;
uint32 CShopTypeManager::_ShopTypeStart;
uint32 CShopTypeManager::_ShopTypeEnd;

uint32 CShopTypeManager::_HairMale;
uint32 CShopTypeManager::_HairColorMale;
uint32 CShopTypeManager::_TatooMale;
uint32 CShopTypeManager::_HairFemale;
uint32 CShopTypeManager::_HairColorFemale;
uint32 CShopTypeManager::_TatooFemale;



NLMISC::CConfigFile	ShopConfigFile;

extern NLMISC::CVariable<bool>	VerboseShopParsing;

NLMISC::CVariable<uint32> MaxNPCRawMaterialQualityInSell("egs","MaxNPCRawMaterialQualityInSell", "maximum quality of raw material selled by NPC", 100, 0, true );

using namespace std;
using namespace NLMISC;


NLMISC_COMMAND(listShopContent, "list the item of a shop category", "<shop_category>|<shop_alias>[:<shop_category>|<shop_alias>]*")
{
	if (args.empty())
		return false;

	// concatenate all the args
	string line;
	for (uint i=0; i<args.size(); ++i)
		line += args[i];

	// explode into shop elements
	vector<string>	shops;
	explode(line, string(":"), shops);

	// remove leading/trailing white char
	for (uint i=0; i<shops.size(); ++i)
	{
		CSString s = shops[i];
		s = s.strip();
		if (!s.empty())
			shops[i] = CSString(shops[i]).strip();
	}

	vector<uint32>							shopCategoryNumber;
	vector < IShopType * >					shopList;
	vector< uint32 >						rmSelector;
	vector< uint32 >						originSelector;
	vector< uint32 >						qualitySelector;
	vector< uint32 >						levelSelector;
	vector< uint32 >						shopTypeSelector;
	
	vector<uint32>	rawCategory;
	// resolve the shop alias/category
	// 1st first pass to resolve alias
	for (uint i=0; i<shops.size(); ++i)
	{
		if (CShopTypeManager::_ShopAliases.find(shops[i]) != CShopTypeManager::_ShopAliases.end())
		{
			// ok, this is an alias, replace it by its content
			std::vector<std::string>::iterator first(CShopTypeManager::_ShopAliases[shops[i]].begin()), last(CShopTypeManager::_ShopAliases[shops[i]].end());
			for (; first != last; ++first)
			{
				shops.push_back(*first);
			}

			shops.erase(shops.begin()+i);
			--i;
		}
	}

	// 1b : remove duplicate category
	sort(shops.begin(), shops.end());
	shops.erase(unique(shops.begin(), shops.end()), shops.end());
	
	// 2nd pass, transform shop cat name into theire respective index
	for (uint i=0; i<shops.size(); ++i)
	{
		std::vector< std::string >::iterator it = find(CShopTypeManager::_CategoryName.begin(), CShopTypeManager::_CategoryName.end(), shops[i]);
		if (it == CShopTypeManager::_CategoryName.end())
		{
			log.displayNL("Invalid shop category name '%s'", shops[i].c_str());
			return true;
		}
		
		rawCategory.push_back((uint32)(it - CShopTypeManager::_CategoryName.begin()));
	}
	
	// 3rd pass, generate list for the addShopBase call
	for (uint i=0; i<rawCategory.size(); ++i)
	{
		uint32 number = rawCategory[i];
//		uint32 index = CShopTypeManager::_CategoryNumberToShopCategory[rawCategory[i]];
		if (number >= CShopTypeManager::_ItemStart && number < CShopTypeManager::_ItemEnd)
		{
			shopCategoryNumber.push_back(number);
		}
		else if (number >= CShopTypeManager::_RmStart && number < CShopTypeManager::_RmEnd)
		{
			shopCategoryNumber.push_back(number);
		}
		else if (number >= CShopTypeManager::_TpStart && number < CShopTypeManager::_TpEnd)
		{
			shopCategoryNumber.push_back(number);
		}
		else if (number >= CShopTypeManager::_RmEcosystemStart && number < CShopTypeManager::_RmEcosystemEnd)
		{
			rmSelector.push_back(number);
		}
		else if (number >= CShopTypeManager::_OriginStart && number < CShopTypeManager::_OriginEnd)
		{
			originSelector.push_back(number);
		}
		else if (number >= CShopTypeManager::_QualityStart && number < CShopTypeManager::_QualityEnd)
		{
			qualitySelector.push_back(number);
		}
		else if (number >= CShopTypeManager::_LevelStart && number < CShopTypeManager::_LevelEnd)
		{
			levelSelector.push_back(number);
		}
		else
		{
			log.displayNL("Warning : can't qualify the category '%s'",
				CShopTypeManager::_CategoryName[number].c_str());
		}
	}

	CCreature c;
	CMerchant merchant( c );

	// now, generate the item list
	for (uint i=0; i<shopCategoryNumber.size(); ++i)
	{
		CShopTypeManager::addShopBase(shopCategoryNumber[i], merchant, rmSelector, originSelector, qualitySelector, levelSelector, shopTypeSelector);
	}

	// display the 'expanded' query
	log.displayNL("Your shop query expanded to %u category :", rawCategory.size());
	for (uint i=0; i<rawCategory.size(); ++i)
	{
		log.displayNL("  %s", CShopTypeManager::_CategoryName[rawCategory[i]].c_str());
	}

	// count the number of item to dump
	uint32 nbItem = 0;
	uint32 emptyShop = 0;
	for (uint s = 0; s < merchant.getMerchantTradeList().size(); ++s)
	{
		if (merchant.getMerchantTradeList()[s]->getShopContent(CONTINENT::UNKNOWN).size() == 0 )
			emptyShop ++;
		for (uint i=0; i<merchant.getMerchantTradeList()[s]->getShopContent(CONTINENT::UNKNOWN).size(); ++i)
		{
			nbItem += 1;
		}
		// and finaly, display the result
		log.displayNL("Dumping %u items selected in the shops (%u empty shops):", 
			nbItem,
			emptyShop);
		for (uint i=0; i<merchant.getMerchantTradeList()[s]->getShopContent(CONTINENT::UNKNOWN).size(); ++i)
		{
			const TItemTradePtr &it = merchant.getMerchantTradeList()[s]->getShopContent(CONTINENT::UNKNOWN)[i];
			log.displayNL("  %s", it->getSheetId().toString().c_str());
		}
	}
	log.displayNL("Done.");

	return true;
}


//----------------------------------------------------------------------------
// CShopTypeManager::initShopBase init shop type string and index
// 
//----------------------------------------------------------------------------
void CShopTypeManager::initShopBase()
{
	// init shop type name and index
	try
	{
		const string path = CPath::lookup( "shop_category.cfg" );
		ShopConfigFile.load( path );

		CConfigFile::CVar& cvShopType = ShopConfigFile.getVar("ShopCategory");
		for (uint i = 0; i < cvShopType.size(); ++i )
		{
			if ( !cvShopType.asString(i).empty() )
			{
				_CategoryName.push_back( cvShopType.asString( i ) );
			}
		}

		// parse the shop alias
		CConfigFile::CVar& cvShopAlias = ShopConfigFile.getVar("ShopNameAliases");
		for ( uint i = 0; i < cvShopAlias.size(); ++i )
		{
			if ( !cvShopAlias.asString(i).empty() )
			{
				CVectorSString	args;
				explode(cvShopAlias.asString(i), string(":"), reinterpret_cast<vector<string> &>(args));
				
				string aliasName = args[0].strip();
				if (_ShopAliases.find(aliasName) != _ShopAliases.end())
				{
					nlwarning("In shop_category.cfg, line %u, alias name '%s' appear more than once",
						i+1,
						aliasName.c_str());
					continue;
				}
				for (uint j=1; j<args.size(); ++j)
					_ShopAliases[aliasName].push_back(args[j].strip());
			}
		}

	}
	catch(const EUnknownVar &) 
	{
		nlwarning("<CShopTypeManager::initShopBase> shop_category.cfg or ShopCategory not found, no shop base type initialized...");
	}
	mountShopBase();
}


//----------------------------------------------------------------------------
// CShopTypeManager::release free all
// 
//----------------------------------------------------------------------------
void CShopTypeManager::release()
{
	_CategoryName.clear();

	for( vector< IShopType * >::iterator it = _ShopTypes.begin(); it != _ShopTypes.end(); ++it )
	{
		delete (*it);
	}
	_ShopTypes.clear();
}


//----------------------------------------------------------------------------
// CShopTypeManager::mountShopBase mount all basic shop type
// 
//----------------------------------------------------------------------------
void CShopTypeManager::mountShopBase()
{
	bool itemCategory = false;
	bool rmCategory = false;
	bool tpCategory = false;
	
	uint32 index = 0;
	uint32 category = 0;
	for( vector< string >::iterator it = _CategoryName.begin(); it != _CategoryName.end(); ++it )
	{
		if( (*it) == string("ITEM_CATEGORY") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> ITEM_CATEGORY: %d", category );
			_ItemStart = category;
			itemCategory = true; // used for mounting shop base: is a items shop base selector
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("END_ITEM_CATEGORY") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> END_ITEM_CATEGORY: %d", category );
			_ItemEnd = category;
			itemCategory = false; // used for mounting shop base: is a end items shop base selector
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("RM_ITEM_PART") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> RM_FAMILY: %d", category );
			_RmStart = category;
			rmCategory = true; // used for mounting shop base: is a RM shop base selector
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("END_RM_ITEM_PART") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> END_RM_FAMILY: %d", category );
			_RmEnd = category;
			rmCategory = false; // used for mounting shop base: is a end RM shop base selector
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("TELEPORT") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> TELEPORT: %d", category );
			_TpStart = category; // used for select tp content for shopkeeper
			tpCategory = true;
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("END_TELEPORT") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> END_TELEPORT: %d", category );
			_TpEnd = category; // used for select tp content for shopkeeper
			tpCategory = false;
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("ECOSYSTEM") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> ECOSYSTEM: %d", category );
			_RmEcosystemStart = category; // used for select ecosystem content for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
			rmCategory = true;
		}
		else if( (*it) == string("END_ECOSYSTEM") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> END_ECOSYSTEM: %d", category );
			_RmEcosystemEnd = category; // used for select ecosystem content for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
			rmCategory = false;
		}
		else if( (*it) == string("ORIGIN") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> ORIGIN: %d", category );
			_OriginStart = category; // used for select origin content for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("END_ORIGIN") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> END_ORIGIN: %d", category );
			_OriginEnd = category; // used for select origin content for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("QUALITY") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> QUALITY: %d", category );
			_QualityStart = category; // used for select quality content for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("END_QUALITY") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> END_QUALITY: %d", category );
			_QualityEnd = category; // used for select quality content for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("LEVEL") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> LEVEL: %d", category );
			_LevelStart = category; // used for select level content for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("END_LEVEL") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> END_LEVEL: %d", category );
			_LevelEnd = category; // used for select level content for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("SHOP_TYPE") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> SHOP_TYPE: %d", category );
			_ShopTypeStart = category; // used for select shop type for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( (*it) == string("END_SHOP_TYPE") )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> END_SHOP_TYPE: %d", category );
			_ShopTypeEnd = category; // used for select shop type for shopkeeper
			_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
		}
		else if( itemCategory  )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> '%s': %d", (*it).c_str(), category );
			mountItemShopBase( (*it) );
			_CategoryNumberToShopCategory.push_back( index );
			++index;
			
			if ( (*it) == string("HAIR_MALE") )
				_HairMale = category;
			else if ( (*it) == string("HAIRCOLOR_MALE") )
				_HairColorMale = category;
			else if ( (*it) == string("TATOO_MALE") )
				_TatooMale = category;
			else if ( (*it) == string("HAIR_FEMALE") )
				_HairFemale = category;
			else if ( (*it) == string("HAIRCOLOR_FEMALE") )
				_HairColorFemale = category;
			else if ( (*it) == string("TATOO_FEMALE") )
				_TatooFemale = category;
		}
		else if( rmCategory )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> '%s': %d", (*it).c_str(), category );
			mountRmShopBase( (*it) );
			_CategoryNumberToShopCategory.push_back( index );
			++index;
		}
		else if( tpCategory )
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> '%s': %d", (*it).c_str(), category );
			mountTpShopBase( (*it) );
			_CategoryNumberToShopCategory.push_back( index );
			++index;
		}
		else
		{
			if (VerboseShopParsing)
				egs_shinfo("<CShopTypeManager::mountShopBase> '%s': %d", (*it).c_str(), category );
			if( (*it) == string( "no_bot_chat" ) )
			{
				_CategoryNumberToShopCategory.push_back( NO_CHAT );
			}
			else
			{
				_CategoryNumberToShopCategory.push_back( INVALID_SHOP_INDEX );
			}
		}
		++category;
	}
}


//----------------------------------------------------------------------------
// add a player item in proper dynamic shop unit
// 
//----------------------------------------------------------------------------
bool CShopTypeManager::addItemInShopUnitDynamic( const TItemTradePtr& item )
{
	bool retValue = false; 
	const CStaticItem * itemForm = CSheets::getForm( item->getSheetId() );
	if( itemForm != 0 && itemForm->Family == ITEMFAMILY::RAW_MATERIAL )
	{
		for( uint32 i = 0; i < itemForm->Mp->MpFaberParameters.size(); ++i )
		{
			IShopUnit * shopUnit = getDynamicShopUnit( item, i );
			if( shopUnit != 0 )
			{
				shopUnit->addContent( item );
				retValue = true;
			}
		}
	}
	else
	{
		IShopUnit * shopUnit = getDynamicShopUnit( item );
		if( shopUnit != 0 )
		{
			shopUnit->addContent( item );
			retValue = true;
		}
	}
	return retValue;
}


//----------------------------------------------------------------------------
// remove a player item from proper dynamic shop unit
// 
//----------------------------------------------------------------------------
bool CShopTypeManager::removeItemFromShopUnitDynamic( const TItemTradePtr& item )
{
	bool retValue = true; 
	const CStaticItem * itemForm = CSheets::getForm( item->getSheetId() );
	if( itemForm != 0 && itemForm->Family == ITEMFAMILY::RAW_MATERIAL )
	{
		for( uint32 i = 0; i < itemForm->Mp->MpFaberParameters.size(); ++i )
		{
			IShopUnit * shopUnit = getDynamicShopUnit( item, i );
			if( shopUnit != 0 )
			{
				retValue &= shopUnit->removeContent( item, i == 0 );
			}
		}
	}
	else
	{
		IShopUnit * shopUnit = getDynamicShopUnit( item );
		if( shopUnit != 0 )
		{
			retValue = shopUnit->removeContent( item );
		}
	}
	return retValue;
}


//----------------------------------------------------------------------------
// return a dynamic shop unit pointer corresponding CItemForSale
// 
//----------------------------------------------------------------------------
IShopUnit * CShopTypeManager::getDynamicShopUnit( const TItemTradePtr& item, uint32 idx )
{
	if( item != 0 )
	{
		const CStaticItem * itemForm = CSheets::getForm( item->getItemPtr()->getSheetId() );
		if( itemForm != 0 )
		{
			switch( itemForm->Family )
			{
			case ITEMFAMILY::RAW_MATERIAL:
				{
					if( idx >= itemForm->Mp->MpFaberParameters.size() )
						return 0;
					uint32 rmType = itemForm->Mp->MpFaberParameters[idx].MpFaberType;
					uint32 ecosystem = itemForm->Mp->Ecosystem;
					uint32 quality = itemForm->Mp->StatEnergy;
					uint32 level = item->getItemPtr()->quality();
					return getDynamicShopUnit( IShopType::rm, rmType, ecosystem, quality, level );
				}
				break;
			case ITEMFAMILY::ARMOR:
			case ITEMFAMILY::MELEE_WEAPON:
			case ITEMFAMILY::RANGE_WEAPON:
			case ITEMFAMILY::AMMO:
			case ITEMFAMILY::SHIELD:
			case ITEMFAMILY::JEWELRY:
			case ITEMFAMILY::CRYSTALLIZED_SPELL:
			case ITEMFAMILY::ITEM_SAP_RECHARGE:
			case ITEMFAMILY::CONSUMABLE:
				{
					uint32 itemType = itemForm->Type;
					uint32 itemOrigin = itemForm->Origin;
					uint32 quality = 1; //todo set right quality when this stats are managed in CGameItem
					uint32 level = item->getItemPtr()->quality();
					return getDynamicShopUnit( IShopType::item, itemType, itemOrigin, quality, level );
				}
				break;
			default:
				break;
			}		
		}
	}
	return 0;
}


//----------------------------------------------------------------------------
// return a dynamic shop unit pointer corresponding to selectors
// 
//----------------------------------------------------------------------------
IShopUnit * CShopTypeManager::getDynamicShopUnit( IShopType::TShopType shopType , uint32 shop, uint32 p1, uint32 p2, uint32 p3 )
{
	uint32 shopIndex = INVALID_SHOP_INDEX;
	string typeName;
	if( shopType == IShopType::item )
		typeName = ITEM_TYPE::toString( (ITEM_TYPE::TItemType) shop );
	else
		typeName = RM_FABER_TYPE::toString( (RM_FABER_TYPE::TRMFType) shop );

	CSString csTypeName = typeName;
	csTypeName = csTypeName.toUpper();
	for( uint32 categoryNumber = 0; categoryNumber < _CategoryName.size(); ++categoryNumber )
	{
		if( _CategoryName[ categoryNumber ] == csTypeName )
		{
			shopIndex = _CategoryNumberToShopCategory[ categoryNumber ];
			break;
		}
	}
	
	if( shopIndex != INVALID_SHOP_INDEX && shopIndex < _ShopTypes.size() )
	{
		IShopType * shop = _ShopTypes[ shopIndex ];
		if( shop != 0 )
		{
			uint32 selector1 = p1;//ITEM_ORIGIN::itemOriginStringToPeopleEnum(_CategoryName[ _OriginStart + p1 + 1 ]);
			uint32 selector2 = convertQualityToQualityIndex( p2 );
			uint32 selector3 = convertLevelToLevelIndex( p3 );
			return shop->createShopUnit( selector1, selector2, selector3, IShopUnit::DynamicShop );
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// CShopTypeManager::convertQualityToQualityIndex return corresponding quality shop index
// 
//----------------------------------------------------------------------------
uint32 CShopTypeManager::convertQualityToQualityIndex( uint32 quality )
{
	for( uint32 index = _QualityStart + 1; index < _QualityEnd; ++index )
	{
		uint32 qualityIndex;
		NLMISC::fromString(_CategoryName[ index ].substr( 1 ), qualityIndex);
		if( quality <= qualityIndex )
		{
			return index - _QualityStart - 1;
		}
	}
	return _QualityEnd - _QualityStart - 1;
}

//----------------------------------------------------------------------------
// CShopTypeManager::convertLevelToLevelIndex return corresponding level shop index
// 
//----------------------------------------------------------------------------
uint32 CShopTypeManager::convertLevelToLevelIndex( uint32 level )
{
	for( uint32 index = _LevelStart + 1; index < _LevelEnd; ++index )
	{
		uint32 levelIndex;
		NLMISC::fromString(_CategoryName[ index ].substr( 1 ), levelIndex);

		if( level <= levelIndex )
		{
			return index - _LevelStart - 1;
		}
	}
	return _LevelEnd - _LevelStart - 1;
}


//----------------------------------------------------------------------------
// CShopTypeManager::mountItemShopBase mount item basic shop type
// 
//----------------------------------------------------------------------------
void CShopTypeManager::mountItemShopBase( const std::string& type )
{
	CItemShopType * shop = new CItemShopType();
	_ShopTypes.push_back( shop );

	egs_shinfo("<CShopTypeManager::mountItemShopBase> Type category '%s'", type.c_str() );

	// check if item type is known
	ITEM_TYPE::TItemType itemTypeEnum = ITEM_TYPE::stringToItemType( type );
	if( itemTypeEnum != ITEM_TYPE::UNDEFINED )
	{
		for( std::vector< CGameItemPtr >::const_iterator it = CStaticItems::getStaticItems().begin(); it != CStaticItems::getStaticItems().end(); ++it )
		{
			CSheetId itemSheet = (*it)->getSheetId();
			const CStaticItem * itemForm = CSheets::getForm( itemSheet );
			if( itemForm )
			{
				if( itemForm->Type == itemTypeEnum && itemForm->Saleable )
				{
					// Match, this item is crafted by skill corresponding to current basic shop
					if( itemForm->Origin != ITEM_ORIGIN::UNKNOWN )
					{
						for( uint quality = 0; quality < min( (uint)NUM_QUALITY, (uint)(_QualityEnd - _QualityStart - 1)); ++quality )
						{
							uint16 q;
							NLMISC::fromString(_CategoryName[ _QualityStart + quality + 1 ].substr(1), q);


							uint maxLevel = min( (uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1) );
							for( uint level = 0; level < maxLevel; ++level )
							{
								uint16 l;
								NLMISC::fromString(_CategoryName[ _LevelStart + level + 1 ].substr(1), l);
								if( l > MaxLevelNpcItemInStore )
									continue;

								IShopUnit * shopUnit = shop->getShopUnit( itemForm->Origin, quality, level, IShopUnit::StaticShop );
								if( shopUnit == 0 )
								{
									shopUnit = shop->createShopUnit( itemForm->Origin, quality, level, IShopUnit::StaticShop );
									nlassert( shopUnit != 0 );
								}

								CSmartPtr< IItemTrade > itemTrade = new CTradeBase();
								RYMSG::TPriceInfo priceInfo;
								priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_dappers);
								priceInfo.setAmount(computeBasePrice( *it, l ));
								itemTrade->setPriceInfo(priceInfo);
								itemTrade->setQuality( q );
								itemTrade->setLevel( l );
								itemTrade->setSheetId( itemSheet );
								itemTrade->setItemPtr( *it );
								shopUnit->addContent( itemTrade );
								if (VerboseShopParsing)
									egs_shinfo("      Selected item by category: '%s' Quality %d Level %d", itemSheet.toString().c_str(), q, l );

								if ( itemTypeEnum == ITEM_TYPE::HAIR_FEMALE || itemTypeEnum == ITEM_TYPE::HAIRCOLOR_FEMALE || itemTypeEnum == ITEM_TYPE::TATOO_FEMALE ||
									itemTypeEnum == ITEM_TYPE::HAIR_MALE || itemTypeEnum == ITEM_TYPE::HAIRCOLOR_MALE || itemTypeEnum == ITEM_TYPE::TATOO_MALE || 
									itemTypeEnum == ITEM_TYPE::MEKTOUB_MOUNT_TICKET || itemTypeEnum == ITEM_TYPE::MEKTOUB_PACKER_TICKET || itemTypeEnum == ITEM_TYPE::FOOD ||
									itemTypeEnum == ITEM_TYPE::SERVICE_STABLE )
									break;
							}
							if ( itemTypeEnum == ITEM_TYPE::HAIR_FEMALE || itemTypeEnum == ITEM_TYPE::HAIRCOLOR_FEMALE || itemTypeEnum == ITEM_TYPE::TATOO_FEMALE ||
								itemTypeEnum == ITEM_TYPE::HAIR_MALE || itemTypeEnum == ITEM_TYPE::HAIRCOLOR_MALE || itemTypeEnum == ITEM_TYPE::TATOO_MALE || 
								itemTypeEnum == ITEM_TYPE::MEKTOUB_MOUNT_TICKET || itemTypeEnum == ITEM_TYPE::MEKTOUB_PACKER_TICKET || itemTypeEnum == ITEM_TYPE::FOOD ||
								itemTypeEnum == ITEM_TYPE::SERVICE_STABLE )
								break;
						}
					}
					else
					{
						nlwarning("<CShopTypeManager::mountItemShopBase> Item '%s' have an unknown origin or not humanoid species and can be affected to a shop...", itemSheet.toString().c_str() );
					}
				}
			}
		}

		for( CAllStaticItems::const_iterator it2 = CSheets::getItemMapForm().begin(); it2 != CSheets::getItemMapForm().end(); ++it2 )
		{
			if( ((*it2).first.toString().c_str() [0] != '_') && ( (*it2).second.Family == ITEMFAMILY::CRAFTING_TOOL || (*it2).second.Family == ITEMFAMILY::HARVEST_TOOL  || (*it2).second.Family == ITEMFAMILY::PET_ANIMAL_TICKET || (*it2).second.Family == ITEMFAMILY::COSMETIC || (*it2).second.Family == ITEMFAMILY::FOOD || (*it2).second.Family == ITEMFAMILY::SERVICE) ) // TODO: remove this horrible hardcoding
			{
				if( (*it2).second.Type == ITEM_TYPE::stringToItemType( type ) && (*it2).second.Saleable )
				{
					// Match, this item is crafted by skill corresponding to current basic shop
					if( (*it2).second.Origin != ITEM_ORIGIN::UNKNOWN )
					{ 
						for( uint quality = 0; quality < min( (uint)NUM_QUALITY, (uint)(_QualityEnd - _QualityStart - 1)); ++quality )
						{
//							if( ( (*it2).second.Family != ITEMFAMILY::CRAFTING_TOOL && (*it2).second.Family != ITEMFAMILY::HARVEST_TOOL && (*it2).second.Family != ITEMFAMILY::PET_ANIMAL_TICKET ) || quality == 0 )
							{
								uint16 q;
								NLMISC::fromString(_CategoryName[ _QualityStart + quality + 1 ].substr(1), q);
								uint maxLevel = min( (uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1) );
								for( uint level = 0; level < maxLevel; ++level )
								{
//									if( ( (*it2).second.Family != ITEMFAMILY::CRAFTING_TOOL && (*it2).second.Family != ITEMFAMILY::HARVEST_TOOL && (*it2).second.Family != ITEMFAMILY::PET_ANIMAL_TICKET ) || level == 2 ) // remove this filter when tool merchant are setted
									{
										uint16 l;
										NLMISC::fromString(_CategoryName[ _LevelStart + level + 1 ].substr(1), l);
										if( l > MaxLevelNpcItemInStore )
											continue;

										IShopUnit * shopUnit = shop->getShopUnit( (*it2).second.Origin, quality, level, IShopUnit::StaticShop );
										if( shopUnit == 0 )
										{
											shopUnit = shop->createShopUnit( (*it2).second.Origin, quality, level, IShopUnit::StaticShop );
											nlassert( shopUnit != 0 );
										}

										CSmartPtr< IItemTrade> itemTrade = new CTradeBase();
										RYMSG::TPriceInfo priceInfo;
										priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_dappers);
										priceInfo.setAmount(computeBasePrice( (*it2).first, l ));
										itemTrade->setPriceInfo(priceInfo);
										itemTrade->setQuality( q );
										itemTrade->setLevel( l );
										itemTrade->setSheetId( (*it2).first );
										itemTrade->setItemPtr( 0 );
										shopUnit->addContent( itemTrade );
										if (VerboseShopParsing)
											egs_shinfo("      Selected item by category: '%s' Quality %d Level %d", (*it2).first.toString().c_str(), q, l );
									}
									if ( itemTypeEnum == ITEM_TYPE::HAIR_FEMALE || itemTypeEnum == ITEM_TYPE::HAIRCOLOR_FEMALE || itemTypeEnum == ITEM_TYPE::TATOO_FEMALE ||
										itemTypeEnum == ITEM_TYPE::HAIR_MALE || itemTypeEnum == ITEM_TYPE::HAIRCOLOR_MALE || itemTypeEnum == ITEM_TYPE::TATOO_MALE || 
										itemTypeEnum == ITEM_TYPE::MEKTOUB_MOUNT_TICKET || itemTypeEnum == ITEM_TYPE::MEKTOUB_PACKER_TICKET || itemTypeEnum == ITEM_TYPE::FOOD ||
										itemTypeEnum == ITEM_TYPE::SERVICE_STABLE )
										break;
								}
							}
							if ( itemTypeEnum == ITEM_TYPE::HAIR_FEMALE || itemTypeEnum == ITEM_TYPE::HAIRCOLOR_FEMALE || itemTypeEnum == ITEM_TYPE::TATOO_FEMALE ||
								itemTypeEnum == ITEM_TYPE::HAIR_MALE || itemTypeEnum == ITEM_TYPE::HAIRCOLOR_MALE || itemTypeEnum == ITEM_TYPE::TATOO_MALE || 
								itemTypeEnum == ITEM_TYPE::MEKTOUB_MOUNT_TICKET || itemTypeEnum == ITEM_TYPE::MEKTOUB_PACKER_TICKET || itemTypeEnum == ITEM_TYPE::FOOD ||
								itemTypeEnum == ITEM_TYPE::SERVICE_STABLE )
								break;
						}
					}
					else
					{
						nlwarning("<CShopTypeManager::mountItemShopBase> Item '%s' have an unknown origin or not humanoid species and can be affected to a shop...", (*it2).first.toString().c_str() );
					}
				}
			}
		}
	}
}


//----------------------------------------------------------------------------
// CShopTypeManager::mountRmShopBase mount rm basic shop type
// 
//----------------------------------------------------------------------------
void CShopTypeManager::mountRmShopBase( const std::string& family )
{
	CRMShopType * shop = new CRMShopType();
	_ShopTypes.push_back( shop );

	egs_shinfo("<CShopTypeManager::mountRmShopBase> Mount shop for item family '%s'", family.c_str() );

	// change type string to skill name format
	for( CAllStaticItems::const_iterator it = CSheets::getItemMapForm().begin(); it != CSheets::getItemMapForm().end(); ++it )
	{
		if( (*it).first.toString().c_str() [0] != '_' && (*it).second.Family == ITEMFAMILY::RAW_MATERIAL && (*it).second.Mp != NULL )
		{
			for( uint32 item_part = 0; item_part < (*it).second.Mp->MpFaberParameters.size(); ++item_part )
			{
				if( (*it).second.Mp->MpFaberParameters[item_part].MpFaberType == RM_FABER_TYPE::toFaberType(family) && (*it).second.Saleable )
				{
					for( uint quality = 0; quality < min((uint)NUM_QUALITY, (uint)(_QualityEnd - _QualityStart - 1)); ++quality )
					{
						uint16 q;
						NLMISC::fromString(_CategoryName[ _QualityStart + quality + 1 ].substr(1), q);

						if( (*it).second.Mp->StatEnergy == q )
						{
							uint maxLevel = min( (uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1) );
							for( uint level = 0; level < maxLevel; ++level )
							{
								uint16 l;
								NLMISC::fromString(_CategoryName[ _LevelStart + level + 1 ].substr(1), l);
								// npc not sell Raw Material of level higher than MaxNPCRawMaterialQualityInSell
								if( l <= MaxNPCRawMaterialQualityInSell )
								{
									IShopUnit * shopUnit = shop->getShopUnit( (*it).second.Mp->Ecosystem, quality, level, IShopUnit::StaticShop );
									if( shopUnit == 0 )
									{
										shopUnit = shop->createShopUnit( (*it).second.Mp->Ecosystem, quality, level, IShopUnit::StaticShop );
										nlassert( shopUnit != 0 );
									}

									CSmartPtr< IItemTrade > rmTrade = new CTradeBase();
									RYMSG::TPriceInfo priceInfo;
									priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_dappers);
									priceInfo.setAmount(computeBasePrice( (*it).first, l ));
									rmTrade->setPriceInfo(priceInfo);
									rmTrade->setQuality( q );
									rmTrade->setLevel( l );
									rmTrade->setSheetId( (*it).first );
									rmTrade->setItemPtr( 0 );
									shopUnit->addContent( rmTrade );
									if (VerboseShopParsing)
										egs_shinfo("      RM '%s' quality %d level %d added to shop", (*it).first.toString().c_str(), q, l );
								}
							}
						}
					}
				}
			}
		}
	}
}


//----------------------------------------------------------------------------
// CShopTypeManager::mountTpShopBase mount tp basic shop type
// 
//----------------------------------------------------------------------------
void CShopTypeManager::mountTpShopBase( const std::string& type )
{
	CTPShopType * shop = new CTPShopType();
	_ShopTypes.push_back( shop );
	
	if (VerboseShopParsing)
		egs_shinfo("<CShopTypeManager::mountTpShopBase> Mount shop for tp category '%s'", type.c_str() );

	// check if tp is a knowned type
	if( TELEPORT_TYPES::getTpTypeFromString( type ) != TELEPORT_TYPES::NONE )
	{
		for( CAllStaticItems::const_iterator it = CSheets::getItemMapForm().begin(); it != CSheets::getItemMapForm().end(); ++it )
		{
			if( (*it).first.toString().c_str() [0] != '_' && (*it).second.Family == ITEMFAMILY::TELEPORT )
			{
				if( (*it).second.TpEcosystem != ECOSYSTEM::unknown && (*it).second.Saleable )
				{
					if( (*it).second.TpType == TELEPORT_TYPES::getTpTypeFromString( type ) )
					{
						if (VerboseShopParsing)
							egs_shinfo("      TP '%s' added to shop", (*it).first.toString().c_str() );
						
						IShopUnit * shopUnit = shop->getShopUnit( (*it).second.TpEcosystem, (*it).second.TpType, 0, IShopUnit::StaticShop );
						if( shopUnit == 0 )
						{
							shopUnit = shop->createShopUnit( (*it).second.TpEcosystem, (*it).second.TpType, 0, IShopUnit::StaticShop );
							nlassert( shopUnit != 0 );
						}

						CSmartPtr< IItemTrade > tpTrade = new CTradeBase();
						RYMSG::TPriceInfo priceInfo;
						priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_dappers);
						priceInfo.setAmount(computeBasePrice( (*it).first, 0 ));
						tpTrade->setPriceInfo(priceInfo);
						tpTrade->setSheetId( (*it).first );
						shopUnit->addContent( tpTrade );
					}
				}
			}
		}
	}	
}


//----------------------------------------------------------------------------
// CShopTypeManager::addItemShopBase add item shop base reference to tradeList
// 
//----------------------------------------------------------------------------
void CShopTypeManager::addItemShopBase( uint32 shopCategoryNumber, uint32 origin, uint32 quality, uint32 level, TShopTypeSelected shopTypeSelected, CMerchant& merchant )
{
	CItemShopType * shop = dynamic_cast<CItemShopType *>(_ShopTypes[ _CategoryNumberToShopCategory [ shopCategoryNumber ] ]);
	if( shop != 0 )
	{
		if( shopTypeSelected == StaticShopSelected || shopTypeSelected == AllShopSelected )
		{
			IShopUnit * shopUnit = shop->getShopUnit( origin, quality, level, IShopUnit::StaticShop );
			if( shopUnit != 0 )
			{
				merchant.addShopUnit( shopUnit );
			}
		}
		if( shopTypeSelected == DynamicShopSelected || shopTypeSelected == AllShopSelected )
		{
			for( uint32 i = ITEM_ORIGIN::stringToEnum( _CategoryName[ _OriginStart + 1 ] ); i <= (uint32)ITEM_ORIGIN::stringToEnum( _CategoryName[ _OriginEnd -1 ] ); ++i )
			{
				for( uint32 q = 0; q < min((uint32)NUM_QUALITY, (uint32)(_QualityEnd - _QualityStart - 1)); ++q )
				{
					IShopUnit * shopUnit = shop->getShopUnit( i, q, level, IShopUnit::DynamicShop );
					if( shopUnit != 0 )
					{
						merchant.addShopUnit( shopUnit );
					}
				}
			}
		}
	}
}


//----------------------------------------------------------------------------
// CShopTypeManager::addRmShopBase add Raw Material shop base reference to tradeList
// 
//----------------------------------------------------------------------------
void CShopTypeManager::addRmShopBase( uint32 shopCategoryNumber, uint32 ecosystem, uint32 quality, uint32 level, TShopTypeSelected shopTypeSelected, CMerchant& merchant )
{
	CRMShopType * shop = dynamic_cast<CRMShopType *>(_ShopTypes[ _CategoryNumberToShopCategory [ shopCategoryNumber ] ]);
	if( shop != 0 )
	{
		if( shopTypeSelected == StaticShopSelected || shopTypeSelected == AllShopSelected )
		{
			IShopUnit * shopUnit = shop->getShopUnit( ecosystem, quality, level, IShopUnit::StaticShop );
			if( shopUnit != 0 )
			{
				merchant.addShopUnit( shopUnit );
			}
		}
		if( shopTypeSelected == DynamicShopSelected || shopTypeSelected == AllShopSelected )
		{
			for( uint32 i = ECOSYSTEM::stringToEcosystem( _CategoryName[ _RmEcosystemStart + 1 ] ); i <= (uint32)ECOSYSTEM::stringToEcosystem( _CategoryName[ _RmEcosystemEnd - 1 ] ); ++i )
			{
				for( uint32 q = 0; q < min((uint32)NUM_QUALITY, (uint32)(_QualityEnd - _QualityStart - 1)); ++q )
				{
					IShopUnit * shopUnit = shop->getShopUnit( i, q, level, IShopUnit::DynamicShop );
					if( shopUnit != 0 )
					{
						merchant.addShopUnit( shopUnit );
					}
				}
			}
		}
	}
}


//----------------------------------------------------------------------------
// CShopTypeManager::addTpShopBase add teleport shop base reference to tradeList
// 
//----------------------------------------------------------------------------
void CShopTypeManager::addTpShopBase( uint32 shopCategoryNumber, uint32 ecosystem, TShopTypeSelected shopTypeSelected, CMerchant& merchant )
{
	CTPShopType * shop = dynamic_cast<CTPShopType *>(_ShopTypes[ _CategoryNumberToShopCategory [ shopCategoryNumber ] ]);
	if( shop != 0 )
	{
		if( shopTypeSelected == StaticShopSelected || shopTypeSelected == AllShopSelected )
		{
			IShopUnit * shopUnit = shop->getShopUnit( ecosystem, 0, 0, IShopUnit::StaticShop );
			if( shopUnit != 0 )
			{
				merchant.addShopUnit( shopUnit );
			}
		}
		if( shopTypeSelected == DynamicShopSelected || shopTypeSelected == AllShopSelected )
		{
			IShopUnit * shopUnit = shop->getShopUnit( ecosystem, 0, 0, IShopUnit::DynamicShop );
			if( shopUnit != 0 )
			{
				merchant.addShopUnit( shopUnit );
			}
		}
	}
}


//----------------------------------------------------------------------------
// CShopTypeManager::addShopBase add shop base reference to merchant
// 
//----------------------------------------------------------------------------
void CShopTypeManager::addShopBase( uint32 shopCategoryNumber, CMerchant& merchant, std::vector< uint32 >& rmSelector, std::vector< uint32 >& originSelector, std::vector< uint32 >& qualitySelector, std::vector< uint32 >& levelSelector, std::vector< uint32 >& shopTypeSelector )
{
	TShopTypeSelected shopTypeSelected;
	if( shopTypeSelector.size() == 0 )
	{
		shopTypeSelected = AllShopSelected;
	}
	else
	{
		for( uint32 i = 0; i < shopTypeSelector.size(); ++i )
		{
			if( _CategoryName[ shopTypeSelector[ i ] ] == string("STATIC_SHOP") )
			{
				shopTypeSelected = StaticShopSelected;
			}
			else if( _CategoryName[ shopTypeSelector[ i ] ] == string("DYNAMIC_SHOP") )
			{
				shopTypeSelected = DynamicShopSelected;
			}
			else if( _CategoryName[ shopTypeSelector[ i ] ] == string("STATIC_DYNAMIC_SHOP") )
			{
				shopTypeSelected = AllShopSelected;
			}
		}
	}
		
	if( shopCategoryNumber < _CategoryNumberToShopCategory.size() )
	{
		if( _CategoryNumberToShopCategory [ shopCategoryNumber ] < INVALID_SHOP_INDEX )
		{
			//*** Items Shop base ***//
			if( shopCategoryNumber > _ItemStart && shopCategoryNumber < _ItemEnd )
			{
				if (VerboseShopParsing)
					egs_shinfo("     <CShopTypeManager::addShopBase> item selected for shop category number: %d - shop index %d", shopCategoryNumber, _CategoryNumberToShopCategory [ shopCategoryNumber ] );
				if( originSelector.size() == 0 )
				{
					// All origin selected
					for( uint32 origin = 0; origin < ITEM_ORIGIN::NUM_ITEM_ORIGIN; ++origin )
					{
						if( qualitySelector.size() == 0 )
						{
							// All quality selected
							for( uint16 q = 0; q < min((uint)NUM_QUALITY, (uint)(_QualityEnd - _QualityStart - 1)); ++q )
							{
//								if( q < 1 ) //temporary until level design enter selector of quality per merchant
								if( levelSelector.size() == 0 )
								{
									// All level selected
									for( uint16 l = 0; l < min((uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1)); ++l )
									{
										addItemShopBase( shopCategoryNumber, origin, q, l, shopTypeSelected, merchant);
									}
								}
								else
								{
									// Some level selected
									for( vector< uint32 >::const_iterator itLevel = levelSelector.begin(); itLevel != levelSelector.end(); ++itLevel )
									{
										addItemShopBase( shopCategoryNumber, origin, q, *itLevel - _LevelStart - 1, shopTypeSelected, merchant );
									}
								}
							}
						}
						else
						{
							// Some quality selected
							for( vector< uint32 >::const_iterator itQuality = qualitySelector.begin(); itQuality != qualitySelector.end(); ++itQuality )
							{
								if( levelSelector.size() == 0 )
								{
									// All level selected
									for( uint16 l = 0; l < min((uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1)); ++l )
									{
										addItemShopBase( shopCategoryNumber, origin, *itQuality - _QualityStart - 1, l, shopTypeSelected, merchant);
									}
								}
								else
								{
									// Some level selected
									for( vector< uint32 >::const_iterator itLevel = levelSelector.begin(); itLevel != levelSelector.end(); ++itLevel )
									{
										addItemShopBase( shopCategoryNumber, origin, *itQuality - _QualityStart - 1, *itLevel - _LevelStart - 1, shopTypeSelected, merchant );
									}
								}
							}
						}
					}
				}
				else
				{
					// Some origin selected
					for( vector< uint32 >::const_iterator itOrigin = originSelector.begin(); itOrigin != originSelector.end(); ++itOrigin )
					{
						if( qualitySelector.size() == 0 )
						{
							// All quality selected
							for( uint16 q = 0; q < min((uint)NUM_QUALITY, (uint)(_QualityEnd - _QualityStart - 1)); ++q )
							{
//								if( q < 1 ) //temporary until level design enter selector of quality per merchant
								if( levelSelector.size() == 0 )
								{
									// All level selected
									for( uint16 l = 0; l < min((uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1)); ++l )
									{
										addItemShopBase( shopCategoryNumber, ITEM_ORIGIN::stringToEnum( _CategoryName[ *itOrigin ] ), q, l, shopTypeSelected, merchant );
									}
								}
								else
								{
									// Some level selected
									for( vector< uint32 >::const_iterator itLevel = levelSelector.begin(); itLevel != levelSelector.end(); ++itLevel )
									{
										addItemShopBase( shopCategoryNumber, ITEM_ORIGIN::stringToEnum( _CategoryName[ *itOrigin ] ), q, *itLevel - _LevelStart - 1, shopTypeSelected, merchant );
									}
								}
							}
						}
						else
						{
							// Some quality selected
							for( vector< uint32 >::const_iterator itQuality = qualitySelector.begin(); itQuality != qualitySelector.end(); ++itQuality )
							{
								if( levelSelector.size() == 0 )
								{
									// All level selected
									for( uint16 l = 0; l < min((uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1)); ++l )
									{
										addItemShopBase( shopCategoryNumber, ITEM_ORIGIN::stringToEnum( _CategoryName[ *itOrigin ] ), *itQuality - _QualityStart - 1, l, shopTypeSelected, merchant );
									}
								}
								else
								{
									// Some level selected
									for( vector< uint32 >::const_iterator itLevel = levelSelector.begin(); itLevel != levelSelector.end(); ++itLevel )
									{
										addItemShopBase( shopCategoryNumber, ITEM_ORIGIN::stringToEnum( _CategoryName[ *itOrigin ] ), *itQuality - _QualityStart - 1, *itLevel - _LevelStart - 1, shopTypeSelected, merchant );
									}
								}
							}
						}
					}						
				}
			}
			//***  Raw materials Shop base ***//
			else if( shopCategoryNumber > _RmStart && shopCategoryNumber < _RmEnd )
			{
				if( rmSelector.size() == 0 )
				{
					// All ecosystem selected					
					egs_shinfo("     <CShopTypeManager::addShopBase> All ecosystem selected for shop category number: %d - shop index %d", shopCategoryNumber, _CategoryNumberToShopCategory [ shopCategoryNumber ] );
					
					for( uint32 eco = 0; eco < ECOSYSTEM::NUM_ECOSYSTEM; ++eco )
					{
						if (VerboseShopParsing)
							egs_shinfo("     <CShopTypeManager::addShopBase> Ecosystem '%s' selected for shop category number: %d - shop index %d", 
										_CategoryName[ eco ].c_str(),
										shopCategoryNumber, 
										_CategoryNumberToShopCategory [ shopCategoryNumber ] );
						if( qualitySelector.size() == 0 )
						{
							// All quality selected
							for( uint16 q = 0; q < min((uint)NUM_QUALITY, (uint)(_QualityEnd - _QualityStart - 1)); ++q )
							{
//								if( q < 1 ) //temporary until level design enter selector of quality per merchant
								if( levelSelector.size() == 0 )
								{
									// All level selected
									for( uint16 l = 0; l < min((uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1)); ++l )
									{
										addRmShopBase( shopCategoryNumber, eco, q, l, shopTypeSelected, merchant );
									}
								}
								else
								{
									// Some level selected
									for( vector< uint32 >::const_iterator itLevel = levelSelector.begin(); itLevel != levelSelector.end(); ++itLevel )
									{
										addRmShopBase( shopCategoryNumber, eco, q, *itLevel - _LevelStart - 1, shopTypeSelected, merchant );
									}
								}
							}
						}
						else
						{
							// Some quality selected
							for( vector< uint32 >::const_iterator itQuality = qualitySelector.begin(); itQuality != qualitySelector.end(); ++itQuality )
							{
								if( levelSelector.size() == 0 )
								{
									// All level selected
									for( uint16 l = 0; l < min((uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1)); ++l )
									{
										addRmShopBase( shopCategoryNumber, eco, *itQuality - _QualityStart - 1, l, shopTypeSelected, merchant );
									}
								}
								else
								{
									// Some level selected
									for( vector< uint32 >::const_iterator itLevel = levelSelector.begin(); itLevel != levelSelector.end(); ++itLevel )
									{
										addRmShopBase( shopCategoryNumber, eco, *itQuality - _QualityStart - 1, *itLevel - _LevelStart - 1, shopTypeSelected, merchant );
									}
								}
							}	
						}
					}
				}
				else
				{
					// Some ecosystem selected
					for( vector< uint32 >::const_iterator itRm = rmSelector.begin(); itRm != rmSelector.end(); ++itRm )
					{
						if( qualitySelector.size() == 0 )
						{
							// All quality selected
							for( uint16 q = 0; q < min((uint)NUM_QUALITY, (uint)(_QualityEnd - _QualityStart - 1)); ++q )
							{
//								if( q < 1 ) //temporary until level design enter selector of quality per merchant
								if( levelSelector.size() == 0 )
								{
									// All level selected
									for( uint16 l = 0; l < min((uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1)); ++l )
									{
										addRmShopBase( shopCategoryNumber, ECOSYSTEM::stringToEcosystem( _CategoryName[ *itRm ] ), q, l, shopTypeSelected, merchant );
									}
								}
								else
								{
									// Some level selected
									for( vector< uint32 >::const_iterator itLevel = levelSelector.begin(); itLevel != levelSelector.end(); ++itLevel )
									{
										addRmShopBase( shopCategoryNumber, ECOSYSTEM::stringToEcosystem( _CategoryName[ *itRm ] ), q, *itLevel - _LevelStart - 1, shopTypeSelected, merchant );
									}
								}
							}
						}
						else
						{
							// Some quality selected
							for( vector< uint32 >::const_iterator itQuality = qualitySelector.begin(); itQuality != qualitySelector.end(); ++itQuality )
							{
								if( levelSelector.size() == 0 )
								{
									// All level selected
									for( uint16 l = 0; l < min((uint)NUM_LEVEL, (uint)(_LevelEnd - _LevelStart - 1)); ++l )
									{
										addRmShopBase( shopCategoryNumber, ECOSYSTEM::stringToEcosystem( _CategoryName[ *itRm ] ), *itQuality - _QualityStart - 1, l, shopTypeSelected, merchant );
									}
								}
								else
								{
									// Some level selected
									for( vector< uint32 >::const_iterator itLevel = levelSelector.begin(); itLevel != levelSelector.end(); ++itLevel )
									{
										addRmShopBase( shopCategoryNumber, ECOSYSTEM::stringToEcosystem( _CategoryName[ *itRm ] ), *itQuality - _QualityStart - 1, *itLevel - _LevelStart - 1, shopTypeSelected, merchant );
									}
								}
							}	
						}
					}
				}
			}
			//***  Teleport Shop base ***//
			else if( shopCategoryNumber > _TpStart && shopCategoryNumber < _TpEnd )
			{
				if( rmSelector.size() == 0 )
				{
					// select all ecosystem
					egs_shinfo("     <CShopTypeManager::addShopBase> All ecosystem selected for shop category number: %d - shop index %d", shopCategoryNumber, _CategoryNumberToShopCategory [ shopCategoryNumber ] );
					for( uint32 eco = 0; eco < ECOSYSTEM::NUM_ECOSYSTEM; ++eco )
					{
						addTpShopBase( shopCategoryNumber, eco, shopTypeSelected, merchant );
					}
				}
				else
				{
					for( vector< uint32 >::const_iterator itTp = rmSelector.begin(); itTp != rmSelector.end(); ++itTp )
					{	
						addTpShopBase( shopCategoryNumber, ECOSYSTEM::stringToEcosystem( _CategoryName[ *itTp ] ), shopTypeSelected, merchant );
					}
				}
			}
		}
	}
}


//----------------------------------------------------------------------------
// Calculate item base price
//
//----------------------------------------------------------------------------
uint32 CShopTypeManager::computeBasePrice( CGameItemPtr itemPtr, uint16 level )
{
	BOMB_IF( itemPtr == 0, "CShopTypeManager::computeBasePrice: itemPtr == 0", return 0 );
	float statEnergyMod = 1.0f + itemPtr->getStatEnergy();
	return computeBasePrice( itemPtr->getSheetId(), level, statEnergyMod );
}

uint32 CShopTypeManager::computeBasePrice( const CSheetId& sheet, uint16 level, float statEnergyModifier )
{
	// get the form
	const CStaticItem * itemForm = CSheets::getForm( sheet );
	BOMB_IF( itemForm == 0, "CShopTypeManager::computeBasePrice: can't found item form", return 0 );

	// process non saleable item like sap recharge
	if( itemForm->Family == ITEMFAMILY::ITEM_SAP_RECHARGE || itemForm->Family == ITEMFAMILY::CRYSTALLIZED_SPELL || itemForm->Family == ITEMFAMILY::XP_CATALYSER )
		return 0;

	// proceed special items ( teleport and animals have their price in sheets )
	if( itemForm->Family == ITEMFAMILY::PET_ANIMAL_TICKET || itemForm->Family == ITEMFAMILY::TELEPORT )
		return itemForm->ItemPrice;

	// tool price are in sheet
	if(itemForm->Family == ITEMFAMILY::CRAFTING_TOOL  || itemForm->Family == ITEMFAMILY::HARVEST_TOOL) 
		return itemForm->ItemPrice;

	if( itemForm->Family == ITEMFAMILY::COSMETIC || itemForm->Family == ITEMFAMILY::FOOD || itemForm->Family == ITEMFAMILY::SERVICE || itemForm->Family == ITEMFAMILY::HANDLED_ITEM || itemForm->Family == ITEMFAMILY::GENERIC_ITEM ) 
		return itemForm->ItemPrice;

	if( itemForm->Family == ITEMFAMILY::CONSUMABLE ) 
		return itemForm->ItemPrice;
	
	// raw material price computation
	if( itemForm->Family == ITEMFAMILY::RAW_MATERIAL )
	{
		return computeRawMaterialBasePrice(level) * ( 100 + itemForm->Mp->StatEnergy ) / 100;
	}

	// item price
	// get the craft plan
	const CStaticBrick * brick = CSheets::getSBrickForm( itemForm->CraftPlan );
	if( !brick )
	{
		nlwarning("<CShopTypeManager> Can't find craft plan sheet '%s' in item form '%s'", itemForm->CraftPlan.toString().c_str(), sheet.toString().c_str() );
		return 0;
	}
	const CFaber* faber = brick->Faber;
	if ( !faber )
	{
		nlwarning("<CShopTypeManager> craft plan sheet %s has no valid faber", itemForm->CraftPlan.toString().c_str() );
		return 0;
	}
	// compute the total quantity of MP in the item
	uint quantity = 0;
	for ( uint i = 0; i < faber->NeededMps.size(); i++ )
	{
		quantity += faber->NeededMps[i].Quantity;
	}

	// compute item price depending of quantity of Mp used for build it, level of item and stat energy of item (depending of stat energy of mp used for craft it)
	uint32 price = (uint32) ( quantity * computeRawMaterialBasePrice(level) * statEnergyModifier );
	if( itemForm->Family == ITEMFAMILY::AMMO )
	{
		price /= max( (uint32)1, itemForm->Stackable );
	}
	return price;
}


/*
//----------------------------------------------------------------------------
// Calculate item price
//
//----------------------------------------------------------------------------
uint32 CShopTypeManager::computePlayerBuyPrice( const NLMISC::CSheetId& sheet, uint16 level )
{
	// get the form
	const CStaticItem * itemForm = CSheets::getForm( sheet );
	if(!itemForm)
	{
		nlwarning("<CShopTypeManager> Can't find item  sheet '%s'", sheet.toString().c_str() );
		return 0;
	}

	// process non saleable item like sap recharge
	if( itemForm->Family == ITEMFAMILY::ITEM_SAP_RECHARGE || itemForm->Family == ITEMFAMILY::CRYSTALLIZED_SPELL )
	{
		return 0;
	}

	// proceed special items ( teleport and animals have their price in sheets )
	if( itemForm->Family == ITEMFAMILY::PET_ANIMAL_TICKET || itemForm->Family == ITEMFAMILY::TELEPORT )
	{
		return itemForm->ItemPrice;
	}

	// tool price are in sheet
	if(itemForm->Family == ITEMFAMILY::CRAFTING_TOOL  || itemForm->Family == ITEMFAMILY::HARVEST_TOOL) 
	{
		return itemForm->ItemPrice;
	}

	// raw materials price computation
	if( itemForm->Family == ITEMFAMILY::RAW_MATERIAL )
	{
		return computeRawMaterialPlayerBuyPrice(level);
	}

	// special factor for ammo ( divided by quantity obtained through craft ( = stackable ) )
	float divide = 1.0f;
	if( itemForm->Family == ITEMFAMILY::AMMO )
	{
		divide = (float)itemForm->Stackable;
		if ( divide == 0.0f )
			divide = 1.0f;
	}
	
	// standard item computation
	// get the craft plan
	const CStaticBrick * brick = CSheets::getSBrickForm( itemForm->CraftPlan );
	if( !brick )
	{
		nlwarning("<CShopTypeManager> Can't find craft plan sheet '%s' in item form '%s'", itemForm->CraftPlan.toString().c_str(), sheet.toString().c_str() );
		return 0;
	}
	const CFaber* faber = brick->Faber;
	if ( !faber )
	{
		nlwarning("<CShopTypeManager> craft plan sheet %s has no valid faber", itemForm->CraftPlan.toString().c_str() );
		return 0;
	}
	// compute the total quantity of MP in the item
	uint quantity = 0;
	for ( uint i = 0; i < faber->NeededMps.size(); i++ )
	{
		quantity += faber->NeededMps[i].Quantity;
	}
	return uint32( quantity * ItemPriceFactor * ( ItemPriceCoeff2 * level * level + ItemPriceCoeff1 * level  + ItemPriceCoeff0 ) / divide );
	
}
*/

//----------------------------------------------------------------------------
// client sell some item, compute sell price
//
//----------------------------------------------------------------------------
/*
uint32 CShopTypeManager::computePlayerSellPrice( const NLMISC::CSheetId& Sheet, uint16 Level, uint16 Quantity, float wornFactor, uint32 & maxPrice )
{
	///WARNING :  we have to round down item price before multiplying by the quantity!
	// get the form
	const CStaticItem * itemForm = CSheets::getForm( Sheet );
	if(!itemForm)
	{
		nlwarning("<CShopTypeManager> Can't find item  sheet %s", Sheet.toString().c_str() );
		return 0;
	}

	// Crystalized Spell are no value for Npc
	if( itemForm->Family == ITEMFAMILY::CRYSTALLIZED_SPELL )
	{
		return 0;
	}

	// proceed special items ( teleport and animals have their price in sheets )
	if( itemForm->Family == ITEMFAMILY::PET_ANIMAL_TICKET)
	{
		return (uint32)(itemForm->ItemPrice * AnimalSellFactor * wornFactor) * Quantity;
	}
	
	// proceed special items ( teleport and animals have their price in sheets )
	if( itemForm->Family == ITEMFAMILY::TELEPORT )
	{
		return (uint32)(itemForm->ItemPrice * TeleportSellFactor * wornFactor) * Quantity;
	}
	
	// tool are free
	if(itemForm->Family == ITEMFAMILY::CRAFTING_TOOL  || itemForm->Family == ITEMFAMILY::HARVEST_TOOL) 
	{
		return uint32(itemForm->ItemPrice * 0.8f * wornFactor);
	}
	
	// raw materials price computation
	if( itemForm->Family == ITEMFAMILY::RAW_MATERIAL )
	{
		return uint32 ( wornFactor * (ItemPriceCoeff1 * Level  + ItemPriceCoeff0) ) * Quantity;
	}

	// special factor for ammo ( divided by quantity obtained through craft ( = stackable ) )
	float divide = 1.0f;
	if( itemForm->Family == ITEMFAMILY::AMMO )
	{
		divide = (float)itemForm->Stackable;
		if ( divide == 0.0f )
			divide = 1.0f;
	}
	// get the craft plan
	const CStaticBrick * brick = CSheets::getSBrickForm( itemForm->CraftPlan );
	if( !brick )
	{
		nlwarning("<CShopTypeManager> Can't find craft plan sheet '%s' in item form '%s'", itemForm->CraftPlan.toString().c_str(), Sheet.toString().c_str() );
		return 0;
	}
	const CFaber* faber = brick->Faber;
	if ( !faber )
	{
		nlwarning("<CShopTypeManager> craft plan sheet %s has no valid faber", itemForm->CraftPlan.toString().c_str() );
		return 0;
	}
	// compute the total quantity of MP in the item
	uint nbMp = 0;
	for ( uint i = 0; i < faber->NeededMps.size(); i++ )
	{
		nbMp += faber->NeededMps[i].Quantity;
	}

	maxPrice = uint32( Quantity * (nbMp * wornFactor *  computeRawMaterialPlayerBuyPrice( Level )) );
	return uint32( Quantity * ( wornFactor * nbMp * uint32( (ItemPriceCoeff1 * Level  + ItemPriceCoeff0 ) ) )/ divide  ) ;
}
*/

//----------------------------------------------------------------------------
// Eval Specialization for return Characteristics value
//
//----------------------------------------------------------------------------
CEvalNumExpr::TReturnState CShopTypeManager::evalValue (const char *value, double &result, uint32 userData)
{
	nlwarning("<CShopTypeManager::evalValue> 'standart.prices_table' still used");
	result = userData * pow( 1.2, (int)userData ); 
	return CEvalNumExpr::UnknownValue;
}


//----------------------------------------------------------------------------
float CShopTypeManager::getFamePriceFactor(sint32 fameValue)
{
	if ( fameValue < MinFameToTrade )
	{
		nlwarning("user has not the right fame : serious bug in fame system : will happen with every user");
		return  0.0f;
	}

	if ( fameValue > MaxFameToTrade )
		fameValue = MaxFameToTrade;
	
	float factor = MaxFamePriceVariation * float( MaxFameToTrade - fameValue) / float ( MaxFameToTrade - MinFameToTrade );
	if ( factor > MaxFamePriceVariation )
		return MaxFamePriceVariation;
	return factor;
}
