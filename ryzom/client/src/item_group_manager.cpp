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
#include "nel/misc/xml_macros.h"
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

// return true if any item in the group match the parameter ; slot is UNDEFINED unless the item has been found in the group
bool CItemGroup::contains(CDBCtrlSheet *pCS)
{
	for (int i = 0; i < items.size(); i++)
	{
		CItem item = items[i];
		if (item.createTime == pCS->getItemCreateTime() && item.serial == pCS->getItemSerial())
			return true;
	}
	return false;
}

void CItemGroup::addSheet(CDBCtrlSheet *pCS, CSlot slot, bool removeEmpty)
{
	if (!pCS)
		return;

	if (pCS->isSheetValid())
	{
		addItem(CItem(pCS->getItemCreateTime(), pCS->getItemSerial(), slot));
	}
	else if (removeEmpty)
	{
		addRemoveSlot(slot);
	}
}

void CItemGroup::addItem(CItem item)
{
	if (!item.dstSlot.isValid())
		return;

	// Check if item already exists in group, this could happen if:
	// 1. Creating a group with a 2-hand item (and the item is found both in handR and handL)
	// 2. If user incorrectly edits the xml file
	for (int i = 0; i < items.size(); i++)
	{
		CItem existingItem = items[i];
		if (existingItem.createTime == item.createTime && existingItem.serial == item.serial)
		{
			nldebug("<CItemGroup::addItem> Not adding duplicate item, createTime: %d, serial: %d", item.createTime, item.serial);
			// If duplicate item because of a 2-hand item:
			// If it's existing as a left hand item, overwrite it as a right hand item instead (so we have only 1 correct item)
			if (existingItem.dstSlot == CSlot::handSlot(1) && item.dstSlot == CSlot::handSlot(0))
				existingItem.dstSlot = CSlot::handSlot(0);
			return;
		}
	}

	items.push_back(item);
}

void CItemGroup::addRemoveSlot(CSlot slot)
{
	if (!slot.isValid())
		return;

	removeSlots.push_back(slot);
}

void CItemGroup::CItem::equip(uint32 &equipTime)
{
	if (!pCS)
	{
		nlwarning("<CItemGroup::CItem::equip> inv item is invalid");
		return;
	}

	if (!dstSlot.isValid())
	{
		nlwarning("<CItemGroup::CItem::equip> item destination slot is invalid");
		return;
	}

	// if item is already equipped, and in the good slot, no need to equip
	if (dstSlot.getSheet()->isSheetEqual(pCS))
	{
		return;
	}

	// make sure the item can go into the slot (for example left-hand item can't always in left-hand slot)
	// this is also checked on the server, but avoids visual glitch if checked on client too
	if (!dstSlot.getSheet()->canDropItem(pCS))
	{
		nlwarning("<CItemGroup::CItem::equip> item %d can't be dropped in slot %s", pCS->getSheetId(), dstSlot.toDbPath().c_str());
		return;
	}

	std::string srcPath = pCS->getSheet();
	equipTime = std::max(equipTime, pCS->asItemSheet()->EquipTime);
	CInventoryManager::getInstance()->equip(srcPath, dstSlot.toDbPath());
}

// return if the item is already in it's destination slot
bool CItemGroup::CItem::isInDestinationSlot()
{
	return dstSlot.getSheet()->getInventoryIndex() == pCS->getInventoryIndex() && dstSlot.getSheet()->getIndexInDB() == pCS->getIndexInDB();
}

void CItemGroup::CSlot::writeTo(xmlNodePtr node)
{
	xmlSetProp(node, (const xmlChar *)"branch", (const xmlChar *)INVENTORIES::toString(branch).c_str());
	xmlSetProp(node, (const xmlChar *)"index", (const xmlChar *)NLMISC::toString(index).c_str());
}

void CItemGroup::CItem::writeTo(xmlNodePtr node)
{
	xmlSetProp(node, (const xmlChar *)"createTime", (const xmlChar *)NLMISC::toString(createTime).c_str());
	xmlSetProp(node, (const xmlChar *)"serial", (const xmlChar *)NLMISC::toString(serial).c_str());
	dstSlot.writeTo(node);
}

