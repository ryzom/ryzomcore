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



#ifndef CL_BOT_CHAT_PAGE_TRADE_H
#define CL_BOT_CHAT_PAGE_TRADE_H

#include "bot_chat_page.h"
#include "obs_huge_list.h"
#include "inventory_manager.h"
#include "game_share/item_type.h"
#include "game_share/pvp_clan.h"

class CDBCtrlSheet;

/** A page for the user to sell / buy items
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date August 2003
  */
class CBotChatPageTrade : public CBotChatPage
{
public:
	// GuildXP is for guild role master
	enum TBuyMean { Money, SkillPoints, MoneyGuildXP, GuildXP, MoneyFactionPoints, GuildMoneyGuildXP, GuildMoney };

	// At Max +9999% resale margin
	enum
	{
		MaxResaleMargin= 9999
	};

public:
	CBotChatPageTrade();
	virtual void begin();
	virtual void init();
	// update displayed prices
	virtual void update();
	virtual void end();
	// start the 'buy' dialog window for the given sheet (the player has clicked on an item to buy)
	void		 startBuyDialog(CDBCtrlSheet *item, CCtrlBase *pCaller);
	// start the 'sell' dialog (the player has clicked on an item to sell)
	void		 startSellDialog(CDBCtrlSheet *item, CCtrlBase *pCaller);
	// should be called when the player has confirmed  the current trade (clicked 'ok' in the modal window for trade)
	void		 confirmTrade( bool enableResale=false );
	// should be called when the player has canceled the current trade (the modal window has been closed)
	void		 cancelTrade();
	// end the modal dialog for trade
	void		 endTradeModal();
	// test if trade can be confirmed yet (price & quantity valid)
	bool		 isTradeValid( bool enableResale ) const;
	// set the page to diplay buy interface (no sell of player item is done)
	void		 setBuyOnly(bool buyOnly) { _BuyOnly = buyOnly; }
	bool		 getBuyOnly() const { return _BuyOnly; }
	// set the buy mean (must be called before 'begin' is called)
	void		 setBuyMean(TBuyMean buyMean) { _BuyMean = buyMean; }
	// set the title of the window (actually applied when 'begin' is called)
	void		 setTitle(const ucstring &title) { _Title = title; }
	// update the 'buy' dialog window for the given sphrase sheet (the player has changed an equipped weapon => action stats change)
	void		 updateSPhraseBuyDialog();
	// invalidate window coords
	void		 invalidateCoords();

	// Filter for BotChat Buy
	void		startChangeBuyFilterDialog(const std::string &dbext, const std::string &title, uint32 maxValue);
	void		confirmChangeBuyFilterDialog();
	void		resetBuyFilterDialog();
	void		giveFocusToMaxEBChangeBuyFilterDialog();
	void		startChangeBuyFilterMPDialog();
	void		confirmChangeBuyFilterMPDialog(uint ft);
	void		startChangeBuyFilterClassDialog();
	void		resetBuyFilterClassDialog();
	void		confirmChangeBuyFilterClassDialog();
	void		startChangeBuyFilterItemTypeDialog();
	void		confirmChangeBuyFilterItemTypeDialog(ITEM_TYPE::TItemType itemType);
	void		startDestroyItemDialog();
	void		confirmDestroyItemDialog();
	void		sendCurrentBuyFilterToServer(bool resetBuyList);
	void		resetItemPartAndTypeFilters();

	// send to server a request for refreshing the User Resale List
	void		refreshResale();

	static void setFocusOnEditBox(CInterfaceGroup *eb);

	// static method to get an item sheet name, according to item type (NULL if not reconized for instance)
	static std::string	getItemSheetNameForItemType(ITEM_TYPE::TItemType it);

///////////////////////////////////////////////////////////////////////////////////////
private:

