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
#include "item_group_manager.h"
#include "interface_v3/inventory_manager.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/stream.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/file.h"
#include "libxml/tree.h"
#include "game_share/item_type.h"
#include "client_sheets/item_sheet.h"
#include "net_manager.h"
#include "connection.h" // Used to access PlayerSelectedFileName for xml filename
#include "nel/gui/db_manager.h"
#include "interface_v3/interface_manager.h"
#include "nel/gui/group_menu.h"
CItemGroupManager *CItemGroupManager::_Instance = NULL;

CItemGroup::CItemGroup()
{
}


bool CItemGroup::contains(CDBCtrlSheet *other)
{
	for(int i=0;i<Items.size();i++)
	{
		CItem item = Items[i];
		NLMISC::CSheetId sheet = NLMISC::CSheetId(other->getSheetId());
		if (sheet.toString()  == item.sheetName  && other->getQuality()   == item.quality &&
				other->getItemWeight() == item.weight     && other->getItemColor() == item.color &&
				(!item.usePrice || (other->getItemPrice()  >= item.minPrice && other->getItemPrice() <= item.maxPrice))
				)
		{
			return true;
		}
	}

	return false;
}

void CItemGroup::addItem(std::string sheetName, uint16 quality, uint32 weight, uint8 color)
{
	Items.push_back(CItem(sheetName, quality, weight, color));
}

void CItemGroup::addRemove(std::string slotName)
{
	SLOT_EQUIPMENT::TSlotEquipment slot = SLOT_EQUIPMENT::stringToSlotEquipment(NLMISC::toUpper(slotName));
	if(slot)
		removeBeforeEquip.push_back(slot);
}

void CItemGroup::addRemove(SLOT_EQUIPMENT::TSlotEquipment slot)
{
	removeBeforeEquip.push_back(slot);
}

void CItemGroup::writeTo(xmlNodePtr node)
{
	xmlNodePtr groupNode = xmlNewChild (node, NULL, (const xmlChar*)"group", NULL );
	xmlSetProp(groupNode, (const xmlChar*)"name", (const xmlChar*)name.c_str());
	for(int i=0;i<Items.size();i++)
	{
		CItem item = Items[i];
		xmlNodePtr itemNode = xmlNewChild(groupNode, NULL, (const xmlChar*)"item", NULL);
		xmlSetProp (itemNode, (const xmlChar*)"sheetName", (const xmlChar*)item.sheetName.c_str());
		xmlSetProp (itemNode, (const xmlChar*)"quality", (const xmlChar*)NLMISC::toString(item.quality).c_str());
		xmlSetProp (itemNode, (const xmlChar*)"weight", (const xmlChar*)NLMISC::toString(item.weight).c_str());
		xmlSetProp (itemNode, (const xmlChar*)"color", (const xmlChar*)NLMISC::toString(item.color).c_str());
		xmlSetProp (itemNode, (const xmlChar*)"minPrice", (const xmlChar*)NLMISC::toString(item.minPrice).c_str());
		xmlSetProp (itemNode, (const xmlChar*)"maxPrice", (const xmlChar*)NLMISC::toString(item.maxPrice).c_str());
	}
	for(int i=0;i<removeBeforeEquip.size();i++)
	{
		xmlNodePtr removeNode = xmlNewChild(groupNode, NULL, (const xmlChar*)"remove", NULL);
		xmlSetProp(removeNode, (const xmlChar*)"slot", (xmlChar*)SLOT_EQUIPMENT::toString(removeBeforeEquip[i]).c_str());
	}
}


