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



#ifndef RY_INVENTORY_MANAGER_H
#define RY_INVENTORY_MANAGER_H

#include "nel/misc/cdb_leaf.h"
#include "dbgroup_list_sheet_text.h"
#include "dbgroup_list_sheet.h"
#include "game_share/item_infos.h"
#include "game_share/temp_inventory_mode.h"
#include "game_share/inventories.h"
#include "game_share/bot_chat_types.h"

namespace NLMISC{
class CCDBNodeBranch;
}
class CDBCtrlSheet;


const uint MAX_TEMPINV_ENTRIES = INVENTORIES::NbTempInvSlots;
const uint MAX_BAGINV_ENTRIES = INVENTORIES::NbBagSlots;
const uint MAX_HANDINV_ENTRIES = 2;
const uint MAX_EQUIPINV_ENTRIES = 19;
const uint MAX_ANIMALINV_ENTRIES = INVENTORIES::NbPackerSlots;
const uint MAX_GUILDINV_ENTRIES = INVENTORIES::NbGuildSlots;
const uint MAX_ROOMINV_ENTRIES = INVENTORIES::NbRoomSlots;
// This is the personal player inventory max (bag and animal)
const uint MAX_PLAYER_INV_ENTRIES = std::max(MAX_BAGINV_ENTRIES, MAX_ANIMALINV_ENTRIES);
// db path for the local inventory
#define LOCAL_INVENTORY "LOCAL:INVENTORY"
#define SERVER_INVENTORY "SERVER:INVENTORY"


// ***************************************************************************
/** Image of an item in the database
  * Unavailable fields are set to NULL
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date September 2003
  */
class CItemImage
{
public:
	NLMISC::CCDBNodeLeaf *Sheet;
	NLMISC::CCDBNodeLeaf *Quality;
	NLMISC::CCDBNodeLeaf *Quantity;
	NLMISC::CCDBNodeLeaf *UserColor;
	NLMISC::CCDBNodeLeaf *Price;
	NLMISC::CCDBNodeLeaf *Weight;
	NLMISC::CCDBNodeLeaf *NameId;
	NLMISC::CCDBNodeLeaf *InfoVersion;
	NLMISC::CCDBNodeLeaf *ResaleFlag;

public:
	// ctor
	CItemImage();
	// build from a branch
	void build(NLMISC::CCDBNodeBranch *branch);
	// shortcuts to avoid NULL pointer tests
	uint32 getSheetID() const						{ return (uint32)			(Sheet ? Sheet->getValue32() : 0); }
	uint16 getQuality() const						{ return (uint16)			(Quality ? Quality->getValue16() : 0); }
	uint16 getQuantity() const						{ return (uint16)			(Quantity ? Quantity->getValue16() : 0); }
	uint8  getUserColor() const						{ return (uint8)			(UserColor ? UserColor->getValue16() : 0); }
	uint32 getPrice() const							{ return (uint32)			(Price ? Price->getValue32() : 0); }
	uint32 getWeight() const						{ return (uint32)			(Weight ? Weight->getValue32() : 0); }
	uint32 getNameId() const						{ return (uint32)			(NameId ? NameId->getValue32() : 0); }
	uint8  getInfoVersion() const					{ return (uint8)			(InfoVersion ? (uint8) InfoVersion->getValue8() : 0); }
	uint8  getResaleFlag() const					{ return (uint8)			(ResaleFlag ? (uint8) ResaleFlag->getValue8() : 0); }
	bool   getLockedByOwner() const                 { return (bool)             (ResaleFlag ? (ResaleFlag->getValue8() == BOTCHATTYPE::ResaleKOLockedByOwner) : false); }
	//
	void   setSheetID(uint32 si)					{ if (Sheet) Sheet->setValue32((sint32) si); }
	void   setQuality(uint16 quality)				{ if (Quality) Quality->setValue16((sint16) quality); }
	void   setQuantity(uint16 quantity)				{ if (Quantity) Quantity->setValue16((sint16) quantity); }
	void   setUserColor(uint8 uc)					{ if (UserColor) UserColor->setValue8((sint8) uc); }
	void   setPrice(uint32 price)					{ if (Price) Price->setValue32((sint32) price); }
	void   setWeight(uint32 wgt)					{ if (Weight) Weight->setValue32((sint32) wgt); }
	void   setNameId(uint32 nid)					{ if (NameId) NameId->setValue32((sint32) nid); }
	void   setInfoVersion(uint8 iv)					{ if (InfoVersion) InfoVersion->setValue8((sint8) iv); }
	void   setResaleFlag(uint8 resale)				{ if (ResaleFlag) ResaleFlag->setValue8(resale); }
};


