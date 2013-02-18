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
#include "dbgroup_list_sheet_trade.h"
#include "interface_manager.h"
#include "inventory_manager.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../sheet_manager.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/view_text.h"
#include "nel/gui/action_handler.h"
#include "sphrase_manager.h"
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/pvp_clan.h"
#include "../string_manager_client.h"
#include "../entity_cl.h"
#include "nel/misc/common.h"


using namespace NLMISC;
using namespace std;

// ***************************************************************************
// CSheetChildTrade
// ***************************************************************************

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetTrade, std::string, "list_sheet_trade");

// ***************************************************************************
void CDBGroupListSheetTrade::CSheetChildTrade::init(CDBGroupListSheetText *pFather, uint index)
{
	// init my parent
	CSheetChild::init(pFather, index);

	CDBGroupListSheetTrade *zeFather = safe_cast<CDBGroupListSheetTrade*>(pFather);
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// **** Bind the quality
	{
		// Basic quality
		string	db= Ctrl->getSheet()+":QUALITY";
		if( NLGUI::CDBManager::getInstance()->getDbProp(db, false) )
			CurrentQuality.link ( db.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentQuality.link("UI:DUMMY:QUALITY");
			CurrentQuality.setSInt32(0);
		}
	}

	// **** Bind the quantity
	{
		// Basic quantity
		string	db= Ctrl->getSheet()+":QUANTITY";
		if( NLGUI::CDBManager::getInstance()->getDbProp(db, false) )
			CurrentQuantity.link ( db.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentQuantity.link("UI:DUMMY:QUANTITY");
			CurrentQuantity.setSInt32(0);
		}
	}

	// **** Bind the price
	if (zeFather->priceWanted())
	{
		// Basic price
		string	priceDB= Ctrl->getSheet()+":PRICE";
		if( NLGUI::CDBManager::getInstance()->getDbProp(priceDB, false) )
			CurrentPrice.link ( priceDB.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentPrice.link("UI:DUMMY:PRICE");
			CurrentPrice.setSInt32(0);
		}

		// Faction Type
		string	factionTypeDB= Ctrl->getSheet()+":FACTION_TYPE";
		if( NLGUI::CDBManager::getInstance()->getDbProp(factionTypeDB, false) )
			CurrentFactionType.link ( factionTypeDB.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentFactionType.link("UI:DUMMY:FACTION_TYPE");
			CurrentFactionType.setSInt32(0);
		}

		// Faction Point Price
		string	factionPointPriceDB= Ctrl->getSheet()+":PRICE";
		if( NLGUI::CDBManager::getInstance()->getDbProp(factionPointPriceDB, false) )
			CurrentFactionPointPrice.link ( factionPointPriceDB.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentFactionPointPrice.link("UI:DUMMY:FACTION_TYPE");
			CurrentFactionPointPrice.setSInt32(0);
		}
	}

	// **** Bind the seller type / retire price
	if (zeFather->sellerTypeWanted())
	{
		// seller type
		string	db= Ctrl->getSheet()+":SELLER_TYPE";
		if( NLGUI::CDBManager::getInstance()->getDbProp(db, false) )
			CurrentSellerType.link ( db.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentSellerType.link("UI:DUMMY:SELLER_TYPE");
			CurrentSellerType.setSInt32(0);
		}

		// Retire price (only valid if sellerType is User or ResaleAndUser)
		db= Ctrl->getSheet()+":PRICE_RETIRE";
		if( NLGUI::CDBManager::getInstance()->getDbProp(db, false) )
			CurrentPriceRetire.link ( db.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentPriceRetire.link("UI:DUMMY:PRICE_RETIRE");
			CurrentPriceRetire.setSInt32(0);
		}

		// Resale Time Left (only valid if sellerType is User or ResaleAndUser)
		db= Ctrl->getSheet()+":RESALE_TIME_LEFT";
		if( NLGUI::CDBManager::getInstance()->getDbProp(db, false) )
			CurrentResaleTimeLeft.link ( db.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentResaleTimeLeft.link("UI:DUMMY:RESALE_TIME_LEFT");
			CurrentResaleTimeLeft.setSInt32(0);
		}

		// VendorNameId
		db= Ctrl->getSheet()+":VENDOR_NAMEID";
		if( NLGUI::CDBManager::getInstance()->getDbProp(db, false) )
			CurrentVendorNameId.link ( db.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentVendorNameId.link("UI:DUMMY:VENDOR_NAMEID");
			CurrentVendorNameId.setSInt32(0);
		}
	}

}

