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

#include "game_item.h"
#include "player_inventory.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "player_manager/player_manager.h"
#include "egs_sheets/egs_sheets.h"
#include "server_share/log_item_gen.h"

using namespace NLMISC;
using namespace std;
using namespace NLNET;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily



// ****************************************************************************
// CInventoryBase
// ****************************************************************************

// ****************************************************************************
CInventoryBase::CInventoryBase()
{
	H_AUTO(CInventoryBase);
	setSlotCount(0);
}

// ****************************************************************************
CInventoryBase::~CInventoryBase()
{
	while (!_InventoryViews.empty())
	{
		_InventoryViews.front()->unbindFromInventory();
	}
}

// ****************************************************************************
void CInventoryBase::setInventoryId(INVENTORIES::TInventory inventoryId)
{
	_InventoryId = inventoryId;
}

// ****************************************************************************
INVENTORIES::TInventory CInventoryBase::getInventoryId()
{
	return _InventoryId;
}

// ****************************************************************************
void CInventoryBase::setSlotCount(uint size)
{
	H_AUTO(setSlotCount);
	_SlotCount = size;
	{
		H_AUTO(resizeSetSlotCount);
		_Items.resize(_SlotCount);
	}

	// recompute free slot, weight and bulk
	_FreeSlotCount = 0;
	_InventoryWeight = 0;
	_InventoryBulk = 0;
	for (uint i=0; i<_Items.size(); ++i)
	{
		if (_Items[i] != NULL)
		{
			_InventoryWeight += _Items[i]->getStackWeight();
			_InventoryBulk += _Items[i]->getStackBulk();
		}
		else
		{
			_FreeSlotCount++;	
		}
	}
}

/// Return the inventory size in slot
// ****************************************************************************
uint CInventoryBase::getSlotCount() const
{
	return _SlotCount;
}

/// Return the number of slots in use
// ****************************************************************************
uint CInventoryBase::getUsedSlotCount() const
{
	return _SlotCount - _FreeSlotCount;
}

/// Return the number of free slots
// ****************************************************************************
uint CInventoryBase::getFreeSlotCount() const
{
	return _FreeSlotCount;
}

// ****************************************************************************
uint32 CInventoryBase::getInventoryWeight() const
{
	return _InventoryWeight;
}

// ****************************************************************************
uint32 CInventoryBase::getInventoryBulk() const
{
	return _InventoryBulk;
}

// ****************************************************************************
CInventoryBase::TInventoryOpResult CInventoryBase::insertItem(CGameItemPtr &item, uint32 slot, bool autoStack)
{
	return doInsertItem(item, slot, autoStack, false);
}

// ****************************************************************************
void CInventoryBase::forceLoadItem(CGameItemPtr &item, uint32 slot)
{
	doInsertItem(item, slot, false, true);
}

/** Insert an item in the inventory
 *	If an item already own the specified slot,
 *	this old item is deleted.
 *	If slot is set to INSERT_IN_FIRST_FREE_SLOT,
 *	the method look for the first available slot.
 * AUTOSTACK :
 *  After a insertItem that is successfull DO NOT use the item anymore !
 *  in case of failure the item remains valid
 */