// ***************************************************************************
/** Extra Item Info retrieved via ItemInfo system
  */
class	CClientItemInfo : public CItemInfos
{
public:
	// System
	uint16				InfoVersionFromMsg;
	uint16				InfoVersionFromSlot;
	// This is the InfoVersionFromSlot when last request was sent to server
	uint16				InfoVersionSlotServerWaiting;

	CClientItemInfo()
	{
		InfoVersionFromMsg= 0;
		InfoVersionFromSlot= 0;
		InfoVersionSlotServerWaiting= 0;
	}

	/// Set InfoVersion from Info message (info requested by the player)
	void			readFromImpulse(const CItemInfos &itemInfo);

	/// Set InfoVersion from a refresh message (to prevent missing a change when InfoVersion in the database has reached a complete cycle)
	void			refreshInfoVersion(uint8 infoVersion) { InfoVersionFromMsg= infoVersion; }
};


class	IItemInfoWaiter
{
public:
	IItemInfoWaiter() {ItemSlotId= 0; ItemSheet= 0;}
	virtual ~IItemInfoWaiter() {}
	// The item SheetId. If differ from current sheet in the SlotId, the infos are not updated / requested
	uint			ItemSheet;
	// The item SlotId to retrieve info.
	uint			ItemSlotId;

	// Called when the info is received for this slot.
	virtual void	infoReceived() =0;
};


// ***************************************************************************
/** This manager gives direct access to inventory slots (bag, temporary inventory, hands, and equip inventory)
  * This also give access to player money
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date September 2003
  */
class CInventoryManager
{
public:
	// db path for all the inventories (without the SERVER: prefix)
	static const std::string InventoryDBs[];
	static const uint	InventoryIndexes[];
	static const uint	NumInventories;

public:
	// get the unique instance of that manager
	static CInventoryManager *getInstance();

	// release singleton
	static void releaseInstance();

	/** Init the manager. This should be done after the local database has been created,
      * because shortcuts pointers are made to the db leaf that represents the inventory.
	  * Interface manager should also have been created
      */
	void init();
	// LOCAL INVENTORY
		// get item of bag (local inventory)
		CItemImage &getBagItem(uint index);
		// get temporary item (local inventory)
		CItemImage &getTempItem(uint index);
		// get hand item (local inventory) can be NULL (nothing in the hand)
		CItemImage *getHandItem(uint index);
		// get equip item (local inventory)
		CItemImage *getEquipItem(uint index);
		// get hand item (local inventory)
		CDBCtrlSheet *getHandSheet(uint index);
		// get equip item (local inventory)
		CDBCtrlSheet *getEquipSheet(uint index);
		// get/set money
		uint64		getMoney() const;
		void		setMoney(uint64 value);
		// get item of pack animal (local inventory). beastIndex ranges from 0 to MAX_INVENTORY_ANIMAL-1
		CItemImage &getPAItem(uint beastIndex, uint index);
		// get the item Image for the given inventory. assert if bad inventory
		CItemImage &getLocalItem(uint inv, uint index);

	// SERVER INVENTORY
		// get item of bag (local inventory)
		CItemImage &getServerBagItem(uint index);
		// get temporary item (local inventory)
		CItemImage &getServerTempItem(uint index);
		// get hand item (local inventory)
		CItemImage *getServerHandItem(uint index);
		// get equip item (local inventory)
		CItemImage *getServerEquipItem(uint index);
		// get/set server money
		uint64		getServerMoney() const;
		void		setServerMoney(uint64 value);
		// get item of pack animal (server inventory). beastIndex ranges from 0 to MAX_INVENTORY_ANIMAL-1
		CItemImage &getServerPAItem(uint beastIndex, uint index);
		// get the item Image for the given inventory. assert if bad inventory
		CItemImage &getServerItem(uint inv, uint index);

