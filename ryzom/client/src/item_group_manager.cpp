// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2017  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/misc/i18n.h"
#include "nel/misc/algo.h"
CItemGroupManager *CItemGroupManager::_Instance = NULL;

CItemGroup::CItemGroup()
{
}

bool CItemGroup::contains(CDBCtrlSheet *other)
{
	for (int i = 0; i < getItems().size(); i++)
	{
		CItem item = *getItems()[i];
		if (item.createTime == other->getItemCreateTime() && item.serial == other->getItemSerial())
		{
			return true;
		}
	}
	return false;
}

// equip
bool CItemGroup::contains(CDBCtrlSheet *other, SLOT_EQUIPMENT::TSlotEquipment &slot)
{
	slot = SLOT_EQUIPMENT::UNDEFINED;
	for (int i = 0; i < equipItems.size(); i++)
	{
		CEquipItem item = equipItems[i];
		if (item.createTime == other->getItemCreateTime() && item.serial == other->getItemSerial())
		{
			slot = item.slot;
			return true;
		}
	}

	return false;
}

// hotbar
bool CItemGroup::contains(CDBCtrlSheet *other, uint16 &slot)
{
	slot = NULL;
	for (int i = 0; i < hotbarItems.size(); i++)
	{
		CHotbarItem item = hotbarItems[i];
		if (item.createTime == other->getItemCreateTime() && item.serial == other->getItemSerial())
		{
			slot = item.slot;
			return true;
		}
	}

	return false;
}

// get all items of equip and hotbar as a vector of the base CItem class (allows us to use the same code for all items)
std::vector<CItemGroup::CItem*> CItemGroup::getItems()
{
	std::vector<CItem*> items;
	for (int i = 0; i < equipItems.size(); i++)
	{
		items.push_back(&equipItems[i]);
	}
	for (int i = 0; i < hotbarItems.size(); i++)
	{
		items.push_back(&hotbarItems[i]);
	}
	return items;
}

// equip
void CItemGroup::addItem(sint32 createTime, sint32 serial, SLOT_EQUIPMENT::TSlotEquipment slot)
{
	// Don't add an item if it already exists, this could cause issue
	//  It's happening either if we are creating a group with a 2 hands items (and the item is found both in handR and handL)
	//  Or if an user incorrectly edit his group file
	for (int i = 0; i < equipItems.size(); i++)
	{
		if (equipItems[i].createTime == createTime && equipItems[i].serial == serial)
		{
			nldebug("Not adding duplicate item, createTime: %d, serial: %d", createTime, serial);
			// In this case, we are adding the duplicate item for a 2 hands item
			// If it's saved as a left hand item, save it as a right hand item instead (so we have only 1 correct item)
			if (equipItems[i].slot == SLOT_EQUIPMENT::HANDL && slot == SLOT_EQUIPMENT::HANDR)
				equipItems[i].slot = SLOT_EQUIPMENT::HANDR;
			return;
		}
	}
	equipItems.push_back(CEquipItem(createTime, serial, slot));
}

// hotbar
void CItemGroup::addItem(sint32 createTime, sint32 serial, uint16 slot)
{
	for (int i = 0; i < hotbarItems.size(); i++)
	{
		if (hotbarItems[i].createTime == createTime && hotbarItems[i].serial == serial)
		{
			nldebug("Not adding duplicate item, createTime: %d, serial: %d", createTime, serial);
			return;
		}
	}
	hotbarItems.push_back(CHotbarItem(createTime, serial, slot));
}

// equip
void CItemGroup::addRemove(std::string slotName)
{
	SLOT_EQUIPMENT::TSlotEquipment slot = SLOT_EQUIPMENT::stringToSlotEquipment(NLMISC::toUpperAscii(slotName));
	if (slot != SLOT_EQUIPMENT::UNDEFINED)
		addRemove(slot);
}

// equip
void CItemGroup::addRemove(SLOT_EQUIPMENT::TSlotEquipment slot)
{
	equipRemoves.push_back(slot);
}

// hotbar
void CItemGroup::addRemove(uint16 slot)
{
	hotbarRemoves.push_back(slot);
}

