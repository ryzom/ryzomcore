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

// client
#include "../sheet_manager.h"
#include "nel/gui/interface_expr.h"
#include "dbctrl_sheet.h"
#include "nel/gui/ctrl_sheet_selection.h"
#include "dbgroup_list_sheet.h"
#include "interface_manager.h"
#include "sbrick_manager.h"
#include "../misc.h"
#include "inventory_manager.h"
// game share
#include "game_share/item_family.h"
#include "game_share/seeds.h"
#include "game_share/slot_equipment.h"
#include "game_share/trade_slot_type.h"
#include "game_share/inventories.h"
// nel
#include "nel/misc/algo.h"


using namespace NLMISC;


/** Here we define some user functions related to items in the game
  *
  * "getSelectedItemPrice" : Get the price of the current selected item (should come from bot inventory)
  * ====
  * "getDraggedSheet"      : Get ptr object on the current dragged sheet
  * "getSelectedSheet"     : Get ptr object on the current selected sheet
  * "getMenuItemSheet"     : Get ptr object on the last sheet that trigerred menu.
  * ====
  * "getSheetID"               : From a pointer on a sheet, get its sheetID
  * "getSheetSelectionGroup"   : From a pointer on a sheet, get its selection group name
  * "getSheetName"             : From a pointer on a sheet, or a sheet ID, get its name
  * "getSheetIndex"            : From a pointer on a sheet, get its index in the parent sheet list
  * "getSheetQuantity"         : From a pointer on a sheet, retrieve its quantity
  * "getSheetQuality"          : From a pointer on a sheet, retrieve its quality
  * ====
  * "isStackable"              : From a pointer on a sheet, get its index in the parent sheet list
  * "isTeleport"               : From a pointer on a sheet or a sheet ID, test if an item is a teleport item
  * "isPlainItem               : From a pointer on a sheet or a sheet ID, test if an item is a plain item (not a teleport, not a brick, not a pact)
  * "isBrick	               : From a sheet ID, test if an item is a brick
  * "isPact		               : From a sheet ID, test if an item is a pact
  * "isRangeWeapon"            : From a pointer on a sheet or a sheet ID, test if an item is a ran,ged weapon
  * "isRoomLeftFor"            : From a sheet ID, test if there's room for it in the inventory
  * ===
  * "getSelectionGroupNameFromId" : Get the name of a selection group from its id
  * "getSelectionGroupIdFromName" : Get the id of a selection group from its name
  * ===
  * "getItemsWeight(basePath, firstItem, numItem)" : Get the sum of all item's weight at the given db path
  * "getItemBranchsWeight(baseBranch, firstBranch, numBranch)" : Get the sum of all items located in several branchs
  * "getSeedsWeight()" : Returns the weight of the seeds owned by the player
  * "getArmoursWeight()" : Returns the weight of all armours pieces worn by the player
  * "getInventoryWeight()" : Returns the weight of the whole inventory, not including beasts'inventory of course
  * "getSheathWeight()" : Returns the weight of the sheaths
  * ===
  * "getBranchSheetCategory() : return the types of items that are contained in a branch (with the enum CDBCtrlSheet::TSheetCategory
  * ===
  * "copySheet" : param 0 = pointer on a sheet, param 1 = dbAdress. This copy characteristics of a sheet (quantity, quality, sheet id) at the given database adress
  *
  * "canAfford(price, ls, ms, bs, vbs)" : test if the given seeds can afford the given price
  */


///////////////////////////////////////
// Test if player can afford an item //
///////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(canAfford)
{
	if (args.size() != 5) return false;
	for(uint k = 0; k < 5; ++k)
	{
		if (!args[k].toInteger()) return false;
	}
	CSeeds rhs((CSeeds::TUInt) args[1].getInteger(), (CSeeds::TUInt) args[2].getInteger(), (CSeeds::TUInt) args[3].getInteger(), (CSeeds::TUInt) args[4].getInteger());
	result.setBool(rhs.canTradeSubtract((CSeeds::TBigUInt) args[0].getInteger()));
	return true;
}
REGISTER_INTERFACE_USER_FCT("canAfford", canAfford)


/////////////////////////////////////////////////////////////////////////////////
// Get the price of the current selected item (should come from bot inventory) //
/////////////////////////////////////////////////////////////////////////////////