	// Drag'n'Drop Management
		enum TFrom { Slot, TextList, IconList, Nowhere };
		void beginDrag(CDBCtrlSheet *pCS, TFrom eFromWhere);
		void endDrag();
		bool isDragging() { return DNDCurrentItem != NULL; }
		// Is the dragged sheet comes from inventory list
		bool isDraggingFromTextList() { return DNDFrom == TextList; }
		bool isDraggingFromIconList() { return DNDFrom == IconList; }
		bool isDraggingFromSlot() { return DNDFrom == Slot; }
		CDBCtrlSheet *getDraggedItem() { return DNDCurrentItem; }

	// Get from an interface object the corresponding database entry (ie: LOCAL:INVENTORY:EQUIP:4)
	std::string getDBIndexPath(CDBCtrlSheet *);
	// Check if an item is a 2 hand item
	bool is2HandItem(uint32 sheetID);
	// Check if an item is a melee weapon item
	bool isMeleeWeaponItem(uint32 sheetID);
	// Check if an item is a range weapon item
	bool isRangeWeaponItem(uint32 sheetID);
	bool isDagger(uint32 sheetID);
	bool isSword(uint32 sheetID);
	// Check if an item is a forage tool
	bool isForageToolItem(uint32 sheetID);
	// Get the Hand item sheet
	uint32 getRightHandItemSheet() const { return getHandItemSheet(true); }
	uint32 getLeftHandItemSheet() const { return getHandItemSheet(false); }
	bool isLeftHandItemCompatibleWithRightHandItem(uint32 leftHandSheet, uint32 rightHandSheet, uint32 lastRightHandSheet);


	// Bag wearing
		// lock an item in the bag because it is weared (done on interface elements of the 2 lists (text and icons))
		void wearBagItem(sint32 bagEntryIndex);
		// unlock an item in the bag because it is unweared
		void unwearBagItem(sint32 bagEntryIndex);
		// check if an item in the bag is weared or not
		bool isBagItemWeared(sint32 bagEntryIndex);
		// Equip a part of the player with a bag entry (the params are path in the database)
		// ie : bagPath = LOCAL:INVENTORY:BAG:0 and invPath = LOCAL:INVENTORY:HAND:0
		void equip(const std::string &bagPath, const std::string &invPath);
		// UnEquip a part of the player (same format as equip method)
		void unequip(const std::string &invPath);
		// auto equip an item (given by index) from the bag (return true if equipped)
		bool autoEquip(sint bagEntryIndex, bool allowReplace);

		void dropOrDestroyItem(CDBCtrlSheet *item, NLMISC::CBitMemStream &out, uint16 quantity);
		// check all equipement slot and reset if they point to an invalid bag index
		void checkIndexInBagIntegrity();

		// Get the current bag bulk
		static double getBranchBulk(const std::string &basePath, uint16 startItemIndex, uint16 numItems);
		// Get the number of used and max slots
		static void getBranchSlotCounts(const std::string &basePath, uint& nbUsedSlots, uint& nbMaxSlots );
		// 0 bag, 1 - 6 pack animal 1 to 5, 7 temp inv
		double getBagBulk(uint32 inventoryIndex);
		double getMaxBagBulk(uint32 inventoryIndex);
		double getItemBulk(uint32 sheetID);
		// item should have :SHEET and :QUANTITY
		bool isSpaceInAllBagsForItem(CDBCtrlSheet *item);
		bool isSpaceInBagForItem(CDBCtrlSheet *item, uint32 quantity, uint32 bagId);

	// ItemExtraInfo management. From each slot is a unique uint16
		uint16				getItemSlotId(CDBCtrlSheet *ctrl);
		uint16				getItemSlotId(const std::string &itemDb, uint slotIndex);
		const	CClientItemInfo	&getItemInfo(uint slotId) const;
		uint				getItemSheetForSlotId(uint slotId) const;
		// Returns true if the item info version already matches
		bool				isItemInfoUpToDate(uint slotId);
		// Add a Waiter on ItemInfo (ItemHelp opening). no-op if here, but reorder (returns true if the version already matches or if waiter is NULL)
		void				addItemInfoWaiter(IItemInfoWaiter *waiter);
		// remove a Waiter on ItemInfo (ItemHelp closing). no-op if not here. NB: no delete
		void				removeItemInfoWaiter(IItemInfoWaiter *waiter);
		// Called on impulse
		void				onReceiveItemInfo(const CItemInfos &itemInfo);
		void				onRefreshItemInfoVersion(uint16 slotId, uint8 infoVersion);
		// Log for debug
		void				debugItemInfoWaiters();