void CItemGroup::writeTo(xmlNodePtr node)
{
	xmlNodePtr groupNode = xmlNewChild(node, NULL, (const xmlChar *)"group", NULL);
	xmlSetProp(groupNode, (const xmlChar *)"name", (const xmlChar *)name.c_str());
	for (int i = 0; i < items.size(); i++)
	{
		xmlNodePtr itemNode = xmlNewChild(groupNode, NULL, (const xmlChar *)"item", NULL);
		items[i].writeTo(itemNode);
	}
	for (int i = 0; i < removeSlots.size(); i++)
	{
		xmlNodePtr removeNode = xmlNewChild(groupNode, NULL, (const xmlChar *)"remove", NULL);
		removeSlots[i].writeTo(removeNode);
	}
}

CItemGroup::CSlot CItemGroup::CSlot::readFromV1(xmlNodePtr node)
{
	CXMLAutoPtr prop;
	string equipSlot;
	XML_READ_STRING(node, "slot", equipSlot, "");
	return CItemGroup::CSlot::fromSlotEquipment(SLOT_EQUIPMENT::stringToSlotEquipment(NLMISC::toUpperAscii(equipSlot)));
}

CItemGroup::CSlot CItemGroup::CSlot::readFromV2(xmlNodePtr node)
{
	CXMLAutoPtr prop;
	INVENTORIES::TInventory branch = INVENTORIES::UNDEFINED;
	prop = xmlGetProp(node, (xmlChar *)"branch");
	if (prop)
		branch = INVENTORIES::toInventory((const char *)prop);

	uint16 index = 0;
	prop = (char *)xmlGetProp(node, (xmlChar *)"index");
	if (prop)
		NLMISC::fromString((const char *)prop, index);

	return CSlot(branch, index);
}

CItemGroup::CItem CItemGroup::CItem::readFrom(xmlNodePtr node)
{
	CXMLAutoPtr prop;

	sint32 createTime = 0;
	prop = (char *)xmlGetProp(node, (xmlChar *)"createTime");
	if (prop)
		NLMISC::fromString((const char *)prop, createTime);

	// Version 0 does not use createTime, version 0 is not supported for migrations
	if (createTime == 0)
		nlwarning("<CItemGroup::CItem::readFrom> Possibly on unsupported item group version, please remake the item group or contact support.");

	sint32 serial = 0;
	prop = (char *)xmlGetProp(node, (xmlChar *)"serial");
	if (prop)
		NLMISC::fromString((const char *)prop, serial);

	return CItem(createTime, serial, CSlot());
}

void CItemGroup::deserialize(xmlNodePtr node, std::string version)
{
	CXMLAutoPtr prop;

	prop = (char *)xmlGetProp(node, (xmlChar *)"name");
	if (prop)
		NLMISC::fromString((const char *)prop, name);

	xmlNodePtr curNode = node->children;
	while (curNode)
	{
		CSlot slot;
		if (version == "1")
			slot = CSlot::readFromV1(curNode);
		else if (version == "2")
			slot = CSlot::readFromV2(curNode);
		else
		{
			nlwarning("<CItemGroup::deserialize> unknown version, can't deserialize group");
			return;
		}

		if (strcmp((char *)curNode->name, "item") == 0)
		{
			CItem item = CItem::readFrom(curNode);
			item.dstSlot = slot;
			addItem(item);
		}
		else if (strcmp((char *)curNode->name, "remove") == 0)
			addRemoveSlot(slot);

		curNode = curNode->next;
	}

	// sort the items by slot (important so that left hand is equipped after right hand)
	std::sort(items.begin(), items.end());
}