// ****************************************************************************
CInventoryBase::TInventoryOpResult CInventoryBase::doInsertItem(CGameItemPtr &item, uint32 slot, bool autoStack, bool ignoreWeightAndBulk)
{
	H_AUTO(doInsertItem);
	
	nlassert(item != NULL);
	nlassert(item->getInventory() == NULL);
	nlassert(slot < _Items.size() || slot == INVENTORIES::INSERT_IN_FIRST_FREE_SLOT);

	if (!ignoreWeightAndBulk)
	{
		if (item->getStackWeight() + getInventoryWeight() > getMaxWeight())
			return ior_overweight;
		if (item->getStackBulk() + getInventoryBulk() > getMaxBulk())
			return ior_overbulk;
	}

	if (autoStack)
	{
		H_AUTO(AutoStack);
		// If slot provided check we can stack if we can't find an empty slot
		if (slot != INVENTORIES::INSERT_IN_FIRST_FREE_SLOT)
			if (canStackItem(item, slot) != ior_ok)
				slot = INVENTORIES::INSERT_IN_FIRST_FREE_SLOT;

		uint32 slotBegin = slot;
		uint32 slotSearch;
		uint32 itemStackSize = item->getStackSize();

		// get first compatible stack
		if (slot == INVENTORIES::INSERT_IN_FIRST_FREE_SLOT)
			slotBegin = 0;
		slotSearch = slotBegin;
		// Modification to do : (slot to put item, stack size to put)
		vector< pair<uint32,uint32> > Modifs;

		// If slot provided is NULL directly insert item in it
		if (_Items[slotBegin] == NULL)
		{
			Modifs.push_back(make_pair(slotBegin, itemStackSize));
		}
		else
		{
			// do the following until all items in the stack are transfered to stacks (and empty slot)
			while (1)
			{
				// Search for a compatible stack (not null slot, not full slot)
				bool bFound = false;
				do
				{
					if (   (_Items[slotSearch] != NULL)
						&& (_Items[slotSearch]->getStackSize() < _Items[slotSearch]->getMaxStackSize())
						&& (CGameItem::areStackable(item, _Items[slotSearch]))) // no check on stack size here
					{
						bFound = true;
						break;
					}

					slotSearch++;
					if (slotSearch == getSlotCount()) slotSearch = 0;
				}
				while (slotSearch != slotBegin);

				if (bFound)
				{
					// We found a slot with an existing stack that is compatible
					// Try to put as much as we can into this stack
					if (itemStackSize > _Items[slotSearch]->getMaxStackSize() - _Items[slotSearch]->getStackSize())
					{
						uint32 sizePut = _Items[slotSearch]->getMaxStackSize() - _Items[slotSearch]->getStackSize();
						itemStackSize -= sizePut;
						Modifs.push_back(make_pair(slotSearch, sizePut));
						slotSearch++;
						if (slotSearch == getSlotCount()) slotSearch = 0;
						if (slotSearch != slotBegin)
							continue; // if we have not finished the loop try the next slot
					}
					else
					{
						Modifs.push_back(make_pair(slotSearch, itemStackSize));
						break; // finished
					}
				}

				// Can't insert item in an already existing stack
				if (getFreeSlotCount() == 0)
					return ior_no_free_slot; // No more empty slot in this inventory !!!
				slotSearch = getFirstFreeSlot();
				Modifs.push_back(make_pair(slotSearch, itemStackSize));
				break; // finished
			}
		}
		
		// Apply all modifs to the inventory
		bool bInserted = false;
		for (uint32 i = 0; i < Modifs.size(); ++i)
		{
			uint32 slotModif = Modifs[i].first;
			uint32 sizeModif = Modifs[i].second;

			if (_Items[slotModif] == NULL)
			{
				// set stack size before we add the item to the inventory to avoid the callback onItemStackSizeChanged()
				// which update weight and bulk
				item->setStackSize(sizeModif);

				// update weight and bulk "manually"
				updateWeightAndBulk(item, sizeModif);

				// put the item in the inventory
				_Items[slotModif] = item;
				item->setInventory(CInventoryPtr(this), slotModif);
				--_FreeSlotCount;
				onItemChanged(slotModif, INVENTORIES::itc_inserted);
				bInserted = true;
			}
			else
			{
				// callbacks are called in setStackSize (inventory onItemStackSizeChanged)
				_Items[slotModif]->setStackSize(_Items[slotModif]->getStackSize()+sizeModif);
			}
		}
		INVENTORIES::TInventoryChangeFlags flagInvChg(INVENTORIES::ic_total_bulk);
		flagInvChg.setEnumValue(INVENTORIES::ic_total_weight);
		flagInvChg.setEnumValue(INVENTORIES::ic_item_list);
		onInventoryChanged(flagInvChg);
		// If the item is not inserted into the inventory it has no more reason to exist because
		// it was fully splitted into all the stacks of the inventory
		if (!bInserted)
		{
			item.deleteItem();
			item = NULL;
		}
	}
	else
	{
		H_AUTO(NoneStackableItem);
		// check that we still have a free slot
		if (getFreeSlotCount() == 0)
			return ior_no_free_slot;

		if (slot == INVENTORIES::INSERT_IN_FIRST_FREE_SLOT)
			slot = getFirstFreeSlot();
		
		// delete any item on this slot
		if (_Items[slot] != NULL)
		{
			deleteItem(slot);
		}
		
		_Items[slot] = item;
		item->setInventory(CInventoryPtr(this), slot);
		updateWeightAndBulk(item, item->getStackSize());
		--_FreeSlotCount;
		
		// callback all the views : 2 messages : 
		// * item inserted 
		// * inventory change bulk, weight and list
		INVENTORIES::TInventoryChangeFlags flagInvChg(INVENTORIES::ic_total_bulk);
		flagInvChg.setEnumValue(INVENTORIES::ic_total_weight);
		flagInvChg.setEnumValue(INVENTORIES::ic_item_list);
		INVENTORIES::TItemChangeFlags flagItemChg(INVENTORIES::itc_inserted);
		TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
		for (; first!= last; ++first)
		{
			CInventoryViewPtr view = *first;
			
			view->onInventoryChanged(flagInvChg);
			view->onItemChanged(slot, flagItemChg);
		}
	}
	return ior_ok;
}

/// Delete the nth item from this inventory
// ****************************************************************************
void CInventoryBase::deleteItem(uint32 slot)
{
	nlassert(slot < _Items.size());

	if (_Items[slot] != NULL)
	{
		// unlink the item
		CGameItemPtr item = removeItem(slot,_Items[slot]->getStackSize());
		// delete it
		item.deleteItem();
	}
}

