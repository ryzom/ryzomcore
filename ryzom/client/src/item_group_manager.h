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

#define ITEMGROUPS_CURRENT_VERSION      "2" // versioning for migrations
#define MENU_IN_BAG                     "ui:interface:item_menu_in_bag"
#define ITEMGROUP_MENU                  "ui:interface:item_menu_in_bag:item_group_menu"
#define LIST_ITEMGROUPS					"ui:interface:inventory:content:equip:midsection:content:itemgroups:list_container:itemgroups_list"
#define LIST_ITEMGROUPS_2				"ui:interface:inv_equip:content:equip:midsection:content:itemgroups:list_container:itemgroups_list"
#define LIST_EMPTY_TEXT					"empty_text"	
#define TEMPLATE_ITEMGROUP_ITEM			"itemgroups_item"
#define TEMPLATE_ITEMGROUP_ITEM_NAME	"name"

class CItemGroup
{
public:
	// designates a given slot in the local inventory
	struct CSlot {
		INVENTORIES::TInventory branch; // the local inventory branch (e.g. INVENTORIES::handling or INVENTORIES::equipment)
		uint16 index; // the index in the branch
		CSlot(INVENTORIES::TInventory branch, uint16 index):
			branch(branch),
			index(index)
			{}
		CSlot():
			branch(INVENTORIES::UNDEFINED),
			index(0)
			{}

		bool operator==(const CSlot &other) const {return (branch == other.branch) && (index == other.index);}
		void writeTo(xmlNodePtr node);
		bool isValid();
		const std::string toDbPath();
		const std::string toString();
		CDBCtrlSheet* getSheet();
		static CSlot readFromV1(xmlNodePtr node);
		static CSlot readFromV2(xmlNodePtr node);	
		static CSlot handSlot(uint16 index) { return CSlot(INVENTORIES::handling, index); }
		static CSlot equipSlot(uint16 index) { return CSlot(INVENTORIES::equipment, index); }
		static CSlot hotbarSlot(uint16 index) { return CSlot(INVENTORIES::hotbar, index); }
		static CSlot fromSlotEquipment(SLOT_EQUIPMENT::TSlotEquipment slotEquipment);
	};

	// an item group item
	struct CItem {
		sint32 createTime; // create time of the item in the inventory (used to find the item in the inventory)
		sint32 serial; // serial of the item in the inventory (used to find the item in the inventory)
		CItemGroup::CSlot dstSlot; // the slot where we want to put the item
		CDBCtrlSheet *pCS; // references an item in the current inventory
		CItem(sint32 createTime, sint32 serial, CSlot dstSlot):
		    createTime(createTime),
		    serial(serial),
			dstSlot(dstSlot),
			pCS(NULL)
			{}
		
		bool operator<(const CItem &item) const { return (dstSlot.branch != item.dstSlot.branch) ? (dstSlot.branch < item.dstSlot.branch) : (dstSlot.index < item.dstSlot.index); } 
		void equip(uint32 &equipTime);
		bool isInDestinationSlot();
		void writeTo(xmlNodePtr node);
		static CItem readFrom(xmlNodePtr node);
	};

public:
	CItemGroup();

	bool contains(CDBCtrlSheet *pCS);
	void addSheet(CDBCtrlSheet *pCS, CSlot slot, bool removeEmpty);
	void addItem(CItem item);
	void addRemoveSlot(CSlot slot);
	void updateSheets();
	void writeTo(xmlNodePtr node);
	void deserialize(xmlNodePtr node, std::string version);
	bool empty() const { return items.size() == 0;} 

	std::string name;
	std::vector<CItemGroup::CItem> items;
	std::vector<CItemGroup::CSlot> removeSlots;
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
	std::string getFilePath(std::string playerName);
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
	bool isItemEquipped(CDBCtrlSheet *item, CItemGroup::CSlot &equipSlot);
	std::string generateDocumentation();
};

#endif // RY_ITEM_GROUP_MANAGER_H
