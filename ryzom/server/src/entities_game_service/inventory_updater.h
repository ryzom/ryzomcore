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

#ifndef NL_INVENTORY_UPDATER_H
#define NL_INVENTORY_UPDATER_H

#include "game_share/inventories.h"
#include "player_manager/cdb_synchronised.h"

/**
 * Generic inventory updater.
 * 
 * Bufferizes and sends item updates to the client.
 * The number of recipients of the updates is not fixed.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2004-2005
 */
template <class CInventoryCategoryTemplate>
class CInventoryUpdater
{
public:

	typedef CInventoryCategoryTemplate CInvCat;
	typedef typename CInventoryCategoryTemplate::TInventoryId TInventoryId;

	/// Constructor
	CInventoryUpdater();

	/**
	 * Setup a queue with an initial size that will absorb updates for initial state of inventory.
	 * Hint: set to the maximum size*2 of inventory if used, do not set at all if not used.
	 * size*2 because setItemProps() + incInfoVersion().
	 * After the initial updates are sent, the reserved memory will be freed because the number of "cruise" updates is low.
	 */
	void		reserve( TInventoryId invId, uint nbSlots )
	{
		_ItemUpdates[invId].reserve( nbSlots );
	}

	/// Reset an item slot. Similar to using setItemProps() with a reset itemSlot(), but saves on bandwith.
	void		resetItem( TInventoryId invId, uint slotIndex );

	/// Set an item slot.
	void		setItemProps( TInventoryId invId, const INVENTORIES::CItemSlot& itemSlot );

	/// Set only one property of an item slot.
	void		setOneItemProp( TInventoryId invId, uint slotIndex, INVENTORIES::TItemPropId propId, sint32 value );

	/**
	 * Write all inventory updates into the stream, and clear the queues of the inventory updater.
	 * Return true if something was written, false if all queues are empty.
	 */
	bool		fillAllUpdates( NLMISC::CBitMemStream& destStream, bool isInitialUpdate=false );

	/// Return true if the specified queue is empty
	bool		empty( TInventoryId invId ) const
	{
		return _ItemUpdates[invId].empty();
	}

protected:

	/**
	 * A slot change block
	 */
	class CItemUpdate
	{
	public:

		enum TItemUpdateMode { IUAll, IUReset, IUOneProp, IUInfoVersion, NbItemUpdateModes };

		/// Constructor
		CItemUpdate() {}

		/// Mode of the update
		TItemUpdateMode				IUMode;

		/// Changed properties of the item slot
		INVENTORIES::CItemSlot		ItemSlot;
	};

	typedef std::vector<CItemUpdate> CItemUpdates;
	typedef std::vector<uint8> CItemInfoVersions;

	/// Increment the stored "Info Version" number, but does not push it into the queue
	void		incInfoVersion( TInventoryId invId, uint slotIndex );
	
	/// Push the "Info Version" number into the queue. It will be included in the next sending.
	void		pushInfoVersion( TInventoryId invId, uint slotIndex );

	/// Cancel all IUOneProp updates from the specified index, for the specified slot
	void		cancelAllUpdatesFromIndex( uint startingIndex, TInventoryId invId, uint slotIndex );

	/// Cancel all IUOneProp updates from the specified index, for the specified slot, except of the specified mode
	void		cancelAllUpdatesFromIndexExcept( uint startingIndex, TInventoryId invId, uint slotIndex, typename CItemUpdate::TItemUpdateMode updatesToKeep );

	/// Item updates
	CItemUpdates		_ItemUpdates [CInventoryCategoryTemplate::NbInventoryIds];

	/// Info version container: hold the latest version number of the extended info on item slots
	CItemInfoVersions	_ItemInfoVersions [CInventoryCategoryTemplate::NbInventoryIds];
};


/**
 * Inventory updater for character inventories.
 * There is one recipient per class.
 * This class holds the "last sent info version" of the recipient.
 */
