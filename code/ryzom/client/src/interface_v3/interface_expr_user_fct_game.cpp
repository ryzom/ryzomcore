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
// Interface
#include "nel/gui/interface_expr.h"
#include "interface_manager.h"
#include "nel/gui/interface_element.h"
#include "chat_window.h"
#include "nel/gui/group_container.h"
// client
#include "../client_chat_manager.h"
#include "../sheet_manager.h"
#include "../string_manager_client.h"
#include "../light_cycle_manager.h"
#include "../weather.h"
#include "../actions_client.h"
#include "../connection.h"
#include "dbctrl_sheet.h"
#include "../net_manager.h"
#include "../user_entity.h"
// game share
#include "game_share/pact.h"
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/client_action_type.h"
#include "game_share/sentence_appraisal.h"
#include "game_share/interface_flags.h"
#include "game_share/inventories.h"
#include "game_share/mission_desc.h"
//#include "game_share/sheath.h"
#include "game_share/slot_equipment.h"
#include "game_share/entity_types.h"
#include "game_share/animal_status.h"
#include "game_share/outpost.h"

#include "skill_manager.h"
#include "inventory_manager.h"
#include "bot_chat_page_trade.h"

#include "../string_manager_client.h"

#include "nel/misc/i18n.h"

#include <string>


using namespace std;
using namespace NLMISC;

////////////
// EXTERN //
////////////

extern CClientChatManager ChatMngr;
extern std::vector<CCharacterSummary>	CharacterSummaries;

// ***************************************************************************
// Usual constants
DECLARE_INTERFACE_CONSTANT(getMaxBagInvSlot, MAX_BAGINV_ENTRIES);
DECLARE_INTERFACE_CONSTANT(getMaxAnimalInvSlot, MAX_ANIMALINV_ENTRIES);
DECLARE_INTERFACE_CONSTANT(getMaxRoomInvSlot, MAX_ROOMINV_ENTRIES);
DECLARE_INTERFACE_CONSTANT(getMaxGuildInvSlot, MAX_GUILDINV_ENTRIES);
DECLARE_INTERFACE_CONSTANT(getMaxTempInvSlot, MAX_TEMPINV_ENTRIES);
DECLARE_INTERFACE_CONSTANT(getFaberTypeUnknown, RM_FABER_TYPE::Unknown);
DECLARE_INTERFACE_CONSTANT(getItemTypeUnknown, ITEM_TYPE::UNDEFINED);
DECLARE_INTERFACE_CONSTANT(getMaxDynChanPerPlayer, CChatGroup::MaxDynChanPerPlayer);
DECLARE_INTERFACE_CONSTANT(getMaxNumMissions, MAX_NUM_MISSIONS);
// Outpost Constants
DECLARE_INTERFACE_CONSTANT(getOutpostEnumUnknownOutpostState, OUTPOSTENUMS::UnknownOutpostState);
DECLARE_INTERFACE_CONSTANT(getOutpostEnumPeace, OUTPOSTENUMS::Peace);
DECLARE_INTERFACE_CONSTANT(getOutpostEnumWarDeclaration, OUTPOSTENUMS::WarDeclaration);
DECLARE_INTERFACE_CONSTANT(getOutpostEnumAttackBefore, OUTPOSTENUMS::AttackBefore);
DECLARE_INTERFACE_CONSTANT(getOutpostEnumAttackRound, OUTPOSTENUMS::AttackRound);
DECLARE_INTERFACE_CONSTANT(getOutpostEnumAttackAfter, OUTPOSTENUMS::AttackAfter);
DECLARE_INTERFACE_CONSTANT(getOutpostEnumDefenseBefore, OUTPOSTENUMS::DefenseBefore);
DECLARE_INTERFACE_CONSTANT(getOutpostEnumDefenseRound, OUTPOSTENUMS::DefenseRound);
DECLARE_INTERFACE_CONSTANT(getOutpostEnumDefenseAfter, OUTPOSTENUMS::DefenseAfter);


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getMissionSmallIcon)
{
	if (args.size() != 1) return false;
	if (!args[0].toInteger()) return false;

	string sTexture = "Small_Task_Generic.tga";
	CEntitySheet *pSheet= SheetMngr.get(CSheetId((sint32)args[0].getInteger()));
	if (pSheet != NULL)
	{
		if (pSheet->Type == CEntitySheet::MISSION_ICON)
		{
			CMissionIconSheet *pMIS = (CMissionIconSheet*)pSheet;
			sTexture = pMIS->SmallIcon;
		}
	}

	result.setString(sTexture);
	return true;
}
REGISTER_INTERFACE_USER_FCT("getMissionSmallIcon", getMissionSmallIcon)