// ***************************************************************************
bool CDBGroupListSheetTrade::CSheetChildTrade::isInvalidated(CDBGroupListSheetText *pFather)
{
	CDBGroupListSheetTrade *zeFather = safe_cast<CDBGroupListSheetTrade*>(pFather);

	// quality change
	if( CurrentQuality.getSInt32() != LastQuality )
		return true;

	// quantity change
	if( CurrentQuantity.getSInt32() != LastQuantity )
		return true;

	// if wantPrice
	if( zeFather->priceWanted() )
	{
		// if some price changed, must update too
		if( CurrentPrice.getSInt32() != LastPrice )
			return true;
		if( CurrentFactionType.getSInt32() != LastFactionType )
			return true;
		if( CurrentFactionPointPrice.getSInt32() != LastFactionPointPrice )
			return true;
	}

	// if reseller filter
	if (zeFather->sellerTypeWanted())
	{
		if( CurrentSellerType.getSInt32() != LastSellerType )
			return true;
		if( CurrentPriceRetire.getSInt32() != LastPriceRetire )
			return true;
		if( CurrentResaleTimeLeft.getSInt32() != LastResaleTimeLeft )
			return true;
		if( (uint32)CurrentVendorNameId.getSInt32() != LastVendorNameId )
			return true;
	}

	return false;
}

// ***************************************************************************
void CDBGroupListSheetTrade::CSheetChildTrade::update(CDBGroupListSheetText *pFather)
{
	H_AUTO(CDBGroupListSheetTrade_update);
	CDBGroupListSheetTrade *zeFather = safe_cast<CDBGroupListSheetTrade*>(pFather);

	LastQuality= CurrentQuality.getSInt32();
	LastQuantity= CurrentQuantity.getSInt32();

	if (zeFather->priceWanted())
	{
		LastPrice = CurrentPrice.getSInt32();
		LastFactionType = CurrentFactionType.getSInt32();
		LastFactionPointPrice = CurrentFactionPointPrice.getSInt32();
	}

	if (zeFather->sellerTypeWanted())
	{
		LastSellerType = CurrentSellerType.getSInt32();
		LastPriceRetire = CurrentPriceRetire.getSInt32();
		LastResaleTimeLeft = CurrentResaleTimeLeft.getSInt32();
		if( LastVendorNameId != (uint32)CurrentVendorNameId.getSInt32() )
		{
			VendorNameString.erase();
			LastVendorNameId = CurrentVendorNameId.getSInt32();
			if( LastVendorNameId == 0 )
				zeFather->VendorNameIdToUpdate.erase( this );
			else
				zeFather->VendorNameIdToUpdate.insert( this );
		}
	}
}