/// Remove the nth item from this inventory
// ****************************************************************************
CGameItemPtr CInventoryBase::removeItem(uint32 slot, uint32 quantity, TInventoryOpResult * res)
{
	nlassert(slot < _Items.size());

	CGameItemPtr ret = _Items[slot];

	if (quantity == 0)
	{
		if (res != NULL)
			*res = ior_ok;
		return NULL;
	}

	if (_Items[slot] != NULL)
	{
		if (quantity > _Items[slot]->getStackSize())
			quantity = _Items[slot]->getStackSize();

		// if we try to remove more items than available do not remove anything
		if (quantity > _Items[slot]->getNonLockedStackSize())
		{
			if (res != NULL)
				*res = ior_item_locked;
			return NULL;
		}

		INVENTORIES::TInventoryChangeFlags flagInvChg(INVENTORIES::ic_total_bulk);
		flagInvChg.setEnumValue(INVENTORIES::ic_total_weight);
		flagInvChg.setEnumValue(INVENTORIES::ic_item_list);

		// If we want to remove the whole stack
		if (quantity == _Items[slot]->getStackSize())
		{
			// callback all the views : 2 messages : 
			// * item removed
			// * inventory change bulk, weight and list
			INVENTORIES::TItemChangeFlags flagItemChg(INVENTORIES::itc_removed);
			TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
			for (; first != last; ++first)
			{
				CInventoryViewPtr view = *first;
				
				view->onInventoryChanged(flagInvChg);
				view->onItemChanged(slot, flagItemChg);
			}

			// unlink from ref inventory
			if (_Items[slot]->getRefInventory() != NULL)
			{
				_Items[slot]->getRefInventory()->removeItem(_Items[slot]->getRefInventorySlot());
			}
			// unlink the item
			_Items[slot]->setInventory(CInventoryPtr(NULL), INVENTORIES::INVALID_INVENTORY_SLOT);
			_Items[slot] = NULL;
			++_FreeSlotCount;
			updateWeightAndBulk(ret, -sint32(ret->getStackSize()));
		}
		else // we want to remove a part of a stack
		{
			// create a new item
			ret = _Items[slot]->getItemCopy();
			ret->setStackSize(quantity);
			_Items[slot]->setStackSize(_Items[slot]->getStackSize()-quantity);

			// Done at the end because the item is not completly removed
			// callback all the views : 1 message :
			// * inventory change bulk, weight and list
			TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
			for (; first != last; ++first)
			{
				CInventoryViewPtr view = *first;
				
				view->onInventoryChanged(flagInvChg);
			}
			
			// update ref inventory if some
			if (_Items[slot]->getRefInventory() != NULL)
			{
				_Items[slot]->getRefInventory()->onInventoryChanged(flagInvChg);
			}
		}
	}

	if (res != NULL)
		*res = ior_ok;

	// may be null
	return ret;
}

// ****************************************************************************
uint32 CInventoryBase::deleteStackItem(uint32 slot, uint32 quantity, TInventoryOpResult * res)
{
	nlassert(slot < _Items.size());
	
	uint32 nbDeletedItem = 0;

	CGameItemPtr item = getItem(slot);
	if(item->getStackSize() > quantity)
	{
		nbDeletedItem = quantity;
		item->setStackSize( item->getStackSize() - quantity );
//		log_Item_UpdateQuantity(item->getItemId(), item->getStackSize(), item->getStackSize() - quantity);
		if( res != NULL )
			*res = ior_ok;
	}
	else
	{
		nbDeletedItem = item->getStackSize();
		deleteItem(slot);
		if( res != NULL )
		{
			if( quantity == nbDeletedItem )
				*res = ior_ok;
			else
				*res = ior_stack_undersize;
		}
	}
	return nbDeletedItem;
}

// ****************************************************************************
CInventoryBase::TInventoryOpResult CInventoryBase::moveItem(
	CInventoryBase* srcInv, uint32 srcSlot,
	CInventoryBase* dstInv, uint32 dstSlot,
	uint32 quantity, bool bestEffort )
{
	
	CGameItemPtr srcItem = srcInv->getItem( srcSlot );
	if (srcItem == NULL)
		return ior_error;

	log_Item_Move(srcItem->getItemId(), srcInv->getInventoryId(), dstInv->getInventoryId());

	// Try to move it
	nlctassert( INVENTORIES::REMOVE_MAX_STACK_QUANTITY == UINT_MAX ); // default arg must be compatible

	TInventoryOpResult res;
	CGameItemPtr removedItem = srcInv->removeItem( srcSlot, quantity, &res );
	if (res != ior_ok)
		return res;

	// if there was no item to remove, just return ok
	if (removedItem == NULL)
		return ior_ok;

	res = dstInv->insertItem( removedItem, dstSlot, true );
	if (res != ior_ok)
	{
		// In case of failure, put it back (assume the stack was compliant with the boundaries of the inventory (bulk, weight...)
		srcInv->insertItem( removedItem, srcSlot, true );
		return res;
	}

	return ior_ok;
}

// ****************************************************************************
CGameItemPtr CInventoryBase::getItem(uint32 slot) const
{
	nlassert(slot < _SlotCount);
	nlassert(slot < _Items.size());

	return _Items[slot];
}

/// test if we can stack an item on an existing one
// ****************************************************************************
CInventoryBase::TInventoryOpResult CInventoryBase::canStackItem(const CGameItemPtr &item, uint32 slotDst)
{
	if (item == NULL || slotDst >= getSlotCount())
		return ior_error;

	CGameItemPtr dstItem = getItem(slotDst);

	// ok if slot is empty
	if (dstItem == NULL)
		return ior_ok;

	// check item sheet and craft params
	if (!CGameItem::areStackable(item, dstItem))
		return ior_item_incompatible;

	const CStaticItem *srcForm = CSheets::getForm(item->getSheetId());

	if (dstItem->getStackSize() + item->getStackSize() > dstItem->getMaxStackSize())
		return ior_stack_oversize;

	return ior_ok;
}

