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

#include "game_share/trade_slot_type.h"
#include "game_share/skills.h"
#include "game_share/bot_chat_types.h"
#include "game_share/msg_ais_egs_gen.h"
#include "../client_sheets/item_sheet.h"
#include "../client_sheets/outpost_building_sheet.h"
//
#include "bot_chat_page_trade.h"
#include "interface_manager.h"
#include "nel/gui/interface_group.h"
#include "inventory_manager.h"
#include "../net_manager.h"
#include "nel/gui/action_handler.h"
#include "bot_chat_page_all.h"
#include "bot_chat_manager.h"
#include "dbctrl_sheet.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_tab.h"
#include "nel/gui/group_container.h"
#include "action_handler_help.h"
#include "../string_manager_client.h"
#include "nel/gui/group_container.h"
#include "dbgroup_list_sheet_text.h"
#include "nel/gui/ctrl_text_button.h"
#include "../client_cfg.h"
#include "../init_main_loop.h"
#include "guild_manager.h"
#include "../sheet_manager.h"
#include "../user_entity.h"
#include "nel/gui/view_bitmap.h"
#include "nel/misc/common.h"

using namespace std::rel_ops;


using namespace std;
using namespace NLMISC;

static const char *WIN_BOT_CHAT_PAGE_TRADE = "ui:interface:bot_chat_trade";
static const char *WIN_BOT_CHAT_PAGE_BUY   = "ui:interface:bot_chat_buy";
static const char *WIN_BOT_CHAT_SELL_BUY_ITEM = "ui:interface:bot_chat_buy_sell_item";
static const char *WIN_BOT_CHAT_CHANGE_BUY_FILTER = "ui:interface:bot_chat_change_buy_filter";
static const char *WIN_BOT_CHAT_CHANGE_BUY_FILTER_MP = "ui:interface:bot_chat_change_buy_filter_mp";
static const char *WIN_BOT_CHAT_CHANGE_BUY_FILTER_CLASS = "ui:interface:bot_chat_change_buy_filter_class";
static const char *WIN_BOT_CHAT_CHANGE_BUY_FILTER_ITEM_TYPE = "ui:interface:bot_chat_change_buy_filter_item_type";
static const char *WIN_BOT_CHAT_DESTROY_ITEM = "ui:interface:bot_chat_destroy_confirm_modal";


#define DB_BOT_CHAT_BASE_BUY_FILTER "UI:SAVE:BOTCHAT_FILTER"


// fill the help setup infos for the trade modal
static void fillHelpSetupInfosForTrade(CSheetHelpSetup &dest, CDBCtrlSheet *sheet, CInterfaceGroup *helpGroup)
{
	dest.HelpWindow = helpGroup;
	dest.SrcSheet = sheet;
	dest.DestSheet = dynamic_cast<CDBCtrlSheet *>(helpGroup->getCtrl("buy_sell_slot"));
	dest.ViewText = "help_text";
	dest.ScrollTextGroup = "scroll_text";
	dest.PrefixForExtra= ":header_opened:scroll_text:text_list:";
	dest.FromBotChat= true;

	// hide some interface elements by default
	resetSheetHelp(dest);
}


/** Listener to update items infos in the buy / sell dlg
  */
class	CInfoWindow : public IItemInfoWaiter
{
public:
	// the sheet for which help must be updated
	CDBCtrlSheet *Sheet;
public:
	virtual void	infoReceived()
	{
		CSheetHelpSetup helpSetup;
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupContainer *ig = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
		fillHelpSetupInfosForTrade(helpSetup, Sheet, ig);
		refreshItemHelp(helpSetup);
	}
};
static CInfoWindow InfoWindowUpdater;


// ***************************************************************************
void		CBotChatPageTrade::CItemCheck::reset()
{
	_SheetId= 0;
	_Quality= 0;
	_Quantity= 0;
	_Price= 0;
	_ResaleFlag= 0;
}
void		CBotChatPageTrade::CItemCheck::init(CDBCtrlSheet *ctrl)
{
	nlassert(ctrl);
	_SheetId= ctrl->getSheetId();
	_Quality= ctrl->getQuality();
	_Quantity= ctrl->getQuantity();
	_Price= ctrl->getItemPrice();
	_ResaleFlag= ctrl->getItemResaleFlag();
}
bool		CBotChatPageTrade::CItemCheck::test(CDBCtrlSheet *ctrl) const
{
	nlassert(ctrl);
	return
		_SheetId== ctrl->getSheetId() &&
		_Quality== ctrl->getQuality() &&
		_Quantity== ctrl->getQuantity() &&
		_Price== ctrl->getItemPrice() &&
		_ResaleFlag== ctrl->getItemResaleFlag();
}


// *******************************************************************************************
CBotChatPageTrade::CBotChatPageTrade()
{
	_TradePagesObs.setListType(CHugeListObs::Trading);
	_BuyDlgOn = false;
	_SellDlgOn = false;
	_ResaleEdit = false;
	_QuantityEdit = false;
	_UsePriceRetire = false;
	_CannotValidateBecauseRetireNotAvailable = false;
	_QuantityCheck = 0;
	_CurrItemIndex = 0;
	_BuyOnly = false;
	_CurrItemSheet = NULL;
	_BuyMean = Money;
	_FamePriceFactorLeaf = NULL;
	_FilterBuyDlgMaxValue= 0;
	_DownloadComplete = false;
}

// *******************************************************************************************
void CBotChatPageTrade::init()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	if (NLGUI::CDBManager::getInstance()->getDbBranch("SERVER:TRADING"))
	{
		NLGUI::CDBManager::getInstance()->addBranchObserver( "SERVER:TRADING", &_TradePagesObs);
	}

	_FamePriceFactorLeaf = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:FAME_PRICE_FACTOR");
}

// *******************************************************************************************
void	CBotChatPageTrade::invalidateCoords()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(_BuyOnly ? WIN_BOT_CHAT_PAGE_BUY : WIN_BOT_CHAT_PAGE_TRADE));
	if (!gc) return;
	// invalidate buy group
	gc->invalidateCoords();
}

// *******************************************************************************************
void CBotChatPageTrade::begin()
{
	CBotChatPage::begin();
	if (!_BuyOnly)
	{
		activateWindow(WIN_BOT_CHAT_PAGE_TRADE, true);
	}
	else
	{
		activateWindow(WIN_BOT_CHAT_PAGE_BUY, true);
	}
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp(BOT_CHAT_BASE_DB_PATH ":TRADE")->setValue32(0);

	// at each new bot chat trade, we must reset filter for ItemPart and ItemType (NB: server should do the same)
	resetItemPartAndTypeFilters();

	// reset also the filters at each open if user wants to
	CCDBNodeLeaf	*dbResetFilterOnOpen= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":RESET_ON_OPEN" , false);
	if(dbResetFilterOnOpen && dbResetFilterOnOpen->getValueBool() && (_BuyMean == Money) )
	{
		// temporary value for conversions
		sint32 value;

		// Reset Price
		CCDBNodeLeaf	*dbPriceMin= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_PRICE") , false);
		CCDBNodeLeaf	*dbPriceMax= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_PRICE") , false);
		if(dbPriceMin)	dbPriceMin->setValue32(0);
		if(dbPriceMax)
		{
			fromString(CWidgetManager::getInstance()->getParser()->getDefine("bot_chat_filter_max_price"), value);
			dbPriceMax->setValue32(value);
		}

		// Reset Quality
		CCDBNodeLeaf	*dbQualityMin= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_QUALITY") , false);
		CCDBNodeLeaf	*dbQualityMax= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_QUALITY") , false);
		if(dbQualityMin)	dbQualityMin->setValue32(0);
		if(dbQualityMax)
		{
			fromString(CWidgetManager::getInstance()->getParser()->getDefine("bot_chat_filter_max_quality"), value);
			dbQualityMax->setValue32(value);
		}

		// Reset Class
		CCDBNodeLeaf	*dbClassMin= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_CLASS" , false);
		CCDBNodeLeaf	*dbClassMax= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_CLASS" , false);
		if(dbClassMin) dbClassMin->setValue32(0);
		if(dbClassMax) dbClassMax->setValue32(RM_CLASS_TYPE::NumTRMClassType-1);

		// send the filter, but don't reset the list, since will do just after :)
		sendCurrentBuyFilterToServer(false);
	}


	// Start the HugeList
	_TradePagesObs.start();
	// If BuyMean is skillpoints, means its phrase selling => allow phrase price update (and possible client fill)
	_TradePagesObs.allowPhrasePriceUpdateAndClientFill(_BuyMean == SkillPoints);
	notifyDownloadComplete(false);
	_DownloadComplete = false;

	// update interface
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(_BuyOnly ? WIN_BOT_CHAT_PAGE_BUY : WIN_BOT_CHAT_PAGE_TRADE));
	if (gc)
	{
		// set the title
		gc->setUCTitle(_Title);
		// show the buy mean
		CInterfaceGroup *money = dynamic_cast<CInterfaceGroup *>(gc->getGroup("money"));
		if (money) money->setActive((_BuyMean == Money) || (_BuyMean == MoneyFactionPoints));
		CInterfaceGroup *factionPoints = dynamic_cast<CInterfaceGroup *>(gc->getGroup("fp"));
		if (factionPoints) factionPoints->setActive(_BuyMean == MoneyFactionPoints);
		CInterfaceGroup *skillPoints = dynamic_cast<CInterfaceGroup *>(gc->getGroup("sp"));
		if (skillPoints) skillPoints->setActive(_BuyMean == SkillPoints);
		CInterfaceGroup *pMoneyGuildXP = dynamic_cast<CInterfaceGroup *>(gc->getGroup("money_guild_xp"));
		if (pMoneyGuildXP) pMoneyGuildXP->setActive((_BuyMean == MoneyGuildXP)||(_BuyMean == GuildXP));
		CInterfaceGroup *pMoney = dynamic_cast<CInterfaceGroup *>(gc->getGroup("money_guild_xp:money"));
		if (pMoney) pMoney->setActive(_BuyMean == MoneyGuildXP);

		CInterfaceGroup *pGuildMoneyGuildXP = dynamic_cast<CInterfaceGroup *>(gc->getGroup("guild_money_guild_xp"));
		if (pGuildMoneyGuildXP) pGuildMoneyGuildXP->setActive((_BuyMean == GuildMoneyGuildXP) || (_BuyMean == GuildMoney));
		CViewText *pGuildXPText = dynamic_cast<CViewText*>(gc->getView("guild_money_guild_xp:xp_text"));
		if (pGuildXPText) pGuildXPText->setActive(_BuyMean == GuildMoneyGuildXP);
		CViewText *pGuildXPValue = dynamic_cast<CViewText*>(gc->getView("guild_money_guild_xp:xp_value"));
		if (pGuildXPValue) pGuildXPValue->setActive(_BuyMean == GuildMoneyGuildXP);

		CViewText *pBuyTitle = dynamic_cast<CViewText*>(gc->getView("buy_title"));
		if (pBuyTitle)
		{
			if(_BuyMean == Money)
				pBuyTitle->setHardText("uiBuyFrom");
			else if(_BuyMean == SkillPoints)
				pBuyTitle->setHardText("uiLearnFrom");
			// MoneyAndFactionPoints, MoneyGuildXP or GuildXP
			else
				pBuyTitle->setHardText("uiBuyFrom");
		}

		// show/hide skill Tabs
		CGroupTab *pTab = dynamic_cast<CGroupTab*>(gc->getGroup("buy_filter:tab"));
		if ( pTab )
		{
			bool activeSkillTabs = (_BuyMean == SkillPoints);
			pTab->setActive(activeSkillTabs);
		}

		// unselect any list
		unselectBuyList(gc);
		if (!_BuyOnly)
			unselectSellList(gc);
	}

}

// *****************************************************************************************
void CBotChatPageTrade::end()
{
	activateWindow(WIN_BOT_CHAT_PAGE_TRADE, false);
	activateWindow(WIN_BOT_CHAT_PAGE_BUY, false);
	endTradeModal();

	// Important to avoid update of the remaining phrase in the DB.
	_TradePagesObs.allowPhrasePriceUpdateAndClientFill(false);
}

// *****************************************************************************************
void CBotChatPageTrade::update()
{
	updateTradeModal();
}


// ***************************************************************************************
uint32 CBotChatPageTrade::getCurrItemQuantity() const
{
	if (!_CurrItemSheet) return std::numeric_limits<uint32>::max();
	if(!_QuantityEdit) return 1;
	if (_CurrItemSheet->getType() == CCtrlSheetInfo::SheetType_Item)
	{
		const CItemSheet *itemSheet = _CurrItemSheet->asItemSheet();
		if (itemSheet && itemSheet->Family != ITEMFAMILY::TELEPORT &&
			itemSheet->Family != ITEMFAMILY::GUILD_OPTION)
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			CInterfaceGroup *ig = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
			if (!ig) return std::numeric_limits<uint32>::max();
			// TODO: edit box in faction points?
			CGroupEditBox *ed = dynamic_cast<CGroupEditBox *>(ig->getGroup("header_opened:standard_price:quantity:edit:eb"));
			if (!ed) return std::numeric_limits<uint32>::max();
			uint32 intQuantity;
			if (fromString(ed->getInputString().toString(), intQuantity))
			{
				return intQuantity;
			}
			else
			{
				return std::numeric_limits<uint32>::max();
			}
		}
		else
		{
			return 1;
		}
		return std::numeric_limits<uint32>::max();
	}
	return 1;
}