// We must link the CItem (which just represents identifiers of an item) of the CItemGroup
// with an item in the inventory.
// Basically, here we are locating the position of the item in the inventory
// and we are setting it to the pItem property of CItem,
// so we can then equip or move CItem via its CInventoryItem.
// ! This function must be called before every interaction with items of a CItemGroup, to update the location in the inventory of the items
void CItemGroup::updateItemsLocation()
{
	CInventoryManager *pIM = CInventoryManager::getInstance();
	for (uint i = 0; i < getItems().size(); i++)
	{
		CItem *item = getItems()[i];
		bool found = false;
		for (int i = 0; i < INVENTORIES::NUM_ALL_INVENTORY; i++)
		{
			INVENTORIES::TInventory inventory = (INVENTORIES::TInventory)i;
			if (pIM->isInventoryAvailable(inventory))
			{
				std::string dbPath = CInventoryManager::invToDbPath(inventory);
				if (dbPath.empty() || (inventory == INVENTORIES::guild && !ClientCfg.ItemGroupAllowGuild))
				{
					nldebug("Inventory type %s not supported", INVENTORIES::toString(inventory).c_str());
					continue;
				}
				IListSheetBase *pList = dynamic_cast<IListSheetBase *>(CWidgetManager::getInstance()->getElementFromId(dbPath));
				for (int i = 0; i < pList->getNbSheet(); i++)
				{
					CDBCtrlSheet *pCS = pList->getSheet(i);
					if (item->createTime == pCS->getItemCreateTime() && item->serial == pCS->getItemSerial())
					{
						// Sometimes, index in the list differ from the index in DB, and we need the index in DB, not the one from the list
						std::string dbPath = pCS->getSheet();
						std::string indexS = dbPath.substr(dbPath.find_last_of(":") + 1);
						uint32 index;
						NLMISC::fromString(indexS, index);
						if (i != index)
						{
							nldebug("Index from list is %d, where index from DB is %d", i, index);
						}

						found = true;
						item->pItem = CInventoryItem(pCS, inventory, index);
						break;
					}
				}
				if (found)
					break;
			}
		}
		if (!found)
		{
			nlinfo("Item not found in inventory: createTime: %d, serial: %d", item->createTime, item->serial);
			item->pItem = CInventoryItem();
		}
	}
}

void CItemGroup::writeTo(xmlNodePtr node)
{
	xmlNodePtr groupNode = xmlNewChild(node, NULL, (const xmlChar *)"group", NULL);
	xmlSetProp(groupNode, (const xmlChar *)"name", (const xmlChar *)name.c_str());
	for (int i = 0; i < equipItems.size(); i++)
	{
		CEquipItem item = equipItems[i];
		xmlNodePtr itemNode = xmlNewChild(groupNode, NULL, (const xmlChar *)"item", NULL);
		xmlSetProp(itemNode, (const xmlChar *)"createTime", (const xmlChar *)NLMISC::toString(item.createTime).c_str());
		xmlSetProp(itemNode, (const xmlChar *)"serial", (const xmlChar *)NLMISC::toString(item.serial).c_str());
		xmlSetProp(itemNode, (const xmlChar *)"slot", (const xmlChar *)SLOT_EQUIPMENT::toString(item.slot).c_str());
	}
	for (int i = 0; i < hotbarItems.size(); i++)
	{
		CHotbarItem item = hotbarItems[i];
		xmlNodePtr itemNode = xmlNewChild(groupNode, NULL, (const xmlChar *)"item", NULL);
		xmlSetProp(itemNode, (const xmlChar *)"createTime", (const xmlChar *)NLMISC::toString(item.createTime).c_str());
		xmlSetProp(itemNode, (const xmlChar *)"serial", (const xmlChar *)NLMISC::toString(item.serial).c_str());
		xmlSetProp(itemNode, (const xmlChar *)"pocket", (const xmlChar *)NLMISC::toString(true).c_str());
		xmlSetProp(itemNode, (const xmlChar *)"slot", (const xmlChar *)NLMISC::toString(item.slot).c_str());
	}
	for (int i = 0; i < equipRemoves.size(); i++)
	{
		xmlNodePtr removeNode = xmlNewChild(groupNode, NULL, (const xmlChar *)"remove", NULL);
		xmlSetProp(removeNode, (const xmlChar *)"slot", (const xmlChar *)SLOT_EQUIPMENT::toString(equipRemoves[i]).c_str());
	}
	for (int i = 0; i < hotbarRemoves.size(); i++)
	{
		xmlNodePtr removeNode = xmlNewChild(groupNode, NULL, (const xmlChar *)"remove", NULL);
		xmlSetProp(removeNode, (const xmlChar *)"pocket", (const xmlChar *)NLMISC::toString(true).c_str());
		xmlSetProp(removeNode, (const xmlChar *)"slot", (const xmlChar *)NLMISC::toString(hotbarRemoves[i]).c_str());
	}
}