// ***************************************************************************
// test if an interface element is open (uses its id)  //
static DECLARE_INTERFACE_USER_FCT(isOpen)
{
	if (args.size() != 1) return false;
	if (!args[0].toString()) return false;
	CGroupContainer *elm = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(args[0].getString()));
	if (!elm)
	{
		nlwarning("<isOpen> : can't find element %s", args[0].getString().c_str());
		return false;
	}
	result.setBool(elm->isOpen());
	return true;
}
REGISTER_INTERFACE_USER_FCT("isOpen", isOpen)

// ***************************************************************************
// Get the compass text by its id
static DECLARE_INTERFACE_USER_FCT(getCompassText)
{
	if (args.size() != 1)
	{
		nlwarning("<getCompassText> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<getCompassText> Can't convert arg 0 to a int value.");
		return false;
	}

	//get the direction
	// sint64 in the databae.
	sint64	angleInt= args[0].getInteger();
	// cast as double now.
	double	angle= (double&)angleInt;
	sint direction =(sint) floor( 0.5 + ( 8.0 * (angle + NLMISC::Pi)/(NLMISC::Pi) ) );
	direction = ((direction%16)+16)%16;
	static const string txts[]=
	{
		"uiW",
		"uiWSW",
		"uiSW",
		"uiSSW",
		"uiS",
		"uiSSE",
		"uiSE",
		"uiESE",
		"uiE",
		"uiENE",
		"uiNE",
		"uiNNE",
		"uiN",
		"uiNNW",
		"uiNW",
		"uiWNW",
	};

	result.setUCString( CI18N::get(txts[direction]) );
	return true;
}
REGISTER_INTERFACE_USER_FCT("getCompassText", getCompassText);


// ***************************************************************************
// Get the compass text by its id
static DECLARE_INTERFACE_USER_FCT(getPactText)
{
	if (args.size() != 1)
	{
		nlwarning("<getPactText> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<getPactText> Can't convert arg 0 to a int value.");
		return false;
	}

	GSPACT::EPactType PactType = (GSPACT::EPactType)args[0].getInteger();

	result.setString (GSPACT::toString(PactType));
	return true;
}
REGISTER_INTERFACE_USER_FCT("getPactText", getPactText);



// ***************************************************************************
// Get the ryzom date by its ryzom day
static DECLARE_INTERFACE_USER_FCT(getDateText)
{
	if (args.size() != 1)
	{
		nlwarning("<getDateText> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<getDateText> Can't convert arg 0 to a int value.");
		return false;
	}

	CRyzomTime	rt;

	string	strRes;
	strRes= NLMISC::toString(rt.getRyzomDayOfMonth()+1);	// Start at 1
	strRes+= " / ";
	strRes+= NLMISC::toString(rt.getRyzomMonth()+1);		// Start at 1 for january
	strRes+= " / ";
	strRes+= NLMISC::toString(rt.getRyzomYear());

	result.setString (strRes);
	return true;
}
REGISTER_INTERFACE_USER_FCT("getDateText", getDateText);


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getDifficultyText)
{
	if (args.size() != 1)
	{
		nlwarning("<getDifficultyText> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<getDifficultyText> Can't convert arg 0 to a int value.");
		return false;
	}

	SENTENCE_APPRAISAL::ESentenceAppraisal sa = (SENTENCE_APPRAISAL::ESentenceAppraisal)args[0].getInteger();
	result.setUCString (CI18N::get(SENTENCE_APPRAISAL::toString(sa)));

	return true;
}
REGISTER_INTERFACE_USER_FCT("getDifficultyText", getDifficultyText);


// ***************************************************************************
// Get for the character selection if the specified slot is empty or not
static DECLARE_INTERFACE_USER_FCT(isCharSelSlotEmpty)
{
	if (args.size() != 1)
	{
		nlwarning("<isCharSelSlotEmpty> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<isCharSelSlotEmpty> Can't convert arg 0 to a int value.");
		return false;
	}

	uint32 nSlotSelected = (uint32)args[0].getInteger();
	bool res = false;
	if (nSlotSelected >= CharacterSummaries.size())
		res = true;
	else
		if (CharacterSummaries[nSlotSelected].Name.empty())
			res = true;
	result.setBool(res);
	return true;
}
REGISTER_INTERFACE_USER_FCT("isCharSelSlotEmpty", isCharSelSlotEmpty);