// ***************************************************************************************
uint32 CBotChatPageTrade::getCurrItemPriceResale() const
{
	if (!_CurrItemSheet) return 0;
	if (!_ResaleEdit) return 0;
	if (_CurrItemSheet->getType() == CCtrlSheetInfo::SheetType_Item)
	{
//		const CItemSheet *itemSheet = _CurrItemSheet->asItemSheet();
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		// get the edited resale price
		CInterfaceGroup *ig = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
		if (!ig) return 0;
		CGroupEditBox *resaleMarginBox = dynamic_cast<CGroupEditBox *>(ig->getGroup("header_opened:resell_group:can_resell:choose_resell:edit:eb"));
		if (!resaleMarginBox) return 0;

		// Calculate the resale price from the resale margin
		uint32 margin = (uint32)resaleMarginBox->getInputStringAsInt();
		// temp uint64 because we add case where basePrice= 50000, and margin= 99999% => 50000*99999 > 4gig
		uint64 basePrice = getCurrItemPrice(false);	// base Price without fame margin
		return uint32(basePrice + basePrice * margin / 100);
	}

	return 0;
}


// ***************************************************************************************
uint64 CBotChatPageTrade::getCurrItemPrice(bool mulByFame) const
{
	if (!_CurrItemSheet) return std::numeric_limits<uint64>::max();
	if (_BuyDlgOn && (_BuyMean == MoneyGuildXP || _BuyMean == GuildMoneyGuildXP || _BuyMean == GuildMoney) )
	{
		if (_BuyMean == MoneyGuildXP)
		{
			// get the price from the sheet
			const CItemSheet *pIS = _CurrItemSheet->asItemSheet();
			if (pIS != NULL)
			{
				if (pIS->Family == ITEMFAMILY::GUILD_OPTION)
					return pIS->GuildOption.MoneyCost;
			}
		}
		if ((_BuyMean == GuildMoneyGuildXP) || (_BuyMean == GuildMoney))
		{
			// This is perhaps an outpost building
			const COutpostBuildingSheet *pOB = _CurrItemSheet->asOutpostBuildingSheet();
			if (pOB != NULL)
				return pOB->CostDapper;
		}
	}
	else
	{
		// for item to buy/sell, price has been received in the db
		if (_CurrItemSheet->getRootBranch())
		{
			// for item to buy/sell, price has already been stored in the database
			ICDBNode *node;
			// take the PRICE_RETIRE if buying an item which actually belongs to the user
			if(_UsePriceRetire)
				node= _CurrItemSheet->getRootBranch()->getNode(ICDBNode::CTextId("PRICE_RETIRE"), false);
			else
				node= _CurrItemSheet->getRootBranch()->getNode(ICDBNode::CTextId("PRICE"), false);
			CCDBNodeLeaf *priceLeaf = dynamic_cast<CCDBNodeLeaf *>(node);
			if (priceLeaf)
			{
				uint64 price = (uint64) priceLeaf->getValue32();
				if (_BuyDlgOn || !mulByFame)
					return price;
				else
					return (uint64)(price * _FamePriceFactorLeaf->getValue16()/10000.0f);
			}
		}
	}
	return std::numeric_limits<uint64>::max();
}

// ***************************************************************************************
uint64 CBotChatPageTrade::getCurrItemXP() const
{
	if (_BuyMean == MoneyGuildXP)
	{
		// get the cp from the sheet
		const CItemSheet *pIS = _CurrItemSheet->asItemSheet();
		if (pIS != NULL)
		{
			if (pIS->Family == ITEMFAMILY::GUILD_OPTION)
				return pIS->GuildOption.XPCost;
		}
	}
	if ((_BuyMean == GuildMoneyGuildXP) || (_BuyMean == GuildMoney))
	{
		return 0; // By now no need for XP even if xp guild required
	}

	return std::numeric_limits<uint64>::max();
}


// ***************************************************************************
void	CBotChatPageTrade::getItemFactionTypePoints(CDBCtrlSheet *sheet, PVP_CLAN::TPVPClan &pvpClan, uint32 &points) const
{
	pvpClan= PVP_CLAN::None;
	points= 0;

	// Bad setup => abort
	if ((_BuyMean != MoneyFactionPoints) ||	(sheet == NULL))
		return;

	//CCDBNodeLeaf *currencyLeaf = dynamic_cast<CCDBNodeLeaf *>(sheet->getRootBranch()->getNode(ICDBNode::CTextId("CURRENCY"), false));
	//if (currencyLeaf == NULL)
	//	return;

	//RYMSG::TTradeCurrency currency((RYMSG::TTradeCurrency::TValues) currencyLeaf->getValue32());
	//if (currency != RYMSG::TTradeCurrency::tc_faction_points)
	//	return;

	// bad DB => abort
	CCDBNodeLeaf *ftLeaf = dynamic_cast<CCDBNodeLeaf *>(sheet->getRootBranch()->getNode(ICDBNode::CTextId("FACTION_TYPE"), false));
	CCDBNodeLeaf *fppLeaf = dynamic_cast<CCDBNodeLeaf *>(sheet->getRootBranch()->getNode(ICDBNode::CTextId("PRICE"), false));
	if (ftLeaf == NULL || fppLeaf == NULL)
		return;

	// else copy
	pvpClan= (PVP_CLAN::TPVPClan)ftLeaf->getValue32();
	points= fppLeaf->getValue32();

	// if only one is disabled, consider full disabled
	if(pvpClan==PVP_CLAN::None || points==0)
	{
		pvpClan= PVP_CLAN::None;
		points= 0;
	}
}

// ***************************************************************************
void	CBotChatPageTrade::getCurrItemFactionTypePoints(PVP_CLAN::TPVPClan &pvpClan, uint32 &points) const
{
	getItemFactionTypePoints(_CurrItemSheet, pvpClan, points);
}

// ***************************************************************************
uint32 CBotChatPageTrade::getUserFactionPoints(PVP_CLAN::TPVPClan clan) const
{
	if ((clan < PVP_CLAN::BeginClans) || (clan > PVP_CLAN::EndClans))
		return 0;

	uint32 nClan = clan - PVP_CLAN::BeginClans;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pLeaf = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:USER:FACTION_POINTS_%d:VALUE", nClan), false);
	if (pLeaf == NULL)
		return 0;

	return pLeaf->getValue32();
}

// ***************************************************************************************
void CBotChatPageTrade::notifyDownloadComplete(bool completed)
{	
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(_BuyOnly ? WIN_BOT_CHAT_PAGE_BUY : WIN_BOT_CHAT_PAGE_TRADE));
	if (!gc) return;
	class CDBListVisitor : public CInterfaceElementVisitor
	{
	public:		
		bool DownloadComplete;
		virtual void visitGroup(CInterfaceGroup *group) 
		{
			CDBGroupListSheetText *dbGroup = dynamic_cast<CDBGroupListSheetText *>(group);
			if (dbGroup)
			{
				dbGroup->notifyDownloadComplete(DownloadComplete);
			}
		}
	};
	CDBListVisitor visitor;
	visitor.DownloadComplete = completed;
	gc->visit(&visitor);
}

// ***************************************************************************************
void CBotChatPageTrade::updateTradeModal()
{
	// if loading is finished, then signal it to all sheet list, so they can display an help message if they
	// are empty
	if (_DownloadComplete != _TradePagesObs.isDownloadComplete())
	{
		notifyDownloadComplete(true);
		_DownloadComplete = _TradePagesObs.isDownloadComplete();
	}

	if(_CurrItemSheet)
	{
		// update buy dialog if it is currently shown
		if (_BuyDlgOn || _SellDlgOn)
		{
			// check that curr item hasn't been modified (so that the index remains valid)
			// this could happen if there was a db correction from the server
			// in this case, the dialog is closed
			if ( !_CurrItemCheck.test(_CurrItemSheet) )
			{
				endTradeModal();
				return;
			}
			// get pointers on interface elements
			CInterfaceManager *im = CInterfaceManager::getInstance();
			CGroupContainer *ig = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
			CInterfaceGroup *confirmTradeGroup = ig->getGroup("confirm_trade");
			CCtrlBase *confirmResellGroup = ig->getCtrl("confirm_trade_resell");
			CInterfaceGroup *cantTradeGroup = ig->getGroup("cant_trade");
			CInterfaceGroup *destroyTradeGroup = ig->getGroup("destroy_trade");

			CCtrlTextButton *cantTradeButton =  dynamic_cast<CCtrlTextButton*>(ig->getCtrl("cant_trade_button"));

			// **** update destroy options
			// if the item belongs to the user, can always destroy it from the resale
			destroyTradeGroup->setActive(_UsePriceRetire);

			// **** update confirmation
			// update price from quantity is the object is an item and not a teleport
			uint32 quantity = getCurrItemQuantity();
			// get price (without fame)
			uint64 priceWithoutFame = getCurrItemPrice(false);
			// get price (with fame)
			uint64 priceWithFame = getCurrItemPrice(true);
			// update resale price from quantity
			uint32	resalePrice = getCurrItemPriceResale();

			// get faction points and type from current item
			PVP_CLAN::TPVPClan	fpType;
			uint32				fpCost;
			getCurrItemFactionTypePoints(fpType, fpCost);
			uint32 userFactionPoints = getUserFactionPoints(fpType);

			// Special case for guild options
			if ((_BuyMean == MoneyGuildXP) || (_BuyMean == GuildMoney) || (_BuyMean == GuildMoneyGuildXP))
			{
				uint64 totalPrice = (uint64) priceWithoutFame * (uint64) quantity;
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE")->setValue64(totalPrice);
				uint64 totalXP = (uint64) getCurrItemXP() * (uint64) quantity;
				CGuildManager *pGM = CGuildManager::getInstance();

				uint64 moneyOwned = (_BuyMean == MoneyGuildXP)?getInventory().getMoney():pGM->getMoney();
				if (totalPrice > moneyOwned)
				{
					if (confirmTradeGroup)	confirmTradeGroup->setActive(false);
					if (cantTradeGroup)		cantTradeGroup->setActive(true);
					if (_BuyMean == MoneyGuildXP)
					{
						if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiNotEnoughMoney"));
						if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittNotEnoughMoney"));
					}
					else
					{
						if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiNotEnoughGuildMoney"));
						if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittNotEnoughGuildMoney"));
					}
					return;
				}
				if ((_BuyMean == MoneyGuildXP) || (_BuyMean == GuildMoneyGuildXP))
				{
					if (totalXP > pGM->getXP())
					{
						if (confirmTradeGroup)	confirmTradeGroup->setActive(false);
						if (cantTradeGroup)		cantTradeGroup->setActive(true);
						if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiNotEnoughGuildXP"));
						if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittNotEnoughGuildXP"));
						return;
					}
				}

				if (confirmTradeGroup) confirmTradeGroup->setActive(true);
				if (cantTradeGroup) cantTradeGroup->setActive(false);

				return;
			}


			// if price/quantity is valid
			bool	validSetup= priceWithoutFame != std::numeric_limits<uint64>::max() && quantity != std::numeric_limits<uint32>::max() && quantity != 0;
			if(validSetup && _BuyMean==MoneyFactionPoints)
			{
				// valid if at least one price type is not 0
				validSetup= priceWithoutFame!=0 || fpCost!=0;
			}
			else if(validSetup && _BuyMean==SkillPoints)
			{
				// (nb: skillpoint==0 is a valid buy)
				validSetup= true;
			}
			else if(validSetup)
			{
				// valid if price is not 0
				validSetup= priceWithoutFame != 0;
			}

			// if the setup is finaly valid
			if(	validSetup )
			{
				// basic price
				uint64 totalPriceWithoutFame = (uint64) priceWithoutFame * (uint64) quantity;
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:UNIT_PRICE")->setValue64(priceWithoutFame);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE")->setValue64(totalPriceWithoutFame);
				// price with fame
				uint64 totalPriceWithFame = (uint64) priceWithFame * (uint64) quantity;
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:UNIT_PRICE_WITH_FAME")->setValue64(priceWithFame);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE_WITH_FAME")->setValue64(totalPriceWithFame);
				// resale price
				uint64 totalResalePrice = (uint64) resalePrice * (uint64) quantity;
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:UNIT_PRICE_RESALE")->setValue64(resalePrice);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE_RESALE")->setValue64(totalResalePrice);
				// quantity edit box
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:EDIT_QUANTITY")->setValue64(quantity);
				// Faction Points
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:UNIT_PRICE_FACTION")->setValue64(fpCost);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE_FACTION")->setValue64(fpCost*quantity);


				// Special retire Check
				bool	ok= true;
				if(_CannotValidateBecauseRetireNotAvailable)
				{
					if (confirmTradeGroup) confirmTradeGroup->setActive(false);
					if (confirmResellGroup) confirmResellGroup->setActive(false);
					if (cantTradeGroup) cantTradeGroup->setActive(true);
					// can't sell more than what is in inventory
					if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiBCNotAvailable"));
					if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittBCNotAvailable"));
					ok= false;
				}

				// Quantity Check (only if previous check passed)
				if(ok && _QuantityCheck)
				{
					// check if not bad quantity edited
					if ((sint32)quantity > _QuantityCheck)
					{
						if (confirmTradeGroup) confirmTradeGroup->setActive(false);
						if (confirmResellGroup) confirmResellGroup->setActive(false);
						if (cantTradeGroup) cantTradeGroup->setActive(true);
						// can't sell more than what is in inventory
						if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiBadQuantity"));
						if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittBadQuantity"));
						ok= false;
					}
				}

				// test only if quantity check passed
				if(ok)
				{
					// see if player has enough money
					if (_BuyDlgOn)
					{
						if ((_BuyMean == Money) || (_BuyMean == MoneyFactionPoints))
						{
							if (totalPriceWithoutFame > getInventory().getMoney())
							{
								if (confirmTradeGroup) confirmTradeGroup->setActive(false);
								if (cantTradeGroup) cantTradeGroup->setActive(true);
								if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiNotEnoughMoney"));
								if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittNotEnoughMoney"));
							}
							else if (_BuyMean == MoneyFactionPoints)
							{
								// Check if the player has enough faction point for the object selected
								if ((confirmTradeGroup != NULL) && (cantTradeButton != NULL) && (cantTradeGroup != NULL))
								{
									if ((fpCost*quantity) > userFactionPoints)
									{
										confirmTradeGroup->setActive(false);
										cantTradeGroup->setActive(true);
										cantTradeButton->setText(CI18N::get("uiNotEnoughFP_"+PVP_CLAN::toString(fpType)));
										cantTradeButton->setDefaultContextHelp(CI18N::get("uittNotEnoughFP_"+PVP_CLAN::toString(fpType)));
									}
									else
									{
										confirmTradeGroup->setActive(true);
										cantTradeGroup->setActive(false);
									}
								}
							}
							else
							{
								if (confirmTradeGroup) confirmTradeGroup->setActive(true);
								if (cantTradeGroup) cantTradeGroup->setActive(false);
							}
						}
						else if (_BuyMean == SkillPoints)
						{
							uint32 numSkillPoints = getSkillPointUsedForCurrentRoleMaster();
							if (totalPriceWithoutFame > numSkillPoints)
							{
								if (confirmTradeGroup) confirmTradeGroup->setActive(false);
								if (cantTradeGroup) cantTradeGroup->setActive(true);
								if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiNotEnoughSkillPoints"));
								if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittNotEnoughSkillPoints"));
							}
							else
							{
								if (confirmTradeGroup) confirmTradeGroup->setActive(true);
								if (cantTradeGroup) cantTradeGroup->setActive(false);
							}
						}
						else if (_BuyMean == GuildXP)
						{
							CGuildManager *pGM = CGuildManager::getInstance();
							if (totalPriceWithoutFame > pGM->getXP())
							{
								if (confirmTradeGroup) confirmTradeGroup->setActive(false);
								if (cantTradeGroup) cantTradeGroup->setActive(true);
								if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiNotEnoughGuildXP"));
								if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittNotEnoughGuildXP"));
							}
							else
							{
								if (confirmTradeGroup) confirmTradeGroup->setActive(true);
								if (cantTradeGroup) cantTradeGroup->setActive(false);
							}
						}
					}
					else
					{
						// check ResalePrice
						if(_ResaleEdit && resalePrice<priceWithoutFame)
						{
							if (confirmTradeGroup) confirmTradeGroup->setActive(false);
							if (confirmResellGroup) confirmResellGroup->setActive(false);
							if (cantTradeGroup) cantTradeGroup->setActive(true);
							// can't sell less than the basic price
							if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiBadResalePrice"));
							if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittBadResalePrice"));
						}
						// else ok, can resell
						else
						{
							if (confirmTradeGroup) confirmTradeGroup->setActive(true);
							if (_ResaleEdit && confirmResellGroup) confirmResellGroup->setActive(true);
							if (cantTradeGroup) cantTradeGroup->setActive(false);
						}
					}
				}
			}
			// else price is not valid
			else
			{
				// set value in database
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:UNIT_PRICE")->setValue64(-1);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:UNIT_PRICE_WITH_FAME")->setValue64(-1);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE")->setValue64(-1);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE_WITH_FAME")->setValue64(-1);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:UNIT_PRICE_RESALE")->setValue64(-1);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE_RESALE")->setValue64(-1);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:EDIT_QUANTITY")->setValue64(0);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:UNIT_PRICE_FACTION")->setValue64(-1);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:PRICE_FACTION")->setValue64(-1);
				// and update ok/cancel groups
				if (confirmTradeGroup) confirmTradeGroup->setActive(false);
				if (confirmResellGroup) confirmResellGroup->setActive(false);
				if (cantTradeGroup) cantTradeGroup->setActive(true);
				if (_SellDlgOn && priceWithoutFame == 0)
				{
					if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiWontBuyThis"));
					if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittWontBuyThis"));
				}
				else if (_SellDlgOn && priceWithoutFame == std::numeric_limits<uint64>::max())
				{
					if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiPriceNotReceived"));
				}
				else if (quantity == 0 || quantity == std::numeric_limits<uint32>::max())
				{
					if (cantTradeButton)	cantTradeButton->setText(CI18N::get("uiBadQuantity"));
					if (cantTradeButton)	cantTradeButton->setDefaultContextHelp(CI18N::get("uittBadQuantity"));
				}
			}
		}
	}
}

