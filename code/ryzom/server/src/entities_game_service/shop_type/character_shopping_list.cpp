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

#include "game_share/bot_chat_types.h"
#include "game_share/rm_family.h"
#include "game_share/visual_slot_manager.h"
#include "game_share/slot_types.h"

#include "shop_type/character_shopping_list.h"
#include "player_manager/character.h"
#include "egs_sheets/egs_sheets.h"
#include "egs_variables.h"
#include "entities_game_service.h"
#include "offline_character_command.h"
#include "dynamic_items.h"

#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "shop_type/items_for_sale.h"
#include "shop_type/shop_type_manager.h"
#include "mission_manager/mission_event.h"
#include "player_manager/item_service_manager.h"
#include "server_share/log_item_gen.h"


using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CCharacterShoppingList);

//-----------------------------------------------------------------------------
CCharacterShoppingList::CCharacterShoppingList( CSmartPtr<CMerchant>& merchant, CCharacter& character, float fameFactor )
{ 
	_CurrentMerchant = merchant; 
	_Character = & character;
/*	if( _CurrentMerchant )
	{
		_CurrentMerchant->addCharacterShowingTradeList( *this );
	}
*/	_FameFactor = fameFactor;
} 

//-----------------------------------------------------------------------------
CCharacterShoppingList::~CCharacterShoppingList()
{
	_CurrentTradeListNpc.clear();
	_CurrentTradeListPlayer.clear();
	_CurrentTradeListYours.clear();
}

//-----------------------------------------------------------------------------
struct TTradeItemCmp : public std::binary_function< TItemTradePtr, TItemTradePtr, bool>
{
	// operator < in fact
	bool operator () (const TItemTradePtr &lItemPtr, const TItemTradePtr &rItemPtr) const
	{
		if (lItemPtr->getSheetId() != rItemPtr->getSheetId())
			return (lItemPtr->getSheetId() < rItemPtr->getSheetId());
		if (lItemPtr->getLevel() != rItemPtr->getLevel())
			return (lItemPtr->getLevel() < rItemPtr->getLevel());
		if (lItemPtr->getQuality() != rItemPtr->getQuality())
			return (lItemPtr->getQuality() < rItemPtr->getQuality());
		return false;
	}
};

//-----------------------------------------------------------------------------
void CCharacterShoppingList::mountShoppingList( CONTINENT::TContinent continent )
{
	set< TItemTradePtr > setOfPlayerItem;
	set< TItemTradePtr, TTradeItemCmp> setOfNpcItem;

	// mount shop list of current merchant
	if( _CurrentMerchant )
	{
		bool altarRestrictionReport = false;

		typedef std::vector< TItemTradePtr > TTradeList;

		const vector< const IShopUnit * >& merchantTradeList = _CurrentMerchant->getMerchantTradeList();
		
		for( uint32 shopIdx = 0; shopIdx < merchantTradeList.size(); ++shopIdx )
		{
			const IShopUnit * shop = merchantTradeList [ shopIdx ];

			for( uint32 item = 0; item < shop->getShopContent( continent ).size(); ++item )
			{
				if( passThruFilter( shop->getShopContent( continent )[ item ], shop->getShopUnitType() == IShopUnit::DynamicShop ) == false )
					continue;

				if( shop->getShopUnitType() == IShopUnit::DynamicShop )
				{
					// if npc selling players item, player can see it's item in sell and retire them from npc
					if( _CurrentTradeListPlayer.size() < NBMaxItemPlayerSellDisplay )
					{
						if( setOfPlayerItem.find( shop->getShopContent( continent )[ item ] ) == setOfPlayerItem.end() )
						{
							bool retirable = false;
							if( shop->getShopContent( continent )[ item ]->getOwner() == _Character->getId() )
							{
								if( shop->getShopContent( continent )[ item ]->getContinent() == continent )
								{
									retirable = true;
								}
							}
							_CurrentTradeListPlayer.push_back( TShopStruct( shop->getShopContent( continent )[ item ], shop, retirable ) );
							setOfPlayerItem.insert( shop->getShopContent( continent )[ item ] );
						}
					}
					else
						break;
				}
				else
				{
					// Stop if we reach the max displayable number of items to sell
					if (_CurrentTradeListNpc.size() >= NBMaxItemNpcSellDisplay)
						break;

					TItemTradePtr curItem = shop->getShopContent(continent)[item];

					// if don't allow same item, must check for duplicate item in the list
					bool bCanInsert = true;
					if (!curItem->allowSameItemInShopList())
					{
						if (setOfNpcItem.find(curItem) != setOfNpcItem.end()) // found same item
							bCanInsert = false;
					}

					// if item is a TP ticket, check altar restriction
					const CStaticItem * sheet = CSheets::getForm( curItem->getSheetId() );
					if( sheet != 0 )
					{
						if( bCanInsert && sheet->Family == ITEMFAMILY::TELEPORT )
						{
							bCanInsert = checkAltarRestriction();
							if( bCanInsert == false )
								altarRestrictionReport = true;
						}
					}

					if (bCanInsert)
					{
						_CurrentTradeListNpc.push_back(TShopStruct(curItem, shop));
						setOfNpcItem.insert(curItem);
						// for debug // nlinfo("inserted item : %s lvl %d qlt %d", curItem->getSheetId().toString().c_str(), curItem->getLevel(), curItem->getQuality());
					}
					else
					{
						// for debug // nlinfo("cannot insert item : %s lvl %d qlt %d", curItem->getSheetId().toString().c_str(), curItem->getLevel(), curItem->getQuality());
					}
				}
			}
		}
		if( altarRestrictionReport)
			CCharacter::sendDynamicSystemMessage(_Character->getEntityRowId(), "ALTAR_RESTRICTION");
	}

	// add character's item in shop store in same continent
	if( _CurrentMerchant->sellPlayerItem() )
	{
		for( uint32 i = 0; i < _Character->getItemInShop().getContent().size(); ++i )
		{
			bool retirable = false;
			if( _Character->getItemInShop().getContent()[ i ]->getContinent() == continent )
			{
				retirable = true;
			}
			if( _CurrentTradeListYours.size() < NBMaxItemYoursSellDisplay )
				_CurrentTradeListYours.push_back( TShopStruct( _Character->getItemInShop().getContent()[ i ], 0, retirable ) );
			else
				break;
		}
	}
}