// ***************************************************************************
// Get for the character selection if the specified character is in a ring session or not
static DECLARE_INTERFACE_USER_FCT(isCharSelSlotInRingSession)
{
	if (args.size() != 1)
	{
		nlwarning("<isCharSelSlotInRingSession> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<isCharSelSlotEmpty> Can't convert arg 0 to a int value.");
		return false;
	}

	uint32 nSlotSelected = (uint32)args[0].getInteger();
	bool res = false;
	if (nSlotSelected < CharacterSummaries.size() && !CharacterSummaries[nSlotSelected].Name.empty())
	{
		res = CharacterSummaries[nSlotSelected].InRingSession;
	}
	result.setBool(res);
	return true;
}
REGISTER_INTERFACE_USER_FCT("isCharSelSlotInRingSession", isCharSelSlotInRingSession);


// Get for the character selection if the specified character is in a ring session or not
static DECLARE_INTERFACE_USER_FCT(isCharSelSlotHasEditSession)
{
	if (args.size() != 1)
	{
		nlwarning("<isCharSelSlotHasEditSession> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<isCharSelSlotHasEditSession> Can't convert arg 0 to a int value.");
		return false;
	}

	uint32 nSlotSelected = (uint32)args[0].getInteger();
	uint32 res = 0;
	if (nSlotSelected < CharacterSummaries.size() && !CharacterSummaries[nSlotSelected].Name.empty())
	{
		res = CharacterSummaries[nSlotSelected].HasEditSession;
	}
	result.setInteger(res);
	return true;
}
REGISTER_INTERFACE_USER_FCT("isCharSelSlotHasEditSession", isCharSelSlotHasEditSession);
// ***************************************************************************
// Get the ryzom action Text by its Enum
static DECLARE_INTERFACE_USER_FCT(getClientActionTypeText)
{
	if (args.size() != 1)
	{
		nlwarning("<getClientActionTypeText> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<getClientActionTypeText> Can't convert arg 0 to a int value.");
		return false;
	}

	// To enum
	CLIENT_ACTION_TYPE::TClientActionType	actType;
	if( args[0].getInteger()<CLIENT_ACTION_TYPE::NumClientActionType )
		actType= (CLIENT_ACTION_TYPE::TClientActionType)args[0].getInteger();
	else
		actType= CLIENT_ACTION_TYPE::None;

	// ToString.
	if( actType==CLIENT_ACTION_TYPE::None )
		result.setString( string() );
	else
		result.setString( CLIENT_ACTION_TYPE::toString(actType) );

	return true;
}
REGISTER_INTERFACE_USER_FCT("getClientActionTypeText", getClientActionTypeText);


// ***************************************************************************
// Get the ryzom action color by its Enum
static DECLARE_INTERFACE_USER_FCT(getClientActionTypeColor)
{
	if (args.size() != 1)
	{
		nlwarning("<getClientActionTypeColor> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<getClientActionTypeColor> Can't convert arg 0 to a int value.");
		return false;
	}

	// To enum
	CLIENT_ACTION_TYPE::TClientActionType	actType;
	if( args[0].getInteger()<CLIENT_ACTION_TYPE::NumClientActionType )
		actType= (CLIENT_ACTION_TYPE::TClientActionType)args[0].getInteger();
	else
		actType= CLIENT_ACTION_TYPE::None;

	// To Color
	// Get the action_bar_color options
	CInterfaceOptions	*options= CWidgetManager::getInstance()->getOptions("action_bar_color");
	if(options)
	{
		const CInterfaceOptionValue	&optVal= options->getValue(CLIENT_ACTION_TYPE::toString(actType));
		if( &optVal != &CInterfaceOptionValue::NullValue)
		{
			result.setString(optVal.getValStr());
		}
	}

	return true;
}
REGISTER_INTERFACE_USER_FCT("getClientActionTypeColor", getClientActionTypeColor);


// ***************************************************************************
// true if the sheetId define a 2Hand item
static DECLARE_INTERFACE_USER_FCT(is2HandItem)
{
	if (args.size() != 1)
	{
		nlwarning("<is2HandItem> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<is2HandItem> Can't convert arg 0 to a int value.");
		return false;
	}

	// Get the item sheet
	result.setBool( false );
	CEntitySheet	*sheet= SheetMngr.get(CSheetId((sint32)args[0].getInteger()));
	if(sheet && sheet->type()== CEntitySheet::ITEM)
	{
		CItemSheet	*item= (CItemSheet*)sheet;
		if( item->hasSlot(SLOTTYPE::TWO_HANDS) || item->hasSlot(SLOTTYPE::RIGHT_HAND_EXCLUSIVE) )
			result.setBool( true );
	}

	return true;
}
REGISTER_INTERFACE_USER_FCT("is2HandItem", is2HandItem);