/// test if we can remove an item (or a stack part)
// ****************************************************************************
/*CInventoryBase::TInventoryOpResult CInventoryBase::canRemoveItem(uint32 slot, uint32 quantity)
{
	if (slot >= getSlotCount())
		return ior_error;

	CGameItemPtr item = getItem(slot);

	if (quantity > item->getStackSize())
		return ior_stack_undersize;

	if (item->getNonLockedStackSize() < quantity)
		return ior_item_locked;

	return ior_ok;
}*/

// ****************************************************************************
void CInventoryBase::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags) 
{
	TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
	for (; first!= last; ++first)
	{
		CInventoryViewPtr view = *first;
		view->onItemChanged(slot, changeFlags);
	}
}

// ****************************************************************************
void CInventoryBase::onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags) 
{
	TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
	for (; first!= last; ++first)
	{
		CInventoryViewPtr view = *first;
		view->onInventoryChanged(changeFlags);
	}
}

/** Callback from item when an item stack size change
 *	This callback is used to update the weight and bulk
 *	of the inventory
 */
// ****************************************************************************
void	CInventoryBase::onItemStackSizeChanged(uint32 slot, uint32 previousStackSize)
{
	CGameItemPtr item = getItem(slot);
	sint32 deltaSize = item->getStackSize() - previousStackSize;

	updateWeightAndBulk(item, deltaSize);

	// callback all the views
	TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
	for (; first!= last; ++first)
	{
		CInventoryViewPtr view = *first;

		view->onItemStackSizeChanged(slot, previousStackSize);
	}
}

// ****************************************************************************
void CInventoryBase::updateWeightAndBulk(const CGameItemPtr &item, sint32 deltaQt)
{
	const CStaticItem *form = item->getStaticForm();

	if (form != NULL)
	{
		_InventoryBulk = uint32(max(sint32(0), sint32(_InventoryBulk + form->Bulk*deltaQt)));
		_InventoryWeight = uint32(max(sint32(0), sint32(_InventoryWeight + item->weight()*deltaQt)));

		if (_InventoryWeight > getMaxWeight())
		{
			nlwarning("Inventory '%s' : weight is overload", INVENTORIES::toString(_InventoryId).c_str());
		}
		if (_InventoryBulk > getMaxBulk())
		{
			nlwarning("Inventory '%s' : bulk is overload", INVENTORIES::toString(_InventoryId).c_str());
		}
	}
}

// ****************************************************************************
void CInventoryBase::bindView(CInventoryViewPtr inventoryView)
{
	H_AUTO(BindView);
	nlassert(find(_InventoryViews.begin(), _InventoryViews.end(), inventoryView) == _InventoryViews.end());

	_InventoryViews.push_back(inventoryView);

	// 'fill' the new view with actual inventory content
	for (uint i=0; i<_Items.size(); ++i)
	{
		if (_Items[i] != NULL)
			inventoryView->onItemChanged(i, INVENTORIES::TItemChangeFlags(INVENTORIES::itc_inserted));
	}
}

// ****************************************************************************
void CInventoryBase::unbindView(CInventoryViewPtr inventoryView)
{
	std::list<CInventoryViewPtr>::iterator it(find(_InventoryViews.begin(), _InventoryViews.end(), inventoryView));
	nlassert(it != _InventoryViews.end());

	_InventoryViews.erase(it);

	/// 'empty the view'
	for (uint i=0; i<_Items.size(); ++i)
	{
		if (_Items[i] != NULL)
			inventoryView->onItemChanged(i, INVENTORIES::TItemChangeFlags(INVENTORIES::itc_removed));
	}
}

// ****************************************************************************
CInventoryPtr CInventoryBase::getInventoryCopy() const
{
	CInventoryBase *inv = new CInventoryBase;

	copyContentToInventory(inv);

	return inv;
}

// ****************************************************************************
void CInventoryBase::copyContentToInventory(CInventoryBase *destInv) const
{
	for (uint i=0; i<_Items.size(); ++i)
	{
		if (_Items[i] != NULL)
		{
			CGameItemPtr item = _Items[i]->getItemCopy();

			destInv->insertItem(item, i);
		}
		else
		{
			destInv->deleteItem(i);
		}
	}
}

// ****************************************************************************
void CInventoryBase::clearInventory()
{
	for (uint i=0; i<_Items.size(); ++i)
	{
		if (_Items[i] != NULL)
		{
			if(_Items[i]->getLockCount() > 0)
			{
				_Items[i]->setLockCount(0);
			}
			deleteItem(i);
		}
	}
}

// ****************************************************************************
void CInventoryBase::forceViewUpdateOfSlot(uint32 slot)
{
	TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
	for ( ; first != last; ++first)
	{
		CInventoryViewPtr view = *first;

		view->forceSlotUpdate(slot);
	}
}

// ****************************************************************************
void CInventoryBase::forceViewUpdateOfInventory(bool skipEmptySlots)
{
	for (uint i = 0; i < _Items.size(); i++)
	{
		if (_Items[i] == NULL && skipEmptySlots)
			continue;

		TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
		for ( ; first != last; ++first)
		{
			CInventoryViewPtr view = *first;

			view->forceSlotUpdate(i);
		}
	}
}

// ****************************************************************************
uint32 CInventoryBase::getFirstFreeSlot() const
{
	nlassert(_FreeSlotCount > 0);

	// look for a valid slot
	for (uint i=0; i<_Items.size(); ++i)
	{
		if (_Items[i] == NULL)
		{
			return i;
		}
	}

	nlassertex(false, ("There are free slot, but I can't find them"));
	return INVENTORIES::INVALID_INVENTORY_SLOT;
}