// Updates the CDBCtrlSheet in each CItem
// ! This function must be called before every interaction with items of a CItemGroup
//
// Explanation: We must link the CItem (which just represents identifiers of an item) of the CItemGroup
// with an item in the current inventory. Basically, here we are locating the position of the item in the inventory
// and we are setting it to the pCS property of CItem, so we can then equip or move CItem via its CDBCtrlSheet.
// It is possible that the item is deleted or moved, so we must check for this.
void CItemGroup::updateSheets()
{
	CInventoryManager *pIM = CInventoryManager::getInstance();
	for (uint i = 0; i < items.size(); i++)
	{
		CItem item = items[i];
		bool found = false;
		for (int i = 0; i < INVENTORIES::NUM_ALL_INVENTORY; i++)
		{
			INVENTORIES::TInventory inventory = (INVENTORIES::TInventory)i;
			if (pIM->isInventoryAvailable(inventory))
			{
				std::string dbPath = CInventoryManager::invToDbPath(inventory);
				if (dbPath.empty() || (inventory == INVENTORIES::guild && !ClientCfg.ItemGroupAllowGuild))
				{
					nldebug("<CItemGroup::updateSheets> Inventory type %s not supported", INVENTORIES::toString(inventory).c_str());
					continue;
				}
				IListSheetBase *pList = dynamic_cast<IListSheetBase *>(CWidgetManager::getInstance()->getElementFromId(dbPath));
				for (int i = 0; i < pList->getNbSheet(); i++)
				{
					CDBCtrlSheet *pCS = pList->getSheet(i);
					if (item.createTime == pCS->getItemCreateTime() && item.serial == pCS->getItemSerial())
					{
						item.pCS = pCS;
						found = true;
						break;
					}
				}
				if (found)
					break;
			}
		}
		if (!found)
		{
			nlinfo("<CItemGroup::updateSheets> Item not found in inventory: createTime: %d, serial: %d", item.createTime, item.serial);
			item.pCS = NULL;
		}
		items.at(i) = item;
	}
}

const std::string CItemGroup::CSlot::toDbPath()
{
	std::string dbPath = "";
	std::string dbBranch = INVENTORIES::toLocalDbBranch(branch);

	if (!dbBranch.empty())
		dbPath = string(LOCAL_INVENTORY) + ":" + dbBranch + ":" + NLMISC::toString(index);

	return dbPath;
}

const std::string CItemGroup::CSlot::toString()
{
	std::string commonName;
	if (branch == INVENTORIES::handling)
		commonName = "Hand" + string(index == 0 ? "R" : "L");
	else if (branch == INVENTORIES::equipment)
		commonName = SLOT_EQUIPMENT::toString((SLOT_EQUIPMENT::TSlotEquipment)index);
	else if (branch == INVENTORIES::hotbar)
		commonName = "Pocket #" + NLMISC::toString(index + 1);
	else
		commonName = "unknown";
	return NLMISC::toString("%s:%sindex=\"%d\"", commonName.c_str(), string(10 - commonName.length(), ' ').c_str(), index);
}

bool CItemGroup::CSlot::isValid()
{
	return ((branch == INVENTORIES::handling && index < MAX_HANDINV_ENTRIES)
	    || (branch == INVENTORIES::equipment && index < MAX_EQUIPINV_ENTRIES)
	    || (branch == INVENTORIES::hotbar && index < MAX_HOTBARINV_ENTRIES));
}

CDBCtrlSheet *CItemGroup::CSlot::getSheet()
{
	CInventoryManager *pIM = CInventoryManager::getInstance();
	CDBCtrlSheet *pCS = NULL;
	if (branch == INVENTORIES::handling)
		pCS = pIM->getHandSheet(index);
	else if (branch == INVENTORIES::equipment)
		pCS = pIM->getEquipSheet(index);
	else if (branch == INVENTORIES::hotbar)
		pCS = pIM->getHotbarSheet(index);
	return pCS;
}