void CItemGroup::readFrom(xmlNodePtr node)
{
	CXMLAutoPtr ptrName;
	ptrName = (char *)xmlGetProp(node, (xmlChar *)"name");
	if (ptrName)
		NLMISC::fromString((const char *)ptrName, name);

	xmlNodePtr curNode = node->children;
	while (curNode)
	{
		if (strcmp((char *)curNode->name, "item") == 0)
		{
			sint32 createTime = 0;
			ptrName = (char *)xmlGetProp(curNode, (xmlChar *)"createTime");
			if (ptrName)
				NLMISC::fromString((const char *)ptrName, createTime);
			sint32 serial = 0;
			ptrName = (char *)xmlGetProp(curNode, (xmlChar *)"serial");
			if (ptrName)
				NLMISC::fromString((const char *)ptrName, serial);
			bool isHotbar = false;
			ptrName = xmlGetProp(curNode, (xmlChar *)"pocket");
			if (ptrName)
				NLMISC::fromString((const char *)ptrName, isHotbar);
			std::string slot;
			ptrName = (char *)xmlGetProp(curNode, (xmlChar *)"slot");
			if (ptrName)
				NLMISC::fromString((const char *)ptrName, slot);
			if (createTime != 0)
			{
				if (isHotbar)
				{
					uint16 tmp;
					NLMISC::fromString(slot, tmp);
					addItem(createTime, serial, tmp);
				}
				else
				{
					addItem(createTime, serial, SLOT_EQUIPMENT::stringToSlotEquipment(slot));
				}
			}
			// Old load : keep for compatibility / migration reasons
			else
			{
				nlwarning("Possibly on deprecated item group file, please recreate it or contact support.");
			}
		}
		if (strcmp((char *)curNode->name, "remove") == 0)
		{
			bool isHotbar = false;
			ptrName = (char *)xmlGetProp(curNode, (xmlChar *)"pocket");
			if (ptrName)
				NLMISC::fromString((const char *)ptrName, isHotbar);
			std::string slot;
			ptrName = (char *)xmlGetProp(curNode, (xmlChar *)"slot");
			if (ptrName)
				NLMISC::fromString((const char *)ptrName, slot);
			if (isHotbar)
			{
				uint16 tmp;
				NLMISC::fromString(slot, tmp);
				addRemove(tmp);
			}
			else
			{
				addRemove(SLOT_EQUIPMENT::stringToSlotEquipment(slot));
			}
		}

		curNode = curNode->next;
	}
}

CItemGroupManager::CItemGroupManager()
{
	_EndInvalidAction = 0;
	_StartInvalidAction = 0;
}

void CItemGroupManager::init()
{
	loadGroups();
	linkInterface();
}

void CItemGroupManager::linkInterface()
{
	// attach item group subgroup to right-click in bag group
	CWidgetManager *pWM = CWidgetManager::getInstance();
	CGroupMenu *pRootMenu = dynamic_cast<CGroupMenu *>(pWM->getElementFromId("ui:interface:item_menu_in_bag"));
	CGroupSubMenu *pMenu = pRootMenu->getRootMenu();
	// get item subgroup
	CGroupMenu *pGroupMenu = dynamic_cast<CGroupMenu *>(pWM->getElementFromId("ui:interface:item_menu_in_bag:item_group_menu"));
	CGroupSubMenu *pGroupSubMenu = NULL;
	if (pGroupMenu)
		pGroupSubMenu = pGroupMenu->getRootMenu();
	if (pMenu && pGroupSubMenu)
		pMenu->setSubMenu(pMenu->getNumLine() - 1, pGroupSubMenu);
	else
		nlwarning("Couldn't link group submenu to item_menu_in_bag, check your widgets.xml file");

	drawGroupsList();
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
	CWidgetManager *pWM = CWidgetManager::getInstance();
	CGroupMenu *pGroupMenu = dynamic_cast<CGroupMenu *>(pWM->getElementFromId("ui:interface:item_menu_in_bag:item_group_menu"));
	CGroupSubMenu *pGroupSubMenu = NULL;
	if (pGroupMenu)
		pGroupSubMenu = pGroupMenu->getRootMenu();
	if (pGroupMenu)
		pGroupMenu->reset();
	if (pGroupMenu && pGroupSubMenu)
		pGroupMenu->delGroup(pGroupSubMenu, true);
	undrawGroupsList();
}