void CItemGroup::readFrom(xmlNodePtr node)
{
	CXMLAutoPtr ptrName;
	ptrName = (char*) xmlGetProp( node, (xmlChar*)"name" );
	if (ptrName) NLMISC::fromString((const char*)ptrName, name);

	xmlNodePtr curNode = node->children;
	while(curNode)
	{
		if (strcmp((char*)curNode->name, "item") == 0)
		{

			CItem item;
			ptrName = (char*) xmlGetProp(curNode, (xmlChar*)"sheetName");
			if (ptrName) NLMISC::fromString((const char*)ptrName, item.sheetName);
			ptrName = (char*) xmlGetProp(curNode, (xmlChar*)"quality");
			if (ptrName) NLMISC::fromString((const char*)ptrName, item.quality);
			ptrName = (char*) xmlGetProp(curNode, (xmlChar*)"weight");
			if (ptrName) NLMISC::fromString((const char*)ptrName, item.weight);
			ptrName = (char*) xmlGetProp(curNode, (xmlChar*)"color");
			if (ptrName) NLMISC::fromString((const char*)ptrName, item.color);
			ptrName = (char*) xmlGetProp(curNode, (xmlChar*)"minPrice");
			if (ptrName) NLMISC::fromString((const char*)ptrName, item.minPrice);
			ptrName = (char*) xmlGetProp(curNode, (xmlChar*)"maxPrice");
			if (ptrName) NLMISC::fromString((const char*)ptrName, item.maxPrice);
			item.usePrice = (item.minPrice != 0 || item.maxPrice != std::numeric_limits<uint32>::max());
			//Old version of groups.xml could save unknown sheets, remove them for clarity
			if(item.sheetName != "unknown.unknown")
				Items.push_back(item);
		}
		if (strcmp((char*)curNode->name, "remove") == 0)
		{
			std::string slot;
			ptrName = (char*) xmlGetProp(curNode, (xmlChar*)"slot");
			if (ptrName) NLMISC::fromString((const char*)ptrName, slot);
			addRemove(slot);
		}

		curNode = curNode->next;
	}

}

void CFakeEquipTime::invalidActions()
{
	NLGUI::CDBManager *pDB = NLGUI::CDBManager::getInstance();
	NLMISC::CCDBNodeLeaf *node;
	// This are the db update server sends when an user equip an item, see egs/player_manager/gear_latency.cpp CGearLatency::setSlot
	node = pDB->getDbProp("SERVER:USER:ACT_TSTART", false);
	if (node) node->setValue64(NetMngr.getCurrentServerTick());

	node = pDB->getDbProp("SERVER:USER:ACT_TEND", false);
	if(node) node->setValue64(NetMngr.getCurrentServerTick() + time);

	node = pDB->getDbProp("SERVER:EXECUTE_PHRASE:SHEET", false);
	static NLMISC::CSheetId equipSheet("big_equip_item.sbrick");
	if(node) node->setValue64((sint64)equipSheet.asInt());


	node = pDB->getDbProp("SERVER:EXECUTE_PHRASE:PHRASE", false);
	if(node) node->setValue64(0);

}

void CFakeEquipTime::validActions()
{
	NLGUI::CDBManager *pDB = NLGUI::CDBManager::getInstance();
	NLMISC::CCDBNodeLeaf *node;
	node = pDB->getDbProp("SERVER:USER:ACT_TSTART", false);
	if (node) node->setValue64(0);

	node = pDB->getDbProp("SERVER:USER:ACT_TEND", false);
	if(node) node->setValue64(0);

	node = pDB->getDbProp("SERVER:EXECUTE_PHRASE:SHEET", false);
	if(node) node->setValue32(0);

	node = pDB->getDbProp("SERVER:EXECUTE_PHRASE:PHRASE", false);
	if(node) node->setValue32(0);
}
void CFakeEquipTime::run()
{
	//We wait a bit before invalidating actions, or server will override us
	//Might not be accurate for everyone, but if it's wrong at worst you'll still get the timer
	// Just with a blank icon instead of a "equipping item" red cross
	NLMISC::nlSleep(600);
	invalidActions();
	NLMISC::nlSleep((time-6) * 100); // time is in ticks, sleep takes ms
	validActions();
}

CItemGroupManager::CItemGroupManager()
{
}

void CItemGroupManager::init()
{
	loadGroups();
	linkInterface();
}

void CItemGroupManager::linkInterface()
{
	//attach item group subgroup to right-click in bag group
	CWidgetManager* pWM = CWidgetManager::getInstance();
	CGroupMenu   *pRootMenu = dynamic_cast<CGroupMenu*>(pWM->getElementFromId("ui:interface:item_menu_in_bag"));
	CGroupSubMenu *pMenu = pRootMenu->getRootMenu();
	//get item subgroup
	CGroupMenu   *pGroupMenu = dynamic_cast<CGroupMenu*>(pWM->getElementFromId("ui:interface:item_menu_in_bag:item_group_menu"));
	CGroupSubMenu *pGroupSubMenu = NULL;
	if (pGroupMenu) pGroupSubMenu = pGroupMenu->getRootMenu();
	if (pMenu && pGroupSubMenu)
		pMenu->setSubMenu(pMenu->getNumLine() - 1, pGroupSubMenu);
	else
		nlwarning("Couldn't link group submenu to item_menu_in_bag, check your widgets.xml file");
}

void CItemGroupManager::uninit()
{
	saveGroups();
	unlinkInterface();
	_Groups.clear();
}