// ***************************************************************************************
void CBotChatPageTrade::setupBuyMeanInModal(CInterfaceGroup *modal)
{
	if (!modal) return;
	CViewBase *equal = modal->getView("standard_price:total_price:equal");
	CInterfaceGroup *money = modal->getGroup("standard_price:total_price:item_price");
	CInterfaceGroup *skillPoints = modal->getGroup("standard_price:total_price:sp");
	if (equal) equal->setActive(_BuyMean == Money);
	if (money) money->setActive(_BuyMean == Money);
	if (skillPoints) skillPoints->setActive(_BuyMean == SkillPoints);
}

// ***************************************************************************************
void CBotChatPageTrade::startBuyDialog(CDBCtrlSheet *sheet, CCtrlBase * /* pCaller */)
{
	if (!sheet) return;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *ig = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
	if (!ig) return;

	// don't know why but in some case, the sheetId is 0
	if(sheet->getSheetId()==0)
		return;

	bool isSPhrase = sheet->isSPhrase();
	bool isItem = !isSPhrase;

	// If this item is a User Item already sold (-1), cancel the RETIRE (in list only for information)
	if( sheet->getType()==CCtrlSheetInfo::SheetType_Item &&
		(sheet->getItemSellerType()==BOTCHATTYPE::User || sheet->getItemSellerType()==BOTCHATTYPE::UserRetirable ) &&
		sheet->getItemPrice()==-1 )
		return;

	// Set price label
	CViewText *priceLabel = dynamic_cast<CViewText*>(ig->getView( "standard_price:total_price_header" ));
	if ( _BuyMean == Money && priceLabel )
	{
		priceLabel->setText( CI18N::get( "uiPrice" ) );
		priceLabel->setActive(true);
	}
	else
		priceLabel->setActive( false );

	// show quantity dialog if a correct item (only stackable items etc....)
	uint	defaultQuantity= initQuantityMode(false, sheet);
	setupPriceGroupQuantity(ig->getGroup("standard_price"), defaultQuantity);

	// hide resell group, (cause its a buy)
	setupResellGroup(false, defaultQuantity, ig, sheet);

	// setup edit focus
	setupEditFocus(ig);

	// setup which price to use
	setupPriceOrPriceRetire(false, sheet);

	// setup faction point price group
	setupFactionPointPrice(false, defaultQuantity, ig, sheet);

	// set confirm button text
	CCtrlTextButton *confirmButton =  dynamic_cast<CCtrlTextButton*>(ig->getCtrl("ok"));
	if (confirmButton)
	{
		confirmButton->setActive( true );
		// no need any context help because too simple
		confirmButton->setDefaultContextHelp(ucstring());
		if(isItem)
		{
			CItemSheet * itemSheet = dynamic_cast<CItemSheet*> ( SheetMngr.get( CSheetId( sheet->getSheetId() ) ) );
			if ( itemSheet && itemSheet->Family == ITEMFAMILY::COSMETIC )
			{
				EGSPD::CPeople::TPeople people = ITEM_ORIGIN::itemOriginStringToPeopleEnum( ITEM_ORIGIN::enumToString( itemSheet->ItemOrigin ) );
				if ( UserEntity->getGender() != itemSheet->Cosmetic.Gender || UserEntity->people() != people )
					confirmButton->setActive( false );
				else
					confirmButton->setText(CI18N::get(_UsePriceRetire?"uiRetire":"uiBuy"));
			}
			else
				confirmButton->setText(CI18N::get(_UsePriceRetire?"uiRetire":"uiBuy"));
		}
		else
			confirmButton->setText(CI18N::get("uiLearn"));
	}

	// Hide direct sell button
	CCtrlBase *resellButton = ig->getCtrl("confirm_trade_resell");
	if ( resellButton )
		resellButton->setActive(false);

	// set help for item
	CSheetHelpSetup helpSetup;
	fillHelpSetupInfosForTrade(helpSetup, sheet, ig);
	setupSheetHelp(helpSetup);
	//
	ig->setActive(true);
	ig->updateCoords();
	ig->center();
	ig->setModalParentList(_BuyOnly ? WIN_BOT_CHAT_PAGE_BUY : WIN_BOT_CHAT_PAGE_TRADE);
	CWidgetManager::getInstance()->setTopWindow(ig);
	//
	_CurrItemSheet = sheet;
	_CurrItemIndex  = sheet->getIndexInDB();
	_CurrItemCheck.init(_CurrItemSheet);
	_BuyDlgOn = true;
	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:IS_SELL_DLG")->setValueBool( false );
	// setup trade mean
	setupBuyMeanInModal(ig);
	// Add waiter to refresh items infos
	InfoWindowUpdater.ItemSheet= _CurrItemSheet->getSheetId();
	InfoWindowUpdater.ItemSlotId= getInventory().getItemSlotId(_CurrItemSheet);
	InfoWindowUpdater.Sheet = _CurrItemSheet;
	// Add the waiter only if really needed (not for raw materials)
	const CItemSheet	*itemSheet= _CurrItemSheet->asItemSheet();
	if(itemSheet && itemSheet->Family != ITEMFAMILY::RAW_MATERIAL )
		getInventory().addItemInfoWaiter(&InfoWindowUpdater);
}

// ***************************************************************************
void CBotChatPageTrade::updateSPhraseBuyDialog()
{
	if(!_CurrItemSheet)
		return;

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CGroupContainer *ig = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
	if (!ig) return;

	CSheetHelpSetup helpSetup;
	fillHelpSetupInfosForTrade(helpSetup, _CurrItemSheet, ig);
	setupSheetHelp(helpSetup);
}

// ***************************************************************************************
void CBotChatPageTrade::startSellDialog(CDBCtrlSheet *sheet, CCtrlBase * /* pCaller */)
{
	nlassert(!_BuyOnly);
	if (!sheet) return;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *ig = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
	if (!ig) return;

	// If this sheet is grayed cause of an "Animal Inventory unavailable" flag, quit
	if (sheet->getItemBeastGrayed()) return;

	// Set price label
	CViewText *priceLabel = dynamic_cast<CViewText*>(ig->getView( "standard_price:total_price_header" ));
	if ( priceLabel )
	{
		priceLabel->setText( CI18N::get( "uiImmediatePrice" ) );
		priceLabel->setActive(true);
	}

	// show quantity dialog if a correct item (only stackable items etc....)
	uint	defaultQuantity= initQuantityMode(true, sheet);
	setupPriceGroupQuantity(ig->getGroup("standard_price"), defaultQuantity);

	// Setup resell group, only if item (cause its a sell)
	setupResellGroup(true, defaultQuantity, ig, sheet);

	// setup edit focus
	setupEditFocus(ig);

	// setup which price to use
	setupPriceOrPriceRetire(true, sheet);

	// setup faction point price
	setupFactionPointPrice(true, defaultQuantity, ig, sheet);

	// set confirm button text
	CCtrlTextButton *confirmButton =  dynamic_cast<CCtrlTextButton*>(ig->getCtrl("ok"));
	if (confirmButton)
	{
		confirmButton->setActive( !sheet->getLockedByOwner() );
		confirmButton->setText(CI18N::get("uiSellImmediately"));
		confirmButton->setDefaultContextHelp(CI18N::get("uittDirectSellButton"));
	}

	// set item or skill name
	CViewText *itemNameView = dynamic_cast<CViewText *>(ig->getView("object_name"));
	if (itemNameView)
	{
		ucstring itemName;
		itemName = sheet->getItemActualName();
		itemNameView->setText(itemName);
	}

	// set help for item
	CSheetHelpSetup helpSetup;
	fillHelpSetupInfosForTrade(helpSetup, sheet, ig);
	setupSheetHelp(helpSetup);
	//
	ig->setActive(true);
	ig->updateCoords();
	ig->center();
	ig->setModalParentList(WIN_BOT_CHAT_PAGE_TRADE);
	CWidgetManager::getInstance()->setTopWindow(ig);
	//
	_CurrItemSheet = sheet;
	_CurrItemIndex  = sheet->getIndexInDB();
	_CurrItemInventory = sheet->getInventoryIndex();
	_CurrItemCheck.init(_CurrItemSheet);
	_SellDlgOn = true;
	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:IS_SELL_DLG")->setValueBool( true );
	updateTradeModal();
	setupBuyMeanInModal(ig);
	// Add waiter to refresh items infos
	InfoWindowUpdater.ItemSheet= _CurrItemSheet->getSheetId();
	InfoWindowUpdater.ItemSlotId= getInventory().getItemSlotId(_CurrItemSheet);
	InfoWindowUpdater.Sheet = _CurrItemSheet;
	// Add the waiter only if really needed (not for raw materials)
	const CItemSheet	*itemSheet= _CurrItemSheet->asItemSheet();
	if(itemSheet && itemSheet->Family != ITEMFAMILY::RAW_MATERIAL )
		getInventory().addItemInfoWaiter(&InfoWindowUpdater);
}