		void				sortBag();

	// Animal Inventories
		// Is the inventory present? if true, it may still not be available for modification (eg: animal too far)
		bool				isInventoryPresent(INVENTORIES::TInventory invId);
		// Is the inventory present and available for modification?
		bool				isInventoryAvailable(INVENTORIES::TInventory invId);
		// Is the inventory empty or not present?
		bool				isInventoryEmpty (INVENTORIES::TInventory invId);


		enum TInvType { InvBag, InvPA0, InvPA1, InvPA2, InvPA3, InvGuild, InvRoom, InvUnknown };
		static TInvType invTypeFromString(const std::string &str);


private:
	// LOCAL INVENTORY
		CItemImage Bag[MAX_BAGINV_ENTRIES];
		CItemImage TempInv[MAX_TEMPINV_ENTRIES];
		sint32 Hands[MAX_HANDINV_ENTRIES];
		CDBCtrlSheet *UIHands[MAX_HANDINV_ENTRIES];
		sint32 Equip[MAX_EQUIPINV_ENTRIES];
		CDBCtrlSheet *UIEquip[MAX_EQUIPINV_ENTRIES];
		CDBCtrlSheet *UIEquip2[MAX_EQUIPINV_ENTRIES];
		NLMISC::CCDBNodeLeaf *Money;
		CItemImage PAInv[MAX_INVENTORY_ANIMAL][MAX_ANIMALINV_ENTRIES];
	// SERVER INVENTORY
		CItemImage ServerBag[MAX_BAGINV_ENTRIES];
		CItemImage ServerTempInv[MAX_TEMPINV_ENTRIES];
		sint32 ServerHands[MAX_HANDINV_ENTRIES];
		sint32 ServerEquip[MAX_EQUIPINV_ENTRIES];
		NLMISC::CCDBNodeLeaf *ServerMoney;
		CItemImage ServerPAInv[MAX_INVENTORY_ANIMAL][MAX_ANIMALINV_ENTRIES];
	// Drag'n'Drop
		TFrom			DNDFrom;
		CDBCtrlSheet	*DNDCurrentItem;

	// ItemExtraInfo management.
		typedef std::map<uint, CClientItemInfo>		TItemInfoMap;
		TItemInfoMap							_ItemInfoMap;
		typedef std::list<IItemInfoWaiter*>		TItemInfoWaiters;
		TItemInfoWaiters						_ItemInfoWaiters;

	// Cache to know if bag is locked or not, because of item worn
		bool			BagItemEquipped[MAX_BAGINV_ENTRIES];

private:
	/// Singleton's instance
	static CInventoryManager *_Instance;

	// ctor
	CInventoryManager();
	// dtor
	~CInventoryManager();

	// init an array of items from a db branch
	void initItemArray(const std::string &dbBranchName, CItemImage *dest, uint numItems);
	// init array of int that represents indirection to the bag
	void initIndirection(const std::string &dbbranch, sint32 *indices, sint32 nbIndex, bool putObs = false);

	// Get the Hand item sheet
	uint32 getHandItemSheet( bool rightHand ) const;

	// ItemExtraInfo management.
		void			onTradeChangeSession();
		void			onReceiveItemSheet(NLMISC::ICDBNode* node);
		void			onReceiveItemInfoSlotVersion(NLMISC::ICDBNode* node);
		void			updateItemInfoQueue();
		void			updateItemInfoWaiters(uint slotId);
		class	CItemInfoSlotVersionObs : public NLMISC::ICDBNode::IPropertyObserver
		{
		public:
			virtual void update(NLMISC::ICDBNode* node);
		};
		class	CItemSheetObs : public NLMISC::ICDBNode::IPropertyObserver
		{
		public:
			virtual void update(NLMISC::ICDBNode* node);
		};
		class	CItemInfoTradeObs : public NLMISC::ICDBNode::IPropertyObserver
		{
		public:
			virtual void update(NLMISC::ICDBNode* node);
		};
		CItemInfoTradeObs			_DBTradeInfoObs;
		CItemInfoSlotVersionObs		_DBInfoSlotVersionObs;
		CItemSheetObs				_DBItemSheetObs;
		friend class CItemInfoTradeObs;
		friend class CItemInfoSlotVersionObs;
		friend class CItemSheetObs;