void CItemGroupManager::unlinkInterface()
{
	// We need to unlink our menu to avoid crash on interface release
	CWidgetManager* pWM = CWidgetManager::getInstance();
	CGroupMenu   *pGroupMenu = dynamic_cast<CGroupMenu*>(pWM->getElementFromId("ui:interface:item_menu_in_bag:item_group_menu"));
	CGroupSubMenu *pGroupSubMenu = NULL;
	if (pGroupMenu) pGroupSubMenu = pGroupMenu->getRootMenu();
	if (pGroupMenu) pGroupMenu->reset();
	if (pGroupMenu && pGroupSubMenu) pGroupMenu->delGroup(pGroupSubMenu, true);
}

// Inspired from macro parsing
void CItemGroupManager::saveGroups()
{
	std::string userGroupFileName = "save/groups_" + PlayerSelectedFileName + ".xml";
	if(PlayerSelectedFileName.empty())
	{
		nlwarning("Trying to save group with an empty PlayerSelectedFileName, aborting");
		return;
	}
	try {
		NLMISC::COFile f;
		if(f.open(userGroupFileName, false, false, true))
		{

			NLMISC::COXml xmlStream;
			xmlStream.init(&f);
			xmlDocPtr doc = xmlStream.getDocument ();
			xmlNodePtr node = xmlNewDocNode(doc, NULL, (const xmlChar*)"item_groups", NULL);
			xmlDocSetRootElement (doc, node);
			for(int i=0;i<_Groups.size();i++)
			{
				CItemGroup group = _Groups[i];
				group.writeTo(node);
			}
			xmlStream.flush();
			f.close();
		}
		else
		{
			nlwarning ("Can't open the file %s", userGroupFileName.c_str());

		}
	}
	catch (const NLMISC::Exception &e)
	{
		nlwarning ("Error while writing the file %s : %s.", userGroupFileName.c_str(), e.what ());
	}
}

bool CItemGroupManager::loadGroups()
{

	std::string userGroupFileName = "save/groups_" + PlayerSelectedFileName + ".xml";
	if(PlayerSelectedFileName.empty())
	{
		nlwarning("Trying to load group with an empty PlayerSelectedFileName, aborting");
		return false;
	}
	if (!NLMISC::CFile::fileExists(userGroupFileName) || NLMISC::CFile::getFileSize(userGroupFileName) == 0)
	{
		nlinfo("No item groups file found !");
		return false;
	}
	//Init loading
	NLMISC::CIFile f;
	f.open(userGroupFileName);
	NLMISC::CIXml xmlStream;
	xmlStream.init(f);
	// Actual loading
	xmlNodePtr globalEnclosing;
	globalEnclosing = xmlStream.getRootNode();
	if(!globalEnclosing)
	{
		nlwarning("no root element in item_group xml, skipping xml parsing");
		return false;
	}
	if(strcmp(( (char*)globalEnclosing->name), "item_groups"))
	{
		nlwarning("wrong root element in item_group xml, skipping xml parsing");
		return false;
	}
	xmlNodePtr curNode = globalEnclosing->children;
	while (curNode)
	{
		if (strcmp((char*)curNode->name, "group") == 0)
		{
			CItemGroup group;
			group.readFrom(curNode);
			_Groups.push_back(group);
		}
		curNode = curNode->next;
	}
	f.close();

	return true;
}

//move a group from all available inventory to dst
bool CItemGroupManager::moveGroup(std::string name, INVENTORIES::TInventory dst)
{
	CItemGroup* group = findGroup(name);
	if(!group)
	{
		nlinfo("group %s not found", name.c_str());
		return false;
	}
	if(dst == INVENTORIES::UNDEFINED)
	{
		nlinfo("Destination inventory not found");
		return false;
	}
	CInventoryManager* pIM = CInventoryManager::getInstance();

	std::string moveParams = "to=lists|nblist=1|listsheet0=" + toDbPath(dst);
	// Grab all matching item from all available inventory and put it in dst
	for (int i=0; i< INVENTORIES::TInventory::NUM_ALL_INVENTORY; i ++)
	{
		INVENTORIES::TInventory inventory = (INVENTORIES::TInventory)i;
		if (inventory != dst && pIM->isInventoryAvailable(inventory))
		{
			std::vector<CInventoryItem> items = matchingItems(group, inventory);
			for(int i=0;i<items.size();i++)
			{
				CInventoryItem item = items[i];
				//If an item is currently equipped, don't move it (or else crash !!)
				if(pIM->isBagItemWeared(item.indexInBag)) continue;
				CAHManager::getInstance()->runActionHandler("move_item", item.pCS, moveParams);
			}

		}
	}
	return true;

}

