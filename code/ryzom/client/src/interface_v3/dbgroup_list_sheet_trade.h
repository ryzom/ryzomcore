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



#ifndef CL_DBGROUP_LIST_SHEET_TRADE_H
#define CL_DBGROUP_LIST_SHEET_TRADE_H

#include "nel/misc/types_nl.h"
#include "dbgroup_list_sheet_text.h"
#include "game_share/bot_chat_types.h"

namespace NLGUI
{
	class CCtrlButton;
	class CCtrlScroll;
}


// ***************************************************************************
class	CHandlerListSheetTradeSelect;
class	CHandlerListSheetTradeRightClick;

// ***************************************************************************
/**
 * List of sheet with a description. Selectable by line.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CDBGroupListSheetTrade : public CDBGroupListSheetText
{
public:
	enum	TSellerTypeFilter
	{
		NPC= 0,
		Resale,
		User,
		NPCAndResale,
		TrainerAction,
		TrainerUpgrade,
		TrainerPower,
		TrainerCarac,
		None
	};
public:

	/// Constructor
	CDBGroupListSheetTrade(const TCtorParam &param);
	~CDBGroupListSheetTrade();

	/// CInterfaceGroup Interface
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

	virtual void checkCoords ();

	virtual bool needCheckAllItems();

	//////////////////////////////////////////////////////////////////////////

	// A child node
	struct	CSheetChildTrade : public CDBGroupListSheetText::CSheetChild
	{
		// quality is important for ItemRequirement color
		sint32				LastQuality;
		sint32				LastQuantity;
		sint32				LastPrice;
		sint32				LastPriceRetire;
		sint32				LastResaleTimeLeft;
		sint32				LastSellerType;
		uint32				LastVendorNameId;
		sint32				LastFactionType;
		sint32				LastFactionPointPrice;
		CInterfaceProperty	CurrentQuality;
		CInterfaceProperty	CurrentQuantity;
		CInterfaceProperty	CurrentPrice;
		CInterfaceProperty	CurrentPriceRetire;
		CInterfaceProperty	CurrentResaleTimeLeft;
		CInterfaceProperty	CurrentSellerType;
		CInterfaceProperty	CurrentVendorNameId;
		CInterfaceProperty	CurrentFactionType;
		CInterfaceProperty	CurrentFactionPointPrice;
		ucstring			VendorNameString;

		virtual void init(CDBGroupListSheetText *pFather, uint index);
		virtual bool isInvalidated(CDBGroupListSheetText *pFather);
		virtual void update(CDBGroupListSheetText *pFather);
		virtual void updateViewText(CDBGroupListSheetText *pFather);
		virtual bool isSheetValid(CDBGroupListSheetText *pFather);

		CSheetChildTrade() : CSheetChild()
		{
			LastQuality = -1;
			LastQuantity = -1;
			LastPrice = -1;
			LastPriceRetire = -1;
			LastResaleTimeLeft = -1;
			LastSellerType = -1;
			LastVendorNameId = 0;
		}
	};

	virtual CSheetChild *createSheetChild() { return new CSheetChildTrade; }

	bool priceWanted() { return _WantPrice; }

	bool getMultiplyPriceByQuantityFlag() const { return _MultiplyPriceByQuantity; }

	bool testDropOrSell() const {return _TestDropOrSell;}

	bool sellerTypeWanted() const {return _SellerTypeFilter != None;}

	bool applyFamePriceFactor() const {return _ApplyFamePriceFactor;}

	TSellerTypeFilter		getSellerTypeFilter() const {return _SellerTypeFilter;}

	virtual void sort();

	std::set< CSheetChildTrade * > VendorNameIdToUpdate;

protected:

	// Tells if must read price and set in the text.
	bool	_WantPrice;

	bool    _MultiplyPriceByQuantity;

	bool	_TestDropOrSell;

	// tells if we must apply fame price factor
	bool	_ApplyFamePriceFactor;

	// keep pointer on leaf for fame price factor
	NLMISC::CCDBNodeLeaf *	_FamePriceFactorLeaf;
	sint16			_LastFamePriceFactor;

	TSellerTypeFilter		_SellerTypeFilter;

public:
	static bool isUpgradePhrase(const CSPhraseSheet &phrase);
	static bool isPowerPhrase(const CSPhraseSheet &phrase);
	static bool isCaracPhrase(const CSPhraseSheet &phrase);
};


#endif // NL_DBGROUP_LIST_SHEET_TRADE_H

/* End of dbgroup_list_sheet_trade.h */
