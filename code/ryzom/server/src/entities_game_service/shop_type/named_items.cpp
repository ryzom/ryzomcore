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
#include "nel/misc/path.h"

#include "named_items.h"
#include "game_item_manager/game_item_manager.h"
#include "game_item_manager/player_inv_auto_resize.h"
#include "mission_manager/mission_parser.h"
#include "egs_sheets/egs_static_rolemaster_phrase.h"
#include "egs_sheets/egs_sheets.h"

#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "player_manager/player.h"


using namespace std;
using namespace NLMISC;

extern CVariable<bool> EGSLight;

CNamedItems * CNamedItems::_Instance = NULL;

//-----------------------------------------------------------------------------
CNamedItems::CNamedItems()
{
	if (EGSLight)
		return;

	loadNamedItemsFromFile("named_items.txt");
}

//-----------------------------------------------------------------------------
void CNamedItems::loadNamedItemsFromFile(const std::string & fileName)
{
	CHashMap<std::string, CGameItemPtr>::iterator it;
	for (it = _NamedItems.begin(); it != _NamedItems.end(); ++it)
	{
		GameItemManager.destroyItem((*it).second);
	}
	_NamedItems.clear();

	string path;
	try
	{
		path = CPath::lookup(fileName);
	}
	catch (const Exception &)
	{
		nlwarning("<NAMED_ITEMS> file '%s' was not found", fileName.c_str());
		return;
	}

	static CPersistentDataRecord	pdr;
	pdr.clear();
	pdr.readFromTxtFile(path.c_str());
	CInventoryPtr inv = loadFromPdr(pdr);
	if (inv == NULL)
	{
		nlwarning("<NAMED_ITEMS> error while loading items from the PDR");
		return;
	}

	const uint size = inv->getSlotCount();
	nlinfo("loading '%u' named items", size);
	for (uint i = 0; inv->getFreeSlotCount() != inv->getSlotCount() && i < size; ++i)
	{
		if (inv->getItem(i) == NULL)
			continue;
		CGameItemPtr item = inv->removeItem(i);
		if (item != NULL)
		{
			if (item->getSheetId() == CSheetId::Unknown)
			{
				nlwarning("<NAMED_ITEMS> item '%u' has invalid sheet id", i);
				GameItemManager.destroyItem(item);
				continue;
			}
			if (item->getPhraseId().empty())
			{
				nlwarning("<NAMED_ITEMS> item '%u' has no name", i);
				GameItemManager.destroyItem(item);
				continue;
			}
			if (_NamedItems.find(item->getPhraseId()) != _NamedItems.end())
			{
				nlwarning("<NAMED_ITEMS> item '%u', name '%s' exists more than once", i, item->getPhraseId().c_str());
				GameItemManager.destroyItem(item);
				continue;
			}

			// Yoyo: force this item to work with the new form requirement system.
			// BUT: do it only if _UseNewSystemRequirement==false (if LDs put true, we suppose that the named item has special req value)
			if(item->getUseNewSystemRequirement()==false)
				item->computeRequirementFromForm();

			nldebug("<NAMED_ITEMS> creating named item '%s'",item->getPhraseId().c_str());
			_NamedItems.insert(make_pair(item->getPhraseId(), item));
		}
	}
}

//-----------------------------------------------------------------------------
CGameItemPtr CNamedItems::createNamedItem(const std::string & name, uint32 quantity)
{
	if (quantity == 0)
		return NULL;
	CGameItemPtr itemSrc = getNamedItemRef(name);
	if( itemSrc == NULL ) 
		return NULL;
	CGameItemPtr item = itemSrc->getItemCopy();
	return item;
}

//-----------------------------------------------------------------------------
CGameItemPtr CNamedItems::getNamedItemRef(const std::string &name)
{
	CHashMap<std::string, CGameItemPtr>::iterator it = _NamedItems.find(name);
	if (it == _NamedItems.end())
	{
		nlwarning("<NAMED_ITEMS> cant find item '%s'", name.c_str());
		return NULL;
	}
	CGameItemPtr itemSrc = (*it).second;
	if (itemSrc == NULL)
	{
		nlwarning("<NAMED_ITEMS> item '%s' is NULL", name.c_str());
		return NULL;
	}
	return itemSrc;
}

//-----------------------------------------------------------------------------
CInventoryPtr CNamedItems::loadFromPdr(CPersistentDataRecord & pdr)
{
	CInventoryPtr namedItemsInv = new CAutoResizeInventory;
	namedItemsInv->apply(pdr, NULL);
	return namedItemsInv;
}

//-----------------------------------------------------------------------------
void CNamedItems::saveIntoPdr(CPersistentDataRecord & pdr, CInventoryPtr namedItemsInv)
{
	if (namedItemsInv == NULL)
	{
		nlwarning("<NAMEDITEMS> NULL root inventory given to PDR system");
		return;
	}
	namedItemsInv->store(pdr);
}

//-----------------------------------------------------------------------------
NLMISC_COMMAND(reloadNamedItems, "reload named items", "[<filename>]")
{
	if (args.size() > 1)
		return false;

	if (args.size() == 1)
	{
		CNamedItems::getInstance().loadNamedItemsFromFile(args[0]);
	}
	else
	{
		CNamedItems::getInstance().loadNamedItemsFromFile("named_items.txt");
	}

	return true;
}