class CInventoryUpdaterForCharacter : public CInventoryUpdater<INVENTORIES::CInventoryCategoryForCharacter>
{
public:
	
	/// Constructor
	CInventoryUpdaterForCharacter( CCDBSynchronised *propertyDatabase );

	/// Send all inventory updates to the client (please call at every game cycle), and clear the queues of the inventory updater
	void		sendAllUpdates( const NLMISC::CEntityId& destEntityId );

	/// Increment the stored "Info Version" number only if it's useful.
	void		incInfoVersion( TInventoryId invId, uint slotIndex );
	
	/// Set the last sent info version, when the client requested to receive info sync.
	void		syncInfoVersion( TInventoryId invId, uint slotIndex );

	/// Return true if the specified inventory is using CInventoryUpdater. If false, it uses the classic database.
	bool		isCompatibleWithInventory( INVENTORIES::TInventory inventory ) const
	{
		TInventoryId invId = InvIdFromEInventory[inventory];
		return ( invId != CInvCat::InvalidInvId );
	}

	/// Increment the stored "Info Version" number (alternate interface, returns true if the inventory is part of CInventoryUpdater, otherwise does nothing)
	bool		incInfoVersion( INVENTORIES::TInventory inventory, uint slotIndex )
	{
		TInventoryId invId = InvIdFromEInventory[inventory];
		if ( invId == CInvCat::InvalidInvId )
			return false;
		incInfoVersion( invId, slotIndex );
		return true;
	}
	
	/// Set the last sent info version, when the client requested to receive info sync
	bool		syncInfoVersion( INVENTORIES::TInventory inventory, uint slotIndex )
	{
		TInventoryId invId = InvIdFromEInventory[inventory];
		if ( invId == CInvCat::InvalidInvId )
			return false;
		syncInfoVersion( invId, slotIndex );
		return true;
	}

	/// Return the "Info Version" number (uses the classic databasase if the specified inventory is NOT part of CInventoryUpdater)
	uint8		getInfoVersion( INVENTORIES::TInventory inventory, uint slotIndex )
	{
		TInventoryId invId = InvIdFromEInventory[inventory];
		if ( invId != CInvCat::InvalidInvId )
			return _ItemInfoVersions[invId][slotIndex];
		else
			return getInfoVersionFromClassicDatabase( inventory, slotIndex );
	}

	/// Return the "Info Version" number, from classic database only (will not work for inventories for which isCompatibleWithInventory() is true)
	uint8		getInfoVersionFromClassicDatabase( INVENTORIES::TInventory inventory, uint slotIndex );

	/// Reset an item slot (alternate version, uses the classic databasase if the specified inventory is NOT part of CInventoryUpdater)
	void		resetItem( INVENTORIES::TInventory inventory, uint slotIndex )
	{
		TInventoryId invId = InvIdFromEInventory[inventory];
		if ( invId != CInvCat::InvalidInvId )
			CInventoryUpdater<CInvCat>::resetItem( invId, slotIndex );	
		else
			resetItemIntoClassicDatabase( inventory, slotIndex );
	}

	/// Reset an item slot, into classic database only (will not work for inventories for which isCompatibleWithInventory() is true)
	void		resetItemIntoClassicDatabase( INVENTORIES::TInventory inventory, uint slotIndex );

	/// Set an item slot (alternate version, uses the classic databasase if the specified inventory is NOT part of CInventoryUpdater)
	void		setItemProps( INVENTORIES::TInventory inventory, const INVENTORIES::CItemSlot& itemSlot )
	{
		TInventoryId invId = InvIdFromEInventory[inventory];
		if ( invId != CInvCat::InvalidInvId )
			CInventoryUpdater<CInvCat>::setItemProps( invId, itemSlot );
		else
			setItemPropsToClassicDatabase( inventory, itemSlot );
	}

