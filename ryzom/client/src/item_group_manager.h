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

#ifndef RY_ITEM_GROUP_MANAGER_H
#define RY_ITEM_GROUP_MANAGER_H
#include <limits>
#include "interface_v3/inventory_manager.h"
#include "interface_v3/dbctrl_sheet.h"
#include "game_share/inventories.h"
#include "game_share/slot_equipment.h"

#define LIST_ITEMGROUPS					"ui:interface:inventory:content:equip:midsection:content:itemgroups:list_container:itemgroups_list"
#define LIST_ITEMGROUPS_2				"ui:interface:inv_equip:content:equip:midsection:content:itemgroups:list_container:itemgroups_list"
#define LIST_EMPTY_TEXT					"empty_text"	
#define TEMPLATE_ITEMGROUP_ITEM			"itemgroups_item"
#define TEMPLATE_ITEMGROUP_ITEM_NAME	"name"

struct CInventoryItem {
public:
	CDBCtrlSheet *pCS;
	INVENTORIES::TInventory origin;
	uint32 indexInBag;
	CInventoryItem(CDBCtrlSheet *pCS, INVENTORIES::TInventory origin, uint32 indexInBag):
	    pCS(pCS),
	    origin(origin),
	    indexInBag(indexInBag)	
		{}
};

class CItemGroup
{
public:
	struct CItem {
		sint32 createTime;
		sint32 serial;
		CInventoryItem pItem;
		CItem(sint32 createTime, sint32 serial):
		    createTime(createTime),
		    serial(serial),
			pItem(NULL, INVENTORIES::UNDEFINED, 0)
		{}
	};
	
	struct CEquipItem : CItem {
		SLOT_EQUIPMENT::TSlotEquipment slot; // Really only necessary for right/left hand slot, but used for clarity in the xml file
		CEquipItem(sint32 createTime, sint32 serial, SLOT_EQUIPMENT::TSlotEquipment slot = SLOT_EQUIPMENT::UNDEFINED) :
			CItem(createTime, serial), slot(slot) {}
	};

	struct CHotbarItem : CItem {
		uint16 slot; // designates the index of the slot of the hotbar
		CHotbarItem(sint32 createTime, sint32 serial, uint16 slot) :
			CItem(createTime, serial), slot(slot) {}
	};

public:
	CItemGroup();

	// return true if any item in the group match the parameter ; slot is UNDEFINED unless the item has been found in the group
	bool contains(CDBCtrlSheet *other);
	bool contains(CDBCtrlSheet *other, SLOT_EQUIPMENT::TSlotEquipment &slot);
	bool contains(CDBCtrlSheet *other, uint16 &slot);
	void addItem(sint32 createTime, sint32 serial, SLOT_EQUIPMENT::TSlotEquipment slot);
	void addItem(sint32 createTime, sint32 serial, uint16 slot);
	void addRemove(std::string slotName);
	void addRemove(SLOT_EQUIPMENT::TSlotEquipment slot);
	void addRemove(uint16 slot);
	void updateItemsLocation(); // update the CInventoryItem in the each CItem
	void writeTo(xmlNodePtr node);
	void readFrom(xmlNodePtr node);

	// return true if no item inside
	bool empty() const { return equipItems.size() == 0 && hotbarItems.size() == 0;}
	std::string name;
	std::vector<CEquipItem> equipItems;
	std::vector<CHotbarItem> hotbarItems;
	std::vector<SLOT_EQUIPMENT::TSlotEquipment> equipRemoves;
	std::vector<uint16> hotbarRemoves;
};

class CItemGroupManager
{
public:
	// Singleton management
	static CItemGroupManager *getInstance();
	static void releaseInstance();
	// Ctor
	CItemGroupManager();
	// Regular function
	void init();
	void uninit();
	void saveGroups();
	bool loadGroups();
	void linkInterface();
	void unlinkInterface();
	void undrawGroupsList();
	void drawGroupsList();
	CInterfaceGroup* generateGroupsListLine(std::string parent, uint i);
	//Return NULL if no group was found
	//Return false if no group was found
	bool moveGroup(std::string name, INVENTORIES::TInventory dst);
	bool equipGroup(std::string name, bool pullBefore = true);
	bool createGroup(std::string name, bool removeUnequiped = false);
	bool deleteGroup(std::string name);
	void listGroup();
	static std::string toDbPath(INVENTORIES::TInventory inventory);
	std::vector<std::string> getGroupNames(CDBCtrlSheet *pCS);
	// Used to fake invalid actions
	void update();

private:
	CItemGroup *findGroup(std::string name);
	std::vector<CItemGroup> _Groups;
	// Singleton's instance
	static CItemGroupManager *_Instance;
	//
	void fakeInvalidActions(NLMISC::TGameCycle time);
	void invalidActions(NLMISC::TGameCycle begin, NLMISC::TGameCycle end);
	void validActions();
	NLMISC::TGameCycle _EndInvalidAction;
	NLMISC::TGameCycle _StartInvalidAction;
	// Workaround: sometimes item are marked as equipped by pIM->isBagItemWeared() even tho they aren't really
	// Because of a synchronisation error between client and server
	bool isItemReallyEquipped(CDBCtrlSheet *item);
};

#endif // RY_ITEM_GROUP_MANAGER_H