// ***************************************************************************************
void CBotChatPageTrade::endTradeModal()
{
	// do the cancel now (NB: will be recalled at close of the window....)
	cancelTrade();

	// hide the dialog
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *ig = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
	if (ig)	ig->setActive(false);

	// Hide any confirmation dialog related to trade (important for building MessageBox confirmation)
	std::string		vmbOnOk;
	if(pIM->getCurrentValidMessageBoxOnOk(vmbOnOk))
	{
		if(vmbOnOk=="confirm_trade")
			CWidgetManager::getInstance()->disableModalWindow();
	}
}

// ***************************************************************************************
void CBotChatPageTrade::setFocusOnEditBox(CInterfaceGroup *ebi)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(ebi);
	if (eb)
	{
		CWidgetManager::getInstance()->setCaptureKeyboard(eb);
		eb->setSelectionAll();
	}
}

// ***************************************************************************************
bool CBotChatPageTrade::isTradeValid( bool enableResale ) const
{
	// If the Current Item has been canceled (eg: move out), then not valid!
	if(!_CurrItemSheet)
		return false;

	uint32 quantity= getCurrItemQuantity();
	uint64 priceWithoutFame = getCurrItemPrice(false);
	sint32 resalePrice = getCurrItemPriceResale();

	// Special case for guild options
	if ((_BuyMean == MoneyGuildXP) || (_BuyMean == GuildMoney) || (_BuyMean == GuildMoneyGuildXP))
	{
		uint64 totalPrice = (uint64) priceWithoutFame * (uint64) quantity;
		uint64 totalXP = (uint64) getCurrItemXP() * (uint64) quantity;
		CGuildManager *pGM = CGuildManager::getInstance();
		uint64 moneyOwned = (_BuyMean == MoneyGuildXP)?getInventory().getMoney():pGM->getMoney();
		if ((_BuyMean == MoneyGuildXP) || (_BuyMean == GuildMoneyGuildXP))
			return (totalPrice <= moneyOwned) && (totalXP <= pGM->getXP());
		return (totalPrice <= moneyOwned);
	}

	// retrieve faction points
	PVP_CLAN::TPVPClan	fpType;
	uint32				fpCost;
	getCurrItemFactionTypePoints(fpType, fpCost);

	// if price/quantity is valid
	bool	validSetup= priceWithoutFame != std::numeric_limits<uint64>::max() && quantity != std::numeric_limits<uint32>::max() && quantity != 0;
	if(validSetup && _BuyMean==MoneyFactionPoints)
	{
		// valid if at least one price type is not 0
		validSetup= priceWithoutFame!=0 || fpCost!=0;
	}
	else if(validSetup && _BuyMean==SkillPoints)
	{
		// (nb: skillpoint==0 is a valid buy)
		validSetup= true;
	}
	else if(validSetup)
	{
		// valid if price is not 0
		validSetup= priceWithoutFame != 0;
	}

	// if the setup is finaly valid
	if(	validSetup )
	{
		// special retire check
		if(_CannotValidateBecauseRetireNotAvailable)
			return false;

		// quantity check
		if(_QuantityCheck)
		{
			if( (sint32)quantity > _QuantityCheck )
				return false;
		}

		// Buy dlg
		if (_BuyDlgOn)
		{
			uint64 totalPrice = (uint64) priceWithoutFame * (uint64) quantity;
			if (_BuyMean == Money)
			{
				return totalPrice <= getInventory().getMoney();
			}
			if (_BuyMean == MoneyFactionPoints)
			{
				return ((totalPrice <= getInventory().getMoney()) &&
						(quantity*fpCost <= getUserFactionPoints(fpType)));
			}
			else if (_BuyMean == SkillPoints)
			{
				uint32 numSkillPoints = getSkillPointUsedForCurrentRoleMaster();
				return totalPrice <= numSkillPoints;
			}
			else if (_BuyMean == GuildXP)
			{
				CGuildManager *pGM = CGuildManager::getInstance();
				return totalPrice <= pGM->getXP();
			}
		}
		// Sell dlg
		else
		{
			return (!_ResaleEdit || !enableResale || (sint32)priceWithoutFame <= resalePrice);
		}
	}
	return false;
}


// ***************************************************************************************
void CBotChatPageTrade::confirmTrade( bool enableResale )
{
	bool resaleEnabled = _SellDlgOn && enableResale;

	if (!_CurrItemSheet)
	{
		endTradeModal();
		return;
	}
	if (_BuyDlgOn || _SellDlgOn)
	{
		uint32	quantity = getCurrItemQuantity();
		sint32	resalePrice = resaleEnabled ? getCurrItemPriceResale() : 0; // getCurrItemPriceResale() returns 0 is !_ResaleEdit
		if (quantity != std::numeric_limits<uint32>::max())
		{
			uint16 u16Quantity = (uint16) quantity;
			// The Item bought is a SPhrase ?
			if(_CurrItemSheet->isSPhrase())
			{
				// cant sell phrase
				if ( _SellDlgOn )
					return;

				// code in action_handler_phrase.cpp
				extern	void	phraseBuyBotChat(CDBCtrlSheet *ctrl, uint8 index, bool useBuySheetMsg);
				// client Side fill Mode? => use PHRASE:BUY_SHEET
				phraseBuyBotChat(_CurrItemSheet, (uint8) _CurrItemIndex, _TradePagesObs.isPhraseClientFill());

				// lower the SkillPoints available.
				sint32 price = (sint32)getCurrItemPrice(false);
				sint32 totalPrice= quantity * price;
				addSkillPointForCurrentRoleMaster(-totalPrice);

				// hide the phrase
				_CurrItemSheet->setSheetId(0);
			}
			// Else standard buy.
			else
			{
				if (_BuyDlgOn)
				{
					NLMISC::CBitMemStream out;

					if ((_BuyMean == MoneyGuildXP) || (_BuyMean == GuildMoney) || ((_BuyMean == GuildMoneyGuildXP)))
					{
						const char	*msg="BOTCHAT:BUY_GUILD_OPTION";
						if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
						{
							uint8 u8Index    = (uint) _CurrItemIndex;
							out.serial(u8Index);
							NetMngr.push(out);
							//nlinfo("impulseCallBack sent: %s %d", msg, u8Index);
						}
						else
							nlwarning(" unknown message name '%s'", msg);
					}
					else
					{
						// bug #212: Server should check if player has enough room in his inventory
						// before removing items in trade window
						// Temp fix on client side : check room in player's bag before removing items

						// Get inventory manager instance
						CInventoryManager *pInv = CInventoryManager::getInstance();

						// Check if player has enough space in his bag
						bool isSpaceInBag = pInv->isSpaceInBagForItem(_CurrItemSheet, quantity, 0);

						const char	*msg="BOTCHAT:BUY";
						if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
						{
							uint16 u16Index    = (uint16) _CurrItemIndex;
							out.serial(u16Index);
							out.serial(u16Quantity);
							NetMngr.push(out);
							//nlinfo("impulseCallBack sent: %s %d %d", msg, u16Index, u16Quantity);
						}
						else
							nlwarning(" unknown message name '%s'", msg);

						// If player has enough space in his bag...
						if (isSpaceInBag)
						{
							// If the Buy is from a Player item or User item (retire) localy delete it
							if (_BuyMean == Money && _CurrItemSheet->getItemSellerType()!=BOTCHATTYPE::NPC)
							{
								// if item stackable, the player was allowed to buy only a subset
								if(_QuantityEdit)
								{
									sint32	curQuantity= _CurrItemSheet->getQuantity();
									if(u16Quantity<curQuantity)
										// lower the quantity
										_CurrItemSheet->setQuantity(curQuantity-u16Quantity);
									else
										// hide the item
										_CurrItemSheet->setSheetId(0);
								}
								else
								{
									// hide the item
									_CurrItemSheet->setSheetId(0);
								}
							}
						}
					}
				}
				else
				{
					const char	*msg="BOTCHAT:SELL";
					NLMISC::CBitMemStream out;
					if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
					{
						uint8	u8Inv		 = (uint8) _CurrItemInventory;
						uint8	u8Index    = (uint8) _CurrItemIndex;
						uint32	u32resalePrice = (uint32)max((sint32)0, resalePrice);
						out.serial(u8Inv);
						out.serial(u8Index);
						out.serial(u16Quantity);
						out.serial(u32resalePrice);
						NetMngr.push(out);
						//nlinfo("impulseCallBack sent: %s %d %d %d %d", msg, u8Inv, u8Index, u16Quantity, u32resalePrice);
					}
					else
						nlwarning(" unknown message name '%s'", msg);

					// Write the value of the resale margin to the persistant database
					if ( _ResaleEdit )
					{
						CInterfaceManager *pIM = CInterfaceManager::getInstance();
						CInterfaceGroup *ig = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
						if (ig)
						{
							CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(ig->getGroup("header_opened:resell_group:can_resell:choose_resell:edit:eb"));
							if (eb)
							{
								NLGUI::CDBManager::getInstance()->getDbProp( "UI:SAVE:TRADE_ITEM:RESALE_MARGIN" )->setValue32( eb->getInputStringAsInt() );
							}
						}
					}
				}
			}
		}
	}

	// get itemSheet before _CurrItemSheet set to NULL in endTradeModal()
	const CItemSheet *itemSheet = _CurrItemSheet->asItemSheet();

	// close modal dialog
	endTradeModal();

	// Close all if its a guild option
	if (itemSheet && itemSheet->Family == ITEMFAMILY::GUILD_OPTION)
		CBotChatManager::getInstance()->endDialog();

}

// ***************************************************************************************
void CBotChatPageTrade::cancelTrade()
{
	// remove infos waiter (if not already canceled)
	if (_CurrItemSheet)
	{
		// Add the waiter only if really needed (not for raw materials)
		const CItemSheet	*itemSheet= _CurrItemSheet->asItemSheet();
		if(itemSheet && itemSheet->Family != ITEMFAMILY::RAW_MATERIAL )
			getInventory().removeItemInfoWaiter(&InfoWindowUpdater);
	}
	//
	_BuyDlgOn  = false;
	_SellDlgOn = false;
	_ResaleEdit = false;
	_QuantityEdit = false;
	_UsePriceRetire = false;
	_CannotValidateBecauseRetireNotAvailable = false;
	_QuantityCheck = 0;
	_CurrItemSheet = NULL;
	_CurrItemCheck.reset();
}

// ***************************************************************************
sint32 CBotChatPageTrade::getSkillPointUsedForCurrentRoleMaster() const
{
	// get the current phrase rolemaster type
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	rmt= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:BOTCHAT:ROLEMASTER_TYPE")->getValue32();
	// get the prop (use local for less lag)
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:USER:SKILL_POINTS_%d:VALUE", rmt), false);
	if(node)
		return node->getValue32();
	else
		return 0;
}


// ***************************************************************************
void	CBotChatPageTrade::addSkillPointForCurrentRoleMaster(sint32 addValue) const
{
	// get the current phrase rolemaster type
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	rmt= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:BOTCHAT:ROLEMASTER_TYPE")->getValue32();
	// add the local prop !!
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:USER:SKILL_POINTS_%d:VALUE", rmt), false);
	if(node)
	{
		node->setValue32(node->getValue32() + addValue);
	}
}



// ***************************************************************************
uint	CBotChatPageTrade::initQuantityMode(bool sellMode, CDBCtrlSheet *sheet)
{
	nlassert(sheet);

	// default
	_QuantityEdit= false;
	_QuantityCheck = 0;
	uint	defaultQuantity= 0;

	// Quantity only for some items
	const CItemSheet	*pIS= sheet->asItemSheet();
	if( pIS && (pIS->Stackable>1 || pIS->Family == ITEMFAMILY::ITEM_SAP_RECHARGE) && pIS->Family!=ITEMFAMILY::TELEPORT )
	{
		// if sell, init quantity with the whole inventory stack
		if(sellMode)
		{
			defaultQuantity= std::max((sint32) 1, sheet->getQuantity());
			_QuantityCheck= defaultQuantity;
		}
		// else
		else
		{
			// Depends on SellerType: If NPC, set 1 by default
			if(sheet->getItemSellerType() == BOTCHATTYPE::NPC)
			{
				defaultQuantity= 1;
				// no quantity check cause infinite
				_QuantityCheck= 0;
			}
			// else set the whole stack (player or user resale)
			else
			{
				defaultQuantity= std::max((sint32) 1, sheet->getQuantity());
				_QuantityCheck= defaultQuantity;
			}
		}

		// edit ok!
		_QuantityEdit= true;
	}

	return defaultQuantity;
}