	/// Set an item slot, into classic database only (will not work for inventories for which isCompatibleWithInventory() is true)
	void		setItemPropsToClassicDatabase( INVENTORIES::TInventory inventory, const INVENTORIES::CItemSlot& itemSlot );
	
	/// Set only one property of an item slot (alternate version, uses the classic databasase if the specified inventory is NOT part of CInventoryUpdater)
	void		setOneItemProp( INVENTORIES::TInventory inventory, uint slotIndex, INVENTORIES::TItemPropId propId, sint32 value )
	{
		TInventoryId invId = InvIdFromEInventory[inventory];
		if ( invId != CInvCat::InvalidInvId )
			CInventoryUpdater<CInvCat>::setOneItemProp( invId, slotIndex, propId, value );
		else
			setOneItemPropToClassicDatabase( inventory, slotIndex, propId, value );
	}

	/// Set only one property of an item slot, into classic database only (will not work for inventories for which isCompatibleWithInventory() is true)
	void		setOneItemPropToClassicDatabase( INVENTORIES::TInventory inventory, uint slotIndex, INVENTORIES::TItemPropId propId, sint32 value );

private:

	/// Last "info version of item" sent to the client, per slot
	CItemInfoVersions	_LastItemInfoVersionsSent [CInvCat::NbInventoryIds];

	/// Pointer to the classic database
	CCDBSynchronised	*_PropertyDatabasePt;

	static const TInventoryId InvIdFromEInventory [INVENTORIES::NUM_ALL_INVENTORY];
};


/**
 * Inventory updater for guild inventory.
 * There are usually more than one recipient per class.
 * This class does not hold the "last sent info version" of the recipients.
 */
class CInventoryUpdaterForGuild : public CInventoryUpdater<INVENTORIES::CInventoryCategoryForGuild>
{
public:
	CInventoryUpdaterForGuild();

	/// Return the current info version of an item slot (slotIndex must be < INVENTORIES::NbGuildSlots)
	uint8	getItemInfoVersion( uint slotIndex )
	{
		return _ItemInfoVersions[CInvCat::GuildInvId][slotIndex];
	}

	/// Push the current item info version. It will be sent to the recipients.
	void	pushItemInfoVersion( uint slotIndex )
	{
		pushInfoVersion( CInvCat::GuildInvId, slotIndex );
	}

	/** Set the new info version of an item slot (slotIndex must be < INVENTORIES::NbGuildSlots).
	 * It will be sent to the recipients.
	 */
	void	setItemInfoVersion( uint slotIndex, uint8 newValue )
	{
		_ItemInfoVersions[CInvCat::GuildInvId][slotIndex] = newValue;

		pushItemInfoVersion( slotIndex );
	}
};


/*
 * Reset an item slot. Similar to using setItemProps() with a reset itemSlot(), but saves on bandwidth.
 *
 * Assertions after this method returns:
 * - For the position at invId/slotIndex:
 *   . There is exactly one IUReset in the queue.
 *   . There is no IUAll, no IUOneProp in the queue.
 */
template <class CInventoryCategoryTemplate>
void		CInventoryUpdater<CInventoryCategoryTemplate>::resetItem( TInventoryId invId, uint slotIndex )
{
	H_AUTO(IU_resetItem);
	
	// Update item update if found in the pending list
	for ( typename CItemUpdates::iterator itu=_ItemUpdates[invId].begin(); itu!=_ItemUpdates[invId].end(); ++itu )
	{
		CItemUpdate& itemUpdate = (*itu);
		if ( itemUpdate.ItemSlot.getSlotIndex() == slotIndex )
		{
			if ( itemUpdate.IUMode != CItemUpdate::IUAll )
			{
				// Cancel any additional IUOneProp updates
				cancelAllUpdatesFromIndex( (uint)(itu - _ItemUpdates[invId].begin() + 1), invId, slotIndex );
			}
			// Overwrite any previous IUAll, IUReset, IUOneProp, IUInfoVersion
			itemUpdate.IUMode = CItemUpdate::IUReset;
			return;
		}
	}
	
	// Otherwise add to list
	CItemUpdate newItemUpdate;
	_ItemUpdates[invId].push_back( newItemUpdate );
	_ItemUpdates[invId].back().IUMode = CItemUpdate::IUReset;
	_ItemUpdates[invId].back().ItemSlot.setSlotIndex( slotIndex );
}