	// Equipment observer
		class CDBEquipObs : public NLMISC::ICDBNode::IPropertyObserver
		{
		public:
			virtual void update(NLMISC::ICDBNode* node);
		};
		CDBEquipObs _DBEquipObs;
		friend class CDBEquipObs;

	// Bag observer for auto equipment (put only on the sheet leaf)
		class CDBBagObs : public NLMISC::ICDBNode::IPropertyObserver
		{
		public:
			virtual void update(NLMISC::ICDBNode* node);
		};
		CDBBagObs _DBBagObs;
};
// shortcut to access the inventory
inline CInventoryManager &getInventory() { return *CInventoryManager::getInstance(); }


// ***************************************************************************
/**
 * class used to manage the temporary inventory
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date August 2003
 */
class CTempInvManager
{

public:

	/// The singleton 's instance
	static CTempInvManager* getInstance()
	{
		if (!_Instance)
			_Instance = new CTempInvManager;
		return _Instance;
	}

	/// Destructor
	virtual ~CTempInvManager();

	// release singleton
	static void releaseInstance();

	// Called internally by the observer on the DB
	void update();

	// Called internally by the type observer on the DB
	void updateType();

	// Called internally by the forage observer on the DB
	void updateForageQQ( uint whichOne );

	// Launch temp_inv waiting for some incoming sheet (reset local DB)
	void open(TEMP_INV_MODE::TInventoryMode m);

	// Close temporary inventory (reset local DB)
	void close();

	bool isOpened();

	TEMP_INV_MODE::TInventoryMode getMode() { return _Mode; }

private:

	/// Constructor
	CTempInvManager();

	// Database management stuff
	class CDBObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode* node);
	};

	class CDBObsType : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode* node);
	};

	// Database management stuff, specialized for forage progress
	class CDBForageQQObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		CDBForageQQObs() : NLMISC::ICDBNode::IPropertyObserver(), WhichOne(~0), FullValue(0.0f) {}
		virtual void update(NLMISC::ICDBNode *node);
		uint	WhichOne;
		float	FullValue;
	};

	CDBObs _DBObs;
	CDBObsType _DBObsType;
	CDBForageQQObs _DBForageQQObs [2];

private:
	/// Singleton's instance
	static CTempInvManager*		_Instance;

	TEMP_INV_MODE::TInventoryMode _Mode;
};

// ***************************************************************************
/**
 * For Bag Management
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date October 2003
 */
struct SSortStruct
{
	CDBGroupListSheetText::CSheetChild	*SheetText;
	CDBGroupListSheet::CSheetChild		*SheetIcon;
	ucstring Pos;
	bool operator < (const SSortStruct &o) const { return Pos < o.Pos; }
};

// Helper to sort with only one way for bag and trade
void initStructForItemSort(std::vector<SSortStruct>&vTemp, sint32 sheetId, sint32 quality, sint32 indexInList, sint32 indexInDB);


// ***************************************************************************
/**
 * For Bag Management (shared structure between 2 lists (text and icon))
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date May 2004
 */
struct SBagOptions
{
	CInventoryManager::TInvType		InvType;

	NLMISC::CCDBNodeLeaf *DbFilterArmor;
	NLMISC::CCDBNodeLeaf *DbFilterWeapon;
	NLMISC::CCDBNodeLeaf *DbFilterTool;
	NLMISC::CCDBNodeLeaf *DbFilterMP;
	NLMISC::CCDBNodeLeaf *DbFilterMissMP;
	NLMISC::CCDBNodeLeaf *DbFilterTP;

	bool LastDbFilterArmor;
	bool LastDbFilterWeapon;
	bool LastDbFilterTool;
	bool LastDbFilterMP;
	bool LastDbFilterMissMP;
	bool LastDbFilterTP;
	// -----------------------
	SBagOptions()
	{
		InvType = CInventoryManager::InvUnknown;
		DbFilterArmor = DbFilterWeapon = DbFilterTool = DbFilterMP = DbFilterMissMP = DbFilterTP = NULL;
		LastDbFilterArmor = LastDbFilterWeapon = LastDbFilterTool = LastDbFilterMP = LastDbFilterMissMP = LastDbFilterTP = false;
	}

	bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

	bool isSomethingChanged(); // From last call ?

	bool getFilterArmor() const
	{
		if (DbFilterArmor == NULL) return true;
		return (DbFilterArmor->getValue8()!=0);
	}

	bool getFilterWeapon() const
	{
		if (DbFilterWeapon == NULL) return true;
		return (DbFilterWeapon->getValue8()!=0);
	}

	bool getFilterTool() const
	{
		if (DbFilterTool == NULL) return true;
		return (DbFilterTool->getValue8()!=0);
	}

	bool getFilterMP() const
	{
		if (DbFilterMP == NULL) return true;
		return (DbFilterMP->getValue8()!=0);
	}

	bool getFilterMissMP() const
	{
		if (DbFilterMissMP == NULL) return true;
		return (DbFilterMissMP->getValue8()!=0);
	}

	bool getFilterTP() const
	{
		if (DbFilterTP == NULL) return true;
		return (DbFilterTP->getValue8() != 0);
	}

	// Return true if the sheet can be displayed due to filters
	bool canDisplay(CDBCtrlSheet *pCS) const;
};


// ***************************************************************************
/**
 * Special list for bag (Text list)
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date September 2003
 */
class CDBGroupListSheetBag : public CDBGroupListSheetText
{
public:
	CDBGroupListSheetBag(const TCtorParam &param)
		: CDBGroupListSheetText(param)
	{
	}

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void checkCoords ();

	//////////////////////////////////////////////////////////////////////////

	// A child node
	struct	CSheetChildBag : public CDBGroupListSheetText::CSheetChild
	{
		// quality is important for ItemRequirement color
		sint32				LastQuality;
		CInterfaceProperty	CurrentQuality;

		virtual void updateViewText(CDBGroupListSheetText *pFather);
		virtual bool isSheetValid(CDBGroupListSheetText *pFather); // To filter
		virtual void init(CDBGroupListSheetText *pFather, uint index);
		virtual bool isInvalidated(CDBGroupListSheetText *pFather);
		virtual void update(CDBGroupListSheetText *pFather);

		CSheetChildBag() : CSheetChild()
		{
			LastQuality = -1;
		}
	};

	virtual CSheetChild *createSheetChild() { return new CSheetChildBag; }
	virtual bool swapable () const {return true;}
	virtual void onSwap (sint nDraggedSheet, sint nDroppedSheet);
	virtual void sort();

	CInventoryManager::TInvType getInvType() const { return _BO.InvType; }

	// Return true if the sheet can be displayed due to filters
	bool canDisplay(CDBCtrlSheet *pCS) { return _BO.canDisplay(pCS); }

private:

	SBagOptions	_BO;
};

// ***************************************************************************
/**
 * Special list for bag (The icon list)
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date September 2003
 */
class CDBGroupIconListBag : public CDBGroupListSheet
{
public:
	CDBGroupIconListBag(const TCtorParam &param)
		: CDBGroupListSheet(param)
	{
	}

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void checkCoords ();

	virtual CSheetChild *createSheetChild() { return new CSheetChildBag; }
	virtual void sort();

	CInventoryManager::TInvType getInvType() const { return _BO.InvType; }

	// Return true if the sheet can be displayed due to filters
	bool canDisplay(CDBCtrlSheet *pCS) const { return _BO.canDisplay(pCS); }

	//////////////////////////////////////////////////////////////////////////

	// A child node
	struct	CSheetChildBag : public CDBGroupListSheet::CSheetChild
	{
		virtual bool isSheetValid(CDBGroupListSheet *pFather);
	};

private:

	SBagOptions	_BO;
};


// ***************************************************************************
/**
 * Special list for filtering items according to the CLM (control launching modal)
 * \author Lionel Berenguier
 */
class CDBGroupListSheetFilterCLMSlot : public CDBGroupListSheet
{
public:
	CDBGroupListSheetFilterCLMSlot (const TCtorParam &param)
		: CDBGroupListSheet(param)
	{}

	virtual CSheetChild *createSheetChild() { return new CSheetChildFilter; }

	// A child node
	struct	CSheetChildFilter : public CDBGroupListSheet::CSheetChild
	{
		virtual bool isSheetValid(CDBGroupListSheet *pFather);
	};
};


