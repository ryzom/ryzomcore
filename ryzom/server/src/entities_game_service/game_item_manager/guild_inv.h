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
	virtual void provideContents( NLMISC::CBitMemStream& stream, const NLMISC::CEntityId &recipient = NLMISC::CEntityId::Unknown, bool first = false ) = 0;
};

class CFakeDataProvider : public IDataProvider
{
public:
	/// Return true if there is a pending update to provide
	virtual bool nonEmpty() const { return false; }

	/// Push data modified since the last update. May be called event if nonEmpty().
	virtual	void provideUpdate( NLMISC::CBitMemStream& stream ) {}

	/// Push all non-empty data, then provide them as update
	virtual void provideContents( NLMISC::CBitMemStream& stream, const NLMISC::CEntityId &recipient = NLMISC::CEntityId::Unknown, bool first = false ) {}
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

	/// Return the subdivisions chest A,B
	uint8 getChestA(const NLMISC::CEntityId &recipient) {
		TChest::const_iterator it = _ChestsA.find(recipient);
		if (it != _ChestsA.end())
			return (*it).second;
		return 0;
	}
	uint8 getChestB(const NLMISC::CEntityId &recipient) {
		TChest::const_iterator it = _ChestsB.find(recipient);
		if (it != _ChestsB.end())
			return (*it).second;
		return 1;
	}

	/// Select the subdivisions
	uint8 setChestA(const NLMISC::CEntityId &recipient, uint8 chest) { _ChestsA[recipient] = chest; }
	uint8 setChestB(const NLMISC::CEntityId &recipient, uint8 chest) { _ChestsB[recipient] = chest; }

	std::string getChestName(uint8 chest) { if (chest >= 20) return ""; return _ChestNames[chest]; }
	EGSPD::CGuildGrade::TGuildGrade getChestViewGrade(uint8 chest) { if (chest >= 20) return EGSPD::CGuildGrade::Leader; return _ChestViewGrades[chest]; }
	EGSPD::CGuildGrade::TGuildGrade getChestPutGrade(uint8 chest) { if (chest >= 20) return EGSPD::CGuildGrade::Leader; return _ChestPutGrades[chest]; }
	EGSPD::CGuildGrade::TGuildGrade getChestGetGrade(uint8 chest) { if (chest >= 20) return EGSPD::CGuildGrade::Leader; return _ChestGetGrades[chest]; }

	void setChestParams(uint8 chest, std::string name, EGSPD::CGuildGrade::TGuildGrade gradeView, EGSPD::CGuildGrade::TGuildGrade gradePut, EGSPD::CGuildGrade::TGuildGrade gradeGet)
	{
		if (chest >= 20)
			return;
		_ChestNames[chest] = name;
		// TODO: send to DB
		_ChestViewGrades[chest] = gradeView;
		_ChestPutGrades[chest] = gradePut;
		_ChestGetGrades[chest] = gradeGet;

		//CBankAccessor_GUILD::getGUILD().getINVENTORY().
	}

	bool haveChestViewGrade( uint8 chest, EGSPD::CGuildGrade::TGuildGrade grade)
	{
		if (chest >= 20)
			return false;

		nlinfo("Check have acces to chest %u => %u vs %u", chest, _ChestViewGrades[chest], grade);
		return _ChestViewGrades[chest] >= grade;
	}

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
	virtual void provideContents( NLMISC::CBitMemStream& stream, const NLMISC::CEntityId &recipient = NLMISC::CEntityId::Unknown, bool first = false);

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
	/// Chests of player
	typedef std::map<NLMISC::CEntityId, uint8> TChest;
	TChest _ChestsA;
	TChest _ChestsB;

	EGSPD::CGuildGrade::TGuildGrade		_ChestViewGrades[20];
	EGSPD::CGuildGrade::TGuildGrade		_ChestPutGrades[20];
	EGSPD::CGuildGrade::TGuildGrade		_ChestGetGrades[20];
	std::string							_ChestNames[20];
};


#endif
