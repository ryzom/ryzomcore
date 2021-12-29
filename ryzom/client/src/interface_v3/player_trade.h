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



#ifndef CL_PLAYER_TRADE_H
#define CL_PLAYER_TRADE_H

#include <vector>
#include "nel/misc/types_nl.h"



class CDBCtrlSheet;

// class to track the traded items during a transaction
class CPlayerTrade
{
public:
	enum { NumTradeSlot = 8 };

	CPlayerTrade() {BotChatGiftContext= false;}

	// Put item to exchange, and remove ALL previous slot (even if same item type exchanged...)
	void putItemInExchange(CDBCtrlSheet *inventorySlot, CDBCtrlSheet *exchangeSlot, uint quantitySrc);
	void restoreItem(CDBCtrlSheet *exchangeSlot);
	void restoreAllItems();
	// get an exchange slot from its index
	static CDBCtrlSheet *getExchangeItem(uint index);

	// true if currently used in BotChaftGift context
	bool			BotChatGiftContext;
};

// the instance to manage player trade
extern CPlayerTrade PlayerTrade;

#endif