// ***************************************************************************
/**
 * Special list for filtering items which are exchangeable only
 * \author Lionel Berenguier
 */
class CDBGroupListSheetFilterExchangeable : public CDBGroupListSheet
{
public:
	CDBGroupListSheetFilterExchangeable (const TCtorParam &param)
		: CDBGroupListSheet(param)
	{}

	virtual CSheetChild *createSheetChild() { return new CSheetChildFilter; }

	// A child node
	struct	CSheetChildFilter : public CDBGroupListSheet::CSheetChild
	{
		virtual bool isSheetValid(CDBGroupListSheet *pFather);
	};

	void sort();
	bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

private:
	SBagOptions	_BO;
};


// ***************************************************************************
// Interface Part
// ***************************************************************************

#define WIN_TEMPINV					"ui:interface:temp_inventory"
#define VIEW_TEMPINV_TITLE			"ui:interface:temp_inventory:title"

#define WIN_TEMPINV_TITLE_LOOT			"uiTILoot"
#define WIN_TEMPINV_TITLE_QUARTERING	"uiTIQuartering"
#define WIN_TEMPINV_TITLE_FORAGING		"uiTIForaging"
#define WIN_TEMPINV_TITLE_FORAGE_RESULT	"uiTIForageResult"
#define WIN_TEMPINV_TITLE_BAGFULL		"uiTIBagFull"
#define WIN_TEMPINV_TITLE_CRAFT			"uiTICraft"
#define WIN_TEMPINV_TITLE_MISSIONREWARD	"uiTIMissionItem" // in fact not only for rewards
#define WIN_TEMPINV_TITLE_CRYSTALLIZE	"uiTICrystallize"

#define WIN_TEMPINV_TITLE_ERROR			"uiTIError"

#define WIN_TEMPINV_TITLE_WAIT_LOOT				"uiTIWaitLoot"
#define WIN_TEMPINV_TITLE_WAIT_QUARTERING		"uiTIWaitQuartering"
#define WIN_TEMPINV_TITLE_WAIT_FORAGING			"uiTIWaitForaging"
#define WIN_TEMPINV_TITLE_WAIT_BAGFULL			"uiTIWaitBagFull"
#define WIN_TEMPINV_TITLE_WAIT_CRAFT			"uiTIWaitCraft"
#define WIN_TEMPINV_TITLE_WAIT_MISSIONREWARD	"uiTIWaitMissionItem"
#define WIN_TEMPINV_TITLE_WAIT_CRYSTALLIZE		"uiTIWaitCrystallize"

#define WIN_TEMPINV_TITLE_WAIT_ERROR			"uiTIError"

#define LIST_BAG_TEXT				"ui:interface:inventory:content:bag:iil:bag_list"
#define LIST_BAG_ICONS				"ui:interface:inventory:content:bag:iil:bag_icons"

#define LIST_ROOM_TEXT				"ui:interface:inv_room:content:iil:bag_list"
#define LIST_ROOM_ICONS				"ui:interface:inv_room:content:iil:bag_icons"

#define LIST_GUILD_TEXT				"ui:interface:inv_guild:content:iil:bag_list"
#define LIST_GUILD_ICONS			"ui:interface:inv_guild:content:iil:bag_icons"

#define LIST_PA0_TEXT				"ui:interface:inv_pa0:content:iil:bag_list"
#define LIST_PA0_ICONS				"ui:interface:inv_pa0:content:iil:bag_icons"

#define LIST_PA1_TEXT				"ui:interface:inv_pa1:content:iil:bag_list"
#define LIST_PA1_ICONS				"ui:interface:inv_pa1:content:iil:bag_icons"

#define LIST_PA2_TEXT				"ui:interface:inv_pa2:content:iil:bag_list"
#define LIST_PA2_ICONS				"ui:interface:inv_pa2:content:iil:bag_icons"

#define LIST_PA3_TEXT				"ui:interface:inv_pa3:content:iil:bag_list"
#define LIST_PA3_ICONS				"ui:interface:inv_pa3:content:iil:bag_icons"

// Theorically never used
#define LIST_BAG2_TEXT				"ui:interface:inv_bag:content:iil:bag_list"
#define LIST_BAG2_ICONS				"ui:interface:inv_bag:content:iil:bag_icons"

#define MENU_BAG_ITEM				"ui:interface:item_menu_in_bag"