bool CItemGroupManager::equipGroup(std::string name, bool pullBefore)
{
	CItemGroup* group = findGroup(name);
	if(!group)
	{
		nlinfo("group %s not found", name.c_str());
		return false;
	}

	if(pullBefore) moveGroup(name, INVENTORIES::TInventory::bag);
	//Start by unequipping all slot that user wants to unequip
	for(int i=0; i < group->removeBeforeEquip.size(); i++)
	{
		SLOT_EQUIPMENT::TSlotEquipment slot = group->removeBeforeEquip[i];
		std::string dbPath;
		// For hands equip, dbPath obviously starts at 0, we need to offset correctly
		if(slot == SLOT_EQUIPMENT::HANDL || slot == SLOT_EQUIPMENT::HANDR)
			dbPath = "LOCAL:INVENTORY:HAND:" + NLMISC::toString((uint32)slot - SLOT_EQUIPMENT::HANDL);
		else
			dbPath = "LOCAL:INVENTORY:EQUIP:" + NLMISC::toString((uint32)slot);
		CInventoryManager::getInstance()->unequip(dbPath);
	}

	uint32 maxEquipTime = 0;

	std::map<ITEM_TYPE::TItemType, bool> possiblyDual =
	{
		{ITEM_TYPE::ANKLET, false},
		{ITEM_TYPE::BRACELET, false},
		{ITEM_TYPE::EARING, false},
		{ITEM_TYPE::RING, false},
		{ITEM_TYPE::DAGGER, false},
	};
	std::vector<CInventoryItem> duals;
	std::vector<CInventoryItem> items = matchingItems(group, INVENTORIES::TInventory::bag);
	for(int i=0; i < items.size(); i++)
	{
		CInventoryItem item = items[i];
		ITEM_TYPE::TItemType itemType = item.pCS->asItemSheet()->ItemType;
		// If the item can be weared 2 times, don't automatically equip the second one
		// Or else it will simply replace the first. We'll deal with them later
		if(possiblyDual.find(itemType) != possiblyDual.end())
		{
			if (possiblyDual[itemType])
			{
				duals.push_back(item);
				continue;
			}
			possiblyDual[itemType] = true;
		}
		maxEquipTime = std::max(maxEquipTime, item.pCS->asItemSheet()->EquipTime);
		CInventoryManager::getInstance()->autoEquip(item.indexInBag, true);
	}
	// Manually equip dual items
	for(int i=0;i < duals.size();i++)
	{
		CInventoryItem item = duals[i];
		ITEM_TYPE::TItemType itemType = item.pCS->asItemSheet()->ItemType;
		std::string dstPath = string(LOCAL_INVENTORY);
		switch(itemType)
		{
		case ITEM_TYPE::ANKLET:
			dstPath += ":EQUIP:" + NLMISC::toString((int)SLOT_EQUIPMENT::ANKLER); break;
		case ITEM_TYPE::BRACELET:
			dstPath += ":EQUIP:" + NLMISC::toString((int)SLOT_EQUIPMENT::WRISTR);; break;
		case ITEM_TYPE::EARING:
			dstPath += ":EQUIP:" + NLMISC::toString((int)SLOT_EQUIPMENT::EARR);; break;
		case ITEM_TYPE::RING:
			dstPath += ":EQUIP:" + NLMISC::toString((int)SLOT_EQUIPMENT::FINGERR);;break;
		case ITEM_TYPE::DAGGER:
			dstPath += ":HAND:1"; break;
		default:
			break;
		}

		std::string srcPath = item.pCS->getSheet();
		maxEquipTime = std::max(maxEquipTime, item.pCS->asItemSheet()->EquipTime);
		CInventoryManager::getInstance()->equip(srcPath, dstPath);
	}
	// For some reason, there is no (visual) invalidation (server still blocks any action), force one
	// Unfortunately, there is no clean way to do this, so we'll simulate one
	NLMISC::IRunnable *runnable = (NLMISC::IRunnable *)(new CFakeEquipTime((NLMISC::TGameCycle)maxEquipTime));
	NLMISC::IThread *thread = NLMISC::IThread::create(runnable);
	thread->start();
	return true;

}