CItemGroup::CSlot CItemGroup::CSlot::fromSlotEquipment(SLOT_EQUIPMENT::TSlotEquipment slotEquipment)
{
	CSlot slot;
	if (slotEquipment == SLOT_EQUIPMENT::HANDR)
		slot = handSlot(0);
	else if (slotEquipment == SLOT_EQUIPMENT::HANDL)
		slot = handSlot(1);
	else
		slot = equipSlot(slotEquipment);
	return slot;
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
	CGroupMenu *pRootMenu = dynamic_cast<CGroupMenu *>(pWM->getElementFromId(MENU_IN_BAG));
	CGroupSubMenu *pMenu = pRootMenu->getRootMenu();
	// get item subgroup
	CGroupMenu *pGroupMenu = dynamic_cast<CGroupMenu *>(pWM->getElementFromId(ITEMGROUP_MENU));
	CGroupSubMenu *pGroupSubMenu = NULL;
	if (pGroupMenu)
		pGroupSubMenu = pGroupMenu->getRootMenu();
	if (pMenu && pGroupSubMenu)
		pMenu->setSubMenu(pMenu->getNumLine() - 1, pGroupSubMenu);
	else
		nlwarning("<CItemGroupManager::linkInterface> Couldn't link group submenu to item_menu_in_bag, check your widgets.xml file");

	// must draw the list equip tab
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
	CGroupMenu *pGroupMenu = dynamic_cast<CGroupMenu *>(pWM->getElementFromId(ITEMGROUP_MENU));
	CGroupSubMenu *pGroupSubMenu = NULL;
	if (pGroupMenu)
		pGroupSubMenu = pGroupMenu->getRootMenu();
	if (pGroupMenu)
		pGroupMenu->reset();
	if (pGroupMenu && pGroupSubMenu)
		pGroupMenu->delGroup(pGroupSubMenu, true);
	undrawGroupsList();
}

std::string CItemGroupManager::getFilePath(std::string playerName)
{
	return "save/groups_" + playerName + ".xml";
}

// Inspired from macro parsing
void CItemGroupManager::saveGroups()
{
	if (PlayerSelectedFileName.empty())
	{
		nlwarning("<CItemGroupManager::saveGroups> Trying to save group with an empty PlayerSelectedFileName, aborting");
		return;
	}

	std::string userGroupFileName = getFilePath(PlayerSelectedFileName);
	try
	{
		NLMISC::COFile f;
		if (f.open(userGroupFileName, false, false, true))
		{
			NLMISC::COXml xmlStream;
			xmlStream.init(&f);
			xmlDocPtr doc = xmlStream.getDocument();
			xmlNodePtr comment = xmlNewDocComment(doc, (const xmlChar *)generateDocumentation().c_str());
			xmlNodePtr node = xmlNewDocNode(doc, NULL, (const xmlChar *)"item_groups", NULL);
			xmlSetProp(node, (const xmlChar *)"version", (const xmlChar *)ITEMGROUPS_CURRENT_VERSION);
			xmlDocSetRootElement(doc, comment);
			xmlAddSibling(comment, node);
			for (int i = 0; i < _Groups.size(); i++)
			{
				CItemGroup group = _Groups[i];
				group.writeTo(node);
			}
			xmlStream.flush();
			f.close();
		}
		else
			nlwarning("<CItemGroupManager::saveGroups> Can't open the file %s", userGroupFileName.c_str());
	}
	catch (const NLMISC::Exception &e)
	{
		nlwarning("<CItemGroupManager::saveGroups> Error while writing the file %s : %s.", userGroupFileName.c_str(), e.what());
	}
}