// ****************************************************************************
void CInventoryBase::dumpInventory(NLMISC::CLog & log, bool dumpItems) const
{
	// inventory description
	log.displayNL("Inventory: %s", INVENTORIES::toString(_InventoryId).c_str());
	log.displayRawNL("Slots: max=%u, count=%u, free=%u", getMaxSlot(), _SlotCount, _FreeSlotCount);
	log.displayRawNL("Weight: %u", _InventoryWeight);
	log.displayRawNL("Bulk: %u", _InventoryBulk);
	log.displayRawNL("Nb views: %u", _InventoryViews.size());

	log.displayNL("(DEBUG) _Items.size() = %u", _Items.size());
	if (dumpItems)
	{
		for (uint i = 0; i < _Items.size(); i++)
		{
			CGameItemPtr item = _Items[i];
			if (item == NULL)
				continue;

			// short display of the item description
			log.displayRawNL( "%u: ", i );
			item->displayInLog( log );
		}
	}
}


// ****************************************************************************
void CInventoryBase::updateAllItemPrerequisit()
{
	for (uint i = 0; i < _Items.size(); i++)
	{
		if (_Items[i] == NULL)
			continue;

		if (_Items[i]->hasPrerequisit() == false)
			continue;

		TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
		for ( ; first != last; ++first)
		{
			CInventoryViewPtr view = *first;
			
			view->updateItemPrerequisit(i);
		}
	}
}



// ****************************************************************************
// CRefInventory
// ****************************************************************************

// ****************************************************************************
CInventoryBase::TInventoryOpResult CRefInventory::doInsertItem(CGameItemPtr &item, uint32 slot, bool autoStack, bool ignoreWeightAndBulk)
{
	nlassert(item != NULL);
	nlassert(item->getRefInventory() == NULL);
	nlassert(slot < _Items.size() || slot == INVENTORIES::INVALID_INVENTORY_SLOT);

	if (!ignoreWeightAndBulk)
	{
		if (item->getStackWeight() + getInventoryWeight() > getMaxWeight())
			return ior_overweight;
		if (item->getStackBulk() + getInventoryBulk() > getMaxBulk())
			return ior_overbulk;
	}

	// check that we still have a free slot
	if (getFreeSlotCount() == 0)
		return ior_no_free_slot;

	if (slot == INVENTORIES::INSERT_IN_FIRST_FREE_SLOT)
		slot = getFirstFreeSlot();

	// remove any item referenced here
	removeItem(slot);

	// insert and link the new item
	_Items[slot] = item;
	item->setRefInventory(CInventoryPtr(this), slot);
	updateWeightAndBulk(item, item->getStackSize());
	--_FreeSlotCount;

	// callbacks for derived class
	onItemChanged(slot, INVENTORIES::itc_inserted);
	// callback views
	/*TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
	for (; first != last; ++first)
	{
		CInventoryViewPtr view = *first;
		view->onItemChanged(slot, itc_inserted);
	}*/

	return ior_ok;
}

// ****************************************************************************
void CRefInventory::deleteItem(uint32 slot)
{
	nlassert(slot < _Items.size());

	// for ref inventory, deleteItem is equivalent to removeItem
	removeItem(slot);
}

// ****************************************************************************
CGameItemPtr CRefInventory::removeItem(uint32 slot, uint32 quantity, TInventoryOpResult * res)
{
	nlassert(slot < _Items.size());

	CGameItemPtr ret = _Items[slot];
	if (_Items[slot] != NULL)
	{
		// unlink the item
		_Items[slot]->setRefInventory(CInventoryPtr(NULL), INVENTORIES::INVALID_INVENTORY_SLOT);
		CGameItemPtr item = _Items[slot];
		_Items[slot] = NULL;
		++_FreeSlotCount;

		updateWeightAndBulk(ret, -sint32(ret->getStackSize()));

		// callbacks for derived class
		onItemChanged(slot, INVENTORIES::itc_removed);
		// callback views
		INVENTORIES::TInventoryChangeFlags flagInvChg(INVENTORIES::ic_total_bulk);
		flagInvChg.setEnumValue(INVENTORIES::ic_total_weight);
		flagInvChg.setEnumValue(INVENTORIES::ic_item_list);
		TViewCont::iterator first(_InventoryViews.begin()), last(_InventoryViews.end());
		for (; first != last; ++first)
		{
			CInventoryViewPtr view = *first;
			view->onInventoryChanged(flagInvChg);
			view->onItemChanged(slot, INVENTORIES::itc_removed);
		}
	}

	if (res != NULL)
		*res = ior_ok;

	return ret;
}

// ****************************************************************************
void CRefInventory::clearInventory()
{
	for (uint i=0; i<getSlotCount(); ++i)
	{
		removeItem(i);
	}
}

// ****************************************************************************
CInventoryPtr CRefInventory::getInventoryCopy() const
{
	CRefInventory *inv = new CRefInventory;

	copyContentToInventory(inv);

	return inv;
}

// ****************************************************************************
// IInventoryView
// ****************************************************************************

// ****************************************************************************
IInventoryView::IInventoryView()
: _BindedInventory(NULL)
{
}

// ****************************************************************************
IInventoryView::~IInventoryView()
{
	if (_BindedInventory != NULL)
	{
		unbindFromInventory();
	}
}

