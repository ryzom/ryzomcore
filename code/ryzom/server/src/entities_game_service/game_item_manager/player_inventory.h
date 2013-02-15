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


#ifndef PLAYER_INVENTORY_H
#define PLAYER_INVENTORY_H

#include "nel/misc/smart_ptr.h"
#include "game_share/inventories.h"
#include "game_item_manager/game_item_ptr.h"
#include "game_item_manager/game_item.h"

//#include "game_item.h"

class CEntityBase;
class CCharacter;
class CGameItemPtr;
class CStaticItem;

class IInventoryView;

class CInventoryBase;
class IInventoryView;

typedef NLMISC::CSmartPtr<CInventoryBase>	CInventoryPtr;
typedef NLMISC::CSmartPtr<IInventoryView> CInventoryViewPtr;


// ****************************************************************************

/**
 * CCharacterOwned
 *
 * \author Borris 'Sonix' Boucher
 * \author Nevrax France
 * \date February 2005
 */
class CCharacterOwned
{
public:
	void setCharacter(CCharacter *character);
	CCharacter *getCharacter();

private:
	CCharacter		*_Character;
};

/**
 * CInventoryBase
 *
 * An inventory hold a set of item.
 *
 * \author Boris 'Sonix' Boucher
 * \author Nevrax France
 * \date February 2005
 */
class CInventoryBase : public NLMISC::CRefCount
{
public:
	/// Define constants for inventory operation and inquiries
	enum TInventoryOpResult
	{
		ior_overbulk,
		ior_overweight,
		/// trying to stack item that are not compatible
		ior_item_incompatible,
		/// trying to add more element in a stack than authorized in item def
		ior_stack_oversize,
		/// trying to remove more element from a stack than the current stack size
		ior_stack_undersize,
		/// trying to remove locked items
		ior_item_locked,
		/// no more free slot (for slot limited inventory)
		ior_no_free_slot,
		/// access to the inventory is denied
		ior_access_denied,
		/// no problem
		ior_ok,
		/// abnormal problem
		ior_error
	};


	DECLARE_PERSISTENCE_METHODS_WITH_APPLY_ARG(CCharacter * owner)

	/// ctor
	CInventoryBase();

	/// virtual dtor
	virtual ~CInventoryBase();

	/// Set inventory id
	void setInventoryId(INVENTORIES::TInventory inventoryId);
	/// Get the inventory id
	INVENTORIES::TInventory getInventoryId();

	/** set the size in slot of the inventory
	 *	Each slot allow an item to be contained.
	 */
	void setSlotCount(uint size);
	/// Return the inventory size in slot
	uint getSlotCount() const;
	/// Return the number of slots in use (can be used to check if inventory empty or not)
	uint getUsedSlotCount() const;
	/// Return the number of free slots
	uint getFreeSlotCount() const;

	/// Return the total weight of the inventory
	uint32 getInventoryWeight() const;
	/// Return the total bulk of the inventory
	uint32 getInventoryBulk() const;
	
	//@{
	//@name Item list manipulation
	/** Insert an item in the inventory
	 *	If an item already own the specified slot,
	 *	this old item is deleted.
	 *	If slot is set to INSERT_IN_FIRST_FREE_SLOT,
	 *	the method look for the first available slot.
	 * AUTOSTACK :
	 *  After a insertItem that is successful DO NOT use the item anymore !
	 *  in case of failure the item remains valid
	 *	Warning: There can be several destination slots filled.
	 *  They are reported in the callbacks of IInventoryView by onItemStackSizeChanged() and
	 *  onItemChanged() with the itc_inserted flag.
	 */
	virtual TInventoryOpResult insertItem(CGameItemPtr &item, uint32 slot = INVENTORIES::INSERT_IN_FIRST_FREE_SLOT, bool autoStack = false);
	/// Delete the nth item from this inventory
	virtual void deleteItem(uint32 slot);
	/// Remove the nth item from this inventory
	virtual CGameItemPtr removeItem(uint32 slot, uint32 quantity = INVENTORIES::REMOVE_MAX_STACK_QUANTITY, TInventoryOpResult * res = NULL);
	/** Move an item from one slot to another slot
	 *	If the destination slot already contains an item,
	 *	the two items are swaped.
	 */
//	void	moveItem(uint32 slotFrom, uint32 slotTo);
	/** Delete item from a stack or unstacked item, quantity can be lower than stack size, the stack still exist with updated quantity.
	 *  If quantity is bigger or egal than the item stack, the stack is deleted.
	 *  Effective deleted item is returned
	 */
	virtual uint32 deleteStackItem(uint32 slot, uint32 quantity = INVENTORIES::REMOVE_MAX_STACK_QUANTITY, TInventoryOpResult * res = NULL);