// ***************************************************************************
void CDBGroupListSheetTrade::CSheetChildTrade::updateViewText(CDBGroupListSheetText *pFather)
{
	H_AUTO(CDBGroupListSheetTrade_updateViewText);
	ucstring text;
	Ctrl->getContextHelp(text);
	// Append first the type of the sheet to select
	switch ( Ctrl->getSheetCategory() )
	{
		case CDBCtrlSheet::Item:		break;		// none for item. consider useless
		case CDBCtrlSheet::Pact:		text= CI18N::get("uiBotChatPact") + text; break;
		case CDBCtrlSheet::Skill:		text= CI18N::get("uiBotChatSkill") + text; break;
		case CDBCtrlSheet::GuildFlag:	text= CI18N::get("uiBotChatSkill") + text; break;
		case CDBCtrlSheet::Phrase:		text= CI18N::get("uiBotChatPhrase") + text; break;
		default: break;
	}

	// For outpost building get description from string manager
	if(Ctrl->getType() == CCtrlSheetInfo::SheetType_OutpostBuilding)
	{
		const COutpostBuildingSheet *pOBS = Ctrl->asOutpostBuildingSheet();
		if (pOBS != NULL)
		{
			STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
			text += string("\n") + pSMC->getOutpostBuildingLocalizedDescription(CSheetId(Ctrl->getSheetId()));
			text += "\n" + CI18N::get("uiBotChatPrice") + NLMISC::formatThousands(toString(pOBS->CostDapper));
			text += CI18N::get("uiBotChatTime") + toString(pOBS->CostTime/60) + CI18N::get("uiBotChatTimeMinute");
			if ((pOBS->CostTime % 60) != 0)
				text += toString(pOBS->CostTime%60) + CI18N::get("uiBotChatTimeSecond");
		}
	}

	// For Phrase, if combat, add Weapon compatibility
	if(Ctrl->getSheetCategory() == CDBCtrlSheet::Phrase)
	{
		// For combat action, Append weapon restriction
		ucstring	weaponRestriction;
		CSPhraseManager	*pPM= CSPhraseManager::getInstance();
		bool melee,range;
		pPM->getCombatWeaponRestriction(weaponRestriction, Ctrl->getSheetId(),melee,range);
		// don't add also if no combat restriction
		if(!weaponRestriction.empty() && weaponRestriction!=CI18N::get("uiawrSF"))
		{
			weaponRestriction= CI18N::get("uiPhraseWRHeader") + weaponRestriction;
			text+= "\n" + weaponRestriction;
		}
	}

	// Get the Text color
	ucstring colorTag("@{FFFF}");
	if(Ctrl->getType() == CCtrlSheetInfo::SheetType_Item)
	{
		if(!Ctrl->checkItemRequirement())
			colorTag= CI18N::get("uiItemCannotUseColor");
	}

	// For item, add some information
	if(Ctrl->getType() == CCtrlSheetInfo::SheetType_Item)
	{
		const CItemSheet *pIS = Ctrl->asItemSheet();
		if(pIS)
		{
			// Add craft info for MP
			if(pIS->Family==ITEMFAMILY::RAW_MATERIAL)
			{
				ucstring	ipList;
				pIS->getItemPartListAsText(ipList);
				if(ipList.empty())
				{
					if(pIS->isUsedAsCraftRequirement())
						text+= "\n" + CI18N::get("uiItemMpCraftRequirement");
					else
						text+= "\n" + CI18N::get("uiItemMpNoCraft");
				}
				else
					text+= "\n" + CI18N::get("uiItemMpCanCraft") + ipList;
			}
		}
	}

	// get the price
	CDBGroupListSheetTrade *zeFather = safe_cast<CDBGroupListSheetTrade*>(pFather);
	if (zeFather->priceWanted())
	{
		if(Ctrl->getSheetCategory() == CDBCtrlSheet::Phrase)
		{
			if (LastPrice != -1)
				text+= "\n" + CI18N::get("uiBotChatSkillPointCost") + toString(LastPrice);
			else
				text+= "\n" + CI18N::get("uiBotChatSkillPointCostNotReceived");
		}
		else
		{
			bool	guildOption= false;

			// Special case if its a guild option
			if (Ctrl->getType() == CCtrlSheetInfo::SheetType_Item)
			{
				const CItemSheet *pIS = Ctrl->asItemSheet();
				if (pIS && pIS->Family == ITEMFAMILY::GUILD_OPTION)
				{
					text+= "\n" + CI18N::get("uiBotChatSkillPointCost") + toString(pIS->GuildOption.XPCost);
					text+= "\n" + CI18N::get("uiBotChatPrice") + NLMISC::formatThousands(toString(pIS->GuildOption.MoneyCost));
					guildOption= true;
				}
			}

			// if not a guild option
			if(!guildOption)
			{
				if (LastPrice != -1)
				{
					const CItemSheet *pIS = Ctrl->asItemSheet();

					uint	factor= 1;
					if(zeFather->getMultiplyPriceByQuantityFlag())
						factor= LastQuantity;

					// factor on price
					float priceFactor = 1.0f;

					// Append std price
					bool	displayMulPrice= pIS && pIS->Stackable>1 && zeFather->getMultiplyPriceByQuantityFlag();
					// If seller type wanted, don't mul price if the item comes from a NPC
					if(zeFather->sellerTypeWanted() && LastSellerType==BOTCHATTYPE::NPC)
						displayMulPrice= false;

					// check if we should apply fame factor
					if( zeFather->applyFamePriceFactor() )
					{
						priceFactor = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:FAME_PRICE_FACTOR")->getValue16()/10000.0f;
					}

					// display with correct format
					if (LastPrice > 0)
					{
						if(displayMulPrice)
							text+= "\n" + CI18N::get("uiBotChatPrice") + NLMISC::formatThousands(toString(sint32(LastPrice * priceFactor))) + " ("
								+ NLMISC::formatThousands(toString( sint32(factor) * sint32(LastPrice * priceFactor) )) + ")";
						else
							text+= "\n" + CI18N::get("uiBotChatPrice") + NLMISC::formatThousands(toString( sint32(factor * LastPrice * priceFactor) ));
					}

					if ((LastFactionPointPrice != 0) && (LastFactionType >= PVP_CLAN::BeginClans) && (LastFactionType <= PVP_CLAN::EndClans))
					{
						if (LastPrice > 0)
							text += ". ";
						else
							text+= "\n";

						text+= CI18N::get("uiBotChatFactionType") + PVP_CLAN::toString((PVP_CLAN::TPVPClan)LastFactionType)
							+  CI18N::get("uiBotChatFactionPointPrice") + NLMISC::formatThousands(toString(LastFactionPointPrice));
					}

					// some additional info for resale
					if(zeFather->sellerTypeWanted())
					{
						// Append retire price if reseller type OK (only if the item belongs to the player)
						if(LastSellerType == BOTCHATTYPE::User || LastSellerType == BOTCHATTYPE::ResaleAndUser ||
							LastSellerType == BOTCHATTYPE::UserRetirable || LastSellerType == BOTCHATTYPE::ResaleAndUserRetirable )
						{
							// append price
							if(pIS && pIS->Stackable>1 && zeFather->getMultiplyPriceByQuantityFlag())
								text+= CI18N::get("uiBotChatRetirePrice") + NLMISC::formatThousands(toString(LastPriceRetire)) + " ("
									+ NLMISC::formatThousands(toString(factor * LastPriceRetire)) + ")";
							else
								text+= CI18N::get("uiBotChatRetirePrice") + NLMISC::formatThousands(toString(factor * LastPriceRetire));
							// set resale time left
							ucstring	fmt= CI18N::get("uiBotChatResaleTimeLeft");
							strFindReplace(fmt, "%d", toString(LastResaleTimeLeft/RYZOM_DAY_IN_HOUR));
							strFindReplace(fmt, "%h", toString(LastResaleTimeLeft%RYZOM_DAY_IN_HOUR));
							text+= "\n" + fmt;
							// force special color (according if retirable or not)
							if(LastSellerType == BOTCHATTYPE::UserRetirable || LastSellerType == BOTCHATTYPE::ResaleAndUserRetirable)
								colorTag= CI18N::get("uiItemUserSellColor");
							else
								colorTag= CI18N::get("uiItemUserSellColorNotRetirable");
						}

						// Append (NPC) tag if NPC item
						if(LastSellerType == BOTCHATTYPE::NPC)
						{
							text+= "\n" + CI18N::get("uiBotChatNPCTag");
						}
						// else display the name of the vendor (not if this is the player himself, to avoid flood)
						else if (LastSellerType == BOTCHATTYPE::Resale)
						{
							text+= "\n" + CI18N::get("uiBotChatVendorTag") + VendorNameString;
						}
					}
				}
				else
				{
					// If comes from User List, -1 means "Already Sold"
					if(zeFather->sellerTypeWanted() &&
						(LastSellerType == BOTCHATTYPE::User || LastSellerType == BOTCHATTYPE::UserRetirable) )
					{
						text+= "\n" + CI18N::get("uiItemSold");
						// force special color
						colorTag= CI18N::get("uiItemSoldColor");
					}
					// error case
					else
					{
						text+= "\n" + CI18N::get("uiPriceNotReceived");
					}
				}
			}
		}
	}

	// setup color and text
	text= colorTag + text;
	Text->setTextFormatTaged(text);
}

