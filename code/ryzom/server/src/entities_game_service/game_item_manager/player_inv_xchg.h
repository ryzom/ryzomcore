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


#ifndef PLAYER_INV_XCHG_H
#define PLAYER_INV_XCHG_H

#include "game_item_manager/game_item.h"
#include "game_item_manager/player_inventory.h"
#include "game_share/item_type.h"


/**
 * CExchangeView
 *
 * \author Sebastien 'kxu' Guignot
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date February 2005
 */
class CExchangeView : public CCharacterInvView
{
public:
	static const uint32 NbExchangeSlots;

	/// ctor
	CExchangeView();

	/// dtor
	virtual ~CExchangeView();

	/// clear exchange view
	void clearExchangeView();

	/// set interlocutor view, interlocutorView can be NULL in the case of bot gift
	void setInterlocutorView(CExchangeView * interlocutorView);

	/// get interlocutor view
	CExchangeView * getInterlocutorView() { return _InterlocutorView; }

	/// the following events are ignored
	virtual void onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags) {}
	virtual void onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags) {}
	virtual void onItemStackSizeChanged(uint32 slot, uint32 previousStackSize) {}
	virtual void forceSlotUpdate(uint32 slot) {}

	/// put an item in exchange
	bool putItemInExchange(uint32 bagSlot, uint32 exchangeSlot, uint32 quantity);
	bool putItemInFirstEmptyExchangeSlot(uint32 bagSlot, uint32 quantity);

	/// remove an item from exchange
	bool removeItemFromExchange(uint32 exchangeSlot, uint32 quantity = INVENTORIES::REMOVE_MAX_STACK_QUANTITY);

	/// validate exchange
	/// \param givenItems : if not NULL, return the given items
	void validateExchange(std::vector<CGameItemPtr> * givenItems);

	/// if we change this exchange view, call this on the interlocutor exchange view
	void onInterlocutorSlotChanged(uint32 interlocutorGiveSlot);

	/// helper: get the item proposed in the given exchange slot
	/// \param exchangeSlot : slot in the exchange view
	/// \param exchangeQuantity : if not NULL, returns the quantity of the item offered in the exchange
	CGameItemPtr getExchangeItem(uint32 exchangeSlot, uint32 * exchangeQuantity = NULL) const;

	/// return true if there is no item in the exchange view
	bool isEmpty() const;

	/// return nb ticket exchanged for an ticket type
	uint32 getPetTicketExchanged(ITEM_TYPE::TItemType itemType) { return * getPetTicketCount(itemType); }

private:
	/// helper: used to update client when an exchange slot changes
	void updateExchangeSlot(uint32 exchangeSlot);

	/// return true if item is a pet animal ticket
	bool isPetTicket(ITEM_TYPE::TItemType itemType) const;

	/// return a pointer of _PackerTicketCount or _MountTicketCount depending on the type
	uint32 * getPetTicketCount(ITEM_TYPE::TItemType itemType);

private:
	struct CExchangeSlot
	{
		CExchangeSlot();
		void reset();

		uint32 BagSlot;
		uint32 Quantity;
	};

	/// exchange view of the interlocutor (can be NULL)
	NLMISC::CSmartPtr<CExchangeView> _InterlocutorView;

	/// exchange slots
	std::vector<CExchangeSlot> _ExchangeSlots;

	/// nb of packer tickets in the exchange view
	uint32 _PackerTicketCount;

	/// nb of mount tickets in the exchange view
	uint32 _MountTicketCount;
};


#endif