// Inspired from macro parsing
void CItemGroupManager::saveGroups()
{
	std::string userGroupFileName = "save/groups_" + PlayerSelectedFileName + ".xml";
	if (PlayerSelectedFileName.empty())
	{
		nlwarning("Trying to save group with an empty PlayerSelectedFileName, aborting");
		return;
	}
	try
	{
		NLMISC::COFile f;
		if (f.open(userGroupFileName, false, false, true))
		{

			NLMISC::COXml xmlStream;
			xmlStream.init(&f);
			xmlDocPtr doc = xmlStream.getDocument();
			xmlNodePtr node = xmlNewDocNode(doc, NULL, (const xmlChar *)"item_groups", NULL);
			xmlDocSetRootElement(doc, node);
			for (int i = 0; i < _Groups.size(); i++)
			{
				CItemGroup group = _Groups[i];
				group.writeTo(node);
			}
			xmlStream.flush();
			f.close();
		}
		else
		{
			nlwarning("Can't open the file %s", userGroupFileName.c_str());
		}
	}
	catch (const NLMISC::Exception &e)
	{
		nlwarning("Error while writing the file %s : %s.", userGroupFileName.c_str(), e.what());
	}
}

bool CItemGroupManager::loadGroups()
{

	std::string userGroupFileName = "save/groups_" + PlayerSelectedFileName + ".xml";
	if (PlayerSelectedFileName.empty())
	{
		nlwarning("Trying to load group with an empty PlayerSelectedFileName, aborting");
		return false;
	}
	if (!NLMISC::CFile::fileExists(userGroupFileName) || NLMISC::CFile::getFileSize(userGroupFileName) == 0)
	{
		nlinfo("No item groups file found !");
		return false;
	}
	// Init loading
	NLMISC::CIFile f;
	f.open(userGroupFileName);
	NLMISC::CIXml xmlStream;
	xmlNodePtr globalEnclosing;
	try
	{
		xmlStream.init(f);
		// Actual loading
		globalEnclosing = xmlStream.getRootNode();
	}
	catch (const NLMISC::EXmlParsingError &ex)
	{
		nlwarning("Failed to parse '%s', skip", userGroupFileName.c_str());
		return false;
	}
	if (!globalEnclosing)
	{
		nlwarning("no root element in item_group xml, skipping xml parsing");
		return false;
	}
	if (strcmp(((char *)globalEnclosing->name), "item_groups"))
	{
		nlwarning("wrong root element in item_group xml, skipping xml parsing");
		return false;
	}
	xmlNodePtr curNode = globalEnclosing->children;
	while (curNode)
	{
		if (strcmp((char *)curNode->name, "group") == 0)
		{
			CItemGroup group;
			group.readFrom(curNode);
			if (group.empty())
			{
				nlwarning("Group '%s' loaded empty. Possibly on deprecated item group file, please recreate it or contact support.", group.name.c_str());
			}
			_Groups.push_back(group);
		}
		curNode = curNode->next;
	}
	f.close();

	return true;
}

void CItemGroupManager::undrawGroupsList()
{
	CGroupList *pParent = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(LIST_ITEMGROUPS));
	if (pParent == NULL)
		return;
	pParent->clearGroups();
	pParent->setDynamicDisplaySize(false);
	CGroupList *pParent2 = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(LIST_ITEMGROUPS_2));
	if (pParent2 == NULL)
		return;
	pParent2->clearGroups();
	pParent2->setDynamicDisplaySize(false);
}

void CItemGroupManager::drawGroupsList()
{
	// rebuild groups list
	undrawGroupsList();
	CGroupList *pParent = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(LIST_ITEMGROUPS));
	if (pParent == NULL)
		return;
	CGroupList *pParent2 = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(LIST_ITEMGROUPS_2));
	if (pParent2 == NULL)
		return;
	CViewBase *pView = pParent->getParent()->getView(LIST_EMPTY_TEXT);
	CViewBase *pView2 = pParent2->getParent()->getView(LIST_EMPTY_TEXT);
	if (_Groups.empty())
	{
		pView->setActive(true);
		pView2->setActive(true);
		return;
	}
	else
	{
		pView->setActive(false);
		pView2->setActive(false);
	}
	for (uint i = 0; i < _Groups.size(); i++)
	{
		CInterfaceGroup *pLine = generateGroupsListLine(LIST_ITEMGROUPS, i);
		if (pLine == NULL)
			continue;
		CInterfaceGroup *pLine2 = generateGroupsListLine(LIST_ITEMGROUPS_2, i);
		if (pLine2 == NULL)
			continue;

		// Add to the list
		pLine->setParent(pParent);
		pParent->addChild(pLine);
		pLine2->setParent(pParent2);
		pParent2->addChild(pLine2);
	}
}