	// Return the item at the nth slot 
	CGameItemPtr getItem(uint32 slot) const;

	/// NEVER use this method. ONLY used when loading inventory from save files.
	/// It does ignore bulk and weight limitations.
	virtual void forceLoadItem(CGameItemPtr &item, uint32 slot);
	//@}


	/// Legacy load function, read the item serialisation
//	void legacyLoad(NLMISC::IStream &f, uint16 savedVersion, CCharacter *owner);

	/// Create a full copy of this inventory (and copying each item inside). Override in derived class to copy specialized class
	virtual CInventoryPtr	getInventoryCopy() const;

	/// Empty the inventory, deleting all contained items (or unreferencing for ref inventory)
	virtual void clearInventory();

	/// Force an update of the given slot for all views associated to the inventory
	void forceViewUpdateOfSlot(uint32 slot);

	/// Force an update of the whole inventory for all views associated to the inventory
	/// \param skipEmptySlots if true empty slots are not updated
	void forceViewUpdateOfInventory(bool skipEmptySlots = false);

	//@{
	//@name Test inventory operation
	/// test if we can stack an item on an existing one
	TInventoryOpResult canStackItem(const CGameItemPtr &item, uint32 slotDst);
	/// test if we can remove an item (or a stack part)
	//TInventoryOpResult canRemoveItem(uint32 slot, uint32 quantity);
	//@}

	//@{
	//@name Constraints inquiries in derived class
	/// Get the maximum bulk accepted, default is almost unlimited
	virtual uint32 getMaxBulk() const { return UINT_MAX;};
	/// Get the maximum weight accepted, default is almost unlimited
	virtual uint32 getMaxWeight() const { return UINT_MAX;};
	/// Get the maximum slot accepted, default is almost unlimited
	virtual uint32 getMaxSlot() const { return UINT_MAX;};
	//@}
	
	//@{
	//@name Static inter inventory operation
	/** Move an item from one inventory to another.
	 *	If the destination slot is specified and the destination slot is not 
	 *	empty, then a stacking operation is attempted.
	 *	If the quantity is not specified or greater than the actual size of the stack
	 *	(default is UINT_MAX), then all the item stack is moved.
	 *	Otherwise, if a quantity is specified and is less than the source stack
	 *	size, then only a part of the source stack is moved.
	 *	If any error occur during transfer, the method return the error
	 *	code and the two inventory are left unchanged.
	 *	The boolean bestEffort will try to move as many stack elements as possible 
	 *	is accordance with the inventory constraint (mostly bulk constraint) [NOT IMPLEMENTED YET]
	 *
	 *	Warning: As in insertItem with autostacking, there can be several destination slots filled.
	 *  They are reported in the callbacks of IInventoryView by onItemStackSizeChanged() and
	 *  onItemChanged() with the itc_inserted flag.
	 */
	static TInventoryOpResult moveItem(
		CInventoryBase* srcInv, uint32 srcSlot,
		CInventoryBase* dstInv, uint32 dstSlot = INVENTORIES::INSERT_IN_FIRST_FREE_SLOT,
		uint32 quantity = UINT_MAX, bool bestEffort = false);
	//@}

	//@{
	//@name Event callback for derived class
	/// An item has changed (can be a removing)
	void onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags);

	/// The inventory information has changed (like total bulk or weight)
	void onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags);

	//@name item callback
	/** Callback from item when an item stack size change
	 *	This callback is used to update the weight and bulk
	 *	of the inventory
	 */
	virtual void onItemStackSizeChanged(uint32 slot, uint32 previousStackSize);
	//@}

	//@{
	//@name Inventory view control
	/// Bind a view to this inventory
	virtual void bindView(CInventoryViewPtr inventoryView);
	/// Unbind a view from this inventory
	virtual void unbindView(CInventoryViewPtr inventoryView);
	//@}

	/// dump the inventory
	virtual void dumpInventory(NLMISC::CLog & log, bool dumpItems = true) const;

	/// return true if inventory is a reference inventory
	virtual bool isRefInventory() const { return false; }

	/// update prerequisits of all items in the inventory
	virtual void updateAllItemPrerequisit();

protected:
	/// look for the first empty slot (assert if no free slot)
	uint32 getFirstFreeSlot() const;
	/// Recompute weight and bulk according to item qt variation
	void updateWeightAndBulk(const CGameItemPtr &item, sint32 deltaQt);
	/// Copy the inventory content from this inventory to another
	void copyContentToInventory(CInventoryBase *destInv) const;
	/// Insert item implementation
	virtual TInventoryOpResult doInsertItem(CGameItemPtr &item, uint32 slot, bool autoStack, bool ignoreWeightAndBulk);