// ***************************************************************************
void	CBotChatPageTrade::setupPriceGroupQuantity(CInterfaceGroup *priceGroup, sint32 defaultQuantity)
{
	// if window not found, quit
	if(!priceGroup)
		return;

	// If don't want to enable quantity
	if(!_QuantityEdit)
	{
		// hide elements and headers
		CInterfaceGroup *quantityGroup = priceGroup->getGroup("quantity");
		if (quantityGroup)	quantityGroup->setActive(false);
		CViewBase *elt = priceGroup->getView("quantity_header");
		if ( elt ) elt->setActive(false);
		elt = priceGroup->getGroup("unit_price");
		if ( elt ) elt->setActive(false);
		elt = priceGroup->getView("unit_price_header");
		if ( elt ) elt->setActive(false);
	}
	else
	{
		// show elements and header
		CInterfaceGroup *quantityGroup = priceGroup->getGroup("quantity");
		if (quantityGroup)	quantityGroup->setActive(true);
		CViewBase *elt = priceGroup->getView("quantity_header");
		if ( elt ) elt->setActive(true);
		elt = priceGroup->getGroup("unit_price");
		if ( elt ) elt->setActive(true);
		elt = priceGroup->getView("unit_price_header");
		if ( elt ) elt->setActive(true);

		// setup the quantity in the edit box
		if(quantityGroup)
		{
			// NB: some group don't have an edit box (just a replication of quantity)
			CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(quantityGroup->getGroup("edit:eb"));
			if (eb)
			{
				eb->setInputString(toString(defaultQuantity));
			}
		}
	}


}


// ***************************************************************************
void	CBotChatPageTrade::setupResellGroup(bool sellMode, uint defaultQuantity, CInterfaceGroup *parentGroup, CDBCtrlSheet *sheet)
{
	nlassert(sheet);

	// default
	_ResaleEdit= false;

	// if window not found, quit
	if(!parentGroup)
		return;

	// must be a sell, must be an item, and must be in a standard BuyMean, also must be a ressellable item
	CInterfaceGroup *resellGroup = parentGroup->getGroup("resell_group");
	if ( ! resellGroup )
		return;

	// Yoyo: can resell only on a "money only" reseler
	const CItemSheet	*pIS= sheet->asItemSheet();
	bool	resellable= sellMode && pIS && _BuyMean == Money && ITEMFAMILY::isResellable(pIS->Family) ;
	// also cannot resell Raw Material that don't have any item part (formula materials, because cannot filter them)
	if(pIS && pIS->Family==ITEMFAMILY::RAW_MATERIAL && !pIS->canBuildSomeItemPart())
		resellable= false;

	// cannot resell?
	if( !resellable )
	{
		// hide
		if ( resellGroup ) resellGroup->setActive(false);
		CCtrlBase *confirmResellGroup = parentGroup->getCtrl("confirm_trade_resell");
		if ( confirmResellGroup ) confirmResellGroup->setActive(false);
		_ResaleEdit = false;
	}
	else
	{
		// show
		if ( resellGroup ) resellGroup->setActive(true);

		// get sub elements
		CInterfaceGroup		*canResellGroup= resellGroup->getGroup("can_resell");
		CInterfaceGroup		*cantResellGroup= resellGroup->getGroup("cant_resell");
		CInterfaceGroup		*chooseResellGroup= resellGroup->getGroup("can_resell:choose_resell");
		CCtrlBase			*confirmResellGroup = parentGroup->getCtrl("confirm_trade_resell");
		if(!canResellGroup || !cantResellGroup || !chooseResellGroup || !confirmResellGroup)
		{
			nlwarning("XML error, cannot setup resale");
			resellGroup->setActive(false);
			_ResaleEdit = false;
			return;
		}

		// Get item state
		string	baseDB= sheet->getSheet();
		BOTCHATTYPE::TBotChatResaleFlag		resaleFlag= (BOTCHATTYPE::TBotChatResaleFlag)sheet->getItemResaleFlag();
//		sint	basePrice = sint(sheet->getItemPrice() * _FamePriceFactorLeaf->getValue16()/10000.0f);

		// if cannot resell this item, setup "cant_resell group"
		if(resaleFlag!=BOTCHATTYPE::ResaleOk)
		{
			canResellGroup->setActive(false);
			confirmResellGroup->setActive(false);
			cantResellGroup->setActive(true);
			// resale is not possible!
			_ResaleEdit = false;

			// setup the reason
			CViewText *vt= dynamic_cast<CViewText*>(cantResellGroup->getView("reason"));
			if(vt)
			{
				if(resaleFlag == BOTCHATTYPE::ResaleKOBroken)
					vt->setHardText("uiCantResaleCauseDamaged");
				else if (resaleFlag == BOTCHATTYPE::ResaleKONoTimeLeft)
					vt->setHardText("uiCantResaleCauseTooLate");
				else
					vt->setHardText("uiCantResaleLockedByOwner");
			}
		}
		// else setup "can_resell:choose_resell group"
		else
		{
			canResellGroup->setActive(true);
			confirmResellGroup->setActive(true);
			cantResellGroup->setActive(false);
			// resale is possible!
			_ResaleEdit = true;

			// setup the quantity of the "choose resell" group
			setupPriceGroupQuantity(chooseResellGroup, defaultQuantity);

			// setup resale margin using value stored in database
			CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(chooseResellGroup->getGroup("edit:eb"));
			if (eb)
			{
				sint32	resaleMargin= NLGUI::CDBManager::getInstance()->getDbProp( "UI:SAVE:TRADE_ITEM:RESALE_MARGIN" )->getValue32();
				clamp(resaleMargin, 0, (sint32)MaxResaleMargin);
				eb->setInputString( toString( resaleMargin ) );
				eb->setPositiveIntegerMaxValue(MaxResaleMargin);
			}
		}
	}
}


// ***************************************************************************
void	CBotChatPageTrade::setupEditFocus(CInterfaceGroup *parentGroup)
{
	// TODODO: edit box in faction points?
	// if Quantity edited, always start focus in quantity edit box
	if(_QuantityEdit)
	{
		nlassert(parentGroup);

		CGroupEditBox	*eb= dynamic_cast<CGroupEditBox*>(parentGroup->getGroup("standard_price:quantity:edit:eb"));
		if(eb)
		{
			// set focus
			setFocusOnEditBox(eb);
		}
	}
}


// ***************************************************************************
void	CBotChatPageTrade::setupPriceOrPriceRetire(bool sellMode, CDBCtrlSheet *sheet)
{
	nlassert(sheet);

	// default
	_UsePriceRetire= false;
	_CannotValidateBecauseRetireNotAvailable= false;

	// must be a buy, must be an item, and must be in a standard BuyMean
	const CItemSheet	*pIS= sheet->asItemSheet();
	if( !sellMode && pIS && _BuyMean == Money )
	{
		// test SELLER_TYPE. if User or ResaleAndUser, then this item belongs to us
		// => just pay the PRICE_RETIRE, not the Full Price
		sint32	st= sheet->getItemSellerType();
		if( st==BOTCHATTYPE::UserRetirable || st==BOTCHATTYPE::ResaleAndUserRetirable ||
			st==BOTCHATTYPE::User || st==BOTCHATTYPE::ResaleAndUser )
		{
			_UsePriceRetire= true;
			// if not retirable, can't validate
			if(st==BOTCHATTYPE::User || st==BOTCHATTYPE::ResaleAndUser)
				_CannotValidateBecauseRetireNotAvailable= true;
		}

		// Check that we can take cosmetic that cannot be applied to the player
		if (pIS->Family == ITEMFAMILY::COSMETIC)
		{
			EGSPD::CPeople::TPeople people = ITEM_ORIGIN::itemOriginStringToPeopleEnum( ITEM_ORIGIN::enumToString( pIS->ItemOrigin ) );
			if (UserEntity->getGender() != pIS->Cosmetic.Gender || UserEntity->people() != people )
				_CannotValidateBecauseRetireNotAvailable= true;
		}
	}
}

// ***************************************************************************
void	CBotChatPageTrade::setupFactionPointPrice(bool /* sellMode */, uint defaultQuantity, CInterfaceGroup *parentGroup, CDBCtrlSheet *sheet)
{
	nlassert(sheet);

	// if window not found, quit
	if(!parentGroup)
		return;

	// get the faction point cost and type
	CInterfaceGroup	*fpGroup= parentGroup->getGroup("faction_price");
	CViewBase *upView= parentGroup->getView("standard_price:unit_price_header");
	CInterfaceGroup *upGroup= parentGroup->getGroup("standard_price:unit_price");

	if(!fpGroup || !upView || !upGroup)
		return;

	// get the current faction point cost and type
	PVP_CLAN::TPVPClan	fpType;
	uint32				fpCost;
	getItemFactionTypePoints(sheet, fpType, fpCost);
	if(fpCost==0 || fpType==PVP_CLAN::None)
	{
		// disable faction price group
		fpGroup->setActive(false);
		upView->setActive(true);
		upGroup->setActive(true);
	}
	else
	{

		//Disable Price
		upView->setActive(false);
		upGroup->setActive(false);

		// enable faction price gropu
		fpGroup->setActive(true);

		// Setup quantity view in this group
		setupPriceGroupQuantity(fpGroup, defaultQuantity);

		// setup faction name according to pvp clan
		string		factionName= PVP_CLAN::toString(fpType);
		CViewText	*vt= dynamic_cast<CViewText*>(fpGroup->getView("unit_price_header"));
		if(vt)
		{
			ucstring	fmt= CI18N::get("uiUnitFPPrice");
			strFindReplace(fmt, "%fac", factionName);
			vt->setText(fmt);
		}
		vt= dynamic_cast<CViewText*>(fpGroup->getView("total_price_header"));
		if(vt)
		{
			ucstring	fmt= CI18N::get("uiTotalFPPrice");
			strFindReplace(fmt, "%fac", factionName);
			vt->setText(fmt);
		}

		// setup icon according to pvp clan
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		factionName = NLMISC::toLower(factionName);
		string	factionIcon= CWidgetManager::getInstance()->getParser()->getDefine(toString("faction_icon_%s", factionName.c_str()));
		CViewBitmap		*vBmp= dynamic_cast<CViewBitmap*>(fpGroup->getView("unit_price:item_price:icone"));
		if(vBmp)	vBmp->setTexture(factionIcon);
		vBmp= dynamic_cast<CViewBitmap*>(fpGroup->getView("total_price:item_price:icone"));
		if(vBmp)	vBmp->setTexture(factionIcon);
	}

}

// ***************************************************************************
void	CBotChatPageTrade::startChangeBuyFilterDialog(const std::string &dbext, const std::string &title, uint32 maxValue)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	_FilterBuyDlgDBExt= dbext;
	_FilterBuyDlgMaxValue= maxValue;

	// change the title
	CInterfaceGroup		*ig= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_CHANGE_BUY_FILTER));
	if(!ig)	return;
	CViewText	*vtitle= dynamic_cast<CViewText*>(ig->getView("title"));
	if(vtitle)
		vtitle->setHardText(title);

	// init min and max edit box
	CGroupEditBox	*edMin = dynamic_cast<CGroupEditBox *>(ig->getGroup("edit_min:eb"));
	CGroupEditBox	*edMax = dynamic_cast<CGroupEditBox *>(ig->getGroup("edit_max:eb"));
	CCDBNodeLeaf	*dbRangeMin= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_") + _FilterBuyDlgDBExt, false);
	CCDBNodeLeaf	*dbRangeMax= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_") + _FilterBuyDlgDBExt, false);
	sint	rangeMin= 0;
	sint	rangeMax= 0;
	if(dbRangeMin)	rangeMin= dbRangeMin->getValue32();
	if(dbRangeMax)	rangeMax= dbRangeMax->getValue32();
	if(edMin)	edMin->setInputString(toString(rangeMin));
	if(edMax)	edMax->setInputString(toString(rangeMax));
	if(edMin)	edMin->setPositiveIntegerMaxValue(maxValue);
	if(edMax)	edMax->setPositiveIntegerMaxValue(maxValue);

	// set focus on MIN edit box
	setFocusOnEditBox(ig->getGroup("edit_min:eb"));

	// go
	CWidgetManager::getInstance()->enableModalWindow(NULL, ig);
}

// ***************************************************************************
void	CBotChatPageTrade::resetBuyFilterDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// get the modal window
	CInterfaceGroup		*ig= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_CHANGE_BUY_FILTER));
	if(!ig)	return;

	// reset the edited values
	CGroupEditBox	*edMin = dynamic_cast<CGroupEditBox *>(ig->getGroup("edit_min:eb"));
	CGroupEditBox	*edMax = dynamic_cast<CGroupEditBox *>(ig->getGroup("edit_max:eb"));
	CCDBNodeLeaf	*dbRangeMin= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_") + _FilterBuyDlgDBExt, false);
	CCDBNodeLeaf	*dbRangeMax= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_") + _FilterBuyDlgDBExt, false);
	sint	rangeMin= 0;
	sint	rangeMax= _FilterBuyDlgMaxValue;
	// write result in EditBox, and in db
	if(edMin)	edMin->setInputString(toString(rangeMin));
	if(edMax)	edMax->setInputString(toString(rangeMax));
	if(dbRangeMin)	dbRangeMin->setValue32(rangeMin);
	if(dbRangeMax)	dbRangeMax->setValue32(rangeMax);

	// reset validate also the filter (as in confirm)
	sendCurrentBuyFilterToServer(true);

	// and leave modal
	CWidgetManager::getInstance()->disableModalWindow();
}