CInterfaceGroup *CItemGroupManager::generateGroupsListLine(std::string parent, uint i)
{
	// create the group line
	std::string templateId = parent + ":g" + NLMISC::toString(i).c_str();
	std::vector<std::pair<string, string>> vParams;
	vParams.push_back(std::pair<string, string>("id", templateId));
	vParams.push_back(std::pair<string, string>("name", _Groups[i].name));
	CInterfaceGroup *pLine = NULL;
	pLine = CWidgetManager::getInstance()->getParser()->createGroupInstance(TEMPLATE_ITEMGROUP_ITEM, parent, vParams);
	if (pLine == NULL)
		return NULL;

	// Set name
	CViewText *pViewName = dynamic_cast<CViewText *>(pLine->getView(TEMPLATE_ITEMGROUP_ITEM_NAME));
	if (pViewName != NULL)
		pViewName->setText(_Groups[i].name);

	return pLine;
}

void CItemGroupManager::update()
{
	if (_StartInvalidAction != 0 && _StartInvalidAction <= NetMngr.getCurrentServerTick())
	{
		invalidActions(_StartInvalidAction, _EndInvalidAction);
		_StartInvalidAction = 0;
	}
	if (_EndInvalidAction != 0 && _EndInvalidAction <= NetMngr.getCurrentServerTick())
	{
		_EndInvalidAction = 0;
		validActions();
	}
}

void CItemGroupManager::fakeInvalidActions(NLMISC::TGameCycle time)
{
	// We cannot directly invalidate action or our invalidate will be overriden by the server
	// (and that means we won't actually have one because it's buggy with multiple equip in a short time)
	// So we wait a bit (currently 6 ticks is enough) to do it
	_StartInvalidAction = NetMngr.getCurrentServerTick() + 6;
	_EndInvalidAction = NetMngr.getCurrentServerTick() + time;
	invalidActions(NetMngr.getCurrentServerTick(), _EndInvalidAction);
}

void CItemGroupManager::invalidActions(NLMISC::TGameCycle begin, NLMISC::TGameCycle end)
{
	NLGUI::CDBManager *pDB = NLGUI::CDBManager::getInstance();
	NLMISC::CCDBNodeLeaf *node;
	// This are the db update server sends when an user equip an item, see egs/player_manager/gear_latency.cpp CGearLatency::setSlot
	node = pDB->getDbProp("SERVER:USER:ACT_TSTART", false);
	if (node)
		node->setValue64(begin);

	node = pDB->getDbProp("SERVER:USER:ACT_TEND", false);
	if (node)
		node->setValue64(end);

	node = pDB->getDbProp("SERVER:EXECUTE_PHRASE:SHEET", false);
	static NLMISC::CSheetId equipSheet("big_equip_item.sbrick");
	if (node)
		node->setValue64((sint64)equipSheet.asInt());

	node = pDB->getDbProp("SERVER:EXECUTE_PHRASE:PHRASE", false);
	if (node)
		node->setValue64(0);
}

void CItemGroupManager::validActions()
{
	NLGUI::CDBManager *pDB = NLGUI::CDBManager::getInstance();
	NLMISC::CCDBNodeLeaf *node;
	node = pDB->getDbProp("SERVER:USER:ACT_TSTART", false);
	if (node)
		node->setValue64(0);

	node = pDB->getDbProp("SERVER:USER:ACT_TEND", false);
	if (node)
		node->setValue64(0);

	node = pDB->getDbProp("SERVER:EXECUTE_PHRASE:SHEET", false);
	if (node)
		node->setValue32(0);

	node = pDB->getDbProp("SERVER:EXECUTE_PHRASE:PHRASE", false);
	if (node)
		node->setValue32(0);
}