protected:
	/// Size of the inventory. Same as _Items.size()
	uint32			_SlotCount;
	/// Number of free slots (number of NULL items in the vector)
	uint32			_FreeSlotCount;

	/// Total inventory weight
	uint32			_InventoryWeight;
	/// Total inventory bulk
	uint32			_InventoryBulk;

	/// The vector of item contained by this inventory
	std::vector<CGameItemPtr>	_Items;

	/// The id of the inventory in the owner inventories list
	INVENTORIES::TInventory		_InventoryId;

	typedef std::list<CInventoryViewPtr>	TViewCont;
	/// The list of view binded to this inventory
	TViewCont	_InventoryViews;
};

/**
 * CRefInventory
 *
 * This class implement a reference inventory.
 *	Reference inventory only store reference on a list
 *	of item, they don't 'own' the item.
 *
 *	When deleting item in the reference inventory,
 *	they are only dereferenced. 
 *
 *	An item is own by one inventory and can be
 *	reference by one reference inventory.
 *
 *	If the item is deleted, it is automatically removed
 *	from the reference inventory.
 *
 * \author Borris 'Sonix' Boucher
 * \author Nevrax France
 * \date February 2005
 */
class CRefInventory : public CInventoryBase
{
public:
	virtual void deleteItem(uint32 slot);
	virtual CGameItemPtr removeItem(uint32 slot, uint32 quantity = INVENTORIES::REMOVE_MAX_STACK_QUANTITY, TInventoryOpResult * res = NULL);
	/// Empty the inventory, unreferencing all items, but not destroying them
	virtual void clearInventory();

	virtual CInventoryPtr	getInventoryCopy() const;

	virtual bool isRefInventory() const { return true; }

protected:
	/// Insert item implementation
	virtual TInventoryOpResult doInsertItem(CGameItemPtr &item, uint32 slot, bool autoStack, bool ignoreWeightAndBulk);
};

/**
 * IInventoryView
 *
 * This interface define the basic behavior
 *	for inventory view.
 *	It is a virtual class that must be specialized for use.
 *
 * \author Borris 'Sonix' Boucher
 * \author Nevrax France
 * \date February 2005
 */
class IInventoryView : public NLMISC::CRefCount
{
public:
	/// ctor
	IInventoryView();

	/// virtual dtor
	virtual ~IInventoryView();

	//@{
	//@name Inventory bind control
	/// Bind this view to the specified inventory.
	void bindToInventory(CInventoryPtr inventory);
	/// Unbind the view from the inventory
	void unbindFromInventory();
	/// Return the binded inventory
	CInventoryBase *getInventory() const;
	//@}

	//@{
	//@name Virtual to overload
	/// An item has changed (can be a removing)
	virtual void onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags) =0;
	/// The inventory information has changed (like total bulk or weight)
	virtual void onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags) =0;
	/** Callback from item when an item stack size change
	 *	This callback is used to update the weight and bulk
	 *	of the inventory
	 */
	virtual void onItemStackSizeChanged(uint32 slot, uint32 previousStackSize) =0;
	/** Force an update of the information related to an item 
	 *	In case of client operation canceled by the server, this method is
	 *	use to reset the client information to normal.
	 */
	virtual void forceSlotUpdate(uint32 slot) =0;
	//@}

	/// update prerequisit state of item
	virtual void updateItemPrerequisit(uint32 slot) {}

private:
	/// Pointer on the inventory this view is binded on (can be NULL)
	CInventoryBase	*_BindedInventory;
};

/**
 * CCharacterInvView
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date February 2005
 */
class CCharacterInvView : public IInventoryView
{
public:

	CCharacterInvView() : _Character(NULL) {}

	/// MUST be called before you do any other operation
	void setCharacter(CCharacter *character);
	/// get the character
	CCharacter *getCharacter();

	// info version mechanism
	virtual void onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags);

	/** Callback from item when an item stack size change
	 *	This callback is used to update the weight and bulk
	 *	of the inventory
	 */
	virtual void onItemStackSizeChanged(uint32 slot, uint32 previousStackSize);

	/** Force an update of the information related to an item 
	 *	In case of client operation canceled by the server, this method is
	 *	use to reset the client information to normal.
	 */
	virtual void forceSlotUpdate(uint32 slot);

	/// update prerequisit state of item
	virtual void updateItemPrerequisit(uint32 slot);

protected:
	/** Update the given client slot with item infos.
	 *  If item is NULL, client slot is updated as empty on the client.
	 */
	virtual void updateClientSlot(uint32 clientSlot, const CGameItemPtr item);

private:
	CCharacter		*_Character;
};


#endif // PLAYER_INVENTORY_H