// ***************************************************************************
void	CBotChatPageTrade::confirmChangeBuyFilterDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// get the modal window
	CInterfaceGroup		*ig= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_CHANGE_BUY_FILTER));
	if(!ig)	return;

	// retrieve the edited values
	CGroupEditBox	*edMin = dynamic_cast<CGroupEditBox *>(ig->getGroup("edit_min:eb"));
	CGroupEditBox	*edMax = dynamic_cast<CGroupEditBox *>(ig->getGroup("edit_max:eb"));
	CCDBNodeLeaf	*dbRangeMin= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_") + _FilterBuyDlgDBExt, false);
	CCDBNodeLeaf	*dbRangeMax= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_") + _FilterBuyDlgDBExt, false);
	sint	rangeMin= 0;
	sint	rangeMax= 0;
	if(edMin)	rangeMin= edMin->getInputStringAsInt();
	if(edMax)	rangeMax= edMax->getInputStringAsInt();
	// only positive values are posssible here
	rangeMin= max(0, rangeMin);
	rangeMax= max(rangeMin, rangeMax);

	// write result in DB
	if(dbRangeMin)	dbRangeMin->setValue32(rangeMin);
	if(dbRangeMax)	dbRangeMax->setValue32(rangeMax);

	// Then send new filter to Server, and reset item list
	sendCurrentBuyFilterToServer(true);

	// and leave modal
	CWidgetManager::getInstance()->disableModalWindow();
}

// ***************************************************************************
void	CBotChatPageTrade::giveFocusToMaxEBChangeBuyFilterDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// get the modal window
	CInterfaceGroup		*ig= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_CHANGE_BUY_FILTER));
	if(!ig)	return;

	// set focus on max
	setFocusOnEditBox(ig->getGroup("edit_max:eb"));
}


// ***************************************************************************
void		CBotChatPageTrade::startChangeBuyFilterMPDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// show the modal
	CInterfaceGroup		*ig= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_CHANGE_BUY_FILTER_MP));
	if(!ig)	return;

	// go
	CWidgetManager::getInstance()->enableModalWindow(NULL, ig);
}


// ***************************************************************************
void		CBotChatPageTrade::confirmChangeBuyFilterMPDialog(uint ft)
{
	ft= min(ft, (uint)RM_FABER_TYPE::Unknown);

	// set ItemPart DB
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf	*dbItemPart= NLGUI::CDBManager::getInstance()->getDbProp(string(DB_BOT_CHAT_BASE_BUY_FILTER":MP_ITEM_PART"), false);
	if(dbItemPart)	dbItemPart->setValue32(ft);

	// Then send new filter to Server, and reset item list
	sendCurrentBuyFilterToServer(true);

	// and leave modal
	CWidgetManager::getInstance()->disableModalWindow();
}


// ***************************************************************************
void	CBotChatPageTrade::resetItemPartAndTypeFilters()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CCDBNodeLeaf	*dbItemPart= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MP_ITEM_PART" , false);
	CCDBNodeLeaf	*dbItemType= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":ITEM_TYPE" , false);

	if(dbItemPart)	dbItemPart->setValue32(RM_FABER_TYPE::Unknown);
	if(dbItemType)	dbItemType->setValue32(ITEM_TYPE::UNDEFINED);
}

// ***************************************************************************
void	CBotChatPageTrade::sendCurrentBuyFilterToServer(bool resetBuyList)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// **** retrieve current DB values
	CCDBNodeLeaf	*dbQualityMin= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_QUALITY" , false);
	CCDBNodeLeaf	*dbQualityMax= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_QUALITY" , false);
	CCDBNodeLeaf	*dbPriceMin= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_PRICE" , false);
	CCDBNodeLeaf	*dbPriceMax= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_PRICE" , false);
	CCDBNodeLeaf	*dbClassMin= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_CLASS" , false);
	CCDBNodeLeaf	*dbClassMax= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_CLASS" , false);
	CCDBNodeLeaf	*dbItemPart= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MP_ITEM_PART" , false);
	CCDBNodeLeaf	*dbItemType= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":ITEM_TYPE" , false);

	uint32			qualityMin=0, qualityMax=0;
	uint32			priceMin=0, priceMax=0;
	uint8			classMin= 0, classMax= RM_CLASS_TYPE::NumTRMClassType-1;	// max class is 4
	// Yoyo: XML is hardcoded for max_class==4. (config.xml and bot_chat_v4.xml)
	nlctassert(RM_CLASS_TYPE::NumTRMClassType-1==4);
	uint8			itemPart= RM_FABER_TYPE::Unknown;
	uint8			itemType= ITEM_TYPE::UNDEFINED;
	if(dbQualityMin)	qualityMin= dbQualityMin->getValue32();
	if(dbQualityMax)	qualityMax= dbQualityMax->getValue32();
	if(dbPriceMin)		priceMin= dbPriceMin->getValue32();
	if(dbPriceMax)		priceMax= dbPriceMax->getValue32();
	if(dbClassMin)		classMin= (uint8)dbClassMin->getValue32();
	if(dbClassMax)		classMax= (uint8)dbClassMax->getValue32();
	if(dbItemPart)		itemPart= (uint8)dbItemPart->getValue32();
	if(dbItemType)		itemType= (uint8)dbItemType->getValue32();

	// **** send msg
	const char *msg="BOTCHAT:SET_FILTERS";
	NLMISC::CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
	{
		out.serial(qualityMin);
		out.serial(qualityMax);
		out.serial(priceMin);
		out.serial(priceMax);
		out.serial(classMin);
		out.serial(classMax);
		out.serial(itemPart);
		out.serial(itemType);
		NetMngr.push(out);
		//nlinfo("impulseCallBack sent: %s %d %d %d %d %d %d %d %d", msg, qualityMin, qualityMax, priceMin, priceMax, classMin, classMax, itemPart, itemType);
	}
	else
		nlwarning(" unknown message name '%s'", msg);


	// **** Server has incremented SessionId, do it on our side
	// increment session id on our side
	CBotChatManager::getInstance()->incrementSessionID();

	// and reset hugeListObs
	if(resetBuyList)
	{
		// Start the HugeList
		_TradePagesObs.start();

		// update interface
		CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(_BuyOnly ? WIN_BOT_CHAT_PAGE_BUY : WIN_BOT_CHAT_PAGE_TRADE));
		if (gc)
		{
			// unselect just buy list (don't need for sell)
			unselectBuyList(gc);
		}
	}

}

// ***************************************************************************
void		CBotChatPageTrade::unselectBuyList(CGroupContainer *gc)
{
	CInterfaceGroup *buyGroup = gc->getGroup("buy");
	if (buyGroup)
	{
		// unselect NPC/Player group
		CDBGroupListSheetText *buyListSheet = dynamic_cast<CDBGroupListSheetText *>(buyGroup->getGroup("general_buy"));
		if (buyListSheet)
			buyListSheet->unselect();
		// unselect Player group
		buyListSheet = dynamic_cast<CDBGroupListSheetText *>(buyGroup->getGroup("player_buy"));
		if (buyListSheet)
			buyListSheet->unselect();
	}
}

// ***************************************************************************
void		CBotChatPageTrade::unselectSellList(CGroupContainer *gc)
{
	CInterfaceGroup *sellGroup = gc->getGroup("sell");
	if (sellGroup)
	{
		// unselect the bag
		CDBGroupListSheetText *sellListSheet = dynamic_cast<CDBGroupListSheetText *>(sellGroup->getGroup("bag_sell"));
		if (sellListSheet)
			sellListSheet->unselect();
		// unselect all pack animals
		for(uint i=0;i<MAX_INVENTORY_ANIMAL;i++)
		{
			sellListSheet = dynamic_cast<CDBGroupListSheetText *>(sellGroup->getGroup(toString("beast%d_sell", i)));
			if (sellListSheet)
				sellListSheet->unselect();
		}
	}
}

// ***************************************************************************
void		CBotChatPageTrade::refreshResale()
{
	// **** send msg
	const char *msg="BOTCHAT:REFRESH_TRADE_LIST";
	NLMISC::CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
	{
		NetMngr.push(out);
		//nlinfo("impulseCallBack sent: %s", msg);
	}
	else
		nlwarning(" unknown message name '%s'", msg);


	// **** Server has incremented SessionId, do it on our side
	// increment session id on our side
	CBotChatManager::getInstance()->incrementSessionID();

	// and reset hugeListObs
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Start the HugeList
		_TradePagesObs.start();

		// update interface
		CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(_BuyOnly ? WIN_BOT_CHAT_PAGE_BUY : WIN_BOT_CHAT_PAGE_TRADE));
		if (gc)
		{
			// unselect just buy list (don't need for sell)
			unselectBuyList(gc);
		}
	}
}


// ***************************************************************************
void		CBotChatPageTrade::startChangeBuyFilterClassDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Copy from save to Temp edition
	CCDBNodeLeaf	*dbClassMin= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_CLASS" , false);
	CCDBNodeLeaf	*dbClassMax= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_CLASS" , false);
	CCDBNodeLeaf	*tempClassMin= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:BOTCHAT:TEMP_FILTER_MIN_CLASS");
	CCDBNodeLeaf	*tempClassMax= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:BOTCHAT:TEMP_FILTER_MAX_CLASS");
	if(dbClassMin) tempClassMin->setValue32(dbClassMin->getValue32());
	if(dbClassMax) tempClassMax->setValue32(dbClassMax->getValue32());

	// show the modal
	CInterfaceGroup		*ig= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_CHANGE_BUY_FILTER_CLASS));
	if(!ig)	return;

	// go
	CWidgetManager::getInstance()->enableModalWindow(NULL, ig);
}


// ***************************************************************************
void		CBotChatPageTrade::resetBuyFilterClassDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Copy from save to Temp edition
	CCDBNodeLeaf	*dbClassMin= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_CLASS" , false);
	CCDBNodeLeaf	*dbClassMax= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_CLASS" , false);
	CCDBNodeLeaf	*tempClassMin= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:BOTCHAT:TEMP_FILTER_MIN_CLASS");
	CCDBNodeLeaf	*tempClassMax= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:BOTCHAT:TEMP_FILTER_MAX_CLASS");
	// write in both DB
	tempClassMin->setValue32(0);
	tempClassMax->setValue32(RM_CLASS_TYPE::NumTRMClassType-1);
	if(dbClassMin) dbClassMin->setValue32(0);
	if(dbClassMax) dbClassMax->setValue32(RM_CLASS_TYPE::NumTRMClassType-1);

	// Reset also validate the filter, commit
	sendCurrentBuyFilterToServer(true);

	// and leave modal
	CWidgetManager::getInstance()->disableModalWindow();
}

// ***************************************************************************
void		CBotChatPageTrade::confirmChangeBuyFilterClassDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Copy from temp edit to final
	CCDBNodeLeaf	*dbClassMin= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MIN_CLASS" , false);
	CCDBNodeLeaf	*dbClassMax= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":MAX_CLASS" , false);
	CCDBNodeLeaf	*tempClassMin= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:BOTCHAT:TEMP_FILTER_MIN_CLASS");
	CCDBNodeLeaf	*tempClassMax= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:BOTCHAT:TEMP_FILTER_MAX_CLASS");
	sint	minClass= tempClassMin->getValue32();
	sint	maxClass= tempClassMax->getValue32();
	// min must be => 0 and max must be >= min
	minClass= max(minClass, 0);
	maxClass= max(minClass, maxClass);
	if(dbClassMin) dbClassMin->setValue32(minClass);
	if(dbClassMax) dbClassMax->setValue32(maxClass);

	// Then send new filter to Server, and reset item list
	sendCurrentBuyFilterToServer(true);

	// and leave modal
	CWidgetManager::getInstance()->disableModalWindow();
}


// ***************************************************************************
std::string	CBotChatPageTrade::getItemSheetNameForItemType(ITEM_TYPE::TItemType it)
{
	// The definition is in .xml
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	string	itemTypeDef= "item_type_to_item_sheet_";
	itemTypeDef+= ITEM_TYPE::toString(it);

	// return empty string if not found
	return CWidgetManager::getInstance()->getParser()->getDefine(itemTypeDef);
}


// ***************************************************************************
void		CBotChatPageTrade::startChangeBuyFilterItemTypeDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// the list of possible item type to select is filled when the DB of bitfield change

	// show the modal
	CInterfaceGroup		*ig= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_CHANGE_BUY_FILTER_ITEM_TYPE));
	if(!ig)	return;

	// go
	CWidgetManager::getInstance()->enableModalWindow(NULL, ig);
}

// ***************************************************************************
void		CBotChatPageTrade::confirmChangeBuyFilterItemTypeDialog(ITEM_TYPE::TItemType itemType)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Copy result to final DB
	CCDBNodeLeaf	*dbItemType= NLGUI::CDBManager::getInstance()->getDbProp(DB_BOT_CHAT_BASE_BUY_FILTER":ITEM_TYPE" , false);
	if(dbItemType) dbItemType->setValue32(itemType);

	// Then send new filter to Server, and reset item list
	sendCurrentBuyFilterToServer(true);

	// and leave modal
	CWidgetManager::getInstance()->disableModalWindow();
}