bool CItemGroupManager::loadGroups()
{
	if (PlayerSelectedFileName.empty())
	{
		nlwarning("<CItemGroupManager::loadGroups> Trying to load group with an empty PlayerSelectedFileName, aborting");
		return false;
	}

	std::string userGroupFileName = getFilePath(PlayerSelectedFileName);
	if (!NLMISC::CFile::fileExists(userGroupFileName) || NLMISC::CFile::getFileSize(userGroupFileName) == 0)
	{
		nlinfo("<CItemGroupManager::loadGroups> No item groups file found !");
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
		nlwarning("<CItemGroupManager::loadGroups> Failed to parse '%s', skip", userGroupFileName.c_str());
		return false;
	}
	if (!globalEnclosing)
	{
		nlwarning("<CItemGroupManager::loadGroups> no root element in item_group xml, skipping xml parsing");
		return false;
	}
	if (strcmp(((char *)globalEnclosing->name), "item_groups"))
	{
		nlwarning("<CItemGroupManager::loadGroups> wrong root element in item_group xml, skipping xml parsing");
		return false;
	}

	// get version of the item groups file
	CXMLAutoPtr prop;
	string version;
	XML_READ_STRING(globalEnclosing, "version", version, "1");

	// check if we need to migrate item groups save file
	if (version != ITEMGROUPS_CURRENT_VERSION)
	{
		nlinfo("<CItemGroupManager::loadGroups> item group version mismatch, performing migration if possible");
		// backup current file
		NLMISC::CFile::copyFile(getFilePath(PlayerSelectedFileName + "_backup"), getFilePath(PlayerSelectedFileName));
	}

	xmlNodePtr curNode = globalEnclosing->children;
	while (curNode)
	{
		if (strcmp((char *)curNode->name, "group") == 0)
		{
			CItemGroup group;
			group.deserialize(curNode, version);
			if (group.empty())
				nlwarning("<CItemGroupManager::loadGroups> Item group '%s' loaded empty. Possibly on unsupported item group version, please remake the item group or contact support.", group.name.c_str());
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
	if (!pParent)
		return;
	pParent->clearGroups();
	pParent->setDynamicDisplaySize(false);
	CGroupList *pParent2 = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(LIST_ITEMGROUPS_2));
	if (!pParent2)
		return;
	pParent2->clearGroups();
	pParent2->setDynamicDisplaySize(false);
}

void CItemGroupManager::drawGroupsList()
{
	// rebuild groups list
	undrawGroupsList();
	CGroupList *pParent = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(LIST_ITEMGROUPS));
	if (!pParent)
		return;
	CGroupList *pParent2 = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(LIST_ITEMGROUPS_2));
	if (!pParent2)
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
		if (!pLine)
			continue;
		CInterfaceGroup *pLine2 = generateGroupsListLine(LIST_ITEMGROUPS_2, i);
		if (!pLine2)
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
	if (!pLine)
		return NULL;

	// Set name
	CViewText *pViewName = dynamic_cast<CViewText *>(pLine->getView(TEMPLATE_ITEMGROUP_ITEM_NAME));
	if (pViewName)
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

// We cannot directly invalidate action or our invalidate will be overriden by the server
// (and that means we won't actually have one because it's buggy with multiple equip in a short time)
// So we wait a bit (currently 6 ticks is enough) to do it
void CItemGroupManager::fakeInvalidActions(NLMISC::TGameCycle time)
{
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
		nlinfo("<CItemGroupManager::moveGroup> group %s not found", name.c_str());
		return false;
	}

	if (dst == INVENTORIES::UNDEFINED)
	{
		nlinfo("<CItemGroupManager::moveGroup> Destination inventory not found");
		return false;
	}

	// update location of items in the inventory
	group->updateSheets();

	CInventoryManager *pIM = CInventoryManager::getInstance();
	for (uint i = 0; i < group->items.size(); i++)
	{
		CItemGroup::CItem item = group->items[i];

		if (!item.pCS)
			continue;

		// if the item is equipped, don't move
		CItemGroup::CSlot currentSlot;
		if (isItemEquipped(item.pCS, currentSlot))
		{
			// if item is equipped, but not already in the wanted slot, unequip it first
			if (!item.dstSlot.getSheet()->isSheetEqual(item.pCS))
				pIM->unequip(currentSlot.toDbPath());
			continue;
		}

		// If the item is already in dst inventory, no need to move it
		if (item.pCS->getInventoryIndex() == dst)
			continue;

		CAHManager::getInstance()->runActionHandler("move_item", item.pCS, "to=lists|nblist=1|listsheet0=" + CInventoryManager::invToDbPath(dst));
	}

	return true;
}