// ***************************************************************************
bool CDBGroupListSheetTrade::CSheetChildTrade::isSheetValid(CDBGroupListSheetText *pFather)
{
	H_AUTO(CDBGroupListSheetTrade_isSheetValid);
	if (!CSheetChild::isSheetValid(pFather))
		return false;

	// Check if the item is saleable by the player
	CDBGroupListSheetTrade	*father= safe_cast<CDBGroupListSheetTrade*>(pFather);
	if(father->testDropOrSell())
	{
		const CItemSheet *pIS = Ctrl->asItemSheet();
		if ((pIS != NULL) && (!pIS->DropOrSell))
			return false;
		// test if this whole family of items can be sold
		if( !ITEMFAMILY::isSellableByPlayer(pIS->Family) )
			return false;
	}

	// Check if the player do not wear this item
	if (Ctrl->getSecondIndexInDB()==INVENTORIES::bag)
	{
		CInventoryManager *pInv = CInventoryManager::getInstance();
		if (pInv->isBagItemWeared(Ctrl->getIndexInDB()))
			return false;
	}

	// Locked by owner; cannot trade
	if (Ctrl->getLockedByOwner())
	{
		return false;
	}

	// Check seller type?
	TSellerTypeFilter		stf= father->getSellerTypeFilter();
	if( stf != None)
	{
		// If NPC filter, display only NPC SELLER_TYPE
		if( stf==NPC && LastSellerType!=BOTCHATTYPE::NPC)
			return false;
		// If Resale filter, display both Resale and ResaleAndUser SELLER_TYPE
		if( stf==Resale && LastSellerType!=BOTCHATTYPE::Resale &&
			LastSellerType!=BOTCHATTYPE::ResaleAndUser &&
			LastSellerType!=BOTCHATTYPE::ResaleAndUserRetirable
			)
			return false;
		// If User filter, display only User SELLER_TYPE
		if( stf==User && LastSellerType!=BOTCHATTYPE::User &&
			LastSellerType!=BOTCHATTYPE::UserRetirable
			)
			return false;
		// If NPCAndResale filter, display both NPC, Resale and ResaleAndUser SELLER_TYPE
		if( stf==NPCAndResale &&
			LastSellerType!=BOTCHATTYPE::Resale &&
			LastSellerType!=BOTCHATTYPE::ResaleAndUser &&
			LastSellerType!=BOTCHATTYPE::ResaleAndUserRetirable &&
			LastSellerType!=BOTCHATTYPE::NPC
			)
			return false;
	}

	const CSPhraseSheet	*phraseSheet = Ctrl->asSPhraseSheet();

	/*
	if (phraseSheet)
	{
		nlwarning("Phrase sheet");
		nlwarning("============");
		for (uint k = 0; k < phraseSheet->Bricks.size(); ++k)
		{
			CSBrickSheet *bs = dynamic_cast<CSBrickSheet *>(SheetMngr.get(phraseSheet->Bricks[k]));
			if (bs)
			{
				nlwarning("Brick %d has type %s", (int) k, BRICK_TYPE::toString(bs->getBrickType()).c_str());
			}
			else
			{	
				nlwarning("Brick %d : not a brick sheet", (int) k);
			}
		}
	}
	*/	

	if (phraseSheet)
	{
		switch(father->getSellerTypeFilter())
		{
			case TrainerAction:
				return (!isUpgradePhrase(*phraseSheet)) && (!isPowerPhrase(*phraseSheet)) && (!isCaracPhrase(*phraseSheet));
			break;
			case TrainerUpgrade:
				return isUpgradePhrase(*phraseSheet);
			break;
			case TrainerPower:
				return isPowerPhrase(*phraseSheet);
			break;
			case TrainerCarac:
				return isCaracPhrase(*phraseSheet);
			break;
			default:
			break;
		}
	}
		


	return true;
}