bool CItemGroupManager::createGroup(std::string name, bool removeUnequiped)
{
	if(findGroup(name)) return false;
	CItemGroup group = CItemGroup();
	group.name = name;
	uint i;
	CDBCtrlSheet* pCS;
	for (i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		SLOT_EQUIPMENT::TSlotEquipment slot = (SLOT_EQUIPMENT::TSlotEquipment)i;
		//Instead of doing two separate for, just be a bit tricky for hand equipment
		if(slot == SLOT_EQUIPMENT::HANDR || slot == SLOT_EQUIPMENT::HANDL)
			pCS = CInventoryManager::getInstance()->getHandSheet((uint32)(slot -  SLOT_EQUIPMENT::HANDL));
		else
			pCS = CInventoryManager::getInstance()->getEquipSheet(i);
		if(!pCS) continue;
		if(pCS->isSheetValid())
		{
		NLMISC::CSheetId sheet(pCS->getSheetId());
		group.addItem(sheet.toString(), pCS->getQuality(), pCS->getItemWeight(), pCS->getItemColor());
		}
		else if(removeUnequiped)
		{
			if(slot != SLOT_EQUIPMENT::UNDEFINED && slot != SLOT_EQUIPMENT::FACE)
				group.addRemove(slot);
		}
	}

	_Groups.push_back(group);
	return true;


}
bool CItemGroupManager::deleteGroup(std::string name)
{
	std::vector<CItemGroup> tmp;
	for(int i=0;i<_Groups.size();i++)
	{
		CItemGroup group = _Groups[i];
		if(group.name == name) continue;
		tmp.push_back(group);
	}
	// Nothing removed, error
	if(tmp.size() == _Groups.size()) return false;
	_Groups = tmp;
	return true;
}

void CItemGroupManager::listGroup()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	pIM->displaySystemInfo(ucstring("Available item groups :"));
	for(int i=0;i<_Groups.size();i++)
	{
		CItemGroup group = _Groups[i];
		ucstring msg = "* " + ucstring(group.name) + ucstring(NLMISC::toString(" with %d items inside.", group.Items.size()));
		pIM->displaySystemInfo(msg);
	}
}

//Used by AH

std::vector<std::string> CItemGroupManager::getGroupNames(CDBCtrlSheet* pCS)
{
	std::vector<std::string> out;
	for(int i=0;i<_Groups.size();i++)
	{
		CItemGroup group = _Groups[i];
		if(group.contains(pCS))
			out.push_back(group.name);

	}
	return out;
}

//Private methods
CItemGroup* CItemGroupManager::findGroup(std::string name)
{
	for(int i=0;i<_Groups.size();i++)
	{
		if (_Groups[i].name == name) return &_Groups[i];
	}
	return NULL;
}
std::string CItemGroupManager::toDbPath(INVENTORIES::TInventory inventory)
{
	switch(inventory)
	{
	case INVENTORIES::TInventory::bag:
		return LIST_BAG_TEXT; break;
	case INVENTORIES::TInventory::pet_animal1:
		return LIST_PA0_TEXT; break;
	case INVENTORIES::TInventory::pet_animal2:
		return LIST_PA1_TEXT; break;
	case INVENTORIES::TInventory::pet_animal3:
		return LIST_PA2_TEXT; break;
	case INVENTORIES::TInventory::pet_animal4:
		return LIST_PA3_TEXT; break;
	case INVENTORIES::TInventory::player_room:
		return LIST_ROOM_TEXT;break;
	case INVENTORIES::TInventory::guild:
		return ClientCfg.ItemGroupAllowGuild ? LIST_GUILD_TEXT : ""; break;
	default:
		return "";
	}
}

std::vector<CInventoryItem> CItemGroupManager::matchingItems(CItemGroup *group, INVENTORIES::TInventory inventory)
{
	//Not very clean, but no choice, it's ugly time
	std::vector<CInventoryItem> out;
	std::string dbPath = toDbPath(inventory);
	if(dbPath.empty())
	{
		nldebug("Inventory type %s not supported", INVENTORIES::toString(inventory).c_str());
		return out;
	}

	IListSheetBase *pList = dynamic_cast<IListSheetBase*>(CWidgetManager::getInstance()->getElementFromId(dbPath));
	for(uint i=0; i < MAX_BAGINV_ENTRIES; i++)
	{
		CDBCtrlSheet *pCS = pList->getSheet(i);
		if(group->contains(pCS))
		{

			out.push_back(CInventoryItem(pCS, inventory, i));
		}
	}

	return out;

}

// Singleton management
CItemGroupManager *CItemGroupManager::getInstance()
{
	if (!_Instance)
		_Instance = new CItemGroupManager();
	return _Instance;
}
void CItemGroupManager::releaseInstance()
{
	if (_Instance)
		delete _Instance;
	_Instance = NULL;
}