/*
 * Set an item slot.
 * It would be probably faster to have a method such as "getNewItemUpdate()" returning a CItemSlot&
 * that could then be accessed to write the properties directly into an element of _ItemUpdates.
 * But then we would not have the common interface for "classic database" properties.
 *
 * Assertions after this method returns:
 * - For the position at invId/itemSlot.getSlotIndex():
 *   . There is exactly one IUAll in the queue.
 *   . There is no IUReset, no IUOneProp in the queue.
 */
template <class CInventoryCategoryTemplate>
void		CInventoryUpdater<CInventoryCategoryTemplate>::setItemProps( TInventoryId invId, const INVENTORIES::CItemSlot& itemSlot )
{
	H_AUTO(IU_setItemProps);
	
	// Update item update if found in the pending list
	for ( typename CItemUpdates::iterator itu=_ItemUpdates[invId].begin(); itu!=_ItemUpdates[invId].end(); ++itu )
	{
		CItemUpdate& itemUpdate = (*itu);
		if ( itemUpdate.ItemSlot.getSlotIndex() == itemSlot.getSlotIndex() )
		{
			if ( itemUpdate.IUMode != CItemUpdate::IUInfoVersion )
			{
				if ( itemUpdate.IUMode != CItemUpdate::IUAll )
				{
					// Cancel any additional IUOneProp updates
					cancelAllUpdatesFromIndexExcept( (uint)(itu - _ItemUpdates[invId].begin() + 1), invId, itemSlot.getSlotIndex(), CItemUpdate::IUInfoVersion );
				}
				// Overwrite previous IUAll, IUReset, or first IUOneProp
				itemUpdate.IUMode = CItemUpdate::IUAll;
				itemUpdate.ItemSlot.copyFrom( itemSlot );
				return;
			}
		}
	}

	// Otherwise add to list
	CItemUpdate newItemUpdate;
	_ItemUpdates[invId].push_back( newItemUpdate );
	_ItemUpdates[invId].back().IUMode = CItemUpdate::IUAll;
	_ItemUpdates[invId].back().ItemSlot = itemSlot;
}


/*
 * Set only one property of an item slot.
 *
 * Assertions after this method returns:
 * - For the position at invId/slotIndex:
 *   . There can't be a IUAll and a IUOneProp at the same time in the queue.
 *   . There can be several IUOneProp with different props in the queue, but only one IUOneProp with the same prop.
 *   . There can be a IUReset somewhere before a IUOneProp in the queue.
 */