// ***************************************************************************
// convert a serverTick in smoothServerTick
static DECLARE_INTERFACE_USER_FCT(getSmoothServerTick)
{
	if (args.size() != 1)
	{
		nlwarning("<getSmoothServerTick> Expecting 1 arg.");
		return false;
	}
	if (!args[0].toInteger())
	{
		nlwarning("<getSmoothServerTick> Can't convert arg 0 to a int value.");
		return false;
	}


	result.setInteger( NetMngr.convertToSmoothServerTick( (sint32)args[0].getInteger() ) );

	return true;
}
REGISTER_INTERFACE_USER_FCT("getSmoothServerTick", getSmoothServerTick);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getVSIndex)
{
	if (args.size() != 2)
	{
		nlwarning("<getVSIndex> Expecting 2 args.");
		return false;
	}
	if (!args[0].toString())
	{
		nlwarning("<getVSIndex> Can't convert arg 0 to a string value.");
		return false;
	}
	if (!args[1].toString())
	{
		nlwarning("<getVSIndex> Can't convert arg 1 to a string value.");
		return false;
	}

	string sItemName = args[0].getString();
	SLOTTYPE::EVisualSlot vs = SLOTTYPE::convertTypeToVisualSlot(SLOTTYPE::stringToSlotType(args[1].getString()));

	result.setInteger( SheetMngr.getVSIndex(sItemName, vs) );

	return true;
}
REGISTER_INTERFACE_USER_FCT("getVSIndex", getVSIndex);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getRoleId)
{
	if (args.size() != 1)
	{
		nlwarning("<getRoleId> Expecting 1 args.");
		return false;
	}
	if (!args[0].toString())
	{
		nlwarning("<getRoleId> Can't convert arg 0 to a string value.");
		return false;
	}

	string sRoleName = args[0].getString();

	result.setInteger( ROLES::toRoleId(sRoleName) );

	return true;
}
REGISTER_INTERFACE_USER_FCT("getRoleId", getRoleId);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getLightLevel)
{
	if (args.size() != 0)
	{
		nlwarning("<getLightLevel> Expecting 0 args.");
		return false;
	}

	result.setDouble(LightCycleManager.getLightLevel());

	return true;
}
REGISTER_INTERFACE_USER_FCT("getLightLevel", getLightLevel);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(strToIntFlag)
{
	if (args.size() != 1)
	{
		nlwarning("<strToIntFlag> Expecting 1 args.");
		return false;
	}

	args[0].toString();
	result.setInteger(INTERFACE_FLAGS::toInterfaceFlag(args[0].getString()));

	return true;
}
REGISTER_INTERFACE_USER_FCT("strToIntFlag", strToIntFlag);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getInventorySlot)
{
	if (args.size() != 1)
	{
		nlwarning("<getInventorySlot> Expecting 1 args.");
		return false;
	}
	if (!args[0].toString())
	{
		nlwarning("<getInventorySlot> Can't convert arg 0 to a string value.");
		return false;
	}

	string sName = args[0].getString();

	result.setInteger( INVENTORIES::toInventory(sName) );

	return true;
}
REGISTER_INTERFACE_USER_FCT("getInventorySlot", getInventorySlot);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getEquipmentSlot)
{
	if (args.size() != 1)
	{
		nlwarning("<getEquipmentSlot> Expecting 1 args.");
		return false;
	}
	if (!args[0].toString())
	{
		nlwarning("<getEquipmentSlot> Can't convert arg 0 to a string value.");
		return false;
	}

	string sName = args[0].getString();

	result.setInteger( SLOT_EQUIPMENT::stringToSlotEquipment(sName) );

	return true;
}
REGISTER_INTERFACE_USER_FCT("getEquipmentSlot", getEquipmentSlot);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getFactionIndex)
{
	if (args.size() != 1)
	{
		nlwarning("<getFactionIndex> Expecting 1 args.");
		return false;
	}
	if (!args[0].toString())
	{
		nlwarning("<getFactionIndex> Can't convert arg 0 to a string value.");
		return false;
	}

	PVP_CLAN::TPVPClan clan = PVP_CLAN::fromString(args[0].getString());

	if (clan < PVP_CLAN::BeginClans || clan == PVP_CLAN::Unknown)
	{
		nlwarning("<getFactionIndex> Arg 0 is not a correct value.");
		return false;
	}

	result.setInteger( clan - PVP_CLAN::BeginClans );

	return true;
}
REGISTER_INTERFACE_USER_FCT("getFactionIndex", getFactionIndex);


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getChatWin)
{
	if (args.size() != 1)
	{
		nlwarning("<getChatWin> Expecting 1 args.");
		return false;
	}


	CChatWindowManager &rCWM = CChatWindowManager::getInstance();
	ucstring title = CI18N::get(args[0].getString());
	CChatWindow *window = rCWM.getChatWindow(title);
	if (!window)
	{
		nlwarning("Can't find window named %s", title.toString().c_str());
		return false;
	}
	string sTmp = window->getContainer()->getId();
	result.setString(sTmp);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getChatWin", getChatWin);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getKey)
{
	if (args.size() < 2 || args.size() > 3)
	{
		nlwarning("<getKey> Expecting 2 or 3 args.");
		return false;
	}

	if ((args[0].toString() == false) ||  (args[1].toString() == false))
	{
		nlwarning("<getKey> Can't convert args to string value.");
		return false;
	}

	string name = args[0].getString();
	string param = args[1].getString();
	bool notna = false;
	if (args.size() > 2 && args[2].toBool() == true)
		notna = args[2].getBool();

	CActionsManager *pAM = &Actions;
	const CActionsManager::TActionComboMap &acmap = pAM->getActionComboMap();
	CActionsManager::TActionComboMap::const_iterator it = acmap.find(CAction::CName(name.c_str(),param.c_str()));
	if (it != acmap.end())
	{
		result.setUCString (it->second.toUCString());
	}
	else
	{
		if (notna)
			result.setUCString (ucstring(""));
		else
			result.setUCString (CI18N::get("uiNotAssigned"));
	}

	return true;
}
REGISTER_INTERFACE_USER_FCT("getKey", getKey);

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getVirtualDesktop)
{
	if (args.size() != 0)
	{
		nlwarning("<getVirtualDesktop> Expecting 0 args.");
		return false;
	}

	result.setInteger(CInterfaceManager::getInstance()->getMode());

	return true;
}
REGISTER_INTERFACE_USER_FCT("getVirtualDesktop", getVirtualDesktop);



// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isPeopleActive)
{
	if (args.size() != 1)
	{
		nlwarning("<isPeopleActive> Expecting 1 args.");
		return false;
	}

	result.setBool( (ServerPeopleActive &  (1 << args[0].getInteger())) != 0 );

	return true;
}
REGISTER_INTERFACE_USER_FCT("isPeopleActive", isPeopleActive)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isCareerActive)
{
	if (args.size() != 1)
	{
		nlwarning("<isCareerActive> Expecting 1 args.");
		return false;
	}

	result.setBool( (ServerCareerActive & (1 << args[0].getInteger())) != 0 );

	return true;
}
REGISTER_INTERFACE_USER_FCT("isCareerActive", isCareerActive)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isCtrlLaunchModalMacro)
{
	if (args.size() != 0)
	{
		nlwarning("<isCtrlLaunchModalMacro> Expecting 0 args.");
		return false;
	}

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CDBCtrlSheet	*ctrl= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());

	result.setBool( ctrl->isMacro() );

	return true;
}
REGISTER_INTERFACE_USER_FCT("isCtrlLaunchModalMacro", isCtrlLaunchModalMacro)


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(cansit)
{

	result.setBool( UserEntity->canSit() );

	return true;
}
REGISTER_INTERFACE_USER_FCT("cansit", cansit)


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isUserEntityDead)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if (args.size() != 0)
	{
		nlwarning("<isUserEntityDead> Expecting 0 args.");
		return false;
	}

	// NB: yoyo: check that the Property mode is 8, because this is what
	// is tested in the link in map.xml
	nlctassert(CLFECOMMON::PROPERTY_MODE==8);
	// suppose also that the DB is init with 0, and that 0 does not means DEATH!
	nlctassert(MBEHAV::DEATH!=0);

	// Get the Visual Property for mode
	string			dbName= toString("SERVER:Entities:E0:P%d", CLFECOMMON::PROPERTY_MODE );
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(dbName, false);
	if(node)
	{
		result.setBool( node->getValue64()== MBEHAV::DEATH );
	}
	else
		result.setBool( false );

	return true;
}
REGISTER_INTERFACE_USER_FCT("isUserEntityDead", isUserEntityDead)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getAnimalInventoryStateText)
{
	if (args.size() != 1)
	{
		nlwarning("<getAnimalInventoryStateText> Expecting 1 args.");
		return false;
	}

	// According to server status, change the inventory text
	uint	status= (uint)args[0].getInteger();
	if(ANIMAL_STATUS::isInStable((ANIMAL_STATUS::EAnimalStatus)status))
	{
		result.setString("uiAnimalInStable");
	}
	else if(!ANIMAL_STATUS::isInventoryAvailable((ANIMAL_STATUS::EAnimalStatus)status))
	{
		result.setString("uiAnimalTooFar");
	}
	else
		result.setString(string());

	return true;
}
REGISTER_INTERFACE_USER_FCT("getAnimalInventoryStateText", getAnimalInventoryStateText)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isAnimalStatusPresent)
{
	if (args.size() != 1)
	{
		nlwarning("<isAnimalStatusPresent> Expecting 1 args.");
		return false;
	}

	// According to server status, change the inventory text
	uint	status= (uint)args[0].getInteger();
	result.setBool(ANIMAL_STATUS::isSpawned(status));

	return true;
}
REGISTER_INTERFACE_USER_FCT("isAnimalStatusPresent", isAnimalStatusPresent)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isAnimalStatusDead)
{
	if (args.size() != 1)
	{
		nlwarning("<isAnimalStatusDead> Expecting 1 args.");
		return false;
	}

	// According to server status, change the inventory text
	uint	status= (uint)args[0].getInteger();
	result.setBool(ANIMAL_STATUS::isDead(status));

	return true;
}
REGISTER_INTERFACE_USER_FCT("isAnimalStatusDead", isAnimalStatusDead)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isAnimalStatusAlive)
{
	if (args.size() != 1)
	{
		nlwarning("<isAnimalStatusAlive> Expecting 1 args.");
		return false;
	}

	// According to server status, change the inventory text
	uint	status= (uint)args[0].getInteger();
	result.setBool(ANIMAL_STATUS::isAlive(status));

	return true;
}
REGISTER_INTERFACE_USER_FCT("isAnimalStatusAlive", isAnimalStatusAlive)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isAnimalStatusInStable)
{
	if (args.size() != 1)
	{
		nlwarning("<isAnimalStatusInStable> Expecting 1 args.");
		return false;
	}

	// According to server status, change the inventory text
	uint	status= (uint)args[0].getInteger();
	result.setBool(ANIMAL_STATUS::isInStable(status));

	return true;
}
REGISTER_INTERFACE_USER_FCT("isAnimalStatusInStable", isAnimalStatusInStable)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isSkillAtMax)
{
	CSkillManager	*pSM= CSkillManager::getInstance();

	if (args.size() != 2)
	{
		nlwarning("<getAnimalInventoryStateText> Expecting 2 args.");
		return false;
	}

	// NB: the 2nd arg is only used as a dependency value (every times any skill change, isSkillAtMax() may change)

	SKILLS::ESkills	skill= (SKILLS::ESkills)args[0].getInteger();
	if(skill>=SKILLS::NUM_SKILLS)
	{
		result.setBool(false);
	}
	else
	{
		sint32	baseVal= pSM->getBaseSkillValue(skill);
		sint32	maxVal= pSM->getMaxSkillValue(skill);
		// if < max value, not at max
		if(baseVal<maxVal)
			result.setBool(false);
		// if > max value, at max
		else if(baseVal>maxVal)
			result.setBool(true);
		// if == depends on sons....
		else
		{
			// depends where sons are at 0 or not
			const vector<SKILLS::ESkills>	&children= pSM->getChildren(skill);
			// no children (skill at 250), suppose OK, at max
			if(children.empty())
				result.setBool(true);
			else
			{
				bool	res= false;
				// check that at least one child is not at 0
				for(uint i=0;i<children.size();i++)
				{
					if(pSM->getBaseSkillValue(children[i])>0)
					{
						res= true;
						break;
					}
				}
				// set result
				result.setBool(res);
			}
		}
	}

	return true;
}
REGISTER_INTERFACE_USER_FCT("isSkillAtMax", isSkillAtMax)


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isQuitBrick)
{
	if(args.size()!=1)
	{
		nlwarning("<isQuitBrick> requires 1 arg");
		return false;
	}

	// static: use uint32, not CSheetId
	static uint32	quitBrick= 0;
	static bool		init= false;
	if(!init)
	{
		init= true;
		CSheetId	id("bapa02.sbrick");
		quitBrick= id.asInt();
	}

	// quit brick?
	result.setBool(args[0].getInteger()==quitBrick);

	return true;
}
REGISTER_INTERFACE_USER_FCT("isQuitBrick", isQuitBrick)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getBotChatBuyFilter)
{
	if(args.size()!=2)
	{
		nlwarning("<getBotChatBuyFilter> requires 2 arg");
		return false;
	}

	result.setString(toString(args[0].getInteger()) + "-" + toString(args[1].getInteger()));

	return true;
}
REGISTER_INTERFACE_USER_FCT("getBotChatBuyFilter", getBotChatBuyFilter)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getBotChatBuyFilterMPIcon)
{
	if(args.size()!=1)
	{
		nlwarning("<getBotChatBuyFilterMPIcon> requires 1 arg");
		return false;
	}

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	result.setString(CWidgetManager::getInstance()->getParser()->getDefine( RM_FABER_TYPE::toIconDefineString(RM_FABER_TYPE::TRMFType(args[0].getInteger())) ));

	return true;
}
REGISTER_INTERFACE_USER_FCT("getBotChatBuyFilterMPIcon", getBotChatBuyFilterMPIcon)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getBotChatBuyFilterMPText)
{
	if(args.size()!=1)
	{
		nlwarning("<getBotChatBuyFilterMPText> requires 1 arg");
		return false;
	}

	RM_FABER_TYPE::TRMFType		faberType= (RM_FABER_TYPE::TRMFType)args[0].getInteger();
	if(faberType>=RM_FABER_TYPE::Unknown)
		result.setUCString(CI18N::get("uittBCNoItemPartFilter"));
	else
		result.setUCString(RM_FABER_TYPE::toLocalString(faberType));

	return true;
}
REGISTER_INTERFACE_USER_FCT("getBotChatBuyFilterMPText", getBotChatBuyFilterMPText)


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getBotChatItemSheetForItemType)
{
	if(args.size()!=1)
	{
		nlwarning("<getBotChatItemSheetForItemType> requires 1 arg");
		return false;
	}

	// get arg
	uint	a0= (uint)args[0].getInteger();
	clamp(a0,(uint)0,uint(ITEM_TYPE::UNDEFINED));

	// get sheet name
	string	name= CBotChatPageTrade::getItemSheetNameForItemType((ITEM_TYPE::TItemType)a0);

	// result
	CSheetId	sheetId;
	sheetId.buildSheetId(name);
	result.setInteger(sheetId.asInt());

	return true;
}
REGISTER_INTERFACE_USER_FCT("getBotChatItemSheetForItemType", getBotChatItemSheetForItemType)