// ****************************************************************************
void IInventoryView::bindToInventory(CInventoryPtr inventory)
{
	H_AUTO(BindToInventory);
	if (_BindedInventory != NULL)
	{
		unbindFromInventory();
	}

	if (inventory != NULL)
	{
		_BindedInventory = inventory;
		_BindedInventory->bindView(this);
	}
}

// ****************************************************************************
void IInventoryView::unbindFromInventory()
{
	// keep a ref on this because binded inventory may own the last reference
	// and unbindView() could delete this
	CInventoryViewPtr selfRef = this;

	if (_BindedInventory != NULL)
	{
		_BindedInventory->unbindView(this);
		_BindedInventory = NULL;
	}
}

// ****************************************************************************
CInventoryBase *IInventoryView::getInventory() const
{
	return _BindedInventory;
}

// ****************************************************************************
// CCharacterInvView
// ****************************************************************************

// ****************************************************************************
void CCharacterInvView::setCharacter(CCharacter *character)
{
	_Character = character;
}

// ****************************************************************************
CCharacter *CCharacterInvView::getCharacter()
{
	return _Character;
}

// ****************************************************************************
void CCharacterInvView::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags)
{
	H_AUTO(OnItemChange);

	if (	changeFlags.checkEnumValue(INVENTORIES::itc_inserted)
		||	changeFlags.checkEnumValue(INVENTORIES::itc_lock_state)
		|| 	changeFlags.checkEnumValue(INVENTORIES::itc_enchant)
		||  changeFlags.checkEnumValue(INVENTORIES::itc_worned)
		)
	{
		const CGameItemPtr item = getInventory()->getItem(slot);
		nlassert(item != NULL);
		updateClientSlot(slot, item);
	}
	else if (changeFlags.checkEnumValue(INVENTORIES::itc_removed))
	{
		// Cleanup the item in player inventory
		updateClientSlot(slot, NULL);
	}
	else if (changeFlags.checkEnumValue(INVENTORIES::itc_hp))
	{
		const CGameItemPtr item = getInventory()->getItem(slot);
		nlassert(item != NULL);
		
		// get new worn state
		ITEM_WORN_STATE::TItemWornState wornState = item->getItemWornState();
		item->computeItemWornState();
				
		// if states differs send a message
		if (wornState != item->getItemWornState())
		{
			if (getCharacter() != NULL)
			{
				string msgName = ITEM_WORN_STATE::getMessageForState(item->getItemWornState());
				if ( !msgName.empty())
				{
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
					params[0].SheetId = item->getSheetId();
					CCharacter::sendDynamicSystemMessage( getCharacter()->getEntityRowId(), msgName, params);
				}
			}
		}

		// send slot update to the client
		updateClientSlot(slot, item);
	}
	
	if (changeFlags.checkEnumValue(INVENTORIES::itc_info_version))
	{
		getCharacter()->_InventoryUpdater.syncInfoVersion(getInventory()->getInventoryId(), slot);
	}
}

// ****************************************************************************
void CCharacterInvView::onItemStackSizeChanged(uint32 slot, uint32 previousStackSize)
{
	forceSlotUpdate(slot);
}

// ****************************************************************************
void CCharacterInvView::forceSlotUpdate(uint32 slot)
{
	CGameItemPtr item = getInventory()->getItem(slot);

	updateClientSlot(slot, item);
}

// ****************************************************************************
void CCharacterInvView::updateItemPrerequisit(uint32 slot)
{
	CGameItemPtr item = getInventory()->getItem(slot);

	if (item != NULL)
	{
		getCharacter()->_InventoryUpdater.setOneItemProp( getInventory()->getInventoryId(), slot, INVENTORIES::PrerequisitValid, getCharacter()->checkPreRequired( item ) );
	}
}

// ****************************************************************************
void CCharacterInvView::updateClientSlot(uint32 slot, const CGameItemPtr item)
{
	// do nothing if client is not ready
	if (!getCharacter()->getEnterFlag())
		return;

	if (item != NULL)
	{
		uint32 price;
		RM_FABER_STAT_TYPE::TRMStatType itemBestStat;

		getCharacter()->queryItemPrice( item, price );
		itemBestStat = item->getCraftParameters() == 0 ? RM_FABER_STAT_TYPE::Unknown : item->getCraftParameters()->getBestItemStat();

		BOTCHATTYPE::TBotChatResaleFlag resaleFlag = (item->durability() == item->maxDurability() ? BOTCHATTYPE::ResaleOk : BOTCHATTYPE::ResaleKOBroken);
		if (item->getLockedByOwner())
		{
			resaleFlag = BOTCHATTYPE::ResaleKOLockedByOwner;
		}

		INVENTORIES::CItemSlot itemSlot( slot );
		itemSlot.setItemProp( INVENTORIES::Sheet, item->getSheetId().asInt() );
		itemSlot.setItemProp( INVENTORIES::Quality, item->quality() );
		itemSlot.setItemProp( INVENTORIES::Quantity, item->getStackSize() );
		itemSlot.setItemProp( INVENTORIES::UserColor, item->color() );
		itemSlot.setItemProp( INVENTORIES::Locked, item->getLockCount() );
		itemSlot.setItemProp( INVENTORIES::Weight, item->weight() / 10 );
		itemSlot.setItemProp( INVENTORIES::NameId, item->sendNameId(getCharacter()) );
		itemSlot.setItemProp( INVENTORIES::Enchant, item->getClientEnchantValue() );
		itemSlot.setItemProp( INVENTORIES::Price, price );
		itemSlot.setItemProp( INVENTORIES::ResaleFlag, resaleFlag );
		itemSlot.setItemProp( INVENTORIES::ItemClass, item->getItemClass() );
		itemSlot.setItemProp( INVENTORIES::ItemBestStat, itemBestStat );
		itemSlot.setItemProp( INVENTORIES::PrerequisitValid, getCharacter()->checkPreRequired( item ) );
		itemSlot.setItemProp( INVENTORIES::Worned, (item->getItemWornState()==ITEM_WORN_STATE::Worned));
		getCharacter()->_InventoryUpdater.setItemProps( getInventory()->getInventoryId(), itemSlot );
	}
	else
	{
		// empty slot
		getCharacter()->_InventoryUpdater.resetItem( getInventory()->getInventoryId(), slot );
	}

	// send slot update to the client
	getCharacter()->_InventoryUpdater.incInfoVersion(getInventory()->getInventoryId(), slot);
}