// ***************************************************************************
void		CBotChatPageTrade::startDestroyItemDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(!_CurrItemSheet)
		return;

	// setup the quantity to destroy (if edited correctly)
	uint32	quantity= getCurrItemQuantity();
	if(quantity==0 || quantity==std::numeric_limits<uint32>::max())
		return;
	// if quantity check, maximize with it (if user entered to big value...)
	if(_QuantityCheck)
		quantity= min(quantity, (uint32)_QuantityCheck);
	// set view
	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TRADE_ITEM:DESTROY_QUANTITY")->setValue32(quantity);

	// show the modal
	CInterfaceGroup		*ig= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_DESTROY_ITEM));
	if(!ig)	return;
	CWidgetManager::getInstance()->enableModalWindow(NULL, ig);
}

// ***************************************************************************
void		CBotChatPageTrade::confirmDestroyItemDialog()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(!_CurrItemSheet)
		return;

	// get the quantity destroyed
	uint32	quantity= getCurrItemQuantity();
	// if correct quantity
	if(quantity!=0 && quantity!=std::numeric_limits<uint32>::max())
	{
		// if quantity check, maximize with it (if user entered too big value...)
		if(_QuantityCheck)
			quantity= min(quantity, (uint32)_QuantityCheck);

		// **** then send msg to server
		uint16	u16Index    = (uint16) _CurrItemIndex;
		uint16	u16Quantity	= (uint16) quantity;

		const char *msg="BOTCHAT:DESTROY_ITEM";
		NLMISC::CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
		{
			out.serial(u16Index);
			out.serial(u16Quantity);
			NetMngr.push(out);
			//nlinfo("impulseCallBack sent: %s %d %d", msg, u16Index, u16Quantity);
		}
		else
			nlwarning(" unknown message name '%s'", msg);


		// **** destroy localy
		// if item stackable, the player was allowed to destroy only a subset
		if(_QuantityEdit)
		{
			sint32	curQuantity= _CurrItemSheet->getQuantity();
			if(u16Quantity<curQuantity)
				// lower the quantity
				_CurrItemSheet->setQuantity(curQuantity-u16Quantity);
			else
				// hide the item
				_CurrItemSheet->setSheetId(0);
		}
		else
		{
			// hide the item
			_CurrItemSheet->setSheetId(0);
		}
	}

	// in all cases, close the modal
	CWidgetManager::getInstance()->disableModalWindow();

	// if the quantity entered was correct
	if(quantity!=0 && quantity!=std::numeric_limits<uint32>::max())
	{
		// close also the container modal
		endTradeModal();
	}

}


/////////////////////
// ACTION HANDLERS //
/////////////////////

// ***************************************************************************************
// the player has clicked on an item to buy it
class CAHBuyItem : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* params */)
	{
		CDBCtrlSheet *sheet = dynamic_cast<CDBCtrlSheet *>(pCaller);
		BotChatPageAll->Trade->startBuyDialog(sheet, pCaller);
	}
};
REGISTER_ACTION_HANDLER(CAHBuyItem, "buy_item");

// ***************************************************************************************
// the player has clicked on an item to sell it
class CAHSellItem : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* params */)
	{
		CDBCtrlSheet *sheet = dynamic_cast<CDBCtrlSheet *>(pCaller);
		BotChatPageAll->Trade->startSellDialog(sheet, pCaller);
	}
};
REGISTER_ACTION_HANDLER(CAHSellItem, "sell_item");


// ***************************************************************************************
// confirm sell or buy item
class CAHConfirmTrade : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// parse resale param
		bool enableResale;
		uint resaleParam;
		fromString(getParam(params, "resale"), resaleParam);
		switch ( resaleParam )
		{
		case 0:		enableResale = false; break;
		case 1:		enableResale = true; break;
		default: // 2: comes only from Enter of Shift+Enter key from an edit box or in the modal window
			{
				const NLGUI::CEventDescriptorKey& keyEvent = CWidgetManager::getInstance()->getLastKeyEvent();
				enableResale = ! keyEvent.getKeyShift();
			}
		}

		// test if we need to confirm the buy?
		bool	canTestConfirmation;
		fromString(getParam(params, "noconfirm"), canTestConfirmation);
		canTestConfirmation = !canTestConfirmation;
		bool	mustConfirm= false;
		if(canTestConfirmation)
		{
			// Building:
			if(NLGUI::CDBManager::getInstance()->getDbValue32("SERVER:TRADING:BUILDING_LOSS_WARNING") == 1)
				mustConfirm= true;
		}

		// don't confirm?
		if(!mustConfirm)
		{
			if (BotChatPageAll->Trade->isTradeValid( enableResale ))
				BotChatPageAll->Trade->confirmTrade( enableResale );
		}
		else
		{
			// need to ask user first. Important unexecpected consequence, so use a warning icon
			pIM->validMessageBox(CInterfaceManager::WarningIconMsg, CI18N::get("uiQReplaceAppartment"),
				"confirm_trade", toString("resale=%d|noconfirm=1", (uint)enableResale));
		}
	}
};
REGISTER_ACTION_HANDLER(CAHConfirmTrade, "confirm_trade");


// ***************************************************************************************
// Trade has been canceled
class CAHCancelTrade : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->cancelTrade();
	}
};
REGISTER_ACTION_HANDLER(CAHCancelTrade, "cancel_trade");


// ***************************************************************************
// Request to Change the Min/Max of Price or quantity
class CAHEnterChangeBotChatBuyFilter : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &params)
	{
		string dbext= getParam(params, "dbext");
		string title= getParam(params, "title");
		uint32 maxvalue;
		fromString(getParam(params, "maxvalue"), maxvalue);
		BotChatPageAll->Trade->startChangeBuyFilterDialog(dbext, title, maxvalue);
	}
};
REGISTER_ACTION_HANDLER(CAHEnterChangeBotChatBuyFilter, "enter_change_botchat_buy_filter");


// ***************************************************************************
// Validate Change the Min/Max of Price or quantity
class CAHConfirmChangeBotChatBuyFilter : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->confirmChangeBuyFilterDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHConfirmChangeBotChatBuyFilter ,"confirm_change_botchat_buy_filter");


// ***************************************************************************
// Reset the Min/Max of Price or quantity
class CAHResetBotChatBuyFilter : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->resetBuyFilterDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHResetBotChatBuyFilter, "reset_botchat_buy_filter");


// ***************************************************************************
// Min EditBox validated, give focus to max EditBox.
class CAHChangeBotChatBuyFilterGiveFocusToMax : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->giveFocusToMaxEBChangeBuyFilterDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHChangeBotChatBuyFilterGiveFocusToMax ,"change_botchat_buy_filter_give_focus_to_max");



// ***************************************************************************
// Request to Change the Min/Max of Price or quantity
class CAHEnterChangeBotChatBuyFilterMP : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->startChangeBuyFilterMPDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHEnterChangeBotChatBuyFilterMP, "enter_change_botchat_buy_filter_mp");


// ***************************************************************************
// Validate Change the Min/Max of Price or quantity
class CAHConfirmChangeBotChatBuyFilterMP : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const string &params)
	{
		BotChatPageAll->Trade->confirmChangeBuyFilterMPDialog(RM_FABER_TYPE::toFaberType(params));
	}
};
REGISTER_ACTION_HANDLER(CAHConfirmChangeBotChatBuyFilterMP, "confirm_change_botchat_buy_filter_mp");


// ***************************************************************************
// Ask for Refresh of the User list
class CAHBotChatRefreshResale : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->refreshResale();
	}
};
REGISTER_ACTION_HANDLER(CAHBotChatRefreshResale, "botchat_refresh_resale");


// ***************************************************************************
// client requires change of Item Class bounds
class CAHEnterChangeBotChatBuyFilterClass : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->startChangeBuyFilterClassDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHEnterChangeBotChatBuyFilterClass, "enter_change_botchat_buy_filter_class");


// ***************************************************************************
// client confirms change of Item Class bounds
class CAHConfirmChangeBotChatBuyFilterClass : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->confirmChangeBuyFilterClassDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHConfirmChangeBotChatBuyFilterClass, "confirm_change_botchat_buy_filter_class");


// ***************************************************************************
// client confirms change of Item Class bounds
class CAHResetBotChatBuyFilterClass : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->Trade->resetBuyFilterClassDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHResetBotChatBuyFilterClass, "reset_botchat_buy_filter_class");


// ***************************************************************************
class CHandlerBotChatTTItemType : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// \todo yoyo: for now disable tooltip
		CWidgetManager::getInstance()->setContextHelpText(ucstring());
	}
};
REGISTER_ACTION_HANDLER(CHandlerBotChatTTItemType, "botchat_tt_item_type");


// ***************************************************************************
class CAHEnterChangeBotchatBuyFilterItemType : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		BotChatPageAll->Trade->startChangeBuyFilterItemTypeDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHEnterChangeBotchatBuyFilterItemType, "enter_change_botchat_buy_filter_item_type");


// ***************************************************************************
class CAHConfirmChangeBotchatBuyFilterItemType : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		ITEM_TYPE::TItemType	itemType= ITEM_TYPE::UNDEFINED;

		// From the index of the ctrl sheet in DB, we can now the item type setuped
		CDBCtrlSheet	*ctrlSheet= dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(ctrlSheet)
		{
			// if it is not the empty sheet (detect empty sheetId)
			if(ctrlSheet->getSheetId())
			{
				sint	index= ctrlSheet->getIndexInDB();
				if(index>=0 && index<ITEM_TYPE::UNDEFINED)
					itemType= (ITEM_TYPE::TItemType)index;
			}
		}

		// then change the filter
		BotChatPageAll->Trade->confirmChangeBuyFilterItemTypeDialog(itemType);
	}
};
REGISTER_ACTION_HANDLER(CAHConfirmChangeBotchatBuyFilterItemType, "confirm_change_botchat_buy_filter_item_type");

// ***************************************************************************
class CAHBotChatRefilItemTypeChoiceList : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		string	destDB= getParam(sParams, "dest");
		string	srcDB= getParam(sParams, "src");
		uint	offset;
		fromString(getParam(sParams, "offset"), offset);

		/* If ItemType grows too big, must change SERVER and CLIENT!!! because of ItemType bitfield system:
			use SERVER:TRADING:ITEM_TYPE_SELLER_BITFILED_0_63 which is a 64 bits
				SERVER:TRADING:ITEM_TYPE_SELLER_BITFILED_64_127 which is a 64 bits
			(param offset gives which part of enum to use)
		*/
		nlctassert(ITEM_TYPE::UNDEFINED<=128);

		// get the src bitfield
		CCDBNodeLeaf	*nodeSrc= NLGUI::CDBManager::getInstance()->getDbProp(srcDB, false);
		if(!nodeSrc)
			return;
		uint64	bfItemType= nodeSrc->getValue64();

		// For all item type possible
		for(uint i=0;i<64;i++)
		{
			bool present= ((bfItemType>>i)&1)!=0;
			// get the dest node
			CCDBNodeLeaf	*nodeDst= NLGUI::CDBManager::getInstance()->getDbProp(destDB+toString(":%d:SHEET", i+offset), false);
			if(nodeDst)
			{
				if(present)
				{
					// get the item sheet id associated to this item type
					CSheetId	sheetId;
					sheetId.buildSheetId(CBotChatPageTrade::getItemSheetNameForItemType((ITEM_TYPE::TItemType)(i+offset)));
					// and set it
					nodeDst->setValue32(sheetId.asInt());
				}
				else
				{
					// reset (cannot select this item type)
					nodeDst->setValue32(0);
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CAHBotChatRefilItemTypeChoiceList, "botchat_refill_item_type_choice_list");


// ***************************************************************************
class CAHBotChatEnterDestroy : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		BotChatPageAll->Trade->startDestroyItemDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHBotChatEnterDestroy, "botchat_enter_destroy");

// ***************************************************************************
class CAHBotChatConfirmDestroy : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		BotChatPageAll->Trade->confirmDestroyItemDialog();
	}
};
REGISTER_ACTION_HANDLER(CAHBotChatConfirmDestroy, "botchat_confirm_destroy");

// ***************************************************************************
class CAHBotChatChangeResaleMargin : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup *ig = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_SELL_BUY_ITEM));
		if (!ig) return;
		CGroupEditBox *resaleMarginBox = dynamic_cast<CGroupEditBox *>(ig->getGroup("header_opened:resell_group:can_resell:choose_resell:edit:eb"));
		if (resaleMarginBox)
		{
			sint32 delta;
			fromString(sParams, delta);
			sint32 newValue = resaleMarginBox->getInputStringAsInt() + delta;
			clamp(newValue, 0, (sint32)CBotChatPageTrade::MaxResaleMargin);
			resaleMarginBox->setInputStringAsInt( newValue );
		}
	}
};
REGISTER_ACTION_HANDLER(CAHBotChatChangeResaleMargin, "change_resale_margin");

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getPriceWithFame)
{
	if (args.size() != 2) return false;
	if (!args[0].toInteger()) return false;
	if (!args[1].toInteger()) return false;

	sint	value= (sint)args[0].getInteger();
	sint	valueFame= (sint)args[1].getInteger();
	if(value==-1)
		result.setUCString(CI18N::get("uiBadPrice"));
	else if(value==valueFame)
		result.setUCString(NLMISC::formatThousands(toString(value)));
	else
		result.setUCString(NLMISC::formatThousands(toString(valueFame)) + " (" + NLMISC::formatThousands(toString(value)) + ")");

	return true;
}
REGISTER_INTERFACE_USER_FCT("getPriceWithFame", getPriceWithFame)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getBonusOnResale)
{
	if (args.size() != 2) return false;
	if (!args[0].toInteger()) return false;
	if (!args[1].toInteger()) return false;

	sint	valueHigh= (sint)args[0].getInteger();
	sint	valueLow= (sint)args[1].getInteger();
	sint	diff = valueHigh - valueLow;
	result.setUCString("+" + NLMISC::formatThousands(toString(diff)));

	return true;
}
REGISTER_INTERFACE_USER_FCT("getBonusOnResale", getBonusOnResale)

