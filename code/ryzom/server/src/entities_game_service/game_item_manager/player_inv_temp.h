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


#ifndef PLAYER_INV_TEMP_H
#define PLAYER_INV_TEMP_H

#include "game_share/temp_inventory_mode.h"

#include "game_item_manager/game_item.h"
#include "game_item_manager/player_inventory.h"

#include "nel/misc/sheet_id.h"

/**
 * CTempInventory
 *
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date February 2005
 */
class CTempInventory : public CInventoryBase
{
public:

	CTempInventory();

	uint32 getMaxSlot() const;

	virtual void bindView(CInventoryViewPtr inventoryView);

	// Inventory Mode

	TEMP_INV_MODE::TInventoryMode getMode() { return _Mode; }
	bool canEnterMode(TEMP_INV_MODE::TInventoryMode mode);
	bool enterMode(TEMP_INV_MODE::TInventoryMode mode);
	void leaveMode();

	// Display functionnalities : these methods do not affect the inventory structure
	// they just display things to the client throught the database
	// before using verify that the temp inventory is empty (else it may have some problem 
	// between displayed items and owned items)

	void enableTakeDisp(bool b);
	
	void clearDisp(uint32 slot);

	void setDispSheetId(uint32 slot, const NLMISC::CSheetId &sheet);
	NLMISC::CSheetId getDispSheetId(uint32 slot);

	void setDispQuality(uint32 slot, uint16 quality);
	uint16 getDispQuality(uint32 slot);

	void setDispQuantity(uint32 slot, uint16 quantity);
	uint16 getDispQuantity(uint32 slot);

protected:

	TEMP_INV_MODE::TInventoryMode _Mode;

	CCharacter *_Char;

};

/**
 * CTempInvView
 *
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date February 2005
 */
class CTempInvView : public CCharacterInvView
{
public:

	void init();

	virtual void onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags);

	virtual void onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags);

	virtual void updateItemPrerequisit(uint32 slot) { /* TODO */ }

protected:

	virtual void updateClientSlot(uint32 clientSlot, const CGameItemPtr item);

	std::vector<uint8> _LastInfoVersion;
};

#endif