// move a group from all available inventory to dst
bool CItemGroupManager::moveGroup(std::string name, INVENTORIES::TInventory dst)
{
	CItemGroup *group = findGroup(name);
	if (!group)
	{
		nlinfo("group %s not found", name.c_str());
		return false;
	}
	if (dst == INVENTORIES::UNDEFINED)
	{
		nlinfo("Destination inventory not found");
		return false;
	}
	CInventoryManager *pIM = CInventoryManager::getInstance();

	std::string moveParams = "to=lists|nblist=1|listsheet0=" + CInventoryManager::invToDbPath(dst);
	group->updateItemsLocation(); // update location of items in the inventory
	for (uint i = 0; i < group->getItems().size(); i++)
	{
		CInventoryItem item = group->getItems()[i]->pItem;
		// Workaround: sometimes item are marked as equipped by pIM->isBagItemWeared() even though they aren't really
		// Because of a synchronisation error between client and server
		// Also, if the item is already in dst inventory, we don't want to move it again.
		if (item.pCS == NULL || item.origin == INVENTORIES::UNDEFINED || item.origin == dst || isItemReallyEquipped(item.pCS))
			continue;
		CAHManager::getInstance()->runActionHandler("move_item", item.pCS, moveParams);
	}

	return true;
}

bool CItemGroupManager::equipGroup(std::string name, bool pullBefore)
{
	CItemGroup *group = findGroup(name);
	if (!group)
	{
		nlinfo("group %s not found", name.c_str());
		return false;
	}
	if (pullBefore)
		moveGroup(name, INVENTORIES::bag);

	// Start by unequipping all slot that user wants to unequip
	for (int i = 0; i < group->equipRemoves.size(); i++)
	{
		SLOT_EQUIPMENT::TSlotEquipment slot = group->equipRemoves[i];
		std::string dbPath;
		// For hands equip, dbPath obviously starts at 0, we need to offset correctly
		if (slot == SLOT_EQUIPMENT::HANDL || slot == SLOT_EQUIPMENT::HANDR)
			dbPath = "LOCAL:INVENTORY:HAND:" + NLMISC::toString((uint32)slot - SLOT_EQUIPMENT::HANDL);
		else
			dbPath = "LOCAL:INVENTORY:EQUIP:" + NLMISC::toString((uint32)slot);
		CInventoryManager::getInstance()->unequip(dbPath);
	}

	for (int i = 0; i < group->hotbarRemoves.size(); i++)
	{
		uint16 slot = group->hotbarRemoves[i];
		std::string dbPath;
		dbPath = "LOCAL:INVENTORY:HOTBAR:" + NLMISC::toString(slot);
		CInventoryManager::getInstance()->unequip(dbPath);
	}

	uint32 maxEquipTime = 0;

#ifdef NL_ISO_CPP0X_AVAILABLE
	std::map<ITEM_TYPE::TItemType, bool> possiblyDual = {
		{ ITEM_TYPE::ANKLET, false },
		{ ITEM_TYPE::BRACELET, false },
		{ ITEM_TYPE::EARING, false },
		{ ITEM_TYPE::RING, false },
	};
#else
	std::map<ITEM_TYPE::TItemType, bool> possiblyDual;

	possiblyDual[ITEM_TYPE::ANKLET] = false;
	possiblyDual[ITEM_TYPE::BRACELET] = false;
	possiblyDual[ITEM_TYPE::EARING] = false;
	possiblyDual[ITEM_TYPE::RING] = false;
#endif

	// TODO: updateItemsLocation() is possibly redundant here, (because it is called via moveGroup() just above) but it's not clear if it's safe to remove it
	group->updateItemsLocation(); // update location of items in the inventory
	std::vector<CInventoryItem> equipDuals;
	// start equipping equip items
	for (int i = 0; i < group->equipItems.size(); i++)
	{
		CItemGroup::CEquipItem equipItem = group->equipItems[i];
		CInventoryItem item = equipItem.pItem;

		if (item.origin == INVENTORIES::UNDEFINED)
		{
			nlwarning("<CItemGroupManager::equipGroup> equip item origin is null");
			continue;
		}

		if (item.pCS == NULL)
		{
			nlwarning("<CItemGroupManager::equipGroup> equip item sheet is null");
			continue;
		}

		// if the item is in the bag, we need to move it to the equip slot
		if (item.origin == INVENTORIES::bag)
		{
			ITEM_TYPE::TItemType itemType = item.pCS->asItemSheet()->ItemType;
			// We'll equip items in left hand later (the right hand will be normally equipped)
			// This way, if we switch from 2 hands to 2 * 1 hands, both hands will be equipped correctly (first right, which will remove the 2 hands, then left)
			// If we don't, we might try to equip the left hand first, which will do nothing because we have a 2 hands equipped
			if (equipItem.slot == SLOT_EQUIPMENT::HANDL)
			{
				equipDuals.push_back(item);
				continue;
			}

			// If the item can be weared 2 times, don't automatically equip the second one
			// Or else it will simply replace the first. We'll deal with them later
			if (possiblyDual.find(itemType) != possiblyDual.end())
			{
				if (possiblyDual[itemType])
				{
					equipDuals.push_back(item);
					continue;
				}
				possiblyDual[itemType] = true;
			}
			maxEquipTime = std::max(maxEquipTime, item.pCS->asItemSheet()->EquipTime);
			CInventoryManager::getInstance()->autoEquip(item.indexInBag, true);
		}
	}

	// Manually equip dual items
	for (int i = 0; i < equipDuals.size(); i++)
	{
		CInventoryItem item = equipDuals[i];
		ITEM_TYPE::TItemType itemType = item.pCS->asItemSheet()->ItemType;
		std::string dstPath = string(LOCAL_INVENTORY);
		switch (itemType)
		{
		case ITEM_TYPE::ANKLET:
			dstPath += ":EQUIP:" + NLMISC::toString((int)SLOT_EQUIPMENT::ANKLER);
			break;
		case ITEM_TYPE::BRACELET:
			dstPath += ":EQUIP:" + NLMISC::toString((int)SLOT_EQUIPMENT::WRISTR);
			;
			break;
		case ITEM_TYPE::EARING:
			dstPath += ":EQUIP:" + NLMISC::toString((int)SLOT_EQUIPMENT::EARR);
			;
			break;
		case ITEM_TYPE::RING:
			dstPath += ":EQUIP:" + NLMISC::toString((int)SLOT_EQUIPMENT::FINGERR);
			;
			break;
		case ITEM_TYPE::DAGGER:
		case ITEM_TYPE::BUCKLER:
		case ITEM_TYPE::SHIELD:
			dstPath += ":HAND:1";
			break;
		default:
			break;
		}

		std::string srcPath = item.pCS->getSheet();
		maxEquipTime = std::max(maxEquipTime, item.pCS->asItemSheet()->EquipTime);
		CInventoryManager::getInstance()->equip(srcPath, dstPath);
	}

	// equip hotbar items
	for (int i = 0; i < group->hotbarItems.size(); i++)
	{
		CItemGroup::CHotbarItem hotbarItem = group->hotbarItems[i];
		CInventoryItem item = hotbarItem.pItem;

		if (item.origin == INVENTORIES::UNDEFINED)
		{
			nlwarning("<CItemGroupManager::equipGroup> hotbar item origin is null");
			continue;
		}

		if (item.pCS == NULL)
		{
			nlwarning("<CItemGroupManager::equipGroup> hotbar item sheet is null");
			continue;
		}

		// if the item is in the bag, we need to move it to the equip slot
		if (item.origin == INVENTORIES::bag)
		{
			std::string srcPath = item.pCS->getSheet();
			std::string dstPath = string(LOCAL_INVENTORY) + ":HOTBAR:" + NLMISC::toString(hotbarItem.slot);
			CInventoryManager::getInstance()->equip(srcPath, dstPath);
		}
	}

	// For some reason, there is no (visual) invalidation (server still blocks any action), force one
	// Unfortunately, there is no clean way to do this, so we'll simulate one
	if (maxEquipTime > 0)
		fakeInvalidActions((NLMISC::TGameCycle)maxEquipTime);
	return true;
}