//-----------------------------------------------------------------------------
bool CCharacterShoppingList::passThruFilter(TItemTradePtr itemTrade, bool dynnamicShop )
{
	const CStaticItem * form = CSheets::getForm( itemTrade->getSheetId() );

	// No filter on Faction trade
	if(_Character && _Character->getBotChatType() == BOTCHATTYPE::TradeFactionFlag)
		return true;

	if( form != 0 )
	{
		uint32 itemEnergy = 0;

		if( form->Family == ITEMFAMILY::RAW_MATERIAL )
		{
			itemEnergy = form->Mp->StatEnergy;

			if( _Character->getRawMaterialItemPartFilter() != RM_FABER_TYPE::Unknown )
			{
				bool itemPartMatch = false;
				for( uint32 i = 0; i < form->Mp->MpFaberParameters.size(); ++i )
				{
					if( form->Mp->MpFaberParameters[ i ].MpFaberType == _Character->getRawMaterialItemPartFilter() )
					{
						itemPartMatch = true;
						break;
					}
				}
				if( itemPartMatch == false )
					return false;
			}
		}
		else if (form->Family == ITEMFAMILY::SERVICE)
		{
			if (!CItemServiceManager::getInstance()->serviceIsAvailable(form, _Character))
				return false;
		}
		else
		{
			if( itemTrade->getItemPtr() != 0 )
			{
				itemEnergy = (uint32) (itemTrade->getItemPtr()->getCraftParameters() == 0 ? 0 : itemTrade->getItemPtr()->getCraftParameters()->StatEnergy * 100);
			}
			else
			{
				itemEnergy = ~0u;
			}
		}


		if( _Character->getItemTypeFilter() != ITEM_TYPE::UNDEFINED )
		{
			switch( form->Family )
			{
			case ITEMFAMILY::ARMOR:
			case ITEMFAMILY::MELEE_WEAPON:
			case ITEMFAMILY::RANGE_WEAPON:
			case ITEMFAMILY::AMMO:
			case ITEMFAMILY::SHIELD:
			case ITEMFAMILY::JEWELRY:
			case ITEMFAMILY::PET_ANIMAL_TICKET:
			case ITEMFAMILY::FOOD:
			case ITEMFAMILY::HARVEST_TOOL:
			case ITEMFAMILY::CRAFTING_TOOL:
			case ITEMFAMILY::COSMETIC:
			case ITEMFAMILY::CONSUMABLE:
				if( form->Type != _Character->getItemTypeFilter() )
					return false;
				break;
			default:
				break;
			}
		}

		if( form->Family != ITEMFAMILY::PET_ANIMAL_TICKET &&
			form->Family != ITEMFAMILY::HARVEST_TOOL &&
			form->Family != ITEMFAMILY::FOOD &&
			form->Family != ITEMFAMILY::HARVEST_TOOL &&
			form->Family != ITEMFAMILY::CRAFTING_TOOL &&
			form->Family != ITEMFAMILY::COSMETIC &&
			form->Family != ITEMFAMILY::TELEPORT &&
			form->Family != ITEMFAMILY::SERVICE && 
			form->Family != ITEMFAMILY::GENERIC_ITEM &&
			form->Family != ITEMFAMILY::CONSUMABLE
			)
		{
			if( _Character->getMinClassItemFilter() != RM_CLASS_TYPE::Unknown && itemEnergy != ~0u )
			{
				switch( _Character->getMinClassItemFilter() )
				{
				case RM_CLASS_TYPE::Basic:
					break;
				case RM_CLASS_TYPE::Fine:
					if( itemEnergy < RM_CLASS_TYPE::MinFineEnergy )
						return false;
					break;
				case RM_CLASS_TYPE::Choice:
					if( itemEnergy < RM_CLASS_TYPE::MinChoiceEnergy )
						return false;
					break;
				case RM_CLASS_TYPE::Excellent:
					if( itemEnergy < RM_CLASS_TYPE::MinExcellentEnergy )
						return false;
					break;
				case RM_CLASS_TYPE::Supreme:
					if( itemEnergy < RM_CLASS_TYPE::MinSupremeEnergy )
						return false;
					break;
				}
			}

			if( _Character->getMaxClassItemFilter() != RM_CLASS_TYPE::Unknown  && itemEnergy != ~0u )
			{
				switch( _Character->getMaxClassItemFilter() )
				{
				case RM_CLASS_TYPE::Basic:
					if( itemEnergy > RM_CLASS_TYPE::MaxBasicEnergy )
						return false;
					break;
				case RM_CLASS_TYPE::Fine:
					if( itemEnergy > RM_CLASS_TYPE::MaxFineEnergy )
						return false;
					break;
				case RM_CLASS_TYPE::Choice:
					if( itemEnergy > RM_CLASS_TYPE::MaxChoiceEnergy )
						return false;
					break;
				case RM_CLASS_TYPE::Excellent:
					if( itemEnergy > RM_CLASS_TYPE::MaxExcellentEnergy )
						return false;
					break;
				case RM_CLASS_TYPE::Supreme:
					break;
				}
			}
	
			if( _Character->getMinQualityFilter() > itemTrade->getLevel() )
				return false;
			if( _Character->getMaxQualityFilter() < itemTrade->getLevel() )
				return false;
		}

		// bypass price check for TP, cosmetics and services
		if (form->Family != ITEMFAMILY::TELEPORT &&	form->Family != ITEMFAMILY::SERVICE && form->Family != ITEMFAMILY::COSMETIC)
		{
			if( _Character->getMinPriceFilter() > getSellPrice( itemTrade, dynnamicShop ) )
				return false;
			if( _Character->getMaxPriceFilter() < getSellPrice( itemTrade, dynnamicShop ) )
				return false;
		}
	}
	
	return true;
}