// ***************************************************************************
bool CDBGroupListSheetTrade::isUpgradePhrase(const CSPhraseSheet &phrase)
{
	// An upgrade phrase has no root brick in it
	for (uint k = 0; k < phrase.Bricks.size(); ++k)
	{
		CSBrickSheet *bs = dynamic_cast<CSBrickSheet *>(SheetMngr.get(phrase.Bricks[k]));
		if (bs && bs->isRoot()) return false;
	}
	return !isCaracPhrase(phrase);
}

// ***************************************************************************
bool CDBGroupListSheetTrade::isPowerPhrase(const CSPhraseSheet &phrase)
{
	for (uint k = 0; k < phrase.Bricks.size(); ++k)
	{
		CSBrickSheet *bs = dynamic_cast<CSBrickSheet *>(SheetMngr.get(phrase.Bricks[k]));
		if (bs && bs->getBrickType() != BRICK_TYPE::SPECIAL_POWER) return false;
	}
	return true;
}

// ***************************************************************************
bool CDBGroupListSheetTrade::isCaracPhrase(const CSPhraseSheet &phrase)
{
	for (uint k = 0; k < phrase.Bricks.size(); ++k)
	{
		CSBrickSheet *bs = dynamic_cast<CSBrickSheet *>(SheetMngr.get(phrase.Bricks[k]));
		if (bs && bs->getBrickType() == BRICK_TYPE::TRAINING) return true;
	}
	return false;
}


// ***************************************************************************
// CDBGroupListSheetTrade
// ***************************************************************************