//////////////
// COMMANDS //
//////////////

#if !FINAL_VERSION
// TEMP TEMP TEMP

NLMISC_COMMAND( testColorItems, "Temp : test some items to trade", "" )
{
	if (args.size() < 1) return false;
	if (args.size() > 2) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	std::string itemName = "icfahv.sitem";
	if (args.size() > 1)
	{
		itemName = args[1];
	}
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SHEET")->setValue32(CSheetId(itemName).asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:PRICE")->setValue32(100);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:USER_COLOR")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SHEET")->setValue32(CSheetId(itemName).asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:USER_COLOR")->setValue32(1);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SHEET")->setValue32(CSheetId(itemName).asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:USER_COLOR")->setValue32(2);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SHEET")->setValue32(CSheetId(itemName).asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SLOT_TYPE")->setValue32(0);//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:USER_COLOR")->setValue32(3);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SHEET")->setValue32(CSheetId(itemName).asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:PRICE")->setValue32(100);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:USER_COLOR")->setValue32(4);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SHEET")->setValue32(CSheetId(itemName).asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:USER_COLOR")->setValue32(5);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SHEET")->setValue32(CSheetId(itemName).asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:USER_COLOR")->setValue32(6);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SHEET")->setValue32(CSheetId(itemName).asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:USER_COLOR")->setValue32(7);
	//
	sint32 value;
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:SESSION")->setValue32(CBotChatManager::getInstance()->getSessionID());
	fromString(args[0], value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:PAGE_ID")->setValue32(value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:HAS_NEXT")->setValue32(1);
	//
	return true;
}


NLMISC_COMMAND( testTradeItems, "Temp : test some items to trade", "" )
{
	if (args.size() != 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	//NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SHEET")->setValue32(CSheetId("icfm1bm.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SHEET")->setValue32(CSheetId("guild_main_building.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:PRICE")->setValue32(100);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SLOT_TYPE")->setValue32(0);
	//
	//NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SHEET")->setValue32(CSheetId("icfm1bs.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SHEET")->setValue32(CSheetId("guild_rm_craft.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SLOT_TYPE")->setValue32(0);
	//
	//NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SHEET")->setValue32(CSheetId("icfm1pd.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SHEET")->setValue32(CSheetId("guild_rm_fight.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:NAMEID")->setValue32(8);
	//
	//NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SHEET")->setValue32(CSheetId("icfr2l.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SHEET")->setValue32(CSheetId("guild_rm_harvest.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SLOT_TYPE")->setValue32(0);//
	//
	//NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SHEET")->setValue32(CSheetId("ictr2b.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SHEET")->setValue32(CSheetId("guild_rm_magic.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:PRICE")->setValue32(100);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SHEET")->setValue32(CSheetId("icfp1pb.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SHEET")->setValue32(CSheetId("icragt.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SHEET")->setValue32(CSheetId("icmss.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SLOT_TYPE")->setValue32(0);
	//
	sint32 value;
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:SESSION")->setValue32(CBotChatManager::getInstance()->getSessionID());
	fromString(args[0], value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:PAGE_ID")->setValue32(value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:HAS_NEXT")->setValue32(1);
	//
	return true;
}


NLMISC_COMMAND( testTradeItems2, "Temp : test some items to trade (2)", "" )
{
	// item with faction points
	if (args.size() != 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SHEET")->setValue32(CSheetId("icfm1bm.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:PRICE")->setValue32(100);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:FACTION_TYPE")->setValue32(0);
//	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:PRICE")->setValue32(130);		// should result to none
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SHEET")->setValue32(CSheetId("icfm1bs.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:FACTION_TYPE")->setValue32(PVP_CLAN::Kami);
//	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:FACTION_POINT_PRICE")->setValue32(0);		// should result to none
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SHEET")->setValue32(CSheetId("icfm1pd.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:PRICE")->setValue32(10);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:NAMEID")->setValue32(8);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:FACTION_TYPE")->setValue32(PVP_CLAN::Kami);
//	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:FACTION_POINT_PRICE")->setValue32(13);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SHEET")->setValue32(CSheetId("icfr2l.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SLOT_TYPE")->setValue32(0);//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:FACTION_TYPE")->setValue32(PVP_CLAN::Tryker);
//	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:FACTION_POINT_PRICE")->setValue32(10);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SHEET")->setValue32(CSheetId("ictr2b.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:FACTION_TYPE")->setValue32(PVP_CLAN::Fyros);
//	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:FACTION_POINT_PRICE")->setValue32(8);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SHEET")->setValue32(CSheetId("icfp1pb.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:FACTION_TYPE")->setValue32(PVP_CLAN::Tryker);
//	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:FACTION_POINT_PRICE")->setValue32(7);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SHEET")->setValue32(CSheetId("icragt.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:FACTION_TYPE")->setValue32(PVP_CLAN::Karavan);
//	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:FACTION_POINT_PRICE")->setValue32(1);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SHEET")->setValue32(CSheetId("icmss.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SLOT_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:FACTION_TYPE")->setValue32(PVP_CLAN::Kami);
//	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:FACTION_POINT_PRICE")->setValue32(1350);
	//
	sint32 value;
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:SESSION")->setValue32(CBotChatManager::getInstance()->getSessionID());
	fromString(args[0], value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:PAGE_ID")->setValue32(value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:HAS_NEXT")->setValue32(1);
	//
	return true;
}

NLMISC_COMMAND( testTradeItems3, "Temp : test some items to trade (3)", "" )
{
	// items
	CInterfaceManager *im = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:0:SHEET")->setValue32(CSheetId("fyros_buckler_lvl_01_05.item").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:0:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:0:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:0:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:1:SHEET")->setValue32(CSheetId("fyros_buckler_lvl_01_05.item").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:1:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:1:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:1:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:2:SHEET")->setValue32(CSheetId("fyros_buckler_lvl_01_05.item").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:2:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:2:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:2:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:3:SHEET")->setValue32(CSheetId("fyros_buckler_lvl_01_05.item").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:3:PRICE")->setValue32(200);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:3:QUALITY")->setValue32(25);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:3:SLOT_TYPE")->setValue32(0);

	return true;
}

NLMISC_COMMAND( testTradeItems4, "Temp : test some items to trade (4)", "" )
{
	// pacts
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:0:SHEET")->setValue32(CSheetId("pacts.death_impact").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:0:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:0:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:1:SHEET")->setValue32(CSheetId("kami_pactes.death_impact").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:1:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:1:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:2:SHEET")->setValue32(CSheetId("karavan_pactes.death_impact").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:2:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:2:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:3:SHEET")->setValue32(CSheetId("kami_pactes.death_impact").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:3:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TRADING:3:SLOT_TYPE")->setValue32(0);
	//
	return true;
}


NLMISC_COMMAND( testPhrases, "Temp : test some items to trade", "" )
{
	if (args.size() != 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SHEET")->setValue32(CSheetId("abfaimhame09.sphrase").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:PRICE")->setValue32(11);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SHEET")->setValue32(CSheetId("abfaimhcme07.sphrase").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:PRICE")->setValue32(22);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SHEET")->setValue32(CSheetId("abfaimhfme04.sphrase").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:PRICE")->setValue32(33);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SHEET")->setValue32(CSheetId("abfaimhfme08.sphrase").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:PRICE")->setValue32(44);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SHEET")->setValue32(CSheetId("abfma05.sphrase").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:PRICE")->setValue32(55);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SLOT_TYPE")->setValue32(0);

	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SHEET")->setValue32(CSheetId("abm_mt_ae_acid_00055.sphrase").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:PRICE")->setValue32(66);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SHEET")->setValue32(CSheetId("abm_mt_heal_00140.sphrase").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:PRICE")->setValue32(77);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SHEET")->setValue32(CSheetId("abm_mt_cannibalism_00020.sphrase").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:PRICE")->setValue32(88);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SLOT_TYPE")->setValue32(0);
	//

	sint32 value;
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:SESSION")->setValue32(CBotChatManager::getInstance()->getSessionID());
	fromString(args[0], value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:PAGE_ID")->setValue32(value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:HAS_NEXT")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:ROLEMASTER_FLAGS")->setValue32(0);

	return true;
}


NLMISC_COMMAND( testResaleItems, "Temp : test resale", "" )
{
	if (args.size() < 1) return false;
	uint	pageId;
	fromString(args[0], pageId);
	uint	pf= pageId+1;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SHEET")->setValue32(CSheetId("m0006dxajf01.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:QUALITY")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SELLER_TYPE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:PRICE_RETIRE")->setValue32(111);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:QUANTITY")->setValue32(62);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SHEET")->setValue32(CSheetId("icfm1bs.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:PRICE")->setValue32(1*pf);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:QUALITY")->setValue32(11);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SELLER_TYPE")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:PRICE_RETIRE")->setValue32(222);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:QUANTITY")->setValue32(1);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SHEET")->setValue32(CSheetId("icfm1pd.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:PRICE")->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:QUALITY")->setValue32(22);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:NAMEID")->setValue32(8);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SELLER_TYPE")->setValue32(2);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:PRICE_RETIRE")->setValue32(333);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:QUANTITY")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:RESALE_TIME_LEFT")->setValue32(10);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SHEET")->setValue32(CSheetId("icfr2l.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:PRICE")->setValue32(3*pf);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:QUALITY")->setValue32(33);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SELLER_TYPE")->setValue32(3);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:PRICE_RETIRE")->setValue32(444);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:QUANTITY")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:RESALE_TIME_LEFT")->setValue32(100);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SHEET")->setValue32(CSheetId("m0006dxajf01.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:PRICE")->setValue32(4*pf);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:QUALITY")->setValue32(44);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SELLER_TYPE")->setValue32(4);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:PRICE_RETIRE")->setValue32(555);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:QUANTITY")->setValue32(1);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SHEET")->setValue32(CSheetId("icfp1pb.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:PRICE")->setValue32(5*pf);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:QUALITY")->setValue32(55);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SELLER_TYPE")->setValue32(5);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:PRICE_RETIRE")->setValue32(666);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:QUANTITY")->setValue32(23);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SHEET")->setValue32(CSheetId("m0119dxajd01.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:PRICE")->setValue32(6*pf);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:QUALITY")->setValue32(66);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SELLER_TYPE")->setValue32(2);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:PRICE_RETIRE")->setValue32(777);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:QUANTITY")->setValue32(19);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:RESALE_TIME_LEFT")->setValue32(48);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SHEET")->setValue32(CSheetId("icmss.sitem").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:PRICE")->setValue32(7*pf);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:QUALITY")->setValue32(77);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SELLER_TYPE")->setValue32(5);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:PRICE_RETIRE")->setValue32(888);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:QUANTITY")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:RESALE_TIME_LEFT")->setValue32(10);


	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:SESSION")->setValue32(CBotChatManager::getInstance()->getSessionID());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:PAGE_ID")->setValue32(pageId);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:HAS_NEXT")->setValue32(1);
	//

	// Force for next page
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	return true;
}


NLMISC_COMMAND( testClientPhrases, "", "" )
{
	if (args.size() != 3) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();

	sint32 value;
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:SESSION")->setValue32(CBotChatManager::getInstance()->getSessionID());
	fromString(args[0], value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:PAGE_ID")->setValue32(value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:HAS_NEXT")->setValue32(1);
	fromString(args[1], value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:ROLEMASTER_FLAGS")->setValue32(value);
	fromString(args[2], value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:ROLEMASTER_RACE")->setValue32(value);

	return true;
}


NLMISC_COMMAND( testOutpostBuildings, "Temp : test some items to trade", "" )
{
	if (args.size() != 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SHEET")->setValue32(CSheetId("driller_bountybeaches_kami_u1_100a.outpost_building").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:0:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SHEET")->setValue32(CSheetId("driller_citiesofintuition_kami_u2_50a.outpost_building").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:1:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SHEET")->setValue32(CSheetId("driller_couloirbrule_kami_u2_150b.outpost_building").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:2:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SHEET")->setValue32(CSheetId("driller_bountybeaches_karavan_u1_100a.outpost_building").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:3:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SHEET")->setValue32(CSheetId("driller_bountybeaches_karavan_u1_150a.outpost_building").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:4:SLOT_TYPE")->setValue32(0);

	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SHEET")->setValue32(CSheetId("driller_bountybeaches_karavan_u1_200a.outpost_building").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:5:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SHEET")->setValue32(CSheetId("driller_bountybeaches_karavan_u2_100a.outpost_building").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:6:SLOT_TYPE")->setValue32(0);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SHEET")->setValue32(CSheetId("driller_bountybeaches_karavan_u2_150a.outpost_building").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:PRICE")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:7:SLOT_TYPE")->setValue32(0);
	//

	sint32 value;
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:SESSION")->setValue32(CBotChatManager::getInstance()->getSessionID());
	fromString(args[0], value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:PAGE_ID")->setValue32(value);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:HAS_NEXT")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:ROLEMASTER_FLAGS")->setValue32(0);

	return true;
}




#endif

