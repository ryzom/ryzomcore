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
#include "game_share/slot_equipment.h"

#include "guild_inv.h"
#include "player_manager/character.h"
#include "egs_sheets/egs_sheets.h"
#include "guild_manager/guild_member_module.h"
#include "cdb_group.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern NLMISC::CVariable<uint32>	MaxPlayerBulk;
extern CGenericXmlMsgHeaderManager	GenericMsgManager;

/////////////////////////////////////////////////////////////
// CGuildInventory
/////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
CGuildInventory::CGuildInventory()
{
	H_AUTO(CGuildInventory);
	setSlotCount( getMaxSlot() );
	setInventoryId( INVENTORIES::guild );
}

//-----------------------------------------------------------------------------
uint32 CGuildInventory::getMaxBulk() const
{
	return BaseGuildBulk; /* keep this until players are able to increase guild max bulk */
}

//-----------------------------------------------------------------------------
uint32 CGuildInventory::getMaxSlot() const
{
	return INVENTORIES::NbGuildSlots;
}

/////////////////////////////////////////////////////////////
// CGuildInventoryView
// TODO: Send only when player is in guild building?
/////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
void CGuildInventoryView::init( CGuildInventory *inventory, CCDBGroup *guildInvDb )
{
	H_AUTO(GuildInventoryInit);
	bindToInventory( inventory );
	{
		H_AUTO(resizeGuildInventoryInit);
		_ItemsSessions.resize( getInventory()->getSlotCount(), 0 );	
	}
	nlassert( INVENTORIES::NbGuildSlots == getInventory()->getSlotCount() );
	_GuildInvDb = guildInvDb;
//	_GuildInvDb->setProp( "GUILD:INVENTORY:BULK_MAX", getInventory()->getMaxBulk() / 1000 );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setBULK_MAX(*_GuildInvDb, getInventory()->getMaxBulk() / 1000 );
//	_GuildInvDb->setProp( "GUILD:INVENTORY:SESSION", _InventorySession );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setSESSION(*_GuildInvDb, _InventorySession );
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::release()
{
	unbindFromInventory();
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags)
{
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags)
{
	if ( changeFlags.checkEnumValue( INVENTORIES::itc_inserted ) )
	{
		updateClientSlot( slot );
		updateInfoVersion( slot );
		updateSession( slot );
	}
	else if ( changeFlags.checkEnumValue( INVENTORIES::itc_removed ) )
	{
		resetClientSlot( slot );
		updateSession( slot );
	}

	// if ( changeFlags.checkEnumValue( CInventoryBase::itc_info_version ) )
		// see CGuild::getAndSyncItemInfoVersion()
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::forceSlotUpdate(uint32 slot)
{
	CGameItemPtr item = getInventory()->getItem(slot);
	if ( item != NULL )
		updateClientSlot(slot);
	else
		resetClientSlot(slot);
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::onItemStackSizeChanged(uint32 slot, uint32 previousStackSize)
{
	CGameItemPtr item = getInventory()->getItem( slot );
	_GuildInvUpdater.setOneItemProp(
		INVENTORIES::CInventoryCategoryForGuild::GuildInvId, slot,
		INVENTORIES::Quantity, item->getStackSize() );

	updateInfoVersion( slot );
	updateSession( slot );
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::updateClientSlot(uint32 slot)
{
	CGameItemPtr item = getInventory()->getItem( slot );
	INVENTORIES::CItemSlot itemSlot( slot );
	itemSlot.setItemProp( INVENTORIES::Sheet, item->getSheetId().asInt() );
	itemSlot.setItemProp( INVENTORIES::Quality, item->quality() );
	itemSlot.setItemProp( INVENTORIES::Quantity, item->getStackSize() );
	itemSlot.setItemProp( INVENTORIES::UserColor, item->color() );
	itemSlot.setItemProp( INVENTORIES::Locked, 0 );
	itemSlot.setItemProp( INVENTORIES::Weight, item->weight() / 10 );
	itemSlot.setItemProp( INVENTORIES::NameId, 0 ); // TODO: name of guild (item->sendNameId())
	itemSlot.setItemProp( INVENTORIES::Enchant, item->getClientEnchantValue() );
	itemSlot.setItemProp( INVENTORIES::Price, 0 );
	itemSlot.setItemProp( INVENTORIES::ResaleFlag, 0 );
	itemSlot.setItemProp( INVENTORIES::ItemClass, item->getItemClass() );
	itemSlot.setItemProp( INVENTORIES::ItemBestStat, item->getCraftParameters() == 0 ? RM_FABER_STAT_TYPE::Unknown : item->getCraftParameters()->getBestItemStat() );
	itemSlot.setItemProp( INVENTORIES::PrerequisitValid, 1 );
	_GuildInvUpdater.setItemProps( INVENTORIES::CInventoryCategoryForGuild::GuildInvId, itemSlot );
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::resetClientSlot(uint32 slot)
{
	_GuildInvUpdater.resetItem( INVENTORIES::CInventoryCategoryForGuild::GuildInvId, slot );
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::updateInfoVersion(uint32 slot)
{
	// Access the current info version of the item
	BOMB_IF( slot >= INVENTORIES::NbGuildSlots,
		NLMISC::toString( "Slot %u out of guild updater bounds %u", slot, INVENTORIES::NbGuildSlots ),
		return );
	uint8 currentVersion = _GuildInvUpdater.getItemInfoVersion( slot );

	// Calculate the incremented version, skipping 0 as it reserved for 'never changed' value
	uint8 newVersion = currentVersion + 1;
	if ( newVersion == 0 )
		++newVersion;

	// If the current version is 0, force increment so that new clients (not online now, with default value 0) will have to request info
	bool mustIncrement = (currentVersion == 0);

	// Check if there are members who have received the info for the current version, if so we'll increment the version
	// Check if there are members who will need a refresh of their 'info msg' version if the version is incremented (otherwise the 256-cycle would make them miss the change)
	vector<CGuildMemberModule*> onlineMembersNeedingRefreshIfIncrement;
	std::map< EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::const_iterator it;
	for ( it=_Guild->getMembersBegin(); it!=_Guild->getMembersEnd(); ++it )
	{
		CGuildMember *member = EGS_PD_CAST<CGuildMember*>((*it).second);
		EGS_PD_AST( member );

		// Skip offline members
		CGuildMemberModule *onlineMember = NULL;
		if ( member->getReferencingModule( onlineMember ) ) // contains slow dynamic cast :(
		{
			uint8 lastSent = onlineMember->getLastSentInfoVersion( slot );
			if ( lastSent == currentVersion )
			{
				// Now, continue browsing the members to fill onlineMembersNeedingRefreshIfIncrement
				mustIncrement = true;
			}
			else if ( lastSent == newVersion )
			{
				// This will be useful at the end only if mustIncrement is true
				onlineMembersNeedingRefreshIfIncrement.push_back( onlineMember );
			}
		}
	}
	if ( ! mustIncrement )
		return;

	// If so, increment the current version (cycles every 256)
	_GuildInvUpdater.setItemInfoVersion( slot, newVersion );

	// Refresh their info version so that they will need a request to know new info
	for ( vector<CGuildMemberModule*>::const_iterator itr=onlineMembersNeedingRefreshIfIncrement.begin(); itr!=onlineMembersNeedingRefreshIfIncrement.end(); ++itr )
	{
		// Refresh item info version
		CGuildMemberModule *memberToRefresh = (*itr);
		memberToRefresh->setLastSentInfoVersion( slot, currentVersion );

		// Send it to the client
		CGuildCharProxy proxy;
		memberToRefresh->getProxy( proxy );
		CMessage msgout( "IMPULSION_ID" );
		CBitMemStream bms;
		CEntityId destCharacterId = proxy.getId();
		msgout.serial( destCharacterId );
		GenericMsgManager.pushNameToStream( "ITEM_INFO:REFRESH_VERSION", bms );
		nlctassert( CItemInfos::SlotIdIndexBitSize >= INVENTORIES::CInventoryCategoryForGuild::SlotBitSize );
		uint16 slotId = ((uint16)slot) | ((uint16)(INVENTORIES::guild << CItemInfos::SlotIdIndexBitSize));
		bms.serial( slotId );
		bms.serial( currentVersion );
		msgout.serialBufferWithSize( (uint8*)bms.buffer(), bms.length() );
		sendMessageViaMirror( NLNET::TServiceId(destCharacterId.getDynamicId()), msgout );	
	}
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::updateSession(uint32 slot)
{
	// update session
	if  ( ++_ItemsSessions[slot] > _InventorySession )
	{
		_InventorySession = _ItemsSessions[slot];
	}
//	_GuildInvDb->setProp( "GUILD:INVENTORY:SESSION", _InventorySession );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setSESSION(*_GuildInvDb, _InventorySession );
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::updateSessionForMoneyTransaction()
{
	if  ( ++_MoneySession > _InventorySession )
	{
		_InventorySession = _MoneySession;
	}
//	_GuildInvDb->setProp( "GUILD:INVENTORY:SESSION", _InventorySession );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setSESSION(*_GuildInvDb, _InventorySession );
}

//-----------------------------------------------------------------------------
bool CGuildInventoryView::checkSession( uint32 slot, uint16 tentativeSession )
{
	bool isSessionOk = !(tentativeSession < _ItemsSessions[slot]);
	if ( ! isSessionOk )
//		_GuildInvDb->setProp( "GUILD:INVENTORY:SESSION", _InventorySession ); // was done per character, but now it's shared
		CBankAccessor_GUILD::getGUILD().getINVENTORY().setSESSION(*_GuildInvDb, _InventorySession ); // was done per character, but now it's shared
	return isSessionOk;
}

//-----------------------------------------------------------------------------
bool CGuildInventoryView::checkMoneySession( uint16 tentativeSession )
{
	bool isSessionOk = !(tentativeSession < _MoneySession);
	if ( ! isSessionOk )
//		_GuildInvDb->setProp( "GUILD:INVENTORY:SESSION", _InventorySession ); // was done per character, but now it's shared
		CBankAccessor_GUILD::getGUILD().getINVENTORY().setSESSION(*_GuildInvDb, _InventorySession );
	return isSessionOk;
}

//-----------------------------------------------------------------------------
bool CGuildInventoryView::nonEmpty() const
{
	return ! _GuildInvUpdater.empty( INVENTORIES::CInventoryCategoryForGuild::GuildInvId );
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::provideUpdate( CBitMemStream& stream )
{
	_GuildInvUpdater.fillAllUpdates( stream );
}

//-----------------------------------------------------------------------------
void CGuildInventoryView::provideContents( CBitMemStream& stream )
{
	// Ensure the updater is empty, otherwise our contents would be mixed with current update
	nlassert( _GuildInvUpdater.empty( INVENTORIES::CInventoryCategoryForGuild::GuildInvId ) );

	CGuildInventory *guildInv = static_cast<CGuildInventory*>(getInventory());
	for ( uint i=0; i!=guildInv->getSlotCount(); ++i )
	{
		CGameItemPtr itemPtr = guildInv->getItem( i );
		if ( itemPtr != NULL )
		{
			updateClientSlot( i );
			_GuildInvUpdater.pushItemInfoVersion( i ); // send the current info version (no change)
		}
	}
	provideUpdate( stream );
}