template <class CInventoryCategoryTemplate>
void		CInventoryUpdater<CInventoryCategoryTemplate>::setOneItemProp( TInventoryId invId, uint slotIndex, INVENTORIES::TItemPropId propId, sint32 value )
{
	H_AUTO(IU_setOneItemProp);
	
	// Update itemSlot if found in the pending list
	for ( typename CItemUpdates::iterator itu=_ItemUpdates[invId].begin(); itu!=_ItemUpdates[invId].end(); ++itu )
	{
		CItemUpdate& itemUpdate = (*itu);
		if ( itemUpdate.ItemSlot.getSlotIndex() == slotIndex )
		{
			if ( itemUpdate.IUMode == CItemUpdate::IUAll )
			{
				// Fit in previous IUAll
				itemUpdate.ItemSlot.setItemProp( propId, value );
				return;
			}
			else if ( (itemUpdate.IUMode == CItemUpdate::IUOneProp) &&
				      (itemUpdate.ItemSlot.getOneProp().ItemPropId == propId) )
			{
				// Replace previous IUOneProp (with same prop)
				itemUpdate.ItemSlot.getOneProp().ItemPropValue = value;
				return;
			}
			// IUReset, IUOneProp (with a different prop), IUInfoVersion => continue searching
		}
	}

	// Otherwise add to list
	CItemUpdate newItemUpdate;
	_ItemUpdates[invId].push_back( newItemUpdate );
	_ItemUpdates[invId].back().IUMode = CItemUpdate::IUOneProp;
	_ItemUpdates[invId].back().ItemSlot.setSlotIndex( slotIndex );
	_ItemUpdates[invId].back().ItemSlot.getOneProp().ItemPropId = propId;
	_ItemUpdates[invId].back().ItemSlot.getOneProp().ItemPropValue = value;
}


/*
 * Increment the stored "Info Version" number.
 */
template <class CInventoryCategoryTemplate>
void		CInventoryUpdater<CInventoryCategoryTemplate>::incInfoVersion( TInventoryId invId, uint slotIndex )
{
	++_ItemInfoVersions[invId][slotIndex];
}


/*
 * Push the "Info Version" number into the queue. It will be included in the next sending.
 */
template <class CInventoryCategoryTemplate>
void		CInventoryUpdater<CInventoryCategoryTemplate>::pushInfoVersion( TInventoryId invId, uint slotIndex )
{
	CItemUpdate newItemUpdate;
	_ItemUpdates[invId].push_back( newItemUpdate );
	_ItemUpdates[invId].back().IUMode = CItemUpdate::IUInfoVersion;
	_ItemUpdates[invId].back().ItemSlot.setSlotIndex( slotIndex );
}


/*
 * Cancel all IUOneProp updates from the specified index, for the specified slot, except IUInfoVersion
 */
template <class CInventoryCategoryTemplate>
void		CInventoryUpdater<CInventoryCategoryTemplate>::cancelAllUpdatesFromIndex( uint startingIndex, TInventoryId invId, uint slotIndex )
{
	while ( startingIndex < _ItemUpdates[invId].size() )
	{
		if ( (_ItemUpdates[invId][startingIndex].ItemSlot.getSlotIndex() == slotIndex) )
		{
			_ItemUpdates[invId].erase( _ItemUpdates[invId].begin() + startingIndex );
			// slow but this case is very rare and works with few items (except at initial update)
			// alternatively, we could set the mode to some kind of IUCancel but sendAllUpdates() would need to write size() minus the number of cancelled items
		}
		else
		{
			++startingIndex;
		}
	}
}


/*
 * Cancel all IUOneProp updates from the specified index, for the specified slot, except of the specified mode
 */
template <class CInventoryCategoryTemplate>
void		CInventoryUpdater<CInventoryCategoryTemplate>::cancelAllUpdatesFromIndexExcept( uint startingIndex, TInventoryId invId, uint slotIndex, typename CItemUpdate::TItemUpdateMode updatesToKeep )
{
	while ( startingIndex < _ItemUpdates[invId].size() )
	{
		if ( (_ItemUpdates[invId][startingIndex].ItemSlot.getSlotIndex() == slotIndex) &&
			 (_ItemUpdates[invId][startingIndex].IUMode != updatesToKeep) )
		{
			_ItemUpdates[invId].erase( _ItemUpdates[invId].begin() + startingIndex );
			// slow but this case is very rare and works with few items (except at initial update)
			// alternatively, we could set the mode to some kind of IUCancel but sendAllUpdates() would need to write size() minus the number of cancelled items
		}
		else
		{
			++startingIndex;
		}
	}
}


/*
 * Write all inventory updates into the stream, and clear the queues of the inventory updater.
 * Return true if something was written, false if all queues are empty.
 */