#define CTRL_HAND_RIGHT				"ui:interface:gestionsets:hands:handr"
#define CTRL_HAND_LEFT				"ui:interface:gestionsets:hands:handl"
#define CTRL_HAND_LEFT_2HAND		"ui:interface:gestionsets:hands:handl_2h"
#define CTRL_HAND_LEFT_AMMO			"ui:interface:gestionsets:hands:handl_ammo"

#define CTRL_JEWEL_EARING_LEFT		"ui:interface:inv_equip:content:equip:jewelry:earing_l"
#define CTRL_JEWEL_BRACELET_LEFT	"ui:interface:inv_equip:content:equip:jewelry:bracelet_l"
#define CTRL_JEWEL_RING_LEFT		"ui:interface:inv_equip:content:equip:jewelry:ring_l"
#define CTRL_JEWEL_ANKLET_LEFT		"ui:interface:inv_equip:content:equip:jewelry:anklet_l"
#define CTRL_JEWEL_EARING_RIGHT		"ui:interface:inv_equip:content:equip:jewelry:earing_r"
#define CTRL_JEWEL_BRACELET_RIGHT	"ui:interface:inv_equip:content:equip:jewelry:bracelet_r"
#define CTRL_JEWEL_RING_RIGHT		"ui:interface:inv_equip:content:equip:jewelry:ring_r"
#define CTRL_JEWEL_ANKLET_RIGHT		"ui:interface:inv_equip:content:equip:jewelry:anklet_r"
#define CTRL_JEWEL_HEADDRESS		"ui:interface:inv_equip:content:equip:jewelry:headdress"
#define CTRL_JEWEL_NECK				"ui:interface:inv_equip:content:equip:jewelry:neck"

#define CTRL_ARMOR_HEAD				"ui:interface:inv_equip:content:equip:armors:head"
#define CTRL_ARMOR_CHEST			"ui:interface:inv_equip:content:equip:armors:chest"
#define CTRL_ARMOR_LEGS				"ui:interface:inv_equip:content:equip:armors:legs"
#define CTRL_ARMOR_FEET				"ui:interface:inv_equip:content:equip:armors:feet"
#define CTRL_ARMOR_ARMS				"ui:interface:inv_equip:content:equip:armors:arms"
#define CTRL_ARMOR_HANDS			"ui:interface:inv_equip:content:equip:armors:hands"

#define CTRL_JEWL2_EARING_LEFT		"ui:interface:inventory:content:equip:jewelry:earing_l"
#define CTRL_JEWL2_BRACELET_LEFT	"ui:interface:inventory:content:equip:jewelry:bracelet_l"
#define CTRL_JEWL2_RING_LEFT		"ui:interface:inventory:content:equip:jewelry:ring_l"
#define CTRL_JEWL2_ANKLET_LEFT		"ui:interface:inventory:content:equip:jewelry:anklet_l"
#define CTRL_JEWL2_EARING_RIGHT		"ui:interface:inventory:content:equip:jewelry:earing_r"
#define CTRL_JEWL2_BRACELET_RIGHT	"ui:interface:inventory:content:equip:jewelry:bracelet_r"
#define CTRL_JEWL2_RING_RIGHT		"ui:interface:inventory:content:equip:jewelry:ring_r"
#define CTRL_JEWL2_ANKLET_RIGHT		"ui:interface:inventory:content:equip:jewelry:anklet_r"
#define CTRL_JEWL2_HEADDRESS		"ui:interface:inventory:content:equip:jewelry:headdress"
#define CTRL_JEWL2_NECK				"ui:interface:inventory:content:equip:jewelry:neck"

#define CTRL_ARMR2_HEAD				"ui:interface:inventory:content:equip:armors:head"
#define CTRL_ARMR2_CHEST			"ui:interface:inventory:content:equip:armors:chest"
#define CTRL_ARMR2_LEGS				"ui:interface:inventory:content:equip:armors:legs"
#define CTRL_ARMR2_FEET				"ui:interface:inventory:content:equip:armors:feet"
#define CTRL_ARMR2_ARMS				"ui:interface:inventory:content:equip:armors:arms"
#define CTRL_ARMR2_HANDS			"ui:interface:inventory:content:equip:armors:hands"

#endif // RY_INVENTORY_MANAGER_H

/* End of inventory_manager.h */