bool CItemGroupManager::equipGroup(std::string name, bool pullBefore)
{
	CItemGroup *pGroup = findGroup(name);
	if (!pGroup)
	{
		nlinfo("<CItemGroupManager::equipGroup> group %s not found", name.c_str());
		return false;
	}

	if (pullBefore)
		moveGroup(name, INVENTORIES::bag);

	// we must update the location of items, even after they were moved
	pGroup->updateSheets();

	// first, unequip all remove slots
	for (int i = 0; i < pGroup->removeSlots.size(); i++)
	{
		CItemGroup::CSlot slot = pGroup->removeSlots[i];
		std::string dbPath = slot.toDbPath();
		if (!dbPath.empty())
			CInventoryManager::getInstance()->unequip(dbPath);
	}

	// then, equip items
	uint32 equipTime = 0;
	for (int i = 0; i < pGroup->items.size(); i++)
	{
		CItemGroup::CItem item = pGroup->items[i];

		item.equip(equipTime);
	}

	// For some reason, there is no (visual) invalidation (server still blocks any action), force one
	// Unfortunately, there is no clean way to do this, so we'll simulate one
	// TODO review invalid actions
	//if (equipTime > 0)
	//	fakeInvalidActions((NLMISC::TGameCycle)equipTime);
	return true;
}

bool CItemGroupManager::createGroup(std::string name, bool removeEmpty)
{
	if (findGroup(name))
		return false;

	CInventoryManager *pIM = CInventoryManager::getInstance();
	CItemGroup group = CItemGroup();
	group.name = name;
	uint16 i;
	for (i = 0; i < MAX_HANDINV_ENTRIES; i++)
	{
		group.addSheet(pIM->getHandSheet(i), CItemGroup::CSlot::handSlot(i), removeEmpty);
	}
	for (i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		group.addSheet(pIM->getEquipSheet(i), CItemGroup::CSlot::equipSlot(i), removeEmpty);
	}
	for (i = 0; i < MAX_HOTBARINV_ENTRIES; ++i)
	{
		group.addSheet(pIM->getHotbarSheet(i), CItemGroup::CSlot::hotbarSlot(i), removeEmpty);
	}

	_Groups.push_back(group);

	// must redraw the list equip tab
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

	// must redraw the list equip tab
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
		NLMISC::strFindReplace(msg, "%name", group.name);
		NLMISC::strFindReplace(msg, "%size", NLMISC::toString(group.items.size()));
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

// Workaround: sometimes item are marked as equipped by pIM->isBagItemWeared() even though they aren't really
// Because of a synchronisation error between client and server
bool CItemGroupManager::isItemEquipped(CDBCtrlSheet *pItem, CItemGroup::CSlot &equipSlot)
{
	if (!pItem)
	{
		nlinfo("<CItemGroupManager::isItemEquipped> item is null");
		return true;
	}

	CInventoryManager *pIM = CInventoryManager::getInstance();
	CDBCtrlSheet *pCS;
	for (uint i = 0; i < MAX_HANDINV_ENTRIES; i++)
	{
		pCS = pIM->getHandSheet(i);
		equipSlot = CItemGroup::CSlot::handSlot(i);
		if (pItem->isSheetEqual(pCS))
			return true;
	}
	for (uint32 i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		pCS = pIM->getEquipSheet(i);
		equipSlot = CItemGroup::CSlot::equipSlot(i);
		if (pItem->isSheetEqual(pCS))
			return true;
	}

	for (uint32 i = 0; i < MAX_HOTBARINV_ENTRIES; ++i)
	{
		pCS = pIM->getHotbarSheet(i);
		equipSlot = CItemGroup::CSlot::hotbarSlot(i);
		if (pItem->isSheetEqual(pCS))
			return true;
	}
	return false;
}

std::string CItemGroupManager::generateDocumentation()
{
	std::string out;
	uint16 i;
	out += "\r\nItem Groups - Version " + string(ITEMGROUPS_CURRENT_VERSION) + "\r\n\r\n/// branch=\"handling\" ///\r\n";
	for (i = 0; i < MAX_HANDINV_ENTRIES; i++)
		out += CItemGroup::CSlot::handSlot(i).toString() + "\r\n";
	out += "\r\n/// branch=\"equipment\" ///\r\n";
	for (i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
		if (i != SLOT_EQUIPMENT::HANDL && i != SLOT_EQUIPMENT::HANDR)
			out += CItemGroup::CSlot::equipSlot(i).toString() + "\r\n";
	out += "\r\n/// branch=\"hotbar\" ///\r\n";
	for (i = 0; i < MAX_HOTBARINV_ENTRIES; ++i)
		out += CItemGroup::CSlot::hotbarSlot(i).toString() + "\r\n";
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