// ****************************************************************************
// Commands
// ****************************************************************************

// ****************************************************************************
NLMISC_COMMAND(dumpPlayerInventories, "dump inventories of the player (DEBUG)", "<eId> [<dumpItems=1/0>]")
{
	if (args.size() < 1 || args.size() > 2)
		return false;

	CEntityId eid(args[0]);

	CCharacter * c = PlayerManager.getChar(eid);
	if (!c)
		return true;

	bool dumpItems;
	if (args.size() < 2)
		dumpItems = true;
	else
		dumpItems = (args[1] == "1");

	for (uint i = 0; i < INVENTORIES::NUM_ALL_INVENTORY; i++)
	{
		log.displayNL("----------------------------------------------------------------------------");
		CInventoryPtr inv = c->getInventory(INVENTORIES::TInventory(i));
		if (inv == NULL)
		{
			log.displayNL("Inventory %s (%u) is NULL", INVENTORIES::toString(INVENTORIES::TInventory(i)).c_str(), i);
			continue;
		}

		inv->dumpInventory(log, dumpItems);
	}

	return true;
}

// ****************************************************************************
// Persistence
// ****************************************************************************

// ****************************************************************************
#define PERSISTENT_CLASS CInventoryBase

#define PERSISTENT_APPLY_ARGS CCharacter * owner

#define PERSISTENT_DATA\
	PROP2(_InventoryId, uint32, uint32(_InventoryId), _InventoryId = INVENTORIES::TInventory(_InventoryId))\
	LSTRUCT_VECT(_Items,\
	ARRAY_LOGIC(getSlotCount()) if (_Items[i]!=NULL && _Items[i]->getStaticForm() != NULL && _Items[i]->getStaticForm()->Family != ITEMFAMILY::SCROLL_R2),\
		_Items[i]->store(pdr),\
		CGameItemPtr item;\
		item.newItem();\
		CGameItem::CPersistentApplyArg applyArgs(owner);\
		item->apply(pdr, applyArgs);\
		/* ANTIBUG: remove items with unknown sheet*/\
		if (item->getSheetId() == CSheetId::Unknown)\
		{\
			( nlwarning("found unknown item in inventory '%s' at slot %u",\
				INVENTORIES::toString(getInventoryId()).c_str(),\
				item->getInventorySlot()\
				) );\
			item.deleteItem();\
		}\
		else\
		{\
			forceLoadItem(item, applyArgs.InventorySlot);\
		}\
	)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

