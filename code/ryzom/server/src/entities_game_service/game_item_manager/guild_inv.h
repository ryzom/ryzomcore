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


#ifndef GUILD_INV_H
#define GUILD_INV_H

#include "game_item_manager/game_item.h"
#include "game_item_manager/player_inventory.h"
#include "inventory_updater.h"

class CGuild;
class CCDBSynchronised;
class CCDBGroup;

namespace NLMISC
{
	class CBitMemStream;
}

/** Interface for shared data provider */
class IDataProvider
{
public:

	/// Return true if there is a pending update to provide
	virtual bool nonEmpty() const = 0;

	/// Push data modified since the last update. May be called event if nonEmpty().
	virtual	void provideUpdate( NLMISC::CBitMemStream& stream ) = 0;

	/// Push all non-empty data, then provide them as update
	virtual void provideContents( NLMISC::CBitMemStream& stream ) = 0;
};

class CFakeDataProvider : public IDataProvider
{
public:
	/// Return true if there is a pending update to provide
	virtual bool nonEmpty() const { return false; }
	
	/// Push data modified since the last update. May be called event if nonEmpty().
	virtual	void provideUpdate( NLMISC::CBitMemStream& stream ) {}
	
	/// Push all non-empty data, then provide them as update
	virtual void provideContents( NLMISC::CBitMemStream& stream ) {}
};

/** Guild inventory */
class CGuildInventory : public CInventoryBase
{
public:
	
	/// Constructor
	CGuildInventory();

	/// Return the max bulk
	virtual uint32 getMaxBulk() const;

	/// Return the max number of slots
	virtual uint32 getMaxSlot() const;
};

/**
 * Guild inventory view.
 * IDataProvider involves only the inventory updater, not the shared database (money etc.)
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2005
 */
class CGuildInventoryView : public IInventoryView, public IDataProvider
{
public:

	/// Constructor. Precondition: ownerGuild not null
	CGuildInventoryView( CGuild *ownerGuild ) :	_Guild(ownerGuild), _InventorySession(0), _MoneySession(0), _GuildInvDb(NULL) {}

	/// Init from guild inventory and shared database (must be done before any inventory event). Precondition: all not null
	void init( CGuildInventory *inventory, CCDBGroup *guildInvDb );

	/// Release
	void release();

	/// Return false if the tentativeSession is lower than the session of the item. If false, resends the correct inventory session
	bool checkSession( uint32 slot, uint16 tentativeSession );

	/// Update the inventory session for a money transaction, and send the inventory session
	void updateSessionForMoneyTransaction();

	/// Same as checkSession but for the money
	bool checkMoneySession( uint16 tentativeSession );

	/// Return the "info version" of the specified item slot
	uint8 getItemInfoVersion( uint32 slot ) { return _GuildInvUpdater.getItemInfoVersion( slot ); }

	/// An item has changed (can be a removing)
	virtual void onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags);

	/// The inventory information has changed (like total bulk or weight)
	virtual void onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags);

	/// Callback from item when an item stack size change (update the weight and bulk of the inventory)
	virtual void onItemStackSizeChanged(uint32 slot, uint32 previousStackSize);

	/// Force an update of the information related to an item (in case of client operation canceled by the server, reset client information)
	virtual void forceSlotUpdate(uint32 slot);

	/// Return true if there is a pending update to provide
	virtual bool nonEmpty() const;

	/// Push data modified since the last update, for all online members
	virtual	void provideUpdate( NLMISC::CBitMemStream& stream );

	/// Push all non-empty data, for connecting members. Precondition: _GuildInvUpdater.empty() (provideUpdate() must have been called before)
	virtual void provideContents( NLMISC::CBitMemStream& stream );

protected:

	/// Update all client data for a slot
	void updateClientSlot(uint32 slot);

	/// Reset all client data for a slot
	void resetClientSlot(uint32 slot);

	/// Handle info version when an item is changed. Precondition: node not null, slot valid
	void updateInfoVersion(uint32 slot);

	/// Update the session to prevent several players modifying the same data at the same time
	void updateSession(uint32 slot);

private:

	/// pointer to the shared database
	CCDBGroup		*_GuildInvDb;
	/// inventory session per item See swap slot
	std::vector<uint16>		_ItemsSessions;
	/// current session of the inventory See swap slot
	uint16					_InventorySession;
	/// current session of the money
	uint16					_MoneySession;
	/// guild owning the view
	CGuild					*_Guild;
	/// client updater for guild inventory
	CInventoryUpdaterForGuild _GuildInvUpdater;
};


#endif