bool CItemGroupManager::createGroup(std::string name, bool removeUnequiped)
{
	if (findGroup(name))
		return false;
	CItemGroup group = CItemGroup();
	group.name = name;
	uint i;
	CDBCtrlSheet *pCS;
	for (i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		SLOT_EQUIPMENT::TSlotEquipment slot = (SLOT_EQUIPMENT::TSlotEquipment)i;
		// Instead of doing two separate for, just be a bit tricky for hand equipment
		if (slot == SLOT_EQUIPMENT::HANDR)
			pCS = CInventoryManager::getInstance()->getHandSheet(0);
		else if (slot == SLOT_EQUIPMENT::HANDL)
			pCS = CInventoryManager::getInstance()->getHandSheet(1);
		else
			pCS = CInventoryManager::getInstance()->getEquipSheet(i);
		if (!pCS)
			continue;
		if (pCS->isSheetValid())
		{
			group.addItem(pCS->getItemCreateTime(), pCS->getItemSerial(), slot);
		}
		else if (removeUnequiped)
		{
			if (slot != SLOT_EQUIPMENT::UNDEFINED && slot != SLOT_EQUIPMENT::FACE)
				group.addRemove(slot);
		}
	}
	for (i = 0; i < MAX_HOTBARINV_ENTRIES; ++i)
	{
		pCS = CInventoryManager::getInstance()->getHotbarSheet(i);
		if (!pCS)
			continue;
		if (pCS->isSheetValid())
		{
			group.addItem(pCS->getItemCreateTime(), pCS->getItemSerial(), (uint16)i);
		}
		else if (removeUnequiped)
		{
			group.addRemove((uint16)i);
		}
	}

	_Groups.push_back(group);

	// must redraw the list
	drawGroupsList();

	return true;
}
bool CItemGroupManager::deleteGroup(std::string name)
{
	std::vector<CItemGroup> tmp;
	for (int i = 0; i < _Groups.size(); i++)
	{
		CItemGroup group = _Groups[i];
		if (group.name == name)
			continue;
		tmp.push_back(group);
	}
	// Nothing removed, error
	if (tmp.size() == _Groups.size())
		return false;
	_Groups = tmp;

	// must redraw the list
	drawGroupsList();

	return true;
}