// ***************************************************************************
CDBGroupListSheetTrade::CDBGroupListSheetTrade(const TCtorParam &param)
:	CDBGroupListSheetText(param)
{
	_WantPrice= false;
	_MultiplyPriceByQuantity= false;
	_TestDropOrSell= false;
	_SellerTypeFilter= None;
	_ApplyFamePriceFactor = false;

	// **** keep link to fame price factor leaf
	_FamePriceFactorLeaf = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:FAME_PRICE_FACTOR");
	_LastFamePriceFactor = _FamePriceFactorLeaf->getValue16();
}
// ***************************************************************************
CDBGroupListSheetTrade::~CDBGroupListSheetTrade()
{
}

// ***************************************************************************
bool CDBGroupListSheetTrade::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CDBGroupListSheetText::parse(cur, parentGroup))
		return false;

	// read params
	CXMLAutoPtr prop;

	// price?
	prop= (char*) xmlGetProp( cur, (xmlChar*)"want_price" );
	if (prop)	_WantPrice = convertBool (prop);
	// multiply price by quantity ?
	prop= (char*) xmlGetProp( cur, (xmlChar*)"multiply_price_by_quantity" );
	if (prop)	_MultiplyPriceByQuantity = convertBool (prop);
	// _TestDropOrSell ?
	prop= (char*) xmlGetProp( cur, (xmlChar*)"test_drop_or_sell" );
	if (prop)	_TestDropOrSell = convertBool (prop);
	// apply fame price factor or not
	prop= (char*) xmlGetProp( cur, (xmlChar*)"apply_fame_price_factor" );
	if (prop)	_ApplyFamePriceFactor = convertBool (prop);
	// filter_seller_type
	_SellerTypeFilter= None;
	prop= (char*) xmlGetProp( cur, (xmlChar*)"filter_seller_type" );
	if(prop)
	{
		string	lwrFilter= toLower(std::string((const char *)prop));
		if(lwrFilter=="npc")
			_SellerTypeFilter= NPC;
		else if(lwrFilter=="resale")
			_SellerTypeFilter= Resale;
		else if(lwrFilter=="user")
			_SellerTypeFilter= User;
		else if(lwrFilter=="npc_and_resale")
			_SellerTypeFilter= NPCAndResale;
		else if(lwrFilter=="action")
			_SellerTypeFilter= TrainerAction;
		else if(lwrFilter=="upgrade")
			_SellerTypeFilter= TrainerUpgrade;
		else if(lwrFilter=="power")
			_SellerTypeFilter= TrainerPower;
		else if(lwrFilter=="carac")
			_SellerTypeFilter= TrainerCarac;
	}
	return true;
}

// ***************************************************************************
void CDBGroupListSheetTrade::checkCoords ()
{
	CDBGroupListSheetText::checkCoords();

	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();

	bool	mustUpdateCoords= false;

	// test each child to update each frame, if need to update text id
	for( set< CSheetChildTrade *>::iterator it = VendorNameIdToUpdate.begin(); it != VendorNameIdToUpdate.end(); )
	{
		CSheetChildTrade * cst = (*it);
		// String result
		ucstring result;
		if( pSMC->getString ( cst->LastVendorNameId, result) )
		{
			cst->VendorNameString = CEntityCL::removeShardFromName(result);
			set< CSheetChildTrade *>::iterator itTmp = it;
			++it;
			VendorNameIdToUpdate.erase(itTmp);
			cst->NeedUpdateText= true;
			mustUpdateCoords= true;
		}
		else
		{
			++it;
		}
	}

	// invalidate coords if one of the child need it
	if(mustUpdateCoords)
		invalidateCoords();
}

void CDBGroupListSheetTrade::sort()
{
	vector<SSortStruct> vTemp;

	vTemp.resize (_SheetChildren.size());

	uint i;
	for (i = 0; i < _SheetChildren.size(); ++i)
	{
		vTemp[i].SheetText = _SheetChildren[i];

		CDBCtrlSheet	*ctrl= _SheetChildren[i]->Ctrl;
		initStructForItemSort (vTemp, ctrl->getSheetId(), ctrl->getQuality(), i, ctrl->getIndexInDB());
	}

	std::sort(vTemp.begin(), vTemp.end());

	for (i = 0; i < _SheetChildren.size(); ++i)
	{
		_SheetChildren[i] = vTemp[i].SheetText;
	}
}


bool CDBGroupListSheetTrade::needCheckAllItems()
{
	if (_LastFamePriceFactor != _FamePriceFactorLeaf->getValue16())
	{
		_LastFamePriceFactor = _FamePriceFactorLeaf->getValue16();
		return true;
	}

	return CDBGroupListSheetText::needCheckAllItems();
}