static DECLARE_INTERFACE_USER_FCT(getSelectedItemPrice)
{
	CDBCtrlSheet *ctrlSheet = CDBCtrlSheet::getCurrSelection();
	if (!ctrlSheet)
	{
		result.setInteger(-1);
		return true;
	}
	if (ctrlSheet->getRootBranch())
	{
		CCDBNodeBranch *branch = ctrlSheet->getRootBranch();
		if (!branch)
		{
			 nlwarning("<getSelectedItemPrice> Can't get price entry in the database");
			 return false;
		}
		ICDBNode::CTextId textId("PRICE");
		uint32 cost = (uint32) branch->getProp(textId);
		result.setInteger((sint64) cost);
		return true;
	}
	return false;
}
REGISTER_INTERFACE_USER_FCT("getSelectedItemPrice", getSelectedItemPrice)


/////////////////////////////////////////////////
// Get ptr object on the current dragged sheet //
/////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getDraggedSheet)
{
	result.setUserType(new CDBCtrlSheetPtrUserType( dynamic_cast< CDBCtrlSheet* >( CDBCtrlSheet::getDraggedSheet() ) ));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getDraggedSheet", getDraggedSheet)


//////////////////////////////////////////////////
// Get ptr object on the current selected sheet //
//////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSelectedSheet)
{
	result.setUserType(new CDBCtrlSheetPtrUserType(CDBCtrlSheet::getCurrSelection()));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSelectedSheet", getSelectedSheet)

//////////////////////////////////////////////
// Get a sheet from its id in the interface //
//////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSheetFromId)
{
	if (args.size() != 1 || !args[0].toString()) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CDBCtrlSheet *sheet = dynamic_cast<CDBCtrlSheet *>(CWidgetManager::getInstance()->getElementFromId(args[0].getString()));
	if (!sheet)
	{
		nlwarning("Sheet %s not found", args[0].getString().c_str());
		return false;
	}
	result.setUserType(new CDBCtrlSheetPtrUserType(sheet));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSheetFromId", getSheetFromId)



//////////////////////////////////////////////////////////
// Get ptr object on the last sheet that trigerred menu.//
//////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getMenuItemSheet)
{
	result.setUserType(new CDBCtrlSheetPtrUserType(CDBCtrlSheet::getCurrSelSheet()));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getMenuItemSheet", getMenuItemSheet)


//=========================================================================================
/** Tool function : convert an interface expr value to a pointer on a CDBCtrlSheet
  */
static CDBCtrlSheet *asCtrlSheet(CInterfaceExprValue &arg)
{
	if (arg.getType() != CInterfaceExprValue::UserType)
	{
		nlwarning("<asCtrlSheet> Arg 0 has wrong type!");
		return NULL;
	}

	CDBCtrlSheetPtrUserType *sheetPtr = dynamic_cast<CDBCtrlSheetPtrUserType *>(arg.getUserType());
	if (!sheetPtr)
	{
		nlwarning("<asCtrlSheet> Arg 0 has wrong type!");
		return NULL;
	}
	return sheetPtr->Sheet;
}

//=========================================================================================
/** Tool function : convert a list of arguments to a pointer on a CDBCtrlSheet (check that the list as one single element of the right type)
  */
static CDBCtrlSheet *asCtrlSheet(CInterfaceExpr::TArgList &args)
{

	if (args.size() != 1)
	{
		nlwarning("<asCtrlSheet> bad number of args");
		return NULL;
	}
	return asCtrlSheet(args[0]);
}

//=========================================================================================
/** Tool function : convert a list of arguments to a pointer on a CItemSheet (check that the list as one single element of the right type)
  */
static const CItemSheet *getItemSheet(CInterfaceExpr::TArgList &args)
{
	if (args.size() != 1)
	{
		return NULL;
	}
	// Check if first arg is a sheet ID
	if (args[0].toInteger())
	{
		if (args[0].getInteger() == 0)
		{
			return NULL;
		}
		CSheetId si((uint32) args[0].getInteger());
		CEntitySheet *entitySheet = SheetMngr.get(si);
		if (!entitySheet || entitySheet->Type != CEntitySheet::ITEM)
		{
			return NULL;
		}
		return static_cast<CItemSheet *>(entitySheet);
	}
	else
	{
		CDBCtrlSheet *ctrlSheet = asCtrlSheet(args);
		if (!ctrlSheet)
		{
			return NULL;
		}
		return ctrlSheet->asItemSheet();
	}
}


/////////////////////////////////////////////////////
// From a pointer on a sheet,  get its sheetID     //
/////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSheetID)
{
	CDBCtrlSheet *ctrlSheet = asCtrlSheet(args);
	if (!ctrlSheet) return false;
	result.setInteger(ctrlSheet->getSheetId());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSheetID", getSheetID)

//////////////////////////////////////////////////////////////
// From a pointer on a sheet, get its selection group name //
/////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSheetSelectionGroup)
{
	CDBCtrlSheet *ctrlSheet = asCtrlSheet(args);
	if (!ctrlSheet) return false;
	result.setString(ctrlSheet->getSelectionGroupAsString());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSheetSelectionGroup", getSheetSelectionGroup)

//////////////////////////////////////////////////////
// From a pointer on a sheet, retrieve its quantity //
//////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSheetQuantity)
{
	CDBCtrlSheet *ctrlSheet = asCtrlSheet(args);
	if (!ctrlSheet) return false;
	result.setInteger(ctrlSheet->getQuantity());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSheetQuantity", getSheetQuantity)

/////////////////////////////////////////////////////
// From a pointer on a sheet, retrieve its quality //
/////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSheetQuality)
{
	CDBCtrlSheet *ctrlSheet = asCtrlSheet(args);
	if (!ctrlSheet) return false;
	result.setInteger(ctrlSheet->getQuality());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSheetQuality", getSheetQuality)


////////////////////////////////////////////////////////////
// From a pointer on a sheet, or a sheet ID, get its name //
////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSheetName)
{
	CDBCtrlSheet	*ctrlSheet= asCtrlSheet(args);
	// if from ctrlSheet, then take the correct ACTUAL name (ie from NAMEID if not 0)
	if(ctrlSheet)
	{
		result.setUCString(ctrlSheet->getItemActualName());
		return true;
	}
	// Standard (but less accurate) way
	else
	{
		const CItemSheet *itemSheet = getItemSheet(args);
		ucstring tmp;
		if (itemSheet != NULL) tmp = STRING_MANAGER::CStringManagerClient::getItemLocalizedName(itemSheet->Id);
		result.setUCString(tmp);
		return true;
	}
}
REGISTER_INTERFACE_USER_FCT("getSheetName", getSheetName)

///////////////////////////////////////////////////////////////////////

static DECLARE_INTERFACE_USER_FCT(getItemTranslatedName)
{
	if (args.size() != 1 || !args[0].toString())
	{
		nlwarning("<getItemTranslatedName> Bad params.");
		return false;
	}

	std::string in = args[0].getString();
	CSheetId sheet(in);
	if (sheet == CSheetId::Unknown)
	{
		nlwarning("<getItemTranslatedName> sheet not known (%s).", in.c_str());
		return false;
	}

	ucstring tmp;
	tmp = STRING_MANAGER::CStringManagerClient::getItemLocalizedName(sheet);
	result.setUCString(tmp);
	return true;
}
REGISTER_INTERFACE_USER_FCT("getItemTranslatedName", getItemTranslatedName)


// From a pointer on a sheet, get its index in the parent sheet list //
///////////////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSheetIndex)
{
	CDBCtrlSheet *ctrlSheet = asCtrlSheet(args);
	if (!ctrlSheet)
	{
		result.setInteger(-1);
		return true;
	}
	IListSheetBase *gls = ctrlSheet->getListSheetParent();
	if (!gls)
	{
		nlwarning("<getSheetIndex> : sheet is not inserted in a sheet list");
		return false;
	}
	result.setInteger(gls->getIndexOf(CDBCtrlSheet::getCurrSelection()));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSheetIndex", getSheetIndex)

////////////////////////////////////////////////////////////////////////////
// From a pointer on a sheet or a sheet ID, tells if an item is stackable //
////////////////////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(userFctIsStackable)
{
	const CItemSheet *itemSheet = getItemSheet(args);
	if (itemSheet != NULL)
	{
		result.setBool(itemSheet->Stackable > 1);
	}
	else
	{
		result.setBool(false);
	}
	return true;
}
REGISTER_INTERFACE_USER_FCT("isStackable", userFctIsStackable)


/////////////////////////////////////////////////////////////////////////////////
// From a pointer on a sheet or a sheet ID, test if an item is a teleport item //
/////////////////////////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(isTeleport)
{
	const CItemSheet *itemSheet = getItemSheet(args);
	if (itemSheet)
	{
		result.setBool(itemSheet->Family == ITEMFAMILY::TELEPORT);
	}
	else
	{
		result.setBool(false);
	}
	return true;
}
REGISTER_INTERFACE_USER_FCT("isTeleport", isTeleport);

////////////////////////////////////////////////////////////////////////////////////////
// From a pointer on a sheet or a sheet ID, test if an item is a ticket for an animal //
////////////////////////////////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(isAnimalTicket)
{
	const CItemSheet *itemSheet = getItemSheet(args);
	if (itemSheet)
	{
		result.setBool(itemSheet->Family == ITEMFAMILY::PET_ANIMAL_TICKET);
	}
	else
	{
		result.setBool(false);
	}
	return true;
}
REGISTER_INTERFACE_USER_FCT("isAnimalTicket", isAnimalTicket);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// From a pointer on a sheet or a sheet ID, test if an item is a plain item (not a teleport, not a brick, not a pact) //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DECLARE_INTERFACE_USER_FCT(isPlainItem)
{
	const CItemSheet *itemSheet = getItemSheet(args);
	if (itemSheet)
	{
		result.setBool(itemSheet->Family != ITEMFAMILY::TELEPORT);
	}
	else
	{
		result.setBool(false);
	}
	return true;
}
REGISTER_INTERFACE_USER_FCT("isPlainItem", isPlainItem);

//////////////////////////////////////////////////////////////
// From a pointer on a sheet ID, test if an item is a brick //
//////////////////////////////////////////////////////////////

static inline bool isBrick(uint32 sheetID)
{
	CSBrickManager *pBM = CSBrickManager::getInstance();
	CSheetId sheetId(sheetID);
	CSBrickSheet *pBR = pBM->getBrick (sheetId);
	return pBR != NULL;
}

static DECLARE_INTERFACE_USER_FCT(isBrickUserFct)
{
	if (args.size() != 1 || !args[0].toInteger())
	{
		nlwarning("<isBrick> Bad params.");
		return false;
	}
	result.setBool(isBrick((uint32) args[0].getInteger()));
	return true;
}
REGISTER_INTERFACE_USER_FCT("isBrick", isBrickUserFct);

////////////////////////////////////////////////
// From a sheet ID, test if an item is a pact //
////////////////////////////////////////////////
static inline bool isPact(uint32 sheetId)
{
	const CEntitySheet *sheet = SheetMngr.get(CSheetId(sheetId));
	if (!sheet) return false;
	return sheet->Type == CEntitySheet::PACT;
}
static DECLARE_INTERFACE_USER_FCT(isPactUserFct)
{
	if (args.size() != 1 || !args[0].toInteger())
	{
		nlwarning("<isPact> Bad params.");
		return false;
	}
	result.setBool(isPact((uint32) args[0].getInteger()));
	return true;
}
REGISTER_INTERFACE_USER_FCT("isPact", isPactUserFct);

///////////////////////////////////////////////////////////////////////////////////////
// From a pointer on a sheet or a sheet ID, test if the item is a ranged weapon      //
///////////////////////////////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(userFctIsRangeWeapon)
{
	const CItemSheet *itemSheet = getItemSheet(args);
	// Test if arg1 sheetid is a range weapon
	if (itemSheet)
	{
		result.setBool(itemSheet->Family == ITEMFAMILY::RANGE_WEAPON);
	}
	else
	{
		result.setBool(false);
	}
	return true;
}
REGISTER_INTERFACE_USER_FCT("isRangeWeapon", userFctIsRangeWeapon)

////////////////////////////////////////////////////////////////////
// From a sheet ID, test if there's room for it in the inventory  //
////////////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(isRoomLeftFor)
{
	if (args.size() != 1)
	{
		nlwarning("<isRoomLeftFor> 1 arg expected");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<isRoomLeftFor> Can't convert arg 0 to integer");
		return false;
	}
	if (args[0].getInteger() == 0)
	{
		// there room for an empty sheet (no item)
		result.setBool(true);
		return true;
	}
	CEntitySheet *es = SheetMngr.get(CSheetId((uint32) args[0].getInteger()));
	if (!es || es->Type != CEntitySheet::ITEM)
	{
		result.setBool(false);
		return true;
	}
	CItemSheet *is = (CItemSheet *) es;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// see if there is room in the bags
	// get the number of bags
	std::string nbBagPath = CWidgetManager::getInstance()->getParser()->getDefine("bag_nb");
	CCDBNodeLeaf *prop = NLGUI::CDBManager::getInstance()->getDbProp(nbBagPath);
	if (!prop) return false;
	uint nbItemInBags = 8 * (uint) prop->getValue32(); // there are 8 item per bag
	std::string bagsPath = CWidgetManager::getInstance()->getParser()->getDefine("bag");
	if (bagsPath.empty()) return false;
	uint k = 0;
	for(k = 0; k < nbItemInBags; ++k)
	{
		std::string bagPath = bagsPath + ":" + toString(k) + ":SHEET";
		CCDBNodeLeaf *bagProp = NLGUI::CDBManager::getInstance()->getDbProp(bagPath);
		if (!bagProp) return false;
		if (bagProp->getValue32() == 0)
		{
			// there's room for an item
			result.setBool(true);
			return true;
		}
	}

	struct CArmourInfo
	{
		const char					*DefineName;
		SLOTTYPE::TSlotType          SlotType;
	};

	// looks in the armors
	static const CArmourInfo armourInfos[] =
	{
		{"head",  SLOTTYPE::HEAD },
		{"chest", SLOTTYPE::CHEST},
		{"arms",  SLOTTYPE::ARMS},
		{"hands", SLOTTYPE::HANDS},
		{"legs",  SLOTTYPE::LEGS},
		{"feet",  SLOTTYPE::FEET}
	};
	uint numArmorSlots =  sizeof(armourInfos) / sizeof(armourInfos[0]);

	for(k = 0; k < numArmorSlots; ++k)
	{
		std::string dbPath = CWidgetManager::getInstance()->getParser()->getDefine(armourInfos[k].DefineName);
		if (dbPath.empty()) return false;
		CCDBNodeLeaf *armorProp = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + ":SHEET");
		if (!armorProp) return false;
		if (armorProp->getValue32() == 0)
		{
			// check if the slot is the asked type
			if( is->hasSlot(armourInfos[k].SlotType) )
			{
				// there's room for an item
				result.setBool(true);
				return true;
			}
		}
	}
	result.setBool(false);
	return true;
}
REGISTER_INTERFACE_USER_FCT("isRoomLeftFor", isRoomLeftFor)

////////////////////////////////////////////////////
// get the name of a selection group from its id  //
////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSelectionGroupNameFromId)
{
	if (args.size() != 1) return false;
	if (!args[0].toInteger()) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CCtrlSheetSelection &css = CWidgetManager::getInstance()->getParser()->getCtrlSheetSelection();
	CSheetSelectionGroup *csg = css.getGroup((uint) args[0].getInteger());
	if (csg) result.setString(csg->getName());
	else result.setString("");
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSelectionGroupNameFromId", getSelectionGroupNameFromId)

////////////////////////////////////////////////////
// get the id of a selection group from its name  //
////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(getSelectionGroupIdFromName)
{
	if (args.size() != 1) return false;
	if (!args[0].toString()) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CCtrlSheetSelection &css = CWidgetManager::getInstance()->getParser()->getCtrlSheetSelection();
	result.setInteger(css.getGroupIndex(args[0].getString().c_str()));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSelectionGroupIdFromName", getSelectionGroupIdFromName)

////////////////////////////////////////////////////////////////////
// param 0 = pointer on a sheet, param 1 = pointer on dest sheet. //
////////////////////////////////////////////////////////////////////
static DECLARE_INTERFACE_USER_FCT(copySheet)
{
	if (args.size() != 2)
	{
		nlwarning("<copySheet> 2 args required.");
		return false;
	}
	CDBCtrlSheet *srcSheet = asCtrlSheet(args[0]);
	if (!srcSheet) return false;
	CDBCtrlSheet *destSheet = asCtrlSheet(args[1]);
	if (!destSheet) return false;
	srcSheet->copyAspect(destSheet);
	return true;
}
REGISTER_INTERFACE_USER_FCT("copySheet", copySheet)




static double getItemsWeight(CCDBNodeBranch *branch, uint16 startItemIndex, uint16 numItems)
{
	double totalWeight = 0;
	//
	uint16 lastIndexItem = std::min((uint16) (startItemIndex + numItems), (uint16) branch->getNbNodes());
	for (uint16 currItem = startItemIndex; currItem < lastIndexItem; ++ currItem)
	{
		ICDBNode *node = branch->getNode(currItem);
		if (node)
		{
			// Must Mul Weight by quantity
			CCDBNodeLeaf *sheetNode = dynamic_cast<CCDBNodeLeaf *>(node->getNode(ICDBNode::CTextId("SHEET")));
			CCDBNodeLeaf *quantityNode = dynamic_cast<CCDBNodeLeaf *>(node->getNode(ICDBNode::CTextId("QUANTITY")));
			CCDBNodeLeaf *wgtNode = dynamic_cast<CCDBNodeLeaf *>(node->getNode(ICDBNode::CTextId("WEIGHT")));
			// must not be empty
			if (wgtNode && sheetNode && quantityNode && sheetNode->getValue32()!=0)
			{
				totalWeight += std::max((sint32)1, quantityNode->getValue32()) * wgtNode->getValue32();
			}
		}
	}
	// Weight in gramme in the database
	// do the div at end for double precision concerns
	return totalWeight / DB_WEIGHT_SCALE;
}


/** Get weights of item in a from its name
  */
static double getItemsWeight(const std::string &basePath, uint16 startItemIndex, uint16 numItems)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CCDBNodeBranch *branch = NLGUI::CDBManager::getInstance()->getDbBranch(basePath);
	if (!branch)
	{
		nlwarning("<getItemsWeight> Branch is NULL");
		return 0;
	}
	return getItemsWeight(branch, startItemIndex, numItems);
}

////////////////////////////////////////////////////////////////////////////////////////
//   getItemsWeight(basePath)" : Get the sum of all item weight in the given db path. //
////////////////////////////////////////////////////////////////////////////////////////

static DECLARE_INTERFACE_USER_FCT(getItemsWeightUserFct)
{
	if (args.size() != 3 || !args[0].toString() || !args[1].toInteger() || !args[2].toInteger())
	{
		nlwarning("<getItemWeight> 3 arguments expected : (dbPath, startItemIndex, numItems)");
		return false;
	}
	const std::string &basePath = args[0].getString();
	uint16 startItemIndex = (uint16) args[1].getInteger();
	uint16 numItems = (uint16) args[2].getInteger();
	result.setDouble(getItemsWeight(basePath, startItemIndex, numItems));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getItemsWeight", getItemsWeightUserFct)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getItemBranchsWeight(baseBranch, firstBranch, numBranch) : Get the sum of all items located in several branchs //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static double getItemBranchsWeight(const std::string &basePath, uint startBranchIndex, uint numBranchs)
{
	//
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	double totalWeight = 0;
	// look for all branchs
	for(uint branchIndex = startBranchIndex; branchIndex < startBranchIndex + numBranchs; ++branchIndex)
	{
		std::string branchName = toString("%s:%d", basePath.c_str(), branchIndex);
		CCDBNodeBranch *currBranch = NLGUI::CDBManager::getInstance()->getDbBranch(branchName);
		if (!currBranch)
		{
			nlwarning("<getItemBranchsWeight> can't get branch %s:%d, or this is a leaf", basePath.c_str(), branchIndex);
		}
		else
		{
			totalWeight += getItemsWeight(branchName, 0, currBranch->getNbNodes());
		}
	}
	return totalWeight;
}

static DECLARE_INTERFACE_USER_FCT(getItemBranchsWeightUserFct)
{
	if (args.size() != 3 || !args[0].toString() || !args[1].toInteger() || !args[2].toInteger())
	{
		nlwarning("<getItemBranchsWeight> 3 arguments expected : (dbBranchPath, startBranchIndex, numBranch)");
		return false;
	}
	//
	const std::string &basePath = args[0].getString();
	uint startBranchIndex = (uint) args[1].getInteger();
	uint numBranchs = (uint) args[2].getInteger();
	result.setDouble(getItemBranchsWeight(basePath, startBranchIndex, numBranchs));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getItemBranchsWeight", getItemBranchsWeightUserFct)

//////////////////////////////////////////////////////////////////////////////
// "getSeedsWeight()" : Return the weight of the seeds owned by the player  //
//////////////////////////////////////////////////////////////////////////////

static double getSeedsWeight()
{
	CSeeds seeds;
	getSeedsFromDB(seeds);
	return seeds.getWeight();
}

static DECLARE_INTERFACE_USER_FCT(getSeedsWeightUserFct)
{
	result.setDouble(getSeedsWeight());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSeedsWeight", getSeedsWeightUserFct)



// get weight of some equipment pieces (armours or jewelry ..)
static double getWeightOfEquipmentPieces(const SLOT_EQUIPMENT::TSlotEquipment * /* slots */, uint /* numSlots */)
{
	// Yoyo: deprecated method (bad DB/Weight).
	/*
	CInterfaceManager *im = CInterfaceManager::getInstance();
	double totalWeight = 0;
	for(uint k = 0; k < numSlots; ++k)
	{
		std::string dbPath = NLMISC::toString("LOCAL:INVENTORY:8:%d:SHEET", (int) slots[k]);
		uint32 sheetID = NLGUI::CDBManager::getInstance()->getDbProp(dbPath)->getValue32();
		if (sheetID)
		{
			CEntitySheet *es = SheetMngr.get(CSheetId(sheetID));
			if (es && es->type() == CEntitySheet::ITEM)
			{
				totalWeight += ((CItemSheet *) es)->Weight;
			}
		}
	}
	return totalWeight;
	*/
	return 0;
}

//////////////////////////
// "getArmoursWeight()" //
//////////////////////////
static double getArmoursWeight()
{
	static const SLOT_EQUIPMENT::TSlotEquipment armourPieces[] =
	{
		SLOT_EQUIPMENT::HEAD,
		SLOT_EQUIPMENT::CHEST,
		SLOT_EQUIPMENT::ARMS,
		SLOT_EQUIPMENT::HANDS,
		SLOT_EQUIPMENT::LEGS,
		SLOT_EQUIPMENT::FEET
	};
	return getWeightOfEquipmentPieces(armourPieces, sizeof(armourPieces) / sizeof(armourPieces[0]));
}


static DECLARE_INTERFACE_USER_FCT(getArmoursWeightUserFct)
{
	result.setDouble(getArmoursWeight());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getArmoursWeight", getArmoursWeightUserFct)

/////////////////////
// "getJewelry()"  //
/////////////////////
static double getJewelryWeight()
{
	static const SLOT_EQUIPMENT::TSlotEquipment jewelryPieces[] =
	{
		SLOT_EQUIPMENT::HEADDRESS,
		SLOT_EQUIPMENT::EARL,
		SLOT_EQUIPMENT::EARR,
		SLOT_EQUIPMENT::NECKLACE,
		SLOT_EQUIPMENT::WRISTL,
		SLOT_EQUIPMENT::WRISTR,
		SLOT_EQUIPMENT::FINGERL,
		SLOT_EQUIPMENT::FINGERR,
		SLOT_EQUIPMENT::ANKLEL,
		SLOT_EQUIPMENT::ANKLER,
	};
	return getWeightOfEquipmentPieces(jewelryPieces, sizeof(jewelryPieces) / sizeof(jewelryPieces[0]));
}

static DECLARE_INTERFACE_USER_FCT(getJewelryWeightUserFct)
{
	result.setDouble(getJewelryWeight());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getJewelryWeight", getJewelryWeightUserFct)

/////////////////////////////////////////////////////////////
// "getSheathWeight()" : Returns the weight of the sheaths //
/////////////////////////////////////////////////////////////
/*
// Get weight of all Sheaths
static double getSheathsWeight()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	uint numSheaths = (uint) NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:NB_SHEATH")->getValue32();
	return getItemBranchsWeight("LOCAL:INVENTORY", 1, numSheaths);
}

static DECLARE_INTERFACE_USER_FCT(getSheathsWeightUserFct)
{
	result.setDouble(getSheathsWeight());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getSheathsWeight", getSheathsWeightUserFct)
*/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// "getInventoryWeight()" : Returns the weight of the whole inventory, not including beasts'inventory of course  //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
// Get weight of all bags
static double getBagsWeight()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	uint numBags = (uint) NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:NB_BAG")->getValue32();
	return getItemsWeight("LOCAL:INVENTORY:9", 0, 8 * numBags);
}


static DECLARE_INTERFACE_USER_FCT(getInventoryWeightUserFct)
{
	result.setDouble(getJewelryWeight() + getArmoursWeight() + getSeedsWeight() + getBagsWeight() + getSheathsWeight());
	return true;
}
REGISTER_INTERFACE_USER_FCT("getInventoryWeight", getInventoryWeightUserFct)
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// "getBranchSheetCategory() : return the types of items that are contained in a branch (with the enum CDBCtrlSheet::TSheetCategory //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DECLARE_INTERFACE_USER_FCT(getBranchSheetCategory)
{
	// the first parameter gives the address of the branch
	if (args.size() != 3)
	{
		nlwarning("bad number of args");
		return false;
	}
	if (!(args[0].toString() && args[1].toInteger() && args[2].toInteger()))
	{
		nlwarning("bad args format");
		return false;
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CCDBNodeBranch *branch = NLGUI::CDBManager::getInstance()->getDbBranch(args[0].getString());
	if (!branch)
	{
		nlwarning("<getBranchSheetCategory> Branch is NULL");
		return 0;
	}
	//
	CDBCtrlSheet::TSheetCategory sheetType = CDBCtrlSheet::DontKnow;
	CDBCtrlSheet::TSheetCategory newSheetType = CDBCtrlSheet::DontKnow;
	for (uint index = (uint) args[1].getInteger(); index < (uint) args[1].getInteger() + (uint) args[2].getInteger(); ++index)
	{
		ICDBNode *node = branch->getNode(index);
		if (node)
		{
			CCDBNodeLeaf *sheetNode = dynamic_cast<CCDBNodeLeaf *>(node->getNode(ICDBNode::CTextId("SHEET")));
			CCDBNodeLeaf *slotTypeNode = dynamic_cast<CCDBNodeLeaf *>(node->getNode(ICDBNode::CTextId("SLOT_TYPE")));
			if (sheetNode)
			{
				uint32 sheetID = (uint32) sheetNode->getValue32();
				if (sheetID == 0) continue;
				if (slotTypeNode)
				{
					if (slotTypeNode->getValue32() == TRADE_SLOT_TYPE::Skill)
					{
						newSheetType = CDBCtrlSheet::Skill;
					}
					else
					{
						if (isBrick(sheetID))
						{
							// TODO_BRICK ???
							//newSheetType = CDBCtrlSheet::Brick;
						}
						else if (isPact(sheetID))
						{
							newSheetType = CDBCtrlSheet::Pact;
						}
						else
						{
							newSheetType = CDBCtrlSheet::Item;
						}
					}
				}
				else
				{
					if (isBrick(sheetID))
					{
						// TODO_BRICK ???
						//newSheetType = CDBCtrlSheet::Brick;
					}
					else if (isPact(sheetID))
					{
						newSheetType = CDBCtrlSheet::Pact;
					}
					else
					{
						newSheetType = CDBCtrlSheet::Item;
					}
				}
			}
			if (index == (uint) args[1].getInteger())
			{
				sheetType = newSheetType;
			}
			else
			{
				if (newSheetType != sheetType)
				{
					result.setInteger(CDBCtrlSheet::DontKnow);
					return true;
				}
			}
		}
	}
	result.setInteger(sheetType);
	return true;
}
REGISTER_INTERFACE_USER_FCT("getBranchSheetCategory", getBranchSheetCategory)


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getItemsBulkUserFct)
{
	if (args.size() != 3 || !args[0].toString() || !args[1].toInteger() || !args[2].toInteger())
	{
		nlwarning("<getItemBulk> 3 arguments expected : (dbPath, startItemIndex, numItems)");
		return false;
	}
	const std::string &basePath = args[0].getString();
	uint16 startItemIndex = (uint16) args[1].getInteger();
	uint16 numItems = (uint16) args[2].getInteger();
	result.setDouble(CInventoryManager::getBranchBulk(basePath, startItemIndex, numItems));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getItemsBulk", getItemsBulkUserFct)

// ***************************************************************************
// return bulk/bulkmax
static DECLARE_INTERFACE_USER_FCT(getBulkStrUserFct)
{
	if (args.size() != 2 || !args[0].toString() || !args[1].toInteger())
	{
		nlwarning("<getBulkStr> 2 arguments expected : (dbPath, maxVal)");
		return false;
	}

	std::string	dbBranch= args[0].getString();

	// Get the sum of the bulk for this db branch
	sint32	val;
	// get max items in the DB
	val= (sint32)CInventoryManager::getBranchBulk(dbBranch, 0, 10000);

	// Get the Max value
	sint32	maxVal= (sint32)args[1].getInteger();

	// Replace in the formated text.
	std::string		str= toString("%d/%d", val, maxVal);
	result.setString(str);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getBulkStr", getBulkStrUserFct)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getInvSlotCounts)
{
	if (args.size() != 1 || !args[0].toString())
	{
		nlwarning("<getInvSlotCounts> 1 argument expected : (dbPath)");
		return false;
	}

	// Get the counts
	std::string	dbBranch= args[0].getString();
	uint nbUsedSlots = 0, nbMaxSlots = 0;
	CInventoryManager::getBranchSlotCounts(dbBranch, nbUsedSlots, nbMaxSlots);

	// Replace in the formated text
	std::string	str = toString("%u/%u", nbUsedSlots, nbMaxSlots);
	result.setString(str);
	return true;
}
REGISTER_INTERFACE_USER_FCT("getInvSlotCounts", getInvSlotCounts)