void CItemGroupManager::listGroup()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	pIM->displaySystemInfo(NLMISC::CI18N::get("cmdListGroupHeader"));
	for (int i = 0; i < _Groups.size(); i++)
	{
		CItemGroup group = _Groups[i];
		string msg = NLMISC::CI18N::get("cmdListGroupLine");
		// Use utf-8 string because group name can contain accentued characters (and stuff like that)
		string nameUC = group.name;
		NLMISC::strFindReplace(msg, "%name", nameUC);
		NLMISC::strFindReplace(msg, "%size", NLMISC::toString(group.getItems().size()));
		pIM->displaySystemInfo(msg);
	}
}

// Used by AH
std::vector<std::string> CItemGroupManager::getGroupNames(CDBCtrlSheet *pCS)
{
	std::vector<std::string> out;
	for (int i = 0; i < _Groups.size(); i++)
	{
		CItemGroup group = _Groups[i];
		if (group.contains(pCS))
			out.push_back(group.name);
	}
	return out;
}

// Private methods
CItemGroup *CItemGroupManager::findGroup(std::string name)
{
	for (int i = 0; i < _Groups.size(); i++)
	{
		if (_Groups[i].name == name)
			return &_Groups[i];
	}
	return NULL;
}

bool CItemGroupManager::isItemReallyEquipped(CDBCtrlSheet *item)
{
	if (item == NULL)
	{
		nlinfo("<CItemGroupManager::isItemReallyEquipped> item is null");
		return true;
	}

	CDBCtrlSheet *pCS;
	for (uint32 i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		SLOT_EQUIPMENT::TSlotEquipment slot = (SLOT_EQUIPMENT::TSlotEquipment)i;
		// Instead of doing two separate for, just be a bit tricky for hand equipment
		if (slot == SLOT_EQUIPMENT::HANDR)
			pCS = CInventoryManager::getInstance()->getHandSheet(0);
		else if (slot == SLOT_EQUIPMENT::HANDL)
			pCS = CInventoryManager::getInstance()->getHandSheet(1);
		else
			pCS = CInventoryManager::getInstance()->getEquipSheet(i);
		if (!pCS)
			continue;
		// Can't directly compare ID (as pCS is like "ui:interface:inv_equip:content:equip:armors:feet" and item is like "ui:interface:inv_pa3:content:iil:bag_list:list:sheet57")
		// Instead check inventory + slot
		if ((pCS->getInventoryIndex() == item->getInventoryIndex())
		    && (pCS->getIndexInDB() == item->getIndexInDB()))
		{
			return true;
		}
	}

	for (uint32 i = 0; i < MAX_HOTBARINV_ENTRIES; ++i)
	{
		pCS = CInventoryManager::getInstance()->getHotbarSheet(i);
		if (!pCS)
			continue;
		if ((pCS->getInventoryIndex() == item->getInventoryIndex())
		    && (pCS->getIndexInDB() == item->getIndexInDB()))
		{
			return true;
		}
	}
	return false;
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