// ****************************************************************************
//void CInventoryBase::legacyLoad(NLMISC::IStream &f, uint16 characterSerialVersion, CCharacter *ownerPtr)
//{
//	// this function while load an inventory using the pre-pdr data format.
//	// this load is mostly a very old recursive item serial loading
//	// where the file level of item (the inventory) is just read to 
//	// advance in the stream
//
//	uint16			slotImage = 0;
//	CEntityId		owner;
//	uint8			slot = 0;
//	sint32			coord = 0;
//	TGameCycle		timeOnGround = 0;
//	sint16			slotCount = 0;
//	uint32			carrionSheetId = 0;
//	CSheetId		sheetId;
//	bool			destroyable = false;
//	bool			dropable = false;
//	CEntityId		creatorID;
//	uint32			hp;
//	uint32			recommended;
//	CItemCraftParameters	craftParameters;
//	float			loadHpRemain = 0;
//	sint16			clientInventoryPos = 0;
//	uint32			sapLoad = 0;
//	vector<CSheetId>	enchantements;
//
//	if ( characterSerialVersion >= 32 )
//			f.serial(slotImage);
//
//	if( characterSerialVersion >= 17 )
//	{
//		// load item infos
//		f.serial( sheetId );
//		f.serial(owner);
//		if( owner != CEntityId::Unknown )
//			f.serial(slot);
//		else
//		{
//			f.serial(coord);
//			f.serial(coord);
//			f.serial(coord);
//		}
//		
//		f.serial(timeOnGround);
//		f.serial(slotCount);
//		f.serial(carrionSheetId);
//
//		f.serial( destroyable );
//		if ( characterSerialVersion >= 19 )
//			f.serial(dropable);
//		
//		f.serial( creatorID );
//		
//		f.serial( hp );
//		f.serial( recommended );
//		f.serial( craftParameters );
//
//		if ( characterSerialVersion >= 55 )
//			f.serial(loadHpRemain);
//
//		std::vector<CSheetId> RmUsedCraft;
//		f.serialCont( RmUsedCraft );
//
//		f.serial(clientInventoryPos);
//
//		if( characterSerialVersion >= 20 )
//		{
//			f.serial( sapLoad );
//			f.serialCont( enchantements );
//		}
//
//		// load children count
//		uint32 childrenCount = 0; 
//		f.serial( childrenCount );
//		// load non null children count
//		uint32 nonNullChildrenCount = 0;
//		f.serial( nonNullChildrenCount );
//
//		// this item CAN'T be a stack !
//		nlassert(sheetId != CSheetId("stack.sitem"));
//
//		// here is the real interesting thing, read of sub old items to fill the inventory
//		for( uint32 i = 0; i < nonNullChildrenCount; ++i )
//		{
//			uint32 index;
//			f.serial( index );
//			CGameItemPtr  item;
//			item.newItem();
//
//			if( item!=NULL )
//			{
//				// we use the legacy load for item, this convert old stack format into new item
//				item->legacyLoad(f, characterSerialVersion, ownerPtr);
//				uint32 slot;
//				if (isRefInventory())
//				{
//					slot = item->_RefInventorySlot;
//					item->_RefInventorySlot = CInventoryBase::INVALID_INVENTORY_SLOT;
//				}
//				else
//				{
//					slot = item->_InventorySlot;
//					item->_InventorySlot = CInventoryBase::INVALID_INVENTORY_SLOT;
//				}
//				forceLoadItem(item, slot);
//			}
//			else
//			{
//				nlwarning("<CGameItem::load>Can't allocate child item %d", i);
//				return;
//			}
//		}
//
//	}
//	else // load more older item format
//	{
//		float dummy_f;
//		uint16 dummy_uint16;
//		uint32 dummy_uint32;
//		bool dummy_bool;
//
//		// load item infos
//		f.serial( sheetId );
//		f.serial( owner );
//		if( owner != CEntityId::Unknown )
//		{
//			f.serial(slot);
//		}
//		else
//		{
//			f.serial(coord);
//			f.serial(coord);
//			f.serial(coord);
//		}
//
//		if( characterSerialVersion >= 8 )
//		{
//			f.serial( dummy_f );
//			f.serial( dummy_f );
//			f.serial( dummy_f );
//		}
//
//		f.serial( dummy_uint16 ); //uint16
//		f.serial( hp ); //uint32
//	
//		if( characterSerialVersion < 7 )
//			f.serial( dummy_uint16 );
//		else if( characterSerialVersion == 7 )
//			f.serial( dummy_f );
//		
//		if( characterSerialVersion < 8 )
//			f.serial( dummy_f );
//		f.serial( dummy_f );
//
//		f.serial( dummy_uint16 );
//		f.serial( dummy_uint16 );
//		
//		if( characterSerialVersion < 8 )
//			f.serial( dummy_f );
//		f.serial( dummy_uint16 );
//		
//		f.serial( dummy_uint32 );
//		f.serial( dummy_uint16 );
//		f.serial( dummy_uint32 );
//		f.serial( dummy_uint16 );
//		f.serial( dummy_uint16 );
//		f.serial( dummy_bool );
//
//		vector< SProtection > dummy_protection;
//		f.serialCont(dummy_protection);
//
//		f.serial( timeOnGround );
//		f.serial( slotCount );
//		uint8 col;
//		f.serial( col );
//		f.serial( carrionSheetId );
//		f.serial( destroyable );
//		f.serial( creatorID );
//
//		// load children count
//		uint32 childrenCount = 0; 
//		f.serial( childrenCount );
//
//		// load non null children count
//		uint32 nonNullChildrenCount = 0;
//		f.serial( nonNullChildrenCount );
//
//		nlassert(sheetId != CSheetId("stack.sitem"));
//
//		for(uint i = 0; i < nonNullChildrenCount; ++i )
//		{
//			// load child index
//			uint32 index;
//			f.serial( index );
//			// load child
//			CGameItemPtr  item;
//			item.newItem();
//			if( item!=NULL )
//			{
//				item->legacyLoad(f, characterSerialVersion, ownerPtr);
//				uint32 slot;
//				if (isRefInventory())
//				{
//					slot = item->_RefInventorySlot;
//					item->_RefInventorySlot = CInventoryBase::INVALID_INVENTORY_SLOT;
//				}
//				else
//				{
//					slot = item->_InventorySlot;
//					item->_InventorySlot = CInventoryBase::INVALID_INVENTORY_SLOT;
//				}
//				forceLoadItem(item, slot);
//			}
//			else
//			{
//				nlwarning("<CGameItem::load> Can't allocate child item %d", i);
//				return;
//			}
//		}
//
//		// the client position has been added with Character version 4, for older item, just init it wiht -1
//		// we do this to keep compatibility with previous character backup version
//		if (characterSerialVersion >= 4)
//			f.serial(clientInventoryPos);
//
//		// serialize raw material used for craft item
//		if( characterSerialVersion >= 10 )
//		{
//			std::vector<CSheetId> RmUsedCraft;
//			f.serialCont( RmUsedCraft );
//		}
//	}
//}