//-----------------------------------------------------------------------------
bool CCharacterShoppingList::checkAltarRestriction()
{
	PVP_CLAN::TPVPClan altarClanRestriction = _CurrentMerchant->getAltarClanRestriction();
	if( altarClanRestriction != PVP_CLAN::None )
	{
		pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegeance = _Character->getAllegiance();
		if( allegeance.first != altarClanRestriction && allegeance.second != altarClanRestriction )
		{
			if( _CurrentMerchant->getAltarForNeutral() == false )
				return false;
			else if( allegeance.first != PVP_CLAN::Neutral /* allegeance.second != PVP_CLAN::Neutral*/ )
				return false;

		}
	}
	
	uint32 altarFameRestriction = _CurrentMerchant->getAltarFameRestriction();
	if( altarFameRestriction != CStaticFames::INVALID_FACTION_INDEX )
	{
		if( CFameInterface::getInstance().getFameIndexed( _Character->getId(), altarFameRestriction ) < _CurrentMerchant->getAltarFameValueRestriction() )
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
uint32 CCharacterShoppingList::getSellPrice( const TItemTradePtr itemTrade, bool dynnamicShop )
{
	float priceFactor = 1.0f;
	if( _CurrentMerchant != 0 && dynnamicShop == false )
	{
		priceFactor = _CurrentMerchant->getPriceFactor() * _FameFactor;

		const CStaticItem * form = CSheets::getForm( itemTrade->getSheetId() );
		if( form != 0 )
		{
			switch( form->Family )
			{
			case ITEMFAMILY::ARMOR:
			case ITEMFAMILY::MELEE_WEAPON:
			case ITEMFAMILY::RANGE_WEAPON:
			case ITEMFAMILY::AMMO:
			case ITEMFAMILY::SHIELD:
			case ITEMFAMILY::JEWELRY:
			case ITEMFAMILY::RAW_MATERIAL:
			case ITEMFAMILY::CONSUMABLE:
				priceFactor *= 4.0f;
				break;
			case ITEMFAMILY::COSMETIC:
				if( ( form->Type == ITEM_TYPE::HAIR_FEMALE || form->Type == ITEM_TYPE::HAIR_MALE ) && _Character->getHairCutDiscount() )
				{
					return 1;
				}
				break;
			case ITEMFAMILY::SERVICE:
				return uint32(CItemServiceManager::getInstance()->getServicePrice(form, _Character) * priceFactor);
			default:
				break;
			}
		}
	}	
	return (uint32) (itemTrade->getPriceInfo().getAmount() * priceFactor);
}

//-----------------------------------------------------------------------------
void CCharacterShoppingList::initPageToUpdate( uint32 nbSlotPerPage )
{
	_NbSlotPerPage = nbSlotPerPage;

	const uint nbPages = (uint)(_CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() + _CurrentTradeListYours.size() + nbSlotPerPage-1) / nbSlotPerPage ;
	
	// Yoyo: to avoid "my YOURS list is refreshing too late"
	/*for (uint i = 0 ; i < nbPages ; ++i)
		_TradePagesToUpdate.push_back(i);*/

	// spread (approximatively) the page index
	const uint32	listCount= 3;
	uint32			curPageForList[listCount];
	uint32			endPageForList[listCount];

	// compute iterator for the 3 lists
	curPageForList[0]= 0;
	endPageForList[0]= (uint32)(_CurrentTradeListNpc.size()) / nbSlotPerPage;
	curPageForList[1]= endPageForList[0];
	endPageForList[1]= (uint32)(_CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size()) / nbSlotPerPage;
	curPageForList[2]= endPageForList[1];
	endPageForList[2]= nbPages;

	// then fill spreaded
	uint	which= 0;
	for(uint i=0;i<nbPages;++i)
	{
		// increment which if the current page choice is empty
		while(curPageForList[which]==endPageForList[which])
		{
			which++;
			which%=listCount;
		}

		// fill which index
		_TradePagesToUpdate.push_back((uint16)curPageForList[which]);

		// increment this list index
		curPageForList[which]++;

		// spread
		which++;
		which%=listCount;
	}
}


//-----------------------------------------------------------------------------
void CCharacterShoppingList::fillTradePage( uint16 session )
{
/*
- Ajout de TRADING:*:PRICE_RETIRE (utile que si SELLER_TYPE == User ou ResaleAndUser). Note: Prix UNITAIRE (si MP, Ammo etc..)
- Si SELLER_TYPE==User seulement: Si PRICE==-1, alors cela veut dire " Item Sold "  (pour afficher la liste des items vendus les plus récents)
- Note: TRADING:*:PRICE reste un Prix UNITAIRE (si MP, Ammo etc..) (cela devait déja être le cas)
*/

	// get page to update if any
	if (_TradePagesToUpdate.empty())
		return;

	uint32 page = (*_TradePagesToUpdate.begin());
	_TradePagesToUpdate.pop_front();
	const uint begin = page * _NbSlotPerPage;

	uint end;
	bool hasNext;

	const uint nbTradeListItems = (uint)(_CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() + _CurrentTradeListYours.size());

	if ( begin + _NbSlotPerPage < nbTradeListItems )
	{
		end = begin + _NbSlotPerPage;
		hasNext = true;
	}
	else
	{
		end = nbTradeListItems;
		hasNext = true;
	}

//	_Character->_PropertyDatabase.setProp( "TRADING:SESSION", session );
	CBankAccessor_PLR::getTRADING().setSESSION(_Character->_PropertyDatabase, session );
//	_Character->_PropertyDatabase.setProp( "TRADING:PAGE_ID", page );
	CBankAccessor_PLR::getTRADING().setPAGE_ID(_Character->_PropertyDatabase, uint8(page) );
//	_Character->_PropertyDatabase.setProp( "TRADING:HAS_NEXT", hasNext );
	CBankAccessor_PLR::getTRADING().setHAS_NEXT(_Character->_PropertyDatabase, hasNext );
//	_Character->_PropertyDatabase.setProp( "TRADING:BUILDING_LOSS_WARNING", 0 );
	CBankAccessor_PLR::getTRADING().setBUILDING_LOSS_WARNING(_Character->_PropertyDatabase, 0 );

	uint index = 0;
	for ( uint i = begin ; i < end; ++i )
	{
		const TShopStruct * trade;
		if( i < _CurrentTradeListNpc.size() )
		{
			trade = &_CurrentTradeListNpc[ i ];
		}
		else if( i < ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() ) )
		{
			trade = &_CurrentTradeListPlayer[ i - _CurrentTradeListNpc.size() ];
		}
		else
		{
			trade = &_CurrentTradeListYours[ i - _CurrentTradeListNpc.size() - _CurrentTradeListPlayer.size() ];
		}
		
		BOTCHATTYPE::TBotChatSellerType sellerType = BOTCHATTYPE::NPC;

		if( trade->ShopUnit == 0 )
		{
			if( trade->Retirable )
			{
				sellerType = BOTCHATTYPE::UserRetirable;
			}
			else
			{
				sellerType = BOTCHATTYPE::User;
			}
		}
		else if( trade->ShopUnit->getShopUnitType() == IShopUnit::DynamicShop )
		{
			if( trade->ItemTrade->getOwner() == _Character->getId() )
			{
				if( trade->Retirable )
				{
					sellerType = BOTCHATTYPE::ResaleAndUserRetirable;				
				}
				else
				{
					sellerType = BOTCHATTYPE::ResaleAndUser;				
				}
			}
			else
			{
				sellerType = BOTCHATTYPE::Resale;
			}
		}

		CBankAccessor_PLR::TTRADING::TArray &tradeElem = CBankAccessor_PLR::getTRADING().getArray(index);

//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SELLER_TYPE",index  ), sellerType );
		tradeElem.setSELLER_TYPE(_Character->_PropertyDatabase, sellerType);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SHEET",index  ), trade->ItemTrade->getSheetId().asInt() );
		tradeElem.setSHEET(_Character->_PropertyDatabase, trade->ItemTrade->getSheetId());
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:QUALITY",index  ), trade->ItemTrade->getLevel() );
		tradeElem.setQUALITY(_Character->_PropertyDatabase, uint16(trade->ItemTrade->getLevel()));

		bool dynamicShop = false;
		if( trade->ShopUnit == 0 )
			dynamicShop = true;
		else if( trade->ShopUnit->getShopUnitType() == IShopUnit::DynamicShop)
			dynamicShop = true;
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:CURRENCY",index ), trade->ItemTrade->getPriceInfo().getCurrency().getValue() );
		tradeElem.setCURRENCY(_Character->_PropertyDatabase, trade->ItemTrade->getPriceInfo().getCurrency().getValue());
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PRICE",index ), getSellPrice( trade->ItemTrade, dynamicShop ) );
		tradeElem.setPRICE(_Character->_PropertyDatabase, getSellPrice( trade->ItemTrade, dynamicShop ));
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RRP_LEVEL",index ),  trade->ItemTrade->getPriceInfo().getRRPLevel().getValue() );
		tradeElem.setRRP_LEVEL(_Character->_PropertyDatabase, trade->ItemTrade->getPriceInfo().getRRPLevel().getValue());
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:MONEY_SHEET",index ),  trade->ItemTrade->getPriceInfo().getSheetId().asInt() );
		tradeElem.setMONEY_SHEET(_Character->_PropertyDatabase, trade->ItemTrade->getPriceInfo().getSheetId());
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:BASE_SKILL",index ),  trade->ItemTrade->getPriceInfo().getSkillType() );
		tradeElem.setBASE_SKILL(_Character->_PropertyDatabase, trade->ItemTrade->getPriceInfo().getSkillType());
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:FACTION_TYPE",index ),  trade->ItemTrade->getPriceInfo().getFaction() );
		tradeElem.setFACTION_TYPE(_Character->_PropertyDatabase, trade->ItemTrade->getPriceInfo().getFaction());

//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:FACTION_TYPE", index ), trade->ItemTrade->getFactionType() );
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PRICE", index ), trade->ItemTrade->getFactionPointPrice() );

//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PRICE_RETIRE",index  ), trade->ItemTrade->getRetirePrice() );
		tradeElem.setPRICE_RETIRE(_Character->_PropertyDatabase, trade->ItemTrade->getRetirePrice());
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RESALE_TIME_LEFT",index  ), (uint32)( trade->ItemTrade->getGameCycleLeft() * CTickEventHandler::getGameTimeStep() / 3600 ) );
		tradeElem.setRESALE_TIME_LEFT(_Character->_PropertyDatabase, uint16( trade->ItemTrade->getGameCycleLeft() * CTickEventHandler::getGameTimeStep() / 3600 ));
		
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SLOT_TYPE",index  ), 0 );
		tradeElem.setSLOT_TYPE(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:INFO_VERSION",index  ), 0 );
		tradeElem.setINFO_VERSION(_Character->_PropertyDatabase, 0);
		const ucstring& vendorName = CEntityIdTranslator::getInstance()->getByEntity( trade->ItemTrade->getOwner() );
//		_Character->_PropertyDatabase.setPropString( NLMISC::toString("TRADING:%u:VENDOR_NAMEID", index), vendorName );
		tradeElem.setVENDOR_NAMEID(_Character->_PropertyDatabase, vendorName);

		if( trade->ItemTrade->getItemPtr() != 0 ) 
		{
			trade->ItemTrade->getItemPtr()->recommended( trade->ItemTrade->getLevel() );
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:USER_COLOR",index  ), trade->ItemTrade->getItemPtr()->color() );
			tradeElem.setUSER_COLOR(_Character->_PropertyDatabase, trade->ItemTrade->getItemPtr()->color());
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:WEIGHT",index  ), (uint16) (trade->ItemTrade->getItemPtr()->weight() / 10 ) );
			tradeElem.setWEIGHT(_Character->_PropertyDatabase, (uint16) (trade->ItemTrade->getItemPtr()->weight() / 10 ));
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:NAMEID",index  ), trade->ItemTrade->getItemPtr()->sendNameId(_Character) );
			tradeElem.setNAMEID(_Character->_PropertyDatabase, trade->ItemTrade->getItemPtr()->sendNameId(_Character));
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:ENCHANT",index  ), trade->ItemTrade->getItemPtr()->getClientEnchantValue() );
			tradeElem.setENCHANT(_Character->_PropertyDatabase, trade->ItemTrade->getItemPtr()->getClientEnchantValue());
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:QUANTITY",index  ), trade->ItemTrade->getQuantity() );
			tradeElem.setQUANTITY(_Character->_PropertyDatabase, uint16(trade->ItemTrade->getQuantity()));
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_CLASS_TYPE",index) , trade->ItemTrade->getItemPtr()->getItemClass() );
			tradeElem.setRM_CLASS_TYPE(_Character->_PropertyDatabase, trade->ItemTrade->getItemPtr()->getItemClass());
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_FABER_STAT_TYPE",index) , trade->ItemTrade->getItemPtr()->getCraftParameters() == 0 ? RM_FABER_STAT_TYPE::Unknown : trade->ItemTrade->getItemPtr()->getCraftParameters()->getBestItemStat() );
			tradeElem.setRM_FABER_STAT_TYPE(_Character->_PropertyDatabase, trade->ItemTrade->getItemPtr()->getCraftParameters() == 0 ? RM_FABER_STAT_TYPE::Unknown : trade->ItemTrade->getItemPtr()->getCraftParameters()->getBestItemStat());
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PREREQUISIT_VALID",index) , _Character->checkPreRequired(trade->ItemTrade->getItemPtr()) );
			tradeElem.setPREREQUISIT_VALID(_Character->_PropertyDatabase, _Character->checkPreRequired(trade->ItemTrade->getItemPtr()));
		}
		else
		{
			const CStaticItem * staticSheet = CSheets::getForm( trade->ItemTrade->getSheetId() );
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:USER_COLOR",index  ), 1 );
			tradeElem.setUSER_COLOR(_Character->_PropertyDatabase, 1);
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:WEIGHT",index  ), staticSheet != NULL ? staticSheet->Weight / 10 : 0 );
			tradeElem.setWEIGHT(_Character->_PropertyDatabase, uint16(staticSheet != NULL ? staticSheet->Weight / 10 : 0));
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:NAMEID",index  ), 0 );
			tradeElem.setNAMEID(_Character->_PropertyDatabase, 0);
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:QUANTITY",index  ), 1 );
			tradeElem.setQUANTITY(_Character->_PropertyDatabase, 1);
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_CLASS_TYPE",index) , 0 );
			tradeElem.setRM_CLASS_TYPE(_Character->_PropertyDatabase, 0);
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_FABER_STAT_TYPE",index) , 0 );
			tradeElem.setRM_FABER_STAT_TYPE(_Character->_PropertyDatabase, 0);
//			_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PREREQUISIT_VALID",index) , 1 );
			tradeElem.setPREREQUISIT_VALID(_Character->_PropertyDatabase, 1);
		}

		++index;
	}

	for ( ; index <  _NbSlotPerPage; ++index )
	{
		CBankAccessor_PLR::TTRADING::TArray &tradeElem = CBankAccessor_PLR::getTRADING().getArray(index);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SHEET",index ), 0 );
		tradeElem.setSHEET(_Character->_PropertyDatabase, CSheetId::Unknown);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:QUALITY",index  ), 0);
		tradeElem.setQUALITY(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:USER_COLOR",index  ), 1);
		tradeElem.setUSER_COLOR(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:WEIGHT",index  ), 0 );
		tradeElem.setWEIGHT(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:NAMEID",index  ), 0 );
		tradeElem.setNAMEID(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:INFO_VERSION",index  ), 0 );
		tradeElem.setINFO_VERSION(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:CURRENCY",index ),  RYMSG::TTradeCurrency::invalid_val );
		tradeElem.setCURRENCY(_Character->_PropertyDatabase, RYMSG::TTradeCurrency::invalid_val);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PRICE",index  ),  0 );
		tradeElem.setPRICE(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RRP_LEVEL",index ),  0 );
		tradeElem.setRRP_LEVEL(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:MONEY_SHEET",index ),  CSheetId::Unknown.asInt() );
		tradeElem.setMONEY_SHEET(_Character->_PropertyDatabase, CSheetId::Unknown);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:BASE_SKILL",index ),  0 );
		tradeElem.setBASE_SKILL(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:FACTION_TYPE", index ), 0 );
		tradeElem.setFACTION_TYPE(_Character->_PropertyDatabase, 0);

//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PRICE", index ), 0 );
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SLOT_TYPE",index  ), 0 );
		tradeElem.setSLOT_TYPE(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:ENCHANT",index  ), 0 );
		tradeElem.setENCHANT(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_CLASS_TYPE",index) , 0 );
		tradeElem.setRM_CLASS_TYPE(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_FABER_STAT_TYPE",index) , 0 );
		tradeElem.setRM_FABER_STAT_TYPE(_Character->_PropertyDatabase, 0);
//		_Character->_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PREREQUISIT_VALID",index) , 0 );
		tradeElem.setPREREQUISIT_VALID(_Character->_PropertyDatabase, 0);
	}
}

//-----------------------------------------------------------------------------
void CCharacterShoppingList::buyItem( uint16 itemNumber, uint32 quantity )
{
	// activate log context
	TLogContext_Item_BuyItem logContext(_Character->getId());;

	// check item index
	if( itemNumber >= ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() + _CurrentTradeListYours.size() ) )
	{
		nlwarning("<CCharacterShoppingList::buyItem> itemNumber not exist...");
		return;
	}
	
	TShopStruct * shop;
	if( itemNumber < _CurrentTradeListNpc.size() )
	{
		shop = &_CurrentTradeListNpc[ itemNumber ];
	}
	else if( itemNumber < ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() ) )
	{
		shop = &_CurrentTradeListPlayer[ itemNumber - _CurrentTradeListNpc.size() ];
	}
	else
	{
		shop = &_CurrentTradeListYours[ itemNumber - _CurrentTradeListNpc.size() - _CurrentTradeListPlayer.size() ];
	}

	uint32 dapperPrice = 0;
	uint32 factionPrice = 0;

	/// check for enough money
	switch (shop->ItemTrade->getPriceInfo().getCurrency().getValue())
	{
	case RYMSG::TTradeCurrency::tc_dappers:
		{
			if( shop->ItemTrade->getOwner() == _Character->getId() )
			{
				log_Item_RemoveFromSaleStore(shop->ItemTrade->getItemPtr()->getItemId());
				dapperPrice = shop->ItemTrade->getRetirePrice();
			}
			else
			{
				if( shop->ShopUnit == 0 )
				{
					nlwarning("<CCharacterShoppingList::buyItem> ShopUnit ptr is NULL..., that must not occurs if item not come from yours list...");
					return;
				}
				dapperPrice = getSellPrice( shop->ItemTrade, shop->ShopUnit->getShopUnitType() == IShopUnit::DynamicShop );
			}

			// check if character have enough money
			if( ( dapperPrice * quantity ) > _Character->getMoney() )
			{
				// return player information 'not enough money'
				_Character->sendDynamicSystemMessage( _Character->getId(), "OPS_NOT_ENOUGHT_SEED" );
				return;
			}
		}
		break;
	case RYMSG::TTradeCurrency::tc_faction_points:
		{
			// check if character have enough faction points
			PVP_CLAN::TPVPClan clan = shop->ItemTrade->getPriceInfo().getFaction();
			if ((clan >= PVP_CLAN::BeginClans) && (clan <= PVP_CLAN::EndClans))
			{
				factionPrice = getSellPrice( shop->ItemTrade, shop->ShopUnit->getShopUnitType() == IShopUnit::DynamicShop );
				if (_Character->getFactionPoint(clan) < (factionPrice*quantity))
				{
					// return player information 'not enough faction point'
					_Character->sendDynamicSystemMessage( _Character->getId(), "OPS_NOT_ENOUGHT_FACTION_POINTS" );
					return;
				}
			}
		}
		break;
	default:
		STOP("Money "<<shop->ItemTrade->getPriceInfo().getCurrency().toString()<<" not supported yet !");
	}

	// check that item is available for the given quantity
	if (!shop->ItemTrade->isAvailable(quantity))
	{
		_Character->sendDynamicSystemMessage( _Character->getId() ,"ITEM_NOT_AVAILABLE" );
		return;
	}

	// Create the item for next checking
	CGameItemPtr item = NULL;
	if( shop->ShopUnit != 0 && shop->ShopUnit->getShopUnitType() == IShopUnit::StaticShop )
	{
		if( shop->ItemTrade->getItemPtr() == 0 )
		{
			CSheetId sheet = shop->ItemTrade->getSheetId();
			uint16 level = (uint16) shop->ItemTrade->getLevel();
			CEntityId parent = CEntityId::Unknown;
			sint16 slot = -1;
			item = GameItemManager.createItem( sheet, level, true, true );
		}
		else 
		{
			item = shop->ItemTrade->getItemPtr()->getItemCopy();
			nlassert( item != 0 );
			item->quality( (uint16) shop->ItemTrade->getLevel() );
		}
		nlassert( item != 0 );
	}
	else
	{
		item = shop->ItemTrade->getItemPtr()->getItemCopy();
		nlassert( item != 0 );
		item->setTotalSaleCycle( item->getTotalSaleCycle() + CTickEventHandler::getGameCycle() - shop->ItemTrade->getStartSaleCycle() );
		// TODO: clean this
		for (uint i = 0; i < quantity; i++)
		{
			CDynamicItems::getInstance()->removeDynamicItemForSell( shop->ItemTrade );
		}
	}

	const CStaticItem * form = CSheets::getForm( shop->ItemTrade->getSheetId() );
	if( form == 0 )
	{
		nlwarning("<CCharacterShoppingList::buyItem> Buy item %s with unknown static form", shop->ItemTrade->getSheetId().toString().c_str() );
		return;
	}

	const uint stackSize = min(item->getMaxStackSize(), quantity);
	item->setStackSize(stackSize);

	if ( form->Family == ITEMFAMILY::COSMETIC )
	{
		if ( form->Cosmetics == NULL )
		{
			nlwarning("<CCharacterShoppingList::buyItem> Buy item %s of type cosmetic with no cosmetic field", shop->ItemTrade->getSheetId().toString().c_str() );
			return;
		}
		if ( form->Type == ITEM_TYPE::HAIR_MALE || form->Type == ITEM_TYPE::HAIR_FEMALE )
		{
			uint32 value = CVisualSlotManager::getInstance()->sheet2Index( form->SheetId, SLOTTYPE::HEAD_SLOT );
			if ( ! _Character->setHair( value ) )
				return;
			else
			{
				_Character->resetHairCutDiscount();
			}
		}
		else if ( form->Type == ITEM_TYPE::TATOO_MALE || form->Type == ITEM_TYPE::TATOO_FEMALE )
		{
			if ( !_Character->setTatoo( form->Cosmetics->VPValue ) )
				return;
		}
		else if ( form->Type == ITEM_TYPE::HAIRCOLOR_MALE || form->Type == ITEM_TYPE::HAIRCOLOR_FEMALE )
		{
			if ( !_Character->setHairColor( form->Cosmetics->VPValue ) )
				return;
		}
		else
		{
			nlwarning("<CCharacterShoppingList::buyItem> Buy item %s of type cosmetic with invalid item type", shop->ItemTrade->getSheetId().toString().c_str() );
			return;
		}
	}
	else if (form->Family == ITEMFAMILY::SERVICE)
	{
		if (!CItemServiceManager::getInstance()->provideService(form, _Character))
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
			params[0].SheetId = form->SheetId;
			CCharacter::sendDynamicSystemMessage(_Character->getId(), "ITEM_SERVICE_NOT_AVAILABLE", params);
			return;
		}
	}
	else
	{
		if (!addBoughtItemToBag(item, dapperPrice, form))
		{
			if( shop->ShopUnit == 0 || shop->ShopUnit->getShopUnitType() == IShopUnit::DynamicShop )
			{
				shop->ItemTrade->addQuantity( quantity );
				CDynamicItems::getInstance()->addDynamicItemForSell( shop->ItemTrade );
			}
			return;
		}
	}

	// consume the money
	switch (shop->ItemTrade->getPriceInfo().getCurrency().getValue())
	{
	case RYMSG::TTradeCurrency::tc_dappers:
		// Spend dappers
		_Character->spendMoney( dapperPrice * stackSize );

		break;
	case RYMSG::TTradeCurrency::tc_faction_points:
		// Spend Faction Point
		{
			PVP_CLAN::TPVPClan clan = shop->ItemTrade->getPriceInfo().getFaction();
			uint32 nTotalFP = factionPrice * stackSize;
			uint32 nNbFP = _Character->getFactionPoint(clan);
			_Character->setFactionPoint(clan, nNbFP - nTotalFP);
		}

		break;
	default:
		STOP("Money "<<shop->ItemTrade->getPriceInfo().getCurrency().toString()<<" not supported");
	}


	CMissionEventBuyItem event( TheDataset.getDataSetRow( _Character->getCurrentInterlocutor() ), shop->ItemTrade->getSheetId(), (uint16)stackSize, (uint16)shop->ItemTrade->getLevel() );
	_Character->processMissionEvent(event);

	SM_STATIC_PARAMS_4(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
	params[0].SheetId = shop->ItemTrade->getSheetId();
	params[1].Int = stackSize;
	params[2].Int = dapperPrice * stackSize;
	if (shop->ItemTrade->getPriceInfo().getCurrency() == RYMSG::TTradeCurrency::tc_faction_points)
		params[3].Int = factionPrice * stackSize;
	else
		params[3].Int = 0;
	_Character->sendDynamicSystemMessage( _Character->getId(), "INVENTORY_BUY_ITEM", params);

	//output stats
	//Bsi.append( StatPath, NLMISC::toString("[AI] %s %s %d %d %d", _Character->getId().toString().c_str(), shop->ItemTrade->getSheetId().toString().c_str(), shop->ItemTrade->getQuality(), buyedQuantity, price * buyedQuantity) );
//	EgsStat.displayNL("[AI] %s %s %d %d %d", _Character->getId().toString().c_str(), shop->ItemTrade->getSheetId().toString().c_str(), shop->ItemTrade->getQuality(), stackSize, price * stackSize);
//	EGSPD::buyItem(_Character->getId(), shop->ItemTrade->getSheetId().toString(), shop->ItemTrade->getQuality(), stackSize, dapperPrice * stackSize);

	if( shop->ShopUnit == 0 || ( shop->ShopUnit != 0 && shop->ShopUnit->getShopUnitType() == IShopUnit::DynamicShop ) )
	{
		string command;
		CSoldItem::makeStringCommande( command, shop->ItemTrade->getOwner(), shop->ItemTrade->getSheetId(), stackSize, dapperPrice, shop->ItemTrade->getRetirePrice(), shop->ItemTrade->getIdentifier(), _Character->getId() );
		COfflineCharacterCommand::getInstance()->addOfflineCommand( command );
	}
} // buyItem //


//-----------------------------------------------------------------------------
bool CCharacterShoppingList::addBoughtItemToBag( CGameItemPtr item, uint32 price, const CStaticItem * form )
{
	// autostack the item in the bag
	if (_Character->addItemToInventory(INVENTORIES::bag, item))
	{
		if (form->Family == ITEMFAMILY::PET_ANIMAL_TICKET)
		{
			// pet animal ticket must not be stackable!
			nlassert(item != NULL);

			if (!_Character->addCharacterAnimal(item->getSheetId(), price, item))
			{
				item.deleteItem();
				return false;
			}
		}
		return true;
	}

	item.deleteItem();
	return false;
}

//-----------------------------------------------------------------------------
const CGameItemPtr CCharacterShoppingList::getItemIndexFromTradeList( uint32 idx )
{
	if( idx < _CurrentTradeListNpc.size() )
	{
		return _CurrentTradeListNpc[ idx ].ItemTrade->getItemPtr();
	}
	else if( idx < ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() ) )
	{
		return _CurrentTradeListPlayer[ idx - _CurrentTradeListNpc.size() ].ItemTrade->getItemPtr();
	}
	else if( idx < ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() + _CurrentTradeListYours.size() ) )
	{
		return _CurrentTradeListYours[ idx - _CurrentTradeListNpc.size() - _CurrentTradeListPlayer.size() ].ItemTrade->getItemPtr();
	}
	return CGameItemPtr();
}

//-----------------------------------------------------------------------------
bool CCharacterShoppingList::isItemSoldedByNpc( uint32 idx ) const
{
	if( idx < _CurrentTradeListNpc.size() )
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CCharacterShoppingList::getItemSheetAndQualityFromTradeList( uint32 idx, NLMISC::CSheetId& sheet, uint32& level ) const
{
	if( idx < _CurrentTradeListNpc.size() )
	{
		sheet = _CurrentTradeListNpc[ idx ].ItemTrade->getSheetId();
		level = _CurrentTradeListNpc[ idx ].ItemTrade->getLevel();
		if( _CurrentTradeListNpc[ idx ].ItemTrade->getItemPtr() != 0 )
		{
			_CurrentTradeListNpc[ idx ].ItemTrade->getItemPtr()->recommended( level );
		}
		return true;
	}
	else if( idx < ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() ) )
	{
		sheet = _CurrentTradeListPlayer[ idx - _CurrentTradeListNpc.size() ].ItemTrade->getSheetId();
		level = _CurrentTradeListPlayer[ idx - _CurrentTradeListNpc.size() ].ItemTrade->getLevel();
		return true;
	}
	else if( idx < ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() + _CurrentTradeListYours.size() ) )
	{
		sheet =  _CurrentTradeListYours[ idx - _CurrentTradeListNpc.size() - _CurrentTradeListPlayer.size() ].ItemTrade->getSheetId();
		level =  _CurrentTradeListYours[ idx - _CurrentTradeListNpc.size() - _CurrentTradeListPlayer.size() ].ItemTrade->getLevel();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CCharacterShoppingList::destroyItem( uint16 itemNumber, uint32 quantity )
{
	// check item index
	if( itemNumber >= ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() + _CurrentTradeListYours.size() ) )
	{
		nlwarning("<CCharacterShoppingList::buyItem> itemNumber not exist...");
		return;
	}
	
	TShopStruct * shop;
	if( itemNumber < _CurrentTradeListNpc.size() )
	{
		nlwarning("Client ask destroy item, but gived index corresponding to a NPC item");
		return;
	}
	else if( itemNumber < ( _CurrentTradeListNpc.size() + _CurrentTradeListPlayer.size() ) )
	{
		shop = &_CurrentTradeListPlayer[ itemNumber - _CurrentTradeListNpc.size() ];
	}
	else
	{
		shop = &_CurrentTradeListYours[ itemNumber - _CurrentTradeListNpc.size() - _CurrentTradeListPlayer.size() ];
	}
	
	if( shop->ItemTrade->getOwner() != _Character->getId() )
	{
		nlwarning("Client ask destroy item, but gived index are not a same owner item");
		return;
	}

	if( shop->ItemTrade->isAvailable(quantity) == false )
	{
		_Character->sendDynamicSystemMessage( _Character->getId() ,"ITEM_NOT_AVAILABLE" );
		return;
	}
	
	uint32 destroyQuantity = 0;
	
	static CSheetId sheetId("stack.sitem");

	for( uint i = 0; i < quantity; ++i )
	{
		CGameItemPtr item = NULL;
		CDynamicItems::getInstance()->removeDynamicItemForSell( shop->ItemTrade );
	}

	SM_STATIC_PARAMS_2(params, STRING_MANAGER::item, STRING_MANAGER::integer);
	params[0].SheetId = shop->ItemTrade->getSheetId();
	params[1].Int = quantity;
	_Character->sendDynamicSystemMessage( _Character->getId(), "INVENTORY_DESTROY_ITEM", params);

	//output stats
	//Bsi.append( StatPath, NLMISC::toString("[DI] %s %s %d %d", _Character->getId().toString().c_str(), shop->ItemTrade->getSheetId().toString().c_str(), shop->ItemTrade->getQuality(), quantity) );
	//EgsStat.displayNL("[DI] %s %s %d %d", _Character->getId().toString().c_str(), shop->ItemTrade->getSheetId().toString().c_str(), shop->ItemTrade->getQuality(), quantity);
//	EGSPD::destroyItem(_Character->getId(), shop->ItemTrade->getSheetId().toString(), shop->ItemTrade->getQuality(), quantity);

	_Character->removeItemFromShop( shop->ItemTrade->getIdentifier(), quantity );
//	PlayerManager.savePlayerActiveChar( PlayerManager.getPlayerId( _Character->getId() ) );
}