	// an observer to update big mission list from littles pages in server database
	CHugeListObs				_TradePagesObs;
	//
	bool						_BuyDlgOn;
	bool						_SellDlgOn;
	CDBCtrlSheet				*_CurrItemSheet;
	uint32						_CurrItemIndex;
	// Cache To verify each frame the properties has not changed (verify approx same item)
	class CItemCheck
	{
	public:
		CItemCheck() {reset();}
		void		reset();
		void		init(CDBCtrlSheet *ctrl);
		bool		test(CDBCtrlSheet *ctrl) const;

	private:
		sint32						_SheetId;
		sint32						_Quality;
		sint32						_Quantity;
		sint32						_Price;
		sint32						_ResaleFlag;
	};
	CItemCheck					_CurrItemCheck;
	// For Sell only, from which the sells comes from
	uint32						_CurrItemInventory;
	bool						_BuyOnly;
	TBuyMean					_BuyMean;
	ucstring					_Title;
	bool						_ResaleEdit;
	bool						_QuantityEdit;
	sint32						_QuantityCheck;
	bool						_UsePriceRetire;	// buy item only (if the item belongs to the user)
	bool						_CannotValidateBecauseRetireNotAvailable;	// buy item only (if the item belongs to the user)
	// For Buy only: Filter for BotChat Buy
	std::string					_FilterBuyDlgDBExt;
	uint32						_FilterBuyDlgMaxValue;

	// keep pointer on leaf for fame price factor
	NLMISC::CCDBNodeLeaf *				_FamePriceFactorLeaf;
	bool						_DownloadComplete;

private:
	// Update the trade modal window (where quantity is entered)
	void		updateTradeModal();
	// get quantity in modal box for current item, or ~0 if not retrieved, or 1 if _QuantityEdit==false
	uint32		getCurrItemQuantity() const;
	// get resale price in modal box for current item, or 0 if not retrieved, or if resale is impossible
	uint32		getCurrItemPriceResale() const;
	// get price of curr selected item, or ~0 if unknown
	uint64      getCurrItemPrice(bool mulByFame) const;
	// in case of a guild option get the guild xp required for this item
	uint64      getCurrItemXP() const;

	// in case of a faction item get the type and number of points
	void				getItemFactionTypePoints(CDBCtrlSheet *sheet, PVP_CLAN::TPVPClan &pvpClan, uint32 &points) const;
	void				getCurrItemFactionTypePoints(PVP_CLAN::TPVPClan &pvpClan, uint32 &points) const;
	uint32				getUserFactionPoints(PVP_CLAN::TPVPClan clan) const;

	// setup display of buy mean (skill points & money) in the modal
	void		setupBuyMeanInModal(CInterfaceGroup *modal);

	// get the skillPoints that will be used
	sint32		getSkillPointUsedForCurrentRoleMaster() const;
	// Modify the local Value
	void		addSkillPointForCurrentRoleMaster(sint32 addValue) const;

	// Setup Modal to trade an item
	// @{
	// Setup the _QuantityEdit and _QauntityCheck according to current sellMode/sheet. return the default quantity to put in the edit box
	uint		initQuantityMode(bool sellMode, CDBCtrlSheet *sheet);
	// Setup a common price group quantity according to _QuantityEdit and default quantity
	void		setupPriceGroupQuantity(CInterfaceGroup *priceGroup, sint32 defaultQuantity);
	// Setup the resell group according to DB and price etc...
	void		setupResellGroup(bool sellMode, uint defaultQuantity, CInterfaceGroup *parentGroup, CDBCtrlSheet *sheet);
	// according to _QuantityEdit, setup Focus on correct group
	void		setupEditFocus(CInterfaceGroup *parentGroup);
	// Setup if must take AMOUNT or PRICE_RETIRE (only on an ITEM buy), and according to SELLER_TYPE
	void		setupPriceOrPriceRetire(bool sellMode, CDBCtrlSheet *sheet);
	// Setup the faction point price
	void		setupFactionPointPrice(bool sellMode, uint defaultQuantity, CInterfaceGroup *parentGroup, CDBCtrlSheet *sheet);
	// @}

	// list unselection
	void		unselectBuyList(CGroupContainer *gc);
	void		unselectSellList(CGroupContainer *gc);

	void		notifyDownloadComplete(bool completed);
};

#endif