// ******* //// ******* //// ******* //// ******* //// ******* //// ******* //
// OUTPOST //// OUTPOST //// OUTPOST //// OUTPOST //// OUTPOST //// OUTPOST //
// ******* //// ******* //// ******* //// ******* //// ******* //// ******* //

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getOutpostName)
{
	if(args.size()!=1)
	{
		nlwarning("<getOutpostName> requires 1 arg (int=sheet)");
		return false;
	}

	// get arg
	uint32	nSheet = (uint32)args[0].getInteger();
	if (nSheet == 0)
	{
		result.setUCString(string(""));
		return true;
	}

	// get sheet name
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	const ucstring name(pSMC->getOutpostLocalizedName(CSheetId(nSheet)));

	result.setUCString(name);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getOutpostName", getOutpostName)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getOutpostDesc)
{
	if(args.size()!=1)
	{
		nlwarning("<getOutpostName> requires 1 arg (int=sheet)");
		return false;
	}

	// get arg
	uint32	nSheet = (uint32)args[0].getInteger();
	if (nSheet == 0)
	{
		result.setUCString(string(""));
		return true;
	}

	// get sheet name
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	const ucstring name(pSMC->getOutpostLocalizedDescription(CSheetId(nSheet)));

	result.setUCString(name);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getOutpostDesc", getOutpostDesc)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getOutpostBuildingName)
{
	if(args.size()!=1)
	{
		nlwarning("<getOutpostBuildingName> requires 1 arg (int=sheet)");
		return false;
	}

	// get arg
	uint32	nSheet = (uint32)args[0].getInteger();
	if (nSheet == 0)
	{
		result.setUCString(string(""));
		return true;
	}

	// get sheet name
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	const ucstring name(pSMC->getOutpostBuildingLocalizedName(CSheetId(nSheet)));

	result.setUCString(name);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getOutpostBuildingName", getOutpostBuildingName)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getOutpostBuildingDesc)
{
	if(args.size()!=1)
	{
		nlwarning("<getOutpostBuildingDesc> requires 1 arg (int=sheet)");
		return false;
	}

	// get arg
	uint32	nSheet = (uint32)args[0].getInteger();
	if (nSheet == 0)
	{
		result.setUCString(string(""));
		return true;
	}

	// get sheet name
	ucstring name;
	CEntitySheet *pSheet= SheetMngr.get(CSheetId(nSheet));
	COutpostBuildingSheet *pOBS = dynamic_cast<COutpostBuildingSheet*>(pSheet);
	if (pOBS && pOBS->OBType == COutpostBuildingSheet::OB_Empty)
	{
		// Don't display description if the building is an empty slot
		name = "";
	}
	else
	{
		STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
		name = pSMC->getOutpostBuildingLocalizedDescription(CSheetId(nSheet));
	}


	result.setUCString(name);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getOutpostBuildingDesc", getOutpostBuildingDesc)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getSquadName)
{
	if(args.size()!=1)
	{
		nlwarning("<getSquadName> requires 1 arg (int=sheet)");
		return false;
	}

	// get arg
	uint32	nSheet = (uint32)args[0].getInteger();
	if (nSheet == 0)
	{
		result.setUCString(string(""));
		return true;
	}

	// get sheet name
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	const ucstring name(pSMC->getSquadLocalizedName(CSheetId(nSheet)));

	result.setUCString(name);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getSquadName", getSquadName)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getSquadDesc)
{
	if(args.size()!=1)
	{
		nlwarning("<getSquadDesc> requires 1 arg (int=sheet)");
		return false;
	}

	// get arg
	uint32	nSheet = (uint32)args[0].getInteger();
	if (nSheet == 0)
	{
		result.setUCString(string(""));
		return true;
	}

	// get sheet name
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	const ucstring name(pSMC->getSquadLocalizedDescription(CSheetId(nSheet)));

	result.setUCString(name);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getSquadDesc", getSquadDesc)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getSquadCost)
{
	if(args.size()!=1)
	{
		nlwarning("<getSquadCost> requires 1 arg (int=sheet)");
		return false;
	}

	// get arg
	uint32	nSheet = (uint32)args[0].getInteger();
	if (nSheet == 0)
	{
		result.setInteger(0);
		return true;
	}

	// Yoyo: avoid warning(bad type) if the sheet is Bad.
	COutpostSquadSheet *pOSS = dynamic_cast<COutpostSquadSheet*>(SheetMngr.get(CSheetId(nSheet)));
	if (pOSS == NULL)
		result.setInteger(0);
	else
		result.setInteger(pOSS->BuyPrice);

	return true;
}
REGISTER_INTERFACE_USER_FCT("getSquadCost", getSquadCost)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(getOutpostPeriod)
{
	if(args.size()!=5)
	{
		nlwarning("<getOutpostPeriod> requires 5 arg");
		return false;
	}

	// get args.
	sint32	timeStartInSec= (sint32)args[0].getInteger();
	sint32	timeLenInMinute= (sint32)args[1].getInteger();
	sint32	timeZoneInHour= (sint32)args[2].getInteger();
	OUTPOSTENUMS::TOutpostState		status= (OUTPOSTENUMS::TOutpostState)args[3].getInteger();
	bool	isAttackPeriod= (sint32)args[4].getInteger() != 0;

	// if status wanted is peace or unknow, then "N/A", because there is no attack period in peace mode
	if( status==OUTPOSTENUMS::Peace || status==OUTPOSTENUMS::UnknownOutpostState )
	{
		result.setUCString(string(" - "));
		return true;
	}

	// If the attack period is passed, then just display a "Done".
	if( (isAttackPeriod && status>OUTPOSTENUMS::AttackRound) ||
		(!isAttackPeriod && status>OUTPOSTENUMS::DefenseRound) )
	{
		result.setUCString(CI18N::get("uiOutpostPeriodEnded"));
		return true;
	}

	// translate from GMT to LOCAL
	timeStartInSec+= timeZoneInHour*3600;
	timeStartInSec= max(timeStartInSec, sint32(0));

	// compute time end
	sint32	timeEndInSec= timeStartInSec + timeLenInMinute*60;

	// convert to readable form
	uint32	dname, dnumber, month, hstart, mstart, hend, mend;
	struct tm	*tstruct;
	time_t		tval;
	// convert time start
	tval= timeStartInSec;
	tstruct= gmtime(&tval);
	if(!tstruct)
	{
		result.setUCString(string("Bad Date Received"));
		return true;
	}
	dname= tstruct->tm_wday;	// 0-6 (Sunday==0!!)
	dnumber= tstruct->tm_mday;	// 1-31
	month= tstruct->tm_mon;		// 0-11 (0==January)
	hstart= tstruct->tm_hour;	// 0-23
	mstart= tstruct->tm_min;	// 0-59
	// convert time end
	tval= timeEndInSec;
	tstruct= gmtime(&tval);
	if(!tstruct)
	{
		result.setUCString(string("Bad Date Received"));
		return true;
	}
	hend= tstruct->tm_hour;	// 0-23
	mend= tstruct->tm_min;	// 0-59

	// translate
	ucstring	res= CI18N::get("uiOutpostPeriodFormat");
	strFindReplace( res, "%dayname", CI18N::get(toString("uiDay%d", dname)) );
	strFindReplace( res, "%daynumber", toString(dnumber) );
	strFindReplace( res, "%month", CI18N::get(toString("uiMonth%02d", month+1)) );
	strFindReplace( res, "%timestart", toString("%02d:%02d", hstart, mstart) );
	strFindReplace( res, "%timeend", toString("%02d:%02d", hend, mend) );

	result.setUCString(res);
	return true;
}
REGISTER_INTERFACE_USER_FCT("getOutpostPeriod", getOutpostPeriod)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isDBInitInProgress)
{
	// init in progress?
	result.setBool(IngameDbMngr.initInProgress());
	return true;
}
REGISTER_INTERFACE_USER_FCT("isDBInitInProgress", isDBInitInProgress)