template <class CInventoryCategoryTemplate>
bool		CInventoryUpdater<CInventoryCategoryTemplate>::fillAllUpdates( NLMISC::CBitMemStream& destStream, bool isInitialUpdate )
{
	// Write the server tick, to ensure old inventory update are not applied after newer
	NLMISC::TGameCycle serverTick = CTickEventHandler::getGameCycle();
	destStream.serial(serverTick);

	// Push inventory updates
	bool hasContentToSend = false;
	for ( uint invId=0; invId!=CInvCat::NbInventoryIds; ++invId )
	{
		// Presence bit
		bool hasContent = (! _ItemUpdates[invId].empty());
		destStream.serialBit( hasContent );
		if ( ! hasContent )
		{
			if ( isInitialUpdate )
				NLMISC::contReset( _ItemUpdates[invId] ); // free the big memory that could be allocated for the first update of all slots (estimated 'cruise' updates from 0 to 5 slots, reserving up to 8 elements)
			continue;
		}
		hasContentToSend = true;

		// Number field
		uint32 nbChanges = (uint32)_ItemUpdates[invId].size();
		if ( nbChanges < INVENTORIES::LowNumberBound )
		{
			destStream.serial( nbChanges, INVENTORIES::LowNumberBits );
		}
		else
		{
			uint32 nbChangesOver4bit = INVENTORIES::LowNumberBound;
			destStream.serial( nbChangesOver4bit, INVENTORIES::LowNumberBits );
			destStream.serial( nbChanges, 32 );
		}

		// List of updates
		for ( typename CItemUpdates::iterator itu=_ItemUpdates[invId].begin(); itu!=_ItemUpdates[invId].end(); ++itu )
		{
			CItemUpdate& itemUpdate = (*itu);
			switch ( itemUpdate.IUMode )
			{
			case CItemUpdate::IUInfoVersion:
				{
					uint32 iuInfoVersion = 0x1; // 1b
					destStream.serial( iuInfoVersion, 1 );
					uint32 slotIndex = itemUpdate.ItemSlot.getSlotIndex();
					destStream.serial( slotIndex, CInvCat::SlotBitSize );
					// Do we need to send the number or only increment it by one?
					if ( CInvCat::needPlainInfoVersionTransfer() )
					{
						uint32 infoVersion = (uint32)_ItemInfoVersions[invId][itemUpdate.ItemSlot.getSlotIndex()];
						destStream.serial( infoVersion, INVENTORIES::InfoVersionBitSize );
					}
					break;
				}
			case CItemUpdate::IUAll:
				{
					uint32 iuAll = 0x1; // 0x01b;
					destStream.serial( iuAll, 2 );
					itemUpdate.ItemSlot.serialAll( destStream, (CInvCat*)0 );
					break;
				}
			case CItemUpdate::IUReset:
				{
					uint32 iuReset = 0x0; // 000b (the bitmemstream is written from high-order to low-order)
					destStream.serial( iuReset, 3 );
					uint32 slotIndex = itemUpdate.ItemSlot.getSlotIndex();
					destStream.serial( slotIndex, CInvCat::SlotBitSize );
					break;
				}
			default: // IUOneProp
				{
					uint32 iuOneProp = 0x1; // 001b
					destStream.serial( iuOneProp, 3 );
					itemUpdate.ItemSlot.serialOneProp( destStream, (CInvCat*)0 );
					//break;
				}
			}
		}

		if ( isInitialUpdate || (nbChanges > 8) )
		{
			NLMISC::contReset( _ItemUpdates[invId] ); // free the big memory that could be allocated for the first update of all slots (estimated 'cruise' updates from 0 to 5 slots, reserving up to 8 elements)
		}
		else
		{
			_ItemUpdates[invId].clear();
		}
	}

	return hasContentToSend;
}


#endif // NL_INVENTORY_UPDATER_H

/* End of inventory_updater.h */
