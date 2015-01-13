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

#include "dbctrl_sheet.h"
#include "interface_manager.h"
#include "nel/gui/view_text.h"
#include "../sheet_manager.h"
#include "../client_sheets/entity_sheet.h"
#include "../client_sheets/pact_sheet.h"
#include "../client_sheets/mission_icon_sheet.h"
#include "../client_sheets/faction_sheet.h"
#include "game_share/skills.h"
#include "game_share/inventories.h"
#include "list_sheet_base.h"
#include "../string_manager_client.h"
#include "nel/gui/interface_options.h"
#include "inventory_manager.h"
#include "skill_manager.h"
#include "../user_entity.h"
#include "../entities.h"
#include "bar_manager.h"

#include "macrocmd_manager.h"
#include "../string_manager_client.h" // For GuildFlag type
#include "guild_manager.h" // For GuildFlag type
#include "sbrick_manager.h"
#include "sphrase_manager.h"
#include "../client_sheets/sphrase_sheet.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/lua_ihm.h"
#include "lua_ihm_ryzom.h"
#include "game_share/bot_chat_types.h"

#include "../r2/editor.h"

#include "nel/gui/lua_manager.h"

extern CSheetManager SheetMngr;

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace STRING_MANAGER;

NLMISC::CSmartPtr<CSPhraseComAdpater> CDBCtrlSheet::_PhraseAdapter;

CDBCtrlSheet *CDBCtrlSheet::_CurrSelection = NULL;
CDBCtrlSheet *CDBCtrlSheet::_CurrMenuSheet = NULL;
UMaterial CDBCtrlSheet::_GuildMat;


NLMISC::CRGBA CDBCtrlSheet::_ArmourColor[RM_COLOR::NumColors];

REGISTER_UI_CLASS(CDBCtrlSheet)


const uint64 NOTIFY_ANIM_MS_DURATION = 1000;

// ***************************************************************************



// **********************************************************************************************************
class CControlSheetTooltipInfoWaiter : public IItemInfoWaiter
{
public:
	// The item used to open this window
	CDBCtrlSheet* CtrlSheet;
	string LuaMethodName;
public:
	ucstring infoValidated(CDBCtrlSheet* ctrlSheet, string luaMethodName);
	virtual void infoReceived();
};
static CControlSheetTooltipInfoWaiter ControlSheetTooltipUpdater;

void CControlSheetTooltipInfoWaiter::infoReceived()
{
	getInventory().removeItemInfoWaiter(&ControlSheetTooltipUpdater);
	infoValidated(CtrlSheet, LuaMethodName);
}


ucstring CControlSheetTooltipInfoWaiter::infoValidated(CDBCtrlSheet* ctrlSheet, string luaMethodName)
{
	ucstring help;

	// delegate setup of context he help ( & window ) to lua		
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CLuaState *ls= CLuaManager::getInstance().getLuaState();
	{
		CLuaStackRestorer lsr(ls, 0);

		CLuaIHM::pushReflectableOnStack(*ls, (CReflectableRefPtrTarget *)ctrlSheet);
		ls->pushGlobalTable();
		CLuaObject game(*ls);
		game = game["game"];		
		game.callMethodByNameNoThrow(luaMethodName.c_str(), 1, 1);

		// retrieve result from stack
		if (!ls->empty())
		{
			CLuaIHM::pop(*ls, help);
		}
		else
		{
			nlwarning(toString("Ucstring result expected when calling '%s', possible script error", luaMethodName.c_str()).c_str());
		}
	}

	return help;
}

// ***************************************************************************
int CDBCtrlSheet::luaGetDraggedSheet(CLuaState &ls)
{
	CLuaIHM::pushUIOnStack(ls, dynamic_cast<CInterfaceElement *>( dynamic_cast< CDBCtrlSheet* >( CCtrlDraggable::getDraggedSheet() ) ));
	return 1;
}

// ***************************************************************************
int CDBCtrlSheet::luaGetHpBuff(CLuaState &ls)
{
	CDBCtrlSheet *ctrlSheet = const_cast<CDBCtrlSheet*>(this);
	uint32	itemSlotId= getInventory().getItemSlotId(ctrlSheet);
	CClientItemInfo itemInfo = getInventory().getItemInfo(itemSlotId);

	ls.push((double)itemInfo.HpBuff);

	return 1;
}

// ***************************************************************************
int CDBCtrlSheet::luaGetSapBuff(CLuaState &ls)
{
	CDBCtrlSheet *ctrlSheet = const_cast<CDBCtrlSheet*>(this);
	uint32	itemSlotId= getInventory().getItemSlotId(ctrlSheet);
	CClientItemInfo itemInfo = getInventory().getItemInfo(itemSlotId);

	ls.push((double)itemInfo.SapBuff);
	
	return 1;
}

// ***************************************************************************
int CDBCtrlSheet::luaGetFocusBuff(CLuaState &ls)
{
	CDBCtrlSheet *ctrlSheet = const_cast<CDBCtrlSheet*>(this);
	uint32	itemSlotId= getInventory().getItemSlotId(ctrlSheet);
	CClientItemInfo itemInfo = getInventory().getItemInfo(itemSlotId);

	ls.push((double)itemInfo.FocusBuff);

	return 1;
}

// ***************************************************************************
int CDBCtrlSheet::luaGetStaBuff(CLuaState &ls)
{
	CDBCtrlSheet *ctrlSheet = const_cast<CDBCtrlSheet*>(this);
	uint32	itemSlotId= getInventory().getItemSlotId(ctrlSheet);
	CClientItemInfo itemInfo = getInventory().getItemInfo(itemSlotId);

	ls.push((double)itemInfo.StaBuff);

	return 1;
}


// ***************************************************************************
int CDBCtrlSheet::luaGetName(CLuaState &ls)
{
	CLuaIHM::push(ls, getItemActualName());
	return 1;
}

// **********************************************************************************************************
class LuaInfoWaiter : public IItemInfoWaiter
{
public:
	volatile bool done;

public:
	virtual void infoReceived();
};


void LuaInfoWaiter::infoReceived()
{
	getInventory().removeItemInfoWaiter(this);
	this->done = true;
}

static LuaInfoWaiter luaInfoWaiter;

// ***************************************************************************
int CDBCtrlSheet::luaGetCreatorName(CLuaState &ls)
{
	uint32	itemSlotId = getInventory().getItemSlotId(this);
	CClientItemInfo itemInfo = getInventory().getItemInfo(itemSlotId);
	ucstring creatorName;
	STRING_MANAGER::CStringManagerClient::instance()->getString(itemInfo.CreatorName, creatorName);
	CLuaIHM::push(ls, creatorName);
	
	return 1;
}

// ***************************************************************************
int CDBCtrlSheet::luaWaitInfo(CLuaState &ls)
{
	static bool sent = false;
	CDBCtrlSheet *ctrlSheet = const_cast<CDBCtrlSheet*>(this);
	uint32	itemSlotId= getInventory().getItemSlotId(ctrlSheet);
	CClientItemInfo itemInfo = getInventory().getItemInfo(itemSlotId);

	if (sent || itemInfo.versionInfo != 0)
	{
		ls.push((bool)(luaInfoWaiter.done));
		if (luaInfoWaiter.done)
			sent = false;
		return luaInfoWaiter.done ? 1 : 0;
	}
	else
	{
		luaInfoWaiter.ItemSlotId = itemSlotId;
		luaInfoWaiter.ItemSheet = this->getSheetId();
		luaInfoWaiter.done = false;
		getInventory().addItemInfoWaiter(&luaInfoWaiter);
		sent = true;
	}
	return 0;
}

// ***************************************************************************
int CDBCtrlSheet::luaBuildCrystallizedSpellListBrick(CLuaState &ls)
{
	

	CDBCtrlSheet *ctrlSheet = const_cast<CDBCtrlSheet*>(this);
	uint32	itemSlotId= getInventory().getItemSlotId(ctrlSheet);
	CClientItemInfo itemInfo = getInventory().getItemInfo(itemSlotId);

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	uint currentBrick = 0;
	uint i;
	for(i=0;i<itemInfo.Enchantment.Bricks.size();i++)
	{
		//if ( ! (pBM->getBrick(itemInfo.Enchantment.Bricks[i])->isCredit() || pBM->getBrick(itemInfo.Enchantment.Bricks[i])->isParameter()))
		{
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("UI:VARIABLES:CRYSTALBRICKS:%d:SHEET", currentBrick++));
			if(node)
				node->setValue32(itemInfo.Enchantment.Bricks[i].asInt());
		}
	}

	
	// Reset other to 0.
	for(;;currentBrick++)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("UI:VARIABLES:CRYSTALBRICKS:%d:SHEET", currentBrick), false);
		if(node)
			node->setValue32(0);
		else
			break;
	}
	return 1;
}


// ***************************************************************************

// ----------------------------------------------------------------------------
CCtrlSheetInfo::CCtrlSheetInfo()
{
	_Type = CCtrlSheetInfo::SheetType_Item;
	_DispNoSheetBmpId = -1;
	_InterfaceColor= true;
	_SheetSelectionGroup = -1;
	_UseQuality = true;
	_DisplayItemQuality = true;
	_UseQuantity = true;
	_DuplicateOnDrag = false;
	_ItemSlot= SLOTTYPE::UNDEFINED;
	_AutoGrayed= false;
	_HasTradeSlotType = false;
	_BrickOverable= false;
	_ReadQuantityFromSheet = false;
	_AHOnLeftClick = NULL;
	_AHOnRightClick = NULL;
	_AHOnCanDrag = NULL;
	_AHOnDrag = NULL;
	_AHOnCanDrop = NULL;
	_AHOnDrop = NULL;
	_AHOnCannotDrop = NULL;
	_DragCopy= false;
	_ForceItemBackGroundGeneric= false;
}

void CDBCtrlSheet::release ()
{
	NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();
	if (Driver)
		Driver->deleteMaterial(_GuildMat);

	// release now the phrase or it ll be release too late and crash
	_PhraseAdapter = 0;
}

// ----------------------------------------------------------------------------
bool CCtrlSheetInfo::parseCtrlInfo(xmlNodePtr cur, CInterfaceGroup * /* parentGroup */)
{
	CXMLAutoPtr prop;

	// read type
	prop = (char*) xmlGetProp( cur, (xmlChar*)"nature" );
	if (prop)
	{
		if (NLMISC::strlwr(prop) == "item")
			_Type = CCtrlSheetInfo::SheetType_Item;
		else if (NLMISC::strlwr(prop) == "pact")
			_Type = CCtrlSheetInfo::SheetType_Pact;
		else if (NLMISC::strlwr(prop) == "skill")
			_Type = CCtrlSheetInfo::SheetType_Skill;
		else if (NLMISC::strlwr(prop) == "auto")
			_Type = CCtrlSheetInfo::SheetType_Auto;
		else if (NLMISC::strlwr(prop) == "macro")
			_Type = CCtrlSheetInfo::SheetType_Macro;
		else if (NLMISC::strlwr(prop) == "guild_flag")
			_Type = CCtrlSheetInfo::SheetType_GuildFlag;
		else if (NLMISC::strlwr(prop) == "mission")
			_Type = CCtrlSheetInfo::SheetType_Mission;
		else if (NLMISC::strlwr(prop) == "sbrick")
			_Type = CCtrlSheetInfo::SheetType_SBrick;
		else if (NLMISC::strlwr(prop) == "sphraseid")
			_Type = CCtrlSheetInfo::SheetType_SPhraseId;
		else if (NLMISC::strlwr(prop) == "sphrase")
			_Type = CCtrlSheetInfo::SheetType_SPhrase;
		else if (NLMISC::strlwr(prop) == "elevator_destination")
			_Type = CCtrlSheetInfo::SheetType_ElevatorDestination;
		else if (NLMISC::strlwr(prop) == "outpost_building")
			_Type = CCtrlSheetInfo::SheetType_OutpostBuilding;
	}

	// Read bitmap
	prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_noitem" );
	if (prop)
	{
		string TxName = (const char *) prop;
		TxName = strlwr (TxName);
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		_DispNoSheetBmpId = rVR.getTextureIdFromName (TxName);
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"col_noitem" );
	if (prop)
		_InterfaceColor = CInterfaceElement::convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"use_slot_type_db_entry" );
	if (prop)
		_HasTradeSlotType= CInterfaceElement::convertBool(prop);

	// Read Action handlers
	CAHManager::getInstance()->parseAH(cur, "onclick_l", "params_l", _AHOnLeftClick, _AHLeftClickParams);
	CAHManager::getInstance()->parseAH(cur, "onclick_r", "params_r", _AHOnRightClick, _AHRightClickParams);
	CAHManager::getInstance()->parseAH(cur, "oncandrop", "params_candrop", _AHOnCanDrop, _AHCanDropParams);
	CAHManager::getInstance()->parseAH(cur, "ondrop", "params_drop", _AHOnDrop, _AHDropParams);
	CAHManager::getInstance()->parseAH(cur, "oncannotdrop", "params_cannotdrop", _AHOnCannotDrop, _AHCannotDropParams);
	CAHManager::getInstance()->parseAH(cur, "oncandrag", "params_candrag", _AHOnCanDrag, _AHCanDragParams);
	CAHManager::getInstance()->parseAH(cur, "ondrag", "params_drag", _AHOnDrag, _AHDragParams);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"selection_group" );
	if (prop)
	{
		const CCtrlSheetSelection &css = CWidgetManager::getInstance()->getParser()->getCtrlSheetSelection();
		_SheetSelectionGroup = css.getGroupIndex((const char *) prop);
		if (_SheetSelectionGroup == -1)
		{
			nlwarning("<CCtrlSheetInfo::parseCtrlInfo> %s if not a sheet selection group", (const char *) prop);
		}
	}
	else _SheetSelectionGroup = -1;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"use_quantity" );
	if (prop) _UseQuantity = CInterfaceElement::convertBool(prop);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"read_quantity_from_sheet" );
	if (prop) _ReadQuantityFromSheet = CInterfaceElement::convertBool(prop);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"use_quality" );
	if (prop) _UseQuality = CInterfaceElement::convertBool(prop);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"duplicate_on_drag" );
	if (prop) _DuplicateOnDrag = CInterfaceElement::convertBool(prop);

	// Context menu association
	prop = (char*) xmlGetProp( cur, (xmlChar*)"menu_l" );
	if (prop)
	{
		string tmp = (const char *) prop;
		_ListMenuLeft = strlwr(tmp);
	}
	prop = (char*) xmlGetProp( cur, (xmlChar*)"menu_r" );
	if (prop)
	{
		string tmp = (const char *) prop;
		_ListMenuRight = strlwr(tmp);
	}
	prop = (char*) xmlGetProp( cur, (xmlChar*)"menu_r_empty_slot" );
	if (prop)
	{
		string tmp = (const char *) prop;
		_ListMenuRightEmptySlot = strlwr(tmp);
	}
	// list menu on both clicks
	prop = (char*) xmlGetProp( cur, (xmlChar*)"menu_b" );
	if (prop)
	{
		string tmp = (const char *) prop;
		setListMenuBoth(strlwr(tmp));
	}

	// _BrickTypeBitField
	prop = (char*) xmlGetProp( cur, (xmlChar*)"brick_type" );
	if(prop)
	{
		// Defined => filter none.
		_BrickTypeBitField= 0;

		// The string may have multiple brick type separated by |
		string	brickTypeArray= (const char*)prop;
		vector<string>	strList;
		NLMISC::splitString(NLMISC::toUpper(brickTypeArray), "|", strList);

		// Test All words
		for(uint i=0;i<strList.size();i++)
		{
			BRICK_TYPE::EBrickType	brickType= BRICK_TYPE::toBrickType(strList[i]);
			if(brickType==BRICK_TYPE::UNKNOWN)
			{
				nlwarning("<CCtrlSheetInfo::parseCtrlInfo> %s has an unvalid Brick Type", (const char *) prop);
			}
			else
			{
				// must not have so much brick type, else must change code!
				nlassert(brickType<32);

				// Ok set the bit associated
				_BrickTypeBitField|= 1<<brickType;
			}
		}
	}

	// _SlotType
	prop = (char*) xmlGetProp( cur, (xmlChar*)"item_slot" );
	if(prop)
	{
		string str= prop;
		_ItemSlot= SLOTTYPE::stringToSlotType(NLMISC::toUpper(str));
	}

	// _AutoGrayed
	prop = (char*) xmlGetProp( cur, (xmlChar*)"auto_grayed" );
	if(prop)	_AutoGrayed= CInterfaceElement::convertBool(prop);

	// _BrickOverable
	prop = (char*) xmlGetProp( cur, (xmlChar*)"brick_over" );
	if(prop)	_BrickOverable= CInterfaceElement::convertBool(prop);

	// Drag Copy support?
	prop = (char*) xmlGetProp( cur, (xmlChar*)"drag_copy" );
	if(prop)	_DragCopy= CInterfaceElement::convertBool(prop);

	// _ForceItemBackGroundGeneric
	prop = (char*) xmlGetProp( cur, (xmlChar*)"force_item_background_generic" );
	if(prop)	_ForceItemBackGroundGeneric= CInterfaceElement::convertBool(prop);

	return true;
}


// ***************************************************************************

NLMISC_REGISTER_OBJECT(CViewBase, CDBCtrlSheet, std::string, "sheet");

// ----------------------------------------------------------------------------
CDBCtrlSheet::CDBCtrlSheet(const TCtorParam &param) :
CCtrlDraggable(param)
{
	_LastSheetId = 0;
	_DispSlotBmpId= -1;
	_DispBackBmpId = -1;
	_DispSheetBmpId = -1;
	_DispOverBmpId = -1;
	_DispOver2BmpId= -1;
	_CanDrop = false;
	_Stackable= 1;
	_DispQuality= -1;
	_DispQuantity= -1;
	_Over = false;
	_TextureIdOver= -1;
	_IndexInDB= 0;
	_SecondIndexInDB= 0;
	_ShortCut= false;
	_SheetColor= CRGBA::White;
	_OtherHandItemFilter= NULL;
	_ActualType = _Type;
	_ItemWeared= false;
	_ItemBeastGrayed= false;
	_Grayed= false;
	_Useable= true;
	_GrayedLink= NULL;
	_NeedSetup= true;
	_IconW = 0;
	_IconH = 0;
	_SetupInit= false;
	_IconBackColor= CRGBA::White;
	_IconColor= CRGBA::White;
	_IconOverColor= CRGBA::White;
	_IconOver2Color= CRGBA::White;
	_PackedArmourColor= 0;
	_MacroID = -1;
	_GuildBack = _GuildSymb = NULL;
	_UseGuildIcon = false;
	_GuildIcon = NLMISC::CSheetId::Unknown;
	_DrawSlot= true;
	_ItemCaracReqType= CHARACTERISTICS::Unknown;
	_ItemCaracReqValue= 0;
	_ArmourColorFromDB= false;
	_ArmourColorBmpOk= false;
	_ArmourColorIndex= 0;
	_UserColor= NULL;
	_ItemSheet = NULL;
	_ItemRMClassType= NULL;
	_ItemRMFaberStatType= NULL;
	_NotifyAnimEndTime = 0;
}

// ----------------------------------------------------------------------------
CDBCtrlSheet::~CDBCtrlSheet()
{
	NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();

	if (_GuildBack)
	{
		if (Driver)
			Driver->deleteTextureFile(_GuildBack);
		_GuildBack = NULL;
	}
	if (_GuildSymb)
	{
		if (Driver)
			Driver->deleteTextureFile(_GuildSymb);
		_GuildSymb = NULL;
	}

	// ensure erase static
	if(this==_CurrMenuSheet)		_CurrMenuSheet = NULL;
	if(this == dynamic_cast< CDBCtrlSheet* >( CCtrlDraggable::getDraggedSheet() ) )
		setDraggedSheet( NULL );
	if(this==_CurrSelection)		_CurrSelection = NULL;
}

// ----------------------------------------------------------------------------
bool CDBCtrlSheet::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	CXMLAutoPtr prop;

	if (! CCtrlBase::parse(cur,parentGroup) )
	{
		string tmp = "cannot parse view:"+getId()+", parent:"+parentGroup->getId();
		nlinfo(tmp.c_str());
		return false;
	}

	// parse the common ctrl info
	if(!parseCtrlInfo(cur, parentGroup))
		return false;

	prop = (char*) xmlGetProp( cur, (xmlChar*)"dragable" );
	if (prop)
		setDraggable( CInterfaceElement::convertBool(prop) );
	else
		setDraggable( false );

	if (_Type != SheetType_Macro)
	{
		prop = (char*) xmlGetProp( cur, (xmlChar*)"value" );
		if (prop)
			initSheet (string((const char*)prop), *this);
		else
		{
			string tmp = "cannot get property value, view:"+getId()+", parent:"+parentGroup->getId();
			nlinfo(tmp.c_str());
			return false;
		}
	}

	if( _Type == SheetType_SBrick || _Type == SheetType_SPhraseId || _Type == SheetType_SPhrase || _Type == SheetType_Macro )
	{
		prop = (char*) xmlGetProp( cur, (xmlChar*)"is_shortcut" );
		if (prop) _ShortCut= convertBool(prop);
	}

	// _OtherHandItemFilter
	prop= (char*) xmlGetProp( cur, (xmlChar*)"other_hand_slot" );
	if(prop)
	{
		_OptString= (const char*)prop;
		// Fill the Pointer in setupInit() at first updateCoords() call...
	}

	// _Grayed
	prop = (char*) xmlGetProp( cur, (xmlChar*)"grayed" );
	if(prop)	_Grayed= CInterfaceElement::convertBool(prop);

	// Draw the slot ?
	_DrawSlot = true;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"slot" );
	if(prop)	_DrawSlot= CInterfaceElement::convertBool(prop);


	updateActualType();
	// Init size for Type
	initSheetSize();
	return true;
}

// ----------------------------------------------------------------------------
void CDBCtrlSheet::initSheet(const std::string &dbBranchId, const CCtrlSheetInfo &ctrlInfo)
{
	H_AUTO ( RZ_CDBCtrlSheet_initSheet )

	nlassert((ctrlInfo._Type == SheetType_Macro) || (dbBranchId.size()>0));
	nlassert((ctrlInfo._Type == SheetType_Macro) || (_Id.size()>0));

	// init
	*static_cast<CCtrlSheetInfo*>(this)= ctrlInfo;

	// link to the branch
	if (ctrlInfo._Type != SheetType_Macro)
	{
		setSheet(dbBranchId);

		// get over for spell
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		_TextureIdOver = CViewRenderer::getInstance()->getTextureIdFromName ("w_slot_spell_over.tga");
	}

	// Init size.
	initSheetSize();
}

// ----------------------------------------------------------------------------
void CDBCtrlSheet::initSheetFast( const std::string &dbParentBranchId, int sheetNum, int slotNum,
	const CCtrlSheetInfo &ctrlInfo )
{
	H_AUTO ( RZ_CDBCtrlSheet_initSheetFast )

	nlassert((ctrlInfo._Type == SheetType_Macro) || (!dbParentBranchId.empty()));
	nlassert((ctrlInfo._Type == SheetType_Macro) || (!_Id.empty()));

	// init
	*static_cast<CCtrlSheetInfo*>(this)= ctrlInfo;

	// link to the branch
	if (ctrlInfo._Type != SheetType_Macro)
	{
		setSheetFast( dbParentBranchId, sheetNum, slotNum );

		// get over for spell
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		_TextureIdOver = CViewRenderer::getInstance()->getTextureIdFromName ("w_slot_spell_over.tga");
	}

	// Init size.
	initSheetSize();
}

// ----------------------------------------------------------------------------
std::string CDBCtrlSheet::getSheet() const
{
	return _DbBranchName;
}


// ***************************************************************************
void CDBCtrlSheet::setSheet (const std::string &dbBranchId)
{
	// link to the DBBranch
	_DbBranchName= dbBranchId;

	// Compute the index in DB
	_IndexInDB= 0;
	_SecondIndexInDB= 0;
	string::size_type	pos= dbBranchId.rfind(':');
	if(pos!=string::npos)
	{
		// get the number after the ':'
		fromString(dbBranchId.substr(pos+1), _IndexInDB);
		// try to get the _SecondIndexInDB.
		if(pos>0)
		{
			_SecondIndexInDB= getInventorySlot(dbBranchId);
		}
	}

	// set sheet DB links
	setupSheetDbLinks();
}


// ***************************************************************************
void CDBCtrlSheet::setSheetFast( const std::string &dbParentBranchId, int sheetNum, int slotNum )
{
	H_AUTO ( RZ_CDBCtrlSheet_setSheet )

	// link to the DBBranch
	_DbBranchName = dbParentBranchId + ":" + NLMISC::toString(sheetNum);

	// Set the index in DB.
	_IndexInDB = sheetNum;
	_SecondIndexInDB = slotNum;

	// set sheet DB links
	setupSheetDbLinks();
}


// ***************************************************************************
void CDBCtrlSheet::setupSheetDbLinks ()
{
	H_AUTO ( RZ_CDBCtrlSheet_setSheet )

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// link to the DBBranch (NB: none for macros)
	CCDBNodeBranch *dbBranch = NLGUI::CDBManager::getInstance()->getDbBranch( _DbBranchName );
	//nlassert(dbBranch || _DbBranchName.empty());

	// link if possible with the database, else dummy link to the interface
	switch( _Type )
	{
	case SheetType_Mission:
	{
		// LastSheetId is used as last mission text (mission text which is a server string id)
		_LastSheetId = 0;

		// Mission icon use _SheetId -> gives the sheet that contains the name of the icon texture
		if( !_SheetId.link( dbBranch, _DbBranchName+":ICON", pIM->_DB_UI_DUMMY_SHEET ) )
			nlwarning ("ERROR !!! Cannot find mission icon");

		_Quality.link( pIM->_DB_UI_DUMMY_QUALITY );
		break;
	}
	case SheetType_SPhraseId:
	{
		_SheetId.link( dbBranch, _DbBranchName+":PHRASE", pIM->_DB_UI_DUMMY_PHRASE );
		break;
	}
	case SheetType_GuildFlag:
	case SheetType_ElevatorDestination:
	{
		// LastSheetId is used as last guild name (guild name which is a server string id)
		_LastSheetId = 0;

		// Guild name use _SheetId
		if( !_SheetId.link( dbBranch, _DbBranchName+":NAME", pIM->_DB_UI_DUMMY_SHEET ) )
			nlwarning ("ERROR !!! Cannot find guild name : %s", (_DbBranchName+":NAME").c_str());

		// Guild icon use _Quality
		if( !_Quality.link( dbBranch, _DbBranchName+":ICON", pIM->_DB_UI_DUMMY_QUALITY ) )
			nlwarning ("ERROR !!! Cannot find guild icon: %s", (_DbBranchName+":ICON").c_str());

		// MacroID is used as the last guild icon
		_MacroID = 0;
		break;
	}
	default:
	{
		// sheet
		_SheetId.link( dbBranch, _DbBranchName+":SHEET", pIM->_DB_UI_DUMMY_SHEET );

		if (_HasTradeSlotType)
			_TradeSlotType.link( dbBranch, _DbBranchName+":SLOT_TYPE", pIM->_DB_UI_DUMMY_SLOT_TYPE );

		updateActualType();

		if(_ActualType == SheetType_Item || _ActualType == SheetType_Pact)
		{
			// quality
			if (_UseQuality)
				_Quality.link( dbBranch, _DbBranchName+":QUALITY", pIM->_DB_UI_DUMMY_QUALITY );
			else
				_Quality.link(pIM->_DB_UI_DUMMY_QUALITY);

			// quantity
			if(_ActualType == SheetType_Item)
			{
				if (_UseQuantity)
					_Quantity.link( dbBranch, _DbBranchName+":QUANTITY", pIM->_DB_UI_DUMMY_QUANTITY );
				else
					_Quantity.link( pIM->_DB_UI_DUMMY_QUANTITY );
			}
			else if(_ActualType == SheetType_Pact)
				_UseQuantity = false;

			// Misc Item
			if(_ActualType == SheetType_Item)
			{
				// Link to worned state if possible
				_Worned.link( dbBranch, _DbBranchName+":WORNED", pIM->_DB_UI_DUMMY_WORNED );

				// Link to _PrerequisitValid state if possible
				_PrerequisitValid.link( dbBranch, _DbBranchName+":PREREQUISIT_VALID", pIM->_DB_UI_DUMMY_PREREQUISIT_VALID );
			}

		}

		break;
	}
	}	// switch( _Type )

	// Link to NameId if possible (if item, else will link to UI:DUMMY:NAMEID).
	_NameId.link( dbBranch, _DbBranchName+":NAMEID", pIM->_DB_UI_DUMMY_NAMEID );

	// Link to Enchant if possible (if item, else will link to UI:DUMMY:ENCHANT).
	_Enchant.link( dbBranch, _DbBranchName+":ENCHANT", pIM->_DB_UI_DUMMY_ENCHANT );

	if(dbBranch)
	{
		// Link to UserColor if possible. else will link NULL.
		_UserColor = dbBranch->getLeaf( _DbBranchName+":USER_COLOR", false );

		// Link to item RM types if possible, else will link NULL
		_ItemRMClassType= dbBranch->getLeaf( _DbBranchName+":RM_CLASS_TYPE", false );
		_ItemRMFaberStatType= dbBranch->getLeaf( _DbBranchName+":RM_FABER_STAT_TYPE", false );

		// If auto grayed, link to "LOCKED" if possible
		if(_AutoGrayed)
			_GrayedLink= dbBranch->getLeaf( _DbBranchName+":LOCKED", false );
		else
			_GrayedLink= NULL;
	}
	else
	{
		_UserColor= NULL;
		_ItemRMClassType= NULL;
		_ItemRMFaberStatType= NULL;
		_GrayedLink= NULL;
	}

	// force reset of cache
	_NeedSetup= true;
}


// ***************************************************************************
// determine the inventory slot from the database branch id
//	return 0 if no match found
uint CDBCtrlSheet::getInventorySlot( const string &dbBranchId )
{
	// skip the SERVER: or LOCAL:
	string::size_type pos = dbBranchId.find( ":" );
	if(pos==string::npos)
		return 0;
	const char *szName = dbBranchId.c_str() + pos+1;
	const char *szName2;

	// fast selection according to first letters
	switch( szName[0] )
	{
	case 'E':
		if( strncmp( "EXCHANGE:", szName, 9 ) )
			break;
		szName2 = &szName[9];
		if( !strncmp( "GIVE", szName2, 4 ) )
			return INVENTORIES::exchange;
		if( !strncmp( "RECEIVE", szName2, 7 ) )
			return INVENTORIES::exchange_proposition;
		break;
	case 'G':
		if( !strncmp( "GUILD:INVENTORY", szName, 15 ) )
			return INVENTORIES::guild;
		break;
	case 'I':
		if( strncmp( "INVENTORY:", szName, 10 ) )
			break;
		szName2 = &szName[10];
		switch( szName2[0] )
		{
		case 'B':
			if( !strncmp( "BAG", szName2, 3 ) )
				return INVENTORIES::bag;
			break;
		case 'P':
			nlctassert(MAX_INVENTORY_ANIMAL==4);
			if( strncmp( "PACK_ANIMAL", szName2, 11 ) )
				break;
			switch( szName2[11] )
			{
			case '0':
				return INVENTORIES::pet_animal1;
			case '1':
				return INVENTORIES::pet_animal2;
			case '2':
				return INVENTORIES::pet_animal3;
			case '3':
				return INVENTORIES::pet_animal4;
			default:
				break;
			}
			break;
		case 'R':
			if( !strncmp( "ROOM", szName2, 4 ) )
				return INVENTORIES::player_room;
			break;
		case 'T':
			if( !strncmp( "TEMP", szName2, 4 ) )
				return INVENTORIES::temporary;
			break;
		default:
			break;
		}
	default:
		break;
	}
	return 0;
}

// ----------------------------------------------------------------------------
void CDBCtrlSheet::initSheetSize()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	// If the user type is auto, then select always item slot.
	if(_Type==SheetType_Auto)
	{
		_DispSlotBmpId = rVR.getTextureIdFromName ("w_slot_item.tga");
		_DispSelSlotId = rVR.getTextureIdFromName ("w_slot_item_selected.tga");
	}
	// Else select from actual type.
	else
	{
		switch(_ActualType)
		{
			case SheetType_Item:
			case SheetType_Pact:
			case SheetType_Mission:
			case SheetType_OutpostBuilding:
				_DispSlotBmpId = rVR.getTextureIdFromName ("w_slot_item.tga");
				_DispSelSlotId = rVR.getTextureIdFromName ("w_slot_item_selected.tga");
			break;
			case SheetType_Macro:
			case SheetType_SBrick:
			case SheetType_SPhraseId:
			case SheetType_SPhrase:
				if(_BrickOverable)
				{
					_DispSlotBmpId = rVR.getTextureIdFromName ("w_slot_spell.tga");
					_DispSelSlotId = rVR.getTextureIdFromName ("w_slot_spell_selected.tga");
				}
				else
				{
					_DispSlotBmpId = rVR.getTextureIdFromName ("w_slot_brick.tga");
					_DispSelSlotId = rVR.getTextureIdFromName ("w_slot_brick_selected.tga");
				}
			break;
			case SheetType_ElevatorDestination:
			case SheetType_GuildFlag:
				_DispSlotBmpId = rVR.getTextureIdFromName ("w_slot_blason.tga");
				_DispSelSlotId = rVR.getTextureIdFromName ("w_slot_blason_over.tga");
			break;
			default:
			break;
		}
	}
	rVR.getTextureSizeFromId (_DispSlotBmpId, _W, _H);
}

// ----------------------------------------------------------------------------
void CDBCtrlSheet::updateCoords ()
{
	if (getActive())
	{
		if(!_SetupInit)
			setupInit();

		if (_Type != CCtrlSheetInfo::SheetType_Macro)
		{
			if (_LastSheetId != _SheetId.getSInt32())
			{
				updateActualType();
			}
		}

		switch(_ActualType)
		{
			case CCtrlSheetInfo::SheetType_Item:  setupItem(); break;
			case CCtrlSheetInfo::SheetType_Pact:  setupPact(); break;
			case CCtrlSheetInfo::SheetType_SBrick:	setupSBrick(); break;
			case CCtrlSheetInfo::SheetType_SPhraseId: setupSPhraseId(); break;
			case CCtrlSheetInfo::SheetType_SPhrase: setupSPhrase(); break;
			case CCtrlSheetInfo::SheetType_Mission: setupMission(); break;
			case CCtrlSheetInfo::SheetType_OutpostBuilding: setupOutpostBuilding(); break;
			default: break;
		}

		if (_Type != CCtrlSheetInfo::SheetType_Macro)
		{
			_LastSheetId = _SheetId.getSInt32();
		}
	}
	CInterfaceElement::updateCoords();
}

// ----------------------------------------------------------------------------
void CDBCtrlSheet::updateIconSize()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	if (_DispSheetBmpId != -1)
	{
		rVR.getTextureSizeFromId(_DispSheetBmpId, _IconW, _IconH);
	}
	else if (_DispBackBmpId != -1)
	{
		rVR.getTextureSizeFromId(_DispBackBmpId, _IconW, _IconH);
	}
	else if (_DispOverBmpId != -1)
	{
		rVR.getTextureSizeFromId(_DispOverBmpId, _IconW, _IconH);
	}
	else if (_DispOver2BmpId != -1)
	{
		rVR.getTextureSizeFromId(_DispOver2BmpId, _IconW, _IconH);
	}
	else
	{
		_IconW = _IconH = 0;
	}
}


// ***************************************************************************
void CDBCtrlSheet::setupPact()
{
	sint32 sheet = _SheetId.getSInt32();
	// If this is the same sheet, need to resetup
	if (_LastSheetId != sheet || _NeedSetup)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		_LastSheetId = sheet;
		CSheetId sheetId(sheet);
		CEntitySheet *pES = SheetMngr.get (sheetId);
		if ((pES != NULL) && (pES->type() == CEntitySheet::PACT))
		{
			CPactSheet *pPS = (CPactSheet*)pES;
			_DispSheetBmpId = rVR.getTextureIdFromName (pPS->Icon);
			_DispBackBmpId = rVR.getTextureIdFromName (pPS->IconBackground);
			_DispOverBmpId = -1;
			_DispOver2BmpId = -1;
			updateIconSize();
			if (_UseQuality)
				_DispQuality = _Quality.getSInt32();
			else
				_DispQuality = -1;
		}
		else
		{
			resetAllTexIDs();
		}
	}
	else
	{
		// update quality
		if(_UseQuality)
			_DispQuality = _Quality.getSInt32();
		else
			_DispQuality = -1;
	}
}


// ***************************************************************************
void CDBCtrlSheet::setupItem ()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	sint32 sheet = _SheetId.getSInt32();
	// If this is the same sheet, need to resetup
	if (_LastSheetId != sheet || _NeedSetup)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		_NeedSetup= false;
		_LastSheetId = sheet;
		CSheetId sheetId(sheet);
		CEntitySheet *pES = SheetMngr.get (sheetId);
		if ((pES != NULL) && (pES->type() == CEntitySheet::ITEM))
		{
			_ItemSheet = (CItemSheet*)pES;

			// Display the item quality?
			_DisplayItemQuality= _UseQuality &&
				_ItemSheet->Family != ITEMFAMILY::COSMETIC &&
				_ItemSheet->Family != ITEMFAMILY::TELEPORT &&
				_ItemSheet->Family != ITEMFAMILY::SERVICE
				;

			_DispSheetBmpId = rVR.getTextureIdFromName (_ItemSheet->getIconMain());
			// if file not found or empty, replace by default icon
			if( _DispSheetBmpId == -1)
				_DispSheetBmpId = rVR.getSystemTextureId(CViewRenderer::DefaultItemTexture);
			if(_ForceItemBackGroundGeneric)
				_DispBackBmpId = rVR.getTextureIdFromName ("BK_generic.tga");
			else
				_DispBackBmpId = rVR.getTextureIdFromName (_ItemSheet->getIconBack());
			_DispOverBmpId = rVR.getTextureIdFromName (_ItemSheet->getIconOver());
			_DispOver2BmpId = rVR.getTextureIdFromName (_ItemSheet->getIconOver2());
			_PackedArmourColor = 0;
			_ArmourColorFromDB= false;
			_ArmourColorBmpOk= false;
			// if icon is an armour, and no over texture is given, build its name
			if (_ItemSheet->Family == ITEMFAMILY::ARMOR)
			{
				sint col = _ItemSheet->Color;
				if (col == -1) // user color wanted ?
				{
					col = getItemColor(); // read color from db
					// If the DB exist
					if(_UserColor)
						_ArmourColorFromDB= true;
				}
				updateArmourColor(col);
			}

			updateIconSize();

			// Icon Colors
			_IconBackColor= _ItemSheet->IconBackColor;
			_IconColor= _ItemSheet->IconColor;
			_IconOverColor= _ItemSheet->IconOverColor;
			_IconOver2Color= _ItemSheet->IconOver2Color;

			// Setup Quantity
			if (_UseQuantity)
			{
				// is this item stackable? more than one.
				_Stackable= _ItemSheet->Stackable;

				if (getStackable() > 1)
				{
					if (_ReadQuantityFromSheet)
					{
						_DispQuantity = getStackable();
					}
					else
					{
						// display the stack quantity
						_DispQuantity = _Quantity.getSInt32();
					}
				}
				else
					// do not display any number
					_DispQuantity = -1;
			}
			else _DispQuantity = -1;

			// Setup quality
			if(_DisplayItemQuality)
			{
				_DispQuality= _Quality.getSInt32();
			}
			else
			{
				_DispQuality= -1;
			}

			// special icon text
			if( _NeedSetup || _ItemSheet->getIconText() != _OptString )
			{
				// compute from OptString. Allow only 1 line and 4 chars
				_OptString= _ItemSheet->getIconText();
				// Display Top Left
				setupCharBitmaps(40, 1, 6, true);
			}

			// Special Item requirement
			updateItemCharacRequirement(_LastSheetId);
		}
		else
		{
			resetAllTexIDs();
		}
	}
	// must update dispNumber for good quantity/quality.
	else
	{
		// update quantity
		if (getStackable() > 1)
		{
			if (_ReadQuantityFromSheet)
			{
				_DispQuantity = getStackable();
			}
			else
			{
				_DispQuantity = _Quantity.getSInt32();
			}
		}

		// update quality. NB: if quality change, the must updateItemCharacRequirement
		if(_DisplayItemQuality)
		{
			sint32	newQuality= _Quality.getSInt32();
			if(newQuality!=_DispQuality)
			{
				_DispQuality= newQuality;
				updateItemCharacRequirement(_LastSheetId);
			}
		}
		else
		{
			_DispQuality= -1;
		}

		// update armour color (if USER_COLOR db change comes after SHEET change)
		if(_ArmourColorFromDB && _UserColor)
		{
			// If the DB has changed
			if(_UserColor->getValue32() != _ArmourColorIndex)
			{
				updateArmourColor((sint8)_UserColor->getValue32());
			}
		}
	}

	// at each frame, must test for grayed.
	if(_AutoGrayed)
	{
		if(_GrayedLink)
		{
			// gray if not 0
			_Grayed= _GrayedLink->getValue32()!=0;
		}
		else
			_Grayed= false;
	}

	// at each frame, must test for Redifyed (player carac does not met item requirement)
	_Useable = _PrerequisitValid.getBool();
/*
	_Useable= pIM->isItemCaracRequirementMet(_ItemCaracReqType, _ItemCaracReqValue);

	if (_Useable && _ItemSheet != NULL)
	{
		if (_ItemSheet->RequiredCharac != CHARACTERISTICS::Unknown && _ItemSheet->RequiredCharacLevel>0)
			_Useable= pIM->isItemCaracRequirementMet(_ItemSheet->RequiredCharac, _ItemSheet->RequiredCharacLevel);

		if (_Useable && _ItemSheet->RequiredSkill != SKILLS::unknown && _ItemSheet->RequiredSkillLevel > 0)
			_Useable = CSkillManager::getInstance()->checkBaseSkillMetRequirement(_ItemSheet->RequiredSkill, _ItemSheet->RequiredSkillLevel);
	}
*/
}


// ***************************************************************************
void		CDBCtrlSheet::updateItemCharacRequirement(sint32 sheet)
{
	CSheetId sheetId(sheet);
	CEntitySheet *pES = SheetMngr.get (sheetId);
	if ((pES != NULL) && (pES->type() == CEntitySheet::ITEM) && _Quality.getNodePtr())
	{
		CItemSheet *pIS = (CItemSheet*)pES;
		_ItemCaracReqType= CHARACTERISTICS::Unknown;
		_ItemCaracReqValue= 0;
		float	tmpVal;
		if(pIS->hasCharacRequirement(_Quality.getSInt32(), _ItemCaracReqType, tmpVal))
			_ItemCaracReqValue= (sint32)floor(tmpVal);
	}
}


// ***************************************************************************
void CDBCtrlSheet::setupMacro()
{
	if (!_NeedSetup) return;

	// compute from OptString
	setupCharBitmaps(26, 4, 5);

	_NeedSetup = false;

}


// ***************************************************************************
void CDBCtrlSheet::setupMission()
{
	sint32 currIcon = _SheetId.getSInt32();
	if (_LastSheetId != currIcon || _NeedSetup)
	{
		_LastSheetId = currIcon;
		_NeedSetup = false;

		CEntitySheet *pES = SheetMngr.get(CSheetId(currIcon));
		if (pES == NULL) return;
		if (pES->Type != CEntitySheet::MISSION_ICON) return;
		CMissionIconSheet *pMIS = (CMissionIconSheet*)pES;

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		_DispBackBmpId  = rVR.getTextureIdFromName(pMIS->MainIconBg);
		_DispSheetBmpId = rVR.getTextureIdFromName(pMIS->MainIconFg);

		// Display a '?' only if not 0.
		if ((_DispBackBmpId == -1) && (currIcon != 0))
		{
			_DispBackBmpId = rVR.getSystemTextureId(CViewRenderer::DefaultItemTexture);
		}
	}
}

// ***************************************************************************
void CDBCtrlSheet::setupGuildFlag ()
{
	// Find the guild name
	ucstring usGuildName;
	sint32 nGuildName = _SheetId.getSInt32();
	if (_LastSheetId != nGuildName || _NeedSetup)
	{
		CStringManagerClient *pSMC = CStringManagerClient::instance();
		bool bValid = pSMC->getDynString (nGuildName, usGuildName);
		if (bValid)
			_NeedSetup = false;
	}

	uint64 nGuildIcon;
	nGuildIcon = _Quality.getSInt64();


	_UseGuildIcon = ((nGuildIcon>>59)&1)!=0;		// 1 bits en pos 59
	if (_UseGuildIcon)
	{
		_GuildIcon = (uint32)(nGuildIcon&0xffffffff);	// 32 bits en pos 0
	}
	else
	{
		sint8 nGuildBack =		(sint8)(nGuildIcon&15);			// 4 bits en pos 0
		sint8 nGuildSymbol =	(sint8)((nGuildIcon>>4)&63);	// 6 bits en pos 4
		sint8 nGuildInvert =	(sint8)((nGuildIcon>>10)&1);	// 1 bit  en pos 10
		uint8 nGuildColBackR =	(uint8)((nGuildIcon>>11)&255);	// 8 bits en pos 11
		uint8 nGuildColBackG =	(uint8)((nGuildIcon>>19)&255);	// 8 bits en pos 19
		uint8 nGuildColBackB =	(uint8)((nGuildIcon>>27)&255);	// 8 bits en pos 27
		uint8 nGuildColSymbR =	(uint8)((nGuildIcon>>35)&255);	// 8 bits en pos 35
		uint8 nGuildColSymbG =	(uint8)((nGuildIcon>>43)&255);	// 8 bits en pos 43
		uint8 nGuildColSymbB =	(uint8)((nGuildIcon>>51)&255);	// 8 bits en pos 51

		sint8 nLastGuildBack =	(sint8)(_MacroID&15);		// 4 bits en pos 0
		sint8 nLastGuildSymbol = (sint8)((_MacroID>>4)&63);	// 6 bits en pos 4

		NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();

		if (_GuildMat.empty())
		{
			_GuildMat = Driver->createMaterial();
			_GuildMat.initUnlit ();

			// Alpha blend must use current alpha
			_GuildMat.setBlend (true);
			_GuildMat.setBlendFunc (UMaterial::one, UMaterial::invsrcalpha);

			// Stages 0
			_GuildMat.texEnvOpRGB (0, UMaterial::Modulate);
			_GuildMat.texEnvArg0RGB (0, UMaterial::Texture, UMaterial::SrcColor);
			_GuildMat.texEnvArg1RGB (0, UMaterial::Diffuse, UMaterial::SrcColor);
			_GuildMat.texEnvOpAlpha (0, UMaterial::Modulate);
			_GuildMat.texEnvArg0Alpha (0, UMaterial::Texture, UMaterial::SrcAlpha);
			_GuildMat.texEnvArg1Alpha (0, UMaterial::Diffuse, UMaterial::SrcAlpha);

			// This is alpha material, so don't zwrite
			_GuildMat.setZWrite (false);
			_GuildMat.setBlendFunc (UMaterial::srcalpha, UMaterial::invsrcalpha);
		}

		if (nGuildInvert == 0)
		{
			// Stages 1
			_GuildMat.texEnvOpRGB (1, UMaterial::Modulate);
			_GuildMat.texEnvArg0RGB (1, UMaterial::Texture, UMaterial::SrcColor);
			_GuildMat.texEnvArg1RGB (1, UMaterial::Previous, UMaterial::SrcColor);
		}
		else
		{
			// Stages 1
			_GuildMat.texEnvOpRGB (1, UMaterial::Add);
			_GuildMat.texEnvArg0RGB (1, UMaterial::Texture, UMaterial::InvSrcColor);
			_GuildMat.texEnvArg1RGB (1, UMaterial::Previous, UMaterial::SrcColor);
		}


		_SheetColor = CRGBA(nGuildColBackR,nGuildColBackG,nGuildColBackB);
		_IconColor = CRGBA(nGuildColSymbR,nGuildColSymbG,nGuildColSymbB);

		if (nLastGuildBack != nGuildBack)
		{
			if (_GuildBack != NULL)
				Driver->deleteTextureFile(_GuildBack);
			_GuildBack = NULL;
			if (nGuildBack > 0)
			{
				string txName = toString("Guild_Back_S_%02d.tga", nGuildBack-1);
				_GuildBack = Driver->createTextureFile (txName);
			}
		}

		if (nLastGuildSymbol != nGuildSymbol)
		{
			if (_GuildSymb != NULL)
				Driver->deleteTextureFile(_GuildSymb);
			_GuildSymb = NULL;
			if (nGuildSymbol > 0)
			{
				string txName = toString("Guild_Symbol_S_%02d.tga", nGuildSymbol-1);
				_GuildSymb = Driver->createTextureFile (txName);
			}
		}
		_GuildMat.setTexture(0, _GuildBack);
		_GuildMat.setTexture(1, _GuildSymb);

		_MacroID = (sint32)(nGuildIcon&1023); // on garde les 10 premiers bits
	}
}


// ***************************************************************************
void CDBCtrlSheet::setupDisplayAsSBrick(sint32 sheet, sint32 optSheet)
{
	// Setup with the param sheet
	CSBrickManager *pBM = CSBrickManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();

 	CSBrickSheet *pBR = pBM->getBrick (CSheetId(sheet));
	CSBrickSheet *pBROpt = pBM->getBrick (CSheetId(optSheet));
	if (pBR != NULL)
	{
		// Get Back and Over.
		_DispBackBmpId = rVR.getTextureIdFromName (pBR->getIconBack());
		_IconBackColor = pBR->IconBackColor;
		_DispOverBmpId = rVR.getTextureIdFromName (pBR->getIconOver());
		_IconOverColor = pBR->IconOverColor;

		// For phrase display, replace icon and over2 with optional brick
		if(pBROpt)
		{
			// get the correct icon
			bool iconOver2NotSuitableForActionDisplay;
			if ( pBM->getSabrinaCom().isMainDisplayIconInOverSlot( CSheetId(optSheet), iconOver2NotSuitableForActionDisplay ) )
			{
				_DispSheetBmpId = rVR.getTextureIdFromName (pBROpt->getIconOver());
				_IconColor = pBROpt->IconOverColor;
			}
			else
			{
				_DispSheetBmpId = rVR.getTextureIdFromName (pBROpt->getIcon());
				_IconColor = pBROpt->IconColor;
			}

			// get the correct over2
			if ( iconOver2NotSuitableForActionDisplay )
			{
				_DispOver2BmpId = rVR.getTextureIdFromName (pBR->getIconOver2());
				_IconOver2Color = pBR->IconOver2Color;
			}
			else
			{
				_DispOver2BmpId = rVR.getTextureIdFromName (pBROpt->getIconOver2());
				_IconOver2Color = pBROpt->IconOver2Color;
			}
		}
		else
		{
			_DispSheetBmpId = rVR.getTextureIdFromName (pBR->getIcon());
			_IconColor = pBR->IconColor;
			_DispOver2BmpId = rVR.getTextureIdFromName (pBR->getIconOver2());
			_IconOver2Color = pBR->IconOver2Color;
		}

		// if file not found or empty, replace by default icon
		if( _DispSheetBmpId == -1)
			_DispSheetBmpId = rVR.getSystemTextureId(CViewRenderer::DefaultBrickTexture);
		updateIconSize();
		_DispLevel = pBR->Level;
		_MustDisplayLevel = pBR->mustDisplayLevel();
	}
	else
	{
		resetAllTexIDs();
	}
}

// ***************************************************************************
void CDBCtrlSheet::setupSBrick ()
{
	sint32 sheet = (sint32)_SheetId.getSInt32();

	// If this is the same sheet id so no need to resetup
	if (_LastSheetId != sheet || _NeedSetup)
	{
		_LastSheetId = sheet;
		setupDisplayAsSBrick(sheet);

		// Reseted.
		_NeedSetup= false;
	}

	// SBrick support auto grayed. test each frame
	if(_AutoGrayed)
	{
		if(_GrayedLink)
		{
			// gray if not 0
			_Grayed= _GrayedLink->getValue32()!=0;
			// Reded, if 2
			_Useable= _GrayedLink->getValue32()!=2;
		}
		else
			_Grayed= false;
	}
}

// ***************************************************************************
void CDBCtrlSheet::setupDisplayAsPhrase(const std::vector<NLMISC::CSheetId> &bricks, const ucstring &phraseName)
{
	CSBrickManager		*pBM = CSBrickManager::getInstance();

	// Get the best SBrick to display.
	CSheetId	rootBrickSheetId= bricks[0];

	{
		CSheetId	bestBrickSheetId= pBM->getSabrinaCom().getPhraseBestDisplayBrick(bricks);
		setupDisplayAsSBrick (rootBrickSheetId.asInt(), bestBrickSheetId.asInt() );
	}

	// not so beautiful to display .sphrase name in progression, and in botchat
	if(_ActualType==SheetType_SPhraseId)
	{
		// Compute the text from the phrase only if needed
//		string	iconName= phraseName.toString();
		string	iconName= phraseName.toUtf8();
		if( _NeedSetup || iconName != _OptString )
		{
			// recompute text
			_OptString= iconName;
			// compute from OptString. Allow only 1 line and 5 chars
			setupCharBitmaps(26, 1, 5);
		}
	}
}

// ***************************************************************************
void CDBCtrlSheet::setupSPhrase()
{
	sint32 sheet = (sint32)_SheetId.getSInt32();

	// If this is the same sheet id so no need to resetup
	if (_LastSheetId != sheet || _NeedSetup)
	{
		_LastSheetId = sheet;
		CSPhraseSheet *pSPS = dynamic_cast<CSPhraseSheet*>(SheetMngr.get(CSheetId(sheet)));
		if (pSPS && !pSPS->Bricks.empty())
		{
			const ucstring phraseName(STRING_MANAGER::CStringManagerClient::getSPhraseLocalizedName(CSheetId(sheet)));
			setupDisplayAsPhrase(pSPS->Bricks, phraseName);
		}
		else
		{
			resetAllTexIDs();
		}

		// Reseted.
		_NeedSetup= false;
	}

	// SPhrase support auto grayed. test each frame
	if(_AutoGrayed)
	{
		if(_GrayedLink)
		{
			// gray if not 0
			_Grayed= _GrayedLink->getValue32()!=0;
			// Reded, if 2
			_Useable= _GrayedLink->getValue32()!=2;
		}
		else
			_Grayed= false;
	}
}

// ***************************************************************************
void CDBCtrlSheet::setupSPhraseId ()
{
	// get the phrase id
	sint32	phraseId = getSPhraseId();

	// get the phrase Data version, to check if it had changed.
	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	sint32	phraseVersion= pPM->getPhraseVersion(phraseId);

	// If this is the same phrase id and phrase version no need to resetup
	if (_NeedSetup || _LastSheetId != phraseId || _LastPhraseVersion!= phraseVersion)
	{
		_LastSheetId= phraseId;
		_LastPhraseVersion= phraseVersion;

		// Empty Slot?
		if(phraseId==0)
		{
			resetAllTexIDs();
		}
		else
		{
			// Get the SPhrase.
			const CSPhraseCom	&phrase= pPM->getPhrase(phraseId);
			if(phrase.empty())
			{
				resetAllTexIDs();
			}
			else
			{
				setupDisplayAsPhrase(phrase.Bricks, phrase.Name);
			}
		}

		// Reseted.
		_NeedSetup= false;
	}
}

// ***************************************************************************
void CDBCtrlSheet::setupOutpostBuilding()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	sint32 sheet = _SheetId.getSInt32();
	// If this is the same sheet, need to resetup
	if (_LastSheetId != sheet || _NeedSetup)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		_NeedSetup= false;
		_LastSheetId = sheet;
		CSheetId sheetId(sheet);
		CEntitySheet *pES = SheetMngr.get (sheetId);
		if ((pES != NULL) && (pES->type() == CEntitySheet::OUTPOST_BUILDING))
		{
			COutpostBuildingSheet *pOBSheet = (COutpostBuildingSheet*)pES;

			_DisplayItemQuality = false;

			_DispSheetBmpId = rVR.getTextureIdFromName (pOBSheet->getIconMain());
			// if file not found or empty, replace by default icon
			if( _DispSheetBmpId == -1)
				_DispSheetBmpId = rVR.getSystemTextureId(CViewRenderer::DefaultItemTexture);
			if(_ForceItemBackGroundGeneric)
				_DispBackBmpId = rVR.getTextureIdFromName ("BK_generic.tga");
			else
				_DispBackBmpId = rVR.getTextureIdFromName (pOBSheet->getIconBack());
			_DispOverBmpId = rVR.getTextureIdFromName (pOBSheet->getIconOver());
			_DispOver2BmpId = -1;
			_PackedArmourColor = 0;
			_ArmourColorFromDB= false;
			_ArmourColorBmpOk= false;
			updateIconSize();

			_DispQuantity = -1;
			_DispQuality = -1;

			// special icon text
			if (pOBSheet->getIconText() != _OptString)
			{
				// compute from OptString. Allow only 1 line and 4 chars
				_OptString= pOBSheet->getIconText();
				// Display Top Left
				setupCharBitmaps(40, 1, 6, true);
			}
		}
		else
		{
			resetAllTexIDs();
		}
	}
	// must update dispNumber for good quantity/quality.
	else
	{
		_DispQuality = -1;
	}

	// at each frame, must test for grayed.
	if(_AutoGrayed)
	{
		if(_GrayedLink)
		{
			// gray if not 0
			_Grayed= _GrayedLink->getValue32()!=0;
		}
		else
			_Grayed= false;
	}
}

// ***************************************************************************
sint32	CDBCtrlSheet::getSPhraseId() const
{
	if(_SheetId.getNodePtr())
		return (sint32)_SheetId.getSInt32();
	else
		return 0;
}

// ***************************************************************************
void CDBCtrlSheet::resetCharBitmaps()
{
	_CharBitmaps.clear();
}

// ***************************************************************************
void CDBCtrlSheet::setupCharBitmaps(sint32 maxW, sint32 maxLine, sint32 maxWChar, bool topDown)
{
	// Use the optString for the Macro name
	_OptString = strlwr(_OptString);
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();

	_CharBitmaps.clear();

	if(maxLine<=0)
		return;

	uint h = rVR.getTypoTextureH('a');
	sint lineNb = 0;
	sint curLineSize = 0;
	uint i;
	uint xChar= 0;
	for (i = 0; i < _OptString.size(); ++i)
	{
		char c = _OptString[i];
		sint32 w = rVR.getTypoTextureW(c);
		if ((curLineSize + w) > maxW || (sint32)xChar>=maxWChar)
		{
			lineNb ++;
			if (lineNb == maxLine) break;
			curLineSize = 0;
			xChar = 0;
		}
		sint32 id = rVR.getTypoTextureId(c);
		if (id != -1)
		{
			CCharBitmap		bmp;
			bmp.X= curLineSize;
			bmp.Y= lineNb;
			bmp.Id= id;
			_CharBitmaps.push_back(bmp);
		}
		curLineSize += w;
		++xChar;
	}

	if (lineNb == maxLine) lineNb = maxLine-1;

	for (i = 0; i < _CharBitmaps.size(); ++i)
	{
		_CharBitmaps[i].Y = (lineNb - _CharBitmaps[i].Y)*h;
	}

	// if topDown, revert Y
	if (topDown)
	{
		for (i = 0; i < _CharBitmaps.size(); ++i)
		{
			_CharBitmaps[i].Y = _IconH - _CharBitmaps[i].Y - h;
		}
	}
}

// ***************************************************************************
void CDBCtrlSheet::displayCharBitmaps(sint32 rdrLayer, sint32 x, sint32 y, CRGBA color)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();

	for (uint i = 0; i < _CharBitmaps.size(); ++i)
	{
		rVR.draw11RotFlipBitmap (rdrLayer, x+_CharBitmaps[i].X, y+_CharBitmaps[i].Y, 0, false,
								_CharBitmaps[i].Id, color);
	}
}


// ***************************************************************************
void CDBCtrlSheet::draw()
{
	H_AUTO( RZ_Interface_CDBCtrlSheet_draw )

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	CRGBA color = CRGBA(255,255,255,255);

	if (_Type != SheetType_Macro)
	{
		if (_LastSheetId != _SheetId.getSInt32())
		{
			updateActualType();
			_LastSheetId = _SheetId.getSInt32();
			_NeedSetup= true;
		}
	}

	// Manage over for brick
	if( _BrickOverable && (isMacro() || isSBrickOrSPhraseId() || isSPhrase()) )
	{
		const vector<CCtrlBase*> &rVB = CWidgetManager::getInstance()->getCtrlsUnderPointer ();
		uint32 i;
		for (i = 0; i < rVB.size(); ++i)
		if (rVB[i] == this)
		{
			_Over = true;
			break;
		}
		if (i == rVB.size())
			_Over = false;
	}


	// Display slot
	if (_DrawSlot)
		rVR.draw11RotFlipBitmap (_RenderLayer, _XReal, _YReal, 0, false, _DispSlotBmpId, CWidgetManager::getInstance()->getGlobalColorForContent());

	// Drag'N'Drop : display the selected slot bitmap if this slot accept the currently dragged element
	_CanDrop = false;
	if (_AHOnCanDrop != NULL)
	if ((CWidgetManager::getInstance()->getCapturePointerLeft() != NULL) && (CWidgetManager::getInstance()->getCapturePointerLeft() != this))
	{
		if ((CWidgetManager::getInstance()->getPointer()->getX() >= _XReal) &&
			(CWidgetManager::getInstance()->getPointer()->getX() < (_XReal + _WReal))&&
			(CWidgetManager::getInstance()->getPointer()->getY() > _YReal) &&
			(CWidgetManager::getInstance()->getPointer()->getY() <= (_YReal+ _HReal)))
		if (CWidgetManager::getInstance()->getCurrentWindowUnder() == CWidgetManager::getInstance()->getWindow(this))
		{
			CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCapturePointerLeft());
			if ((pCSSrc != NULL) && pCSSrc->isDragged())
			{
				string params = string("src=") + pCSSrc->getId();
				if (!_AHCanDropParams.empty())
				{
					if ( CAHManager::getInstance()->getAHName(_AHOnCanDrop) == "lua")
					{
						params = _AHCanDropParams;
						strFindReplace(params, "%src", pCSSrc->getId());
					}
					else
					{
						string sTmp = _AHCanDropParams;
						params = sTmp + "|" + params;
					}
				}
				CAHManager::getInstance()->runActionHandler (_AHOnCanDrop, this, params);
			}
		}
	}

	drawSheet (_XReal+1, _YReal+1, isDragged() );

	// Draw the selection after the sheet. Important for spells because selection border is same size as spell square
	if (_CanDrop)
	{
		// decal layer because must drawn after Items/Brick in DXTC
		rVR.draw11RotFlipBitmap (_RenderLayer+1, _XReal, _YReal, 0, false, _DispSelSlotId, CWidgetManager::getInstance()->getGlobalColorForContent());
	}

	if (_RegenTickRange.EndTick != _RegenTickRange.StartTick)
	{
		if (!_LastSheetId)
		{
			_RegenTickRange = CTickRange();
		}
		else
		{
			// Render regen texture in overlay, in clock wise order
			float amount = (LastGameCycle - _RegenTickRange.StartTick) / float(_RegenTickRange.EndTick - _RegenTickRange.StartTick);
			clamp(amount, 0.f, 1.f);

			sint32 texWidth;
			sint32 texHeight;
			uint32 frontTex = rVR.getSystemTextureId(CViewRenderer::RegenTexture);
			uint32 backTex = rVR.getSystemTextureId(CViewRenderer::RegenBackTexture);

			rVR.getTextureSizeFromId(frontTex, texWidth, texHeight);
			CQuadUV regenTris[5];
			uint numTris = buildPie(regenTris, 0.f, amount, texWidth);	
			nlassert(numTris <= sizeofarray(regenTris));
			for (uint tri = 0; tri < numTris; ++tri)
			{
				rVR.drawQuad(_RenderLayer + 1, regenTris[tri], frontTex, CRGBA::White, false);
			}
			numTris = buildPie(regenTris, amount, 1.f, texWidth);	
			nlassert(numTris <= sizeofarray(regenTris));
			for (uint tri = 0; tri < numTris; ++tri)
			{
				rVR.drawQuad(_RenderLayer + 1, regenTris[tri], backTex, CRGBA::White, false);
			}
		}
	}

	if (_NotifyAnimEndTime > T1)
	{
		if (!_LastSheetId)
		{
			_NotifyAnimEndTime = 0;
		}
		else
		{
			float animProgress = 1.f - float(_NotifyAnimEndTime - T1) / float(NOTIFY_ANIM_MS_DURATION);
			sint32 texId = rVR.getSystemTextureId(CViewRenderer::GlowStarTexture);
			if (texId != -1)
			{
				sint32 texWidth, texHeight;
				rVR.getTextureSizeFromId(texId, texWidth, texHeight);			
				const float freq0 = 1.f;
				const float phase0 = 0.f;
				const float freq1 = -1.f;
				const float phase1 = 0.f;
				float scale = sqrtf(1.f - animProgress);
				drawRotatedQuad(rVR, float(NLMISC::Pi) * animProgress * freq0 + phase0, scale, _RenderLayer + 3, (uint32)texId, texWidth, texHeight);
				drawRotatedQuad(rVR, float(NLMISC::Pi) * animProgress * freq1 + phase1, scale, _RenderLayer + 3, (uint32)texId, texWidth, texHeight);
			}
		}
	}
}


// ----------------------------------------------------------------------------
void CDBCtrlSheet::drawRotatedQuad(CViewRenderer &vr, float angle, float scale, uint renderLayer, uint32 texId, sint32 texWidth, sint32 texHeight)
{
	NLMISC::CQuadUV quv;	
	float cosA =  cosf(angle);
	float sinA =  sinf(angle);
	//
	quv.V0.set(_XReal + 0.5f * _WReal + 0.5f * scale * texWidth * (- cosA + sinA), 
			   _YReal + 0.5f * _HReal + 0.5f * scale * texHeight * (- sinA - cosA), 0.5f);
	//
	quv.V1.set(_XReal + 0.5f * _WReal + 0.5f * scale * texWidth * (cosA + sinA), 
			   _YReal + 0.5f * _HReal + 0.5f * scale * texHeight * (sinA - cosA), 0.5f);
	//
	quv.V2.set(_XReal + 0.5f * _WReal + 0.5f * scale * texWidth * (cosA - sinA), 
			   _YReal + 0.5f * _HReal + 0.5f * scale * texHeight * (sinA + cosA), 0.5f);	
	//
	quv.V3.set(_XReal + 0.5f * _WReal + 0.5f * scale * texWidth * (- cosA - sinA), 
			   _YReal + 0.5f * _HReal + 0.5f * scale * texHeight * (- sinA + cosA), 0.5f);
	//
	quv.Uv0.set(0.f, 0.f);
	quv.Uv1.set(1.f, 0.f);
	quv.Uv2.set(1.f, 1.f);
	quv.Uv3.set(0.f, 1.f);
	//
	vr.drawQuad(renderLayer, quv, texId, CRGBA::White, true);
}


// ----------------------------------------------------------------------------
inline void CDBCtrlSheet::uvToScreen(float x, float y, CVector &screenPos, uint texSize) const
{
	screenPos.set(_XReal + texSize * x, _YReal + texSize * (1.f - y), 0.5f);	
}


// ----------------------------------------------------------------------------
void CDBCtrlSheet::buildPieCorner(float angle, CUV &uv, CVector &pos, uint texSize) const
{	
	float radAngle = angle * 2.f * float(NLMISC::Pi);
	// angle origin is at 12'o'clock
	float x = cosf(0.5f * float(NLMISC::Pi) - radAngle);
	float y = sinf(0.5f * float(NLMISC::Pi) - radAngle);
	// project on sides
	if (fabsf(y) > fabsf(x))
	{
		if (y > 0.f)
		{
			// top
			x /= y;
			y = 1.f;
		}
		else
		{
			// bottom
			x /= -y;
			y = -1.f;
		}
	}
	else
	{
		if (x > 0.f)
		{
			y /= x;
			x = 1.f;
		}
		else
		{
			y /= -x;
			x = -1.f;
		}
	}	
	// remap to unit quad
	// (well we could have worked with tan() too, I find it simpler this way ....)
	uv.set(0.5f * x + 0.5f, 0.5f - 0.5f * y);
	uvToScreen(uv.U, uv.V, pos, texSize);		
}

// ----------------------------------------------------------------------------
uint CDBCtrlSheet::buildPie(CQuadUV *triPtr, float startAngle, float endAngle, uint texSize)
{			
	static volatile bool exit1 = false;
	nlassert(startAngle <= endAngle);
	const sint32 factor = 65536; 
	const float invFactor = 1.f / factor; 
	sint32 iCurr = (uint32) (startAngle * factor) ;
	sint32 iEnd = (uint32) (endAngle * factor);
	clamp(iCurr, 0, factor);
	clamp(iEnd, 0, factor);
	uint triCount = 0;
	CVector quadCenter;
	uvToScreen(0.5f, 0.5f, quadCenter, texSize);	

	while (iCurr != iEnd)
	{
		sint32 iNext = iCurr + (factor / 4);
		iNext -= ((iNext - factor / 8) % (factor / 4)); // snap to nearest corner		
		iNext = std::min(iNext, iEnd);	
		// well, not really a quad, but we don't have yet simple triangles rendering in the ui
		triPtr->Uv0.set(0.5f, 0.5f);
		triPtr->V0 = quadCenter;
		buildPieCorner(iCurr * invFactor, triPtr->Uv3, triPtr->V3, texSize);
		buildPieCorner(iNext * invFactor, triPtr->Uv2, triPtr->V2, texSize);
		// turn a quad into a tri ... :/
		triPtr->Uv1 = triPtr->Uv0;
		triPtr->V1 = triPtr->V0;
		//
		iCurr = iNext;
		//
		++ triPtr;
		++ triCount;
	}
	return triCount;
}

// ----------------------------------------------------------------------------
// Modulate only if iconColor is not White.
static inline CRGBA	fastMulRGB(CRGBA sheetColor, CRGBA iconColor)
{
	// NB: the test here is more to avoid loss of precision when icon value is 255,255,255
	if(iconColor==CRGBA::White)
	{
		return sheetColor;
	}
	else
	{
		CRGBA	ret= sheetColor;		// copy alpha from sheetColor
		ret.modulateFromColorRGBOnly(sheetColor, iconColor);
		return ret;
	}
}

// ----------------------------------------------------------------------------
void CDBCtrlSheet::drawSheet (sint32 x, sint32 y, bool draging, bool showSelectionBorder /*= true*/)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	// The sheet is the slot-2
	sint32	wSheet= _WReal-2;
	sint32	hSheet= _HReal-2;

	// the sheet color is modulated by GlobalAlpha, but not by global RGB
	CRGBA	curSheetColor= _SheetColor;
	curSheetColor.A= ( (CWidgetManager::getInstance()->getGlobalColorForContent().A+1) * _SheetColor.A )>>8;
	// The "disp with no sheet" case is a bit different
	CRGBA	curNoSheetColor;
	if(_InterfaceColor)
		curNoSheetColor= CWidgetManager::getInstance()->getGlobalColorForContent();
	else
		curNoSheetColor= CRGBA(255,255,255, CWidgetManager::getInstance()->getGlobalColorForContent().A);

	// The gray color
	CRGBA	grayColor= CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlSheetGrayColor).getValColor();
	CRGBA	redifyColor= CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlSheetRedifyColor).getValColor();

	// The color of the number.
	CRGBA	numberColor;
	if(!_Useable)
	{
		// do not modulate color for redifyed color
		numberColor= redifyColor;
		numberColor.A= (uint8)(((uint32)redifyColor.A * (CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
	}
	else if(_Grayed)
		numberColor.modulateFromColor(grayColor, CWidgetManager::getInstance()->getGlobalColorForContent());
	else
		numberColor= CWidgetManager::getInstance()->getGlobalColorForContent();

	// Different draws according to Sheet Type.
	switch (_ActualType)
	{
		case CCtrlSheetInfo::SheetType_Pact:
			setupPact();
			rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispBackBmpId, curSheetColor);
			rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispSheetBmpId, curSheetColor);
			if (_DispQuality != -1)
			{
				// For pact sheets, the quality gives the level of the sheet
				drawNumber(x+1, y+1, wSheet, hSheet, numberColor, _DispQuality+1);
			}
		break;
		case CCtrlSheetInfo::SheetType_Skill:
			rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, rVR.getSystemTextureId(CViewRenderer::SkillTexture), curSheetColor);
		break;
		case CCtrlSheetInfo::SheetType_Item:
			setupItem();

			if(!_Useable)
				curSheetColor.modulateFromColor(redifyColor, curSheetColor);
			else if (_Grayed || _ItemWeared || _ItemBeastGrayed)
				curSheetColor.modulateFromColor(grayColor, curSheetColor);

			// if worned display an icon over the item's icon (a red cross for exemple)
			if (_Worned.getBool())
			{
				rVR.draw11RotFlipBitmap (_RenderLayer+1, x, y, 0, false, rVR.getSystemTextureId(CViewRenderer::ItemWornedTexture), curSheetColor);
			}

			// Display
			if ((_DispSheetBmpId == -1) || (draging && !_DuplicateOnDrag))
			{
				if(_DispNoSheetBmpId!=-1)
				{
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispNoSheetBmpId, curNoSheetColor);
				}
			}
			else
			{
				// Background and icon
				rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispBackBmpId, fastMulRGB(curSheetColor, _IconBackColor));
				rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispSheetBmpId, fastMulRGB(curSheetColor, _IconColor));

				// if an armour, draw colored part
				if (_PackedArmourColor != 0)
				{
					CRGBA armourCol;
					armourCol.setPacked(_PackedArmourColor);
					armourCol.modulateFromColor(curSheetColor, armourCol);
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispOverBmpId, armourCol);
					// decal layer because must drawn after Items/Brick in DXTC
					// NB: use OverColor, not Over2Color here. Because of hack in updateArmourColor()
					rVR.draw11RotFlipBitmap (_RenderLayer+1, x, y, 0, false, _DispOver2BmpId, fastMulRGB(curSheetColor, _IconOverColor));
				}
				else
				{
					// decal layer because must drawn after Items/Brick in DXTC
					rVR.draw11RotFlipBitmap (_RenderLayer+1, x, y, 0, false, _DispOverBmpId, fastMulRGB(curSheetColor, _IconOverColor));
					rVR.draw11RotFlipBitmap (_RenderLayer+1, x, y, 0, false, _DispOver2BmpId, fastMulRGB(curSheetColor, _IconOver2Color));
				}

				// Draw Quality. -1 for lookandfeel. Draw it with global color
				if (_DispQuality != -1)
				{
					drawNumber(x-1,y+1,wSheet, hSheet, numberColor, _DispQuality);
				}
				// Draw Quantity
				if (_UseQuantity && _DispQuantity>-1)
				{
					sint32	crossId= rVR.getSystemTextureId(CViewRenderer::QuantityCrossTexture);
					sint32	crossW= rVR.getSystemTextureW(CViewRenderer::QuantityCrossTexture);
					// +1 for lookandfeel
					// draw the "x" bitmap. decal layer because must drawn after Items/Brick in DXTC
					rVR.draw11RotFlipBitmap (_RenderLayer+2, x+1, y+1, 0, false, crossId, curSheetColor);
					// draw the number next to it

					sint32 quantity = _DispQuantity;
					if (getLockValuePtr())
					{
						quantity -= getLockValuePtr()->getValue32();
					}
					drawNumber(x+1+crossW, y+1, wSheet, hSheet, curSheetColor, quantity, false);
				}
				// Is the item enchanted ?
				sint32 enchant = _Enchant.getSInt32();
				if (enchant > 0)
				{
					// Yes draw the additionnal bitmap and the charge (number of enchanted spell we can launch with the enchanted item)
					enchant--;
					rVR.draw11RotFlipBitmap (_RenderLayer+2, x, y, 0, false, rVR.getSystemTextureId(CViewRenderer::ItemEnchantedTexture), curSheetColor);
					drawNumber(x+1, y-2+hSheet-rVR.getFigurTextureH(), wSheet, hSheet, numberColor, enchant, false);
				}

				// if a raw material for example, must add special icon text.
				displayCharBitmaps(_RenderLayer+2, x, y, curSheetColor);

				// Add the lock overlay if needed
				if (getLockedByOwner())
				{
					rVR.draw11RotFlipBitmap (_RenderLayer+1, x - 2, y + 8, 0, false, rVR.getSystemTextureId(CViewRenderer::ItemLockedByOwnerTexture), curSheetColor);
				}
			}
			break;
		// Action
		case CCtrlSheetInfo::SheetType_Macro:
			setupMacro();
			if ( ((_DispBackBmpId == -1) && (_DispOverBmpId == -1) && (_DispOver2BmpId == -1)) ||
				 (draging && isShortCut()) )
			{
				if(_DispNoSheetBmpId != -1)
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispNoSheetBmpId, curNoSheetColor);
			}
			else
			{
				if (_DispBackBmpId != -1)
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispBackBmpId, curSheetColor);
				if (_DispOverBmpId != -1)
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispOverBmpId, curSheetColor);
				if (_DispOver2BmpId != -1)
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispOver2BmpId, curSheetColor);

				// if the pointer is over the button
				if (_Over)
					// Draw -1,-1 because the slot over is 26x26
					rVR.draw11RotFlipBitmap (_RenderLayer+1,  x-1, y-1, 0, false, _TextureIdOver, curSheetColor );
			}
			// Display Macro Text
			{
				displayCharBitmaps(_RenderLayer+2, x, y, curSheetColor);
			}
			break;
		case CCtrlSheetInfo::SheetType_GuildFlag:
			setupGuildFlag();
			if (_UseGuildIcon)
			{
				sint32 guildFlagBmpId = -1;
				CFactionSheet* guildIconSheet = NULL;
				if (_GuildIcon!=NLMISC::CSheetId::Unknown)
					guildIconSheet = dynamic_cast<CFactionSheet*>(SheetMngr.get(_GuildIcon));
				if (guildIconSheet!=NULL && !guildIconSheet->Icon.empty())
					guildFlagBmpId = rVR.getTextureIdFromName(guildIconSheet->Icon);
			//	else
			//		guildFlagBmpId = rVR.getTextureIdFromName("asc_unknown.tga");
				if (guildFlagBmpId != -1)
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, guildFlagBmpId, curSheetColor);
			}
			else if (_MacroID == 0)
			{
				if(_DispNoSheetBmpId != -1)
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispNoSheetBmpId, curNoSheetColor);
			}
			else
			{
				CRGBA col;
				col = _SheetColor;
				col.modulateFromui (col, curSheetColor.A);
				col.A = curSheetColor.A;

				rVR.drawCustom (x, y, 32, 32, CUV(0,0), CUV(0.5f,1), CUV(0,0), CUV(1,1), col, _GuildMat);

				col= _IconColor;
				col.modulateFromui (col, curSheetColor.A);
				col.A = curSheetColor.A;

				rVR.drawCustom (x, y, 32, 32, CUV(0.5f,0), CUV(1,1), CUV(0,0), CUV(1,1), col, _GuildMat);

				// if the pointer is over the button
				if (_Over)
					// Draw -1,-1 because the slot over is 26x26
					rVR.draw11RotFlipBitmap (_RenderLayer+2,  x-1, y-1, 0, false, _TextureIdOver, curSheetColor );
			}
			break;
		case CCtrlSheetInfo::SheetType_Mission:
			setupMission();
			rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispBackBmpId, curSheetColor);
			rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispSheetBmpId, curSheetColor);
			break;
		// same drawing for sbrick and sphrase
		case CCtrlSheetInfo::SheetType_SBrick:
		case CCtrlSheetInfo::SheetType_SPhraseId:
		case CCtrlSheetInfo::SheetType_SPhrase:
		{
			switch(_ActualType)
			{
				case SheetType_SBrick : setupSBrick(); break;
				case SheetType_SPhraseId : setupSPhraseId(); break;
				case SheetType_SPhrase : setupSPhrase(); break;
				default: nlassert(true); break;
			}
				
			
			bool showOutOfRangeSymbol = false;
			bool forceGrayed = false;

			// for phrases, display special icon if a target is selected and if it is too far
			if (_ActualType == CCtrlSheetInfo::SheetType_SPhraseId)
			{
				sint32	phraseId = getSPhraseId();
				if (phraseId)
				{
					CSPhraseManager	*pPM= CSPhraseManager::getInstance();
					const CSPhraseCom	&phrase= pPM->getPhrase(phraseId);
					// get the phrase Data version, to check if it had changed.					
					uint32 totalActionMalus = pPM->getTotalActionMalus(phrase);
					uint8 targetSlot = UserEntity->targetSlot();
					if (targetSlot < CLFECOMMON::INVALID_SLOT)
					{
						CEntityCL *target = EntitiesMngr.entity(targetSlot);
						if (target && UserEntity)
						{							
							double dist2 = (target->pos() - UserEntity->pos()).sqrnorm();							
							CSBrickManager	*pBM= CSBrickManager::getInstance();
							CSBrickSheet	*rootBrick= NULL;
							if(phrase.Bricks.size())
								rootBrick= pBM->getBrick(phrase.Bricks[0]);
							//
							if(rootBrick && rootBrick->isMagic())
							{
								// offensive magic can be used against an ennemy only
								if(rootBrick->ActionNature == ACTNATURE::OFFENSIVE_MAGIC)
								{
									forceGrayed = !target->isEnemy();
								}
								else
								{

									bool isPrimalMagic = false;
									uint	i;
									for(i=0;i<phrase.Bricks.size();i++)
									{
										CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
										if(brick && brick->BrickFamily == BRICK_FAMILIES::BMSTEA)
										{
											isPrimalMagic = true;
											break;
										}
									}
									
									if (!isPrimalMagic)
									{
										forceGrayed = !(target->isPlayer() && target->isFriend());
									}
									else
									{
										forceGrayed = false;
									}
									
								}
			
								if (!forceGrayed)
								{
									sint phraseRange;
									sint rangeMalus;
									pPM->getPhraseMagicRange(phrase, totalActionMalus, phraseRange, rangeMalus);
									double rangeDist = (float) (phraseRange + rangeMalus);							
									if (phraseRange != 0) // if range is '0' then it is a 'self' action
									{
										rangeDist += 0.5 + target->getSheetScale() * target->getSheetColRadius(); // player radius							
										if (dist2 > rangeDist * rangeDist)
										{
											showOutOfRangeSymbol = true;										
										}
									}
								}
							}
							else if(rootBrick && rootBrick->isCombat())
							{
								forceGrayed = target->isNeutral();
							}
							else if(rootBrick && rootBrick->isSpecialPower())
							{
								bool isTaunt = false;

								uint	maxRange= 255;
								uint	i;
								for(i=0;i<phrase.Bricks.size();i++)
								{
									CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
									if (brick)
									{
										maxRange= min(maxRange, (uint)brick->MaxRange);
										if(brick->BrickFamily == BRICK_FAMILIES::BSFMA)
										{

											forceGrayed = target->isNeutral();
											isTaunt = true;
										}
									}
								}

								if (isTaunt && !forceGrayed)
								{
										double rangeDist = (float) maxRange;							
										rangeDist += 0.5 + target->getSheetScale() * target->getSheetColRadius(); // player radius							
										if (dist2 > rangeDist * rangeDist)
										{
											showOutOfRangeSymbol = true;										
										}
								}
							}
						}
					}
					// in every cases, test against current gauges to see if the action is possible
					if (UserEntity)
					{
						sint cost;
						sint costMalus;
						CBarManager &bm = *CBarManager::getInstance();
						//
						pPM->getPhraseHpCost(phrase, totalActionMalus, cost, costMalus);
						if (cost > bm.getUserScore(SCORES::hit_points)) forceGrayed = true;
						//
						pPM->getPhraseSapCost(phrase, totalActionMalus, cost, costMalus);
						if (cost > bm.getUserScore(SCORES::sap)) forceGrayed = true;
						//
						pPM->getPhraseStaCost(phrase, totalActionMalus, cost, costMalus);
						if (cost > bm.getUserScore(SCORES::stamina)) forceGrayed = true;
						//
						pPM->getPhraseFocusCost(phrase, totalActionMalus, cost, costMalus);
						if (cost > bm.getUserScore(SCORES::focus)) forceGrayed = true;
					}
				}
			}


			if(!_Useable)
				curSheetColor.modulateFromColor(redifyColor, curSheetColor);
			else if(_Grayed || forceGrayed)
				curSheetColor.modulateFromColor(grayColor, curSheetColor);

			// display even if draging, fut for memory: act like shortcut
			if (_DispSheetBmpId == -1 || (draging && isShortCut()))
			{
				if(_DispNoSheetBmpId!=-1)
				{
					rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispNoSheetBmpId, curNoSheetColor);
				}
			}
			else
			{
				// center the brick (because may be in an item slot when sheetType is auto)
				sint32 px = x + ((_W - 2) / 2) - (BrickSheetWidth / 2);
				sint32 py = y + ((_H - 2) / 2) - (BrickSheetHeight / 2);

				// The brick must drawn in 24x24 even if bitmap is an item-like (MP knowledge bricks for example)
				rVR.drawRotFlipBitmap (_RenderLayer, px, py, BrickSheetWidth, BrickSheetHeight, 0, false, _DispBackBmpId, fastMulRGB(curSheetColor, _IconBackColor));
				rVR.drawRotFlipBitmap (_RenderLayer, px, py, BrickSheetWidth, BrickSheetHeight, 0, false, _DispSheetBmpId, fastMulRGB(curSheetColor, _IconColor));
				// decal layer because must drawn after Items/Brick in DXTC
				rVR.drawRotFlipBitmap (_RenderLayer+1, px, py, BrickSheetWidth, BrickSheetHeight, 0, false, _DispOverBmpId, fastMulRGB(curSheetColor, _IconOverColor));
				rVR.drawRotFlipBitmap (_RenderLayer+1, px, py, BrickSheetWidth, BrickSheetHeight, 0, false, _DispOver2BmpId, fastMulRGB(curSheetColor, _IconOver2Color));
				// Draw Quality. -1 for lookandfeel.
				if( _ActualType == CCtrlSheetInfo::SheetType_SBrick )
				{
					if (_UseQuality && _MustDisplayLevel) drawNumber(px-1,py+1,BrickSheetWidth, BrickSheetHeight, curSheetColor, _DispLevel);
				}
				else
				{
					// Display SPhrase Icon Name
					displayCharBitmaps(_RenderLayer+1, x, y, curSheetColor);
				}
				// if the pointer is over the button
				if (_Over)
					// Draw -1,-1 because the slot over is 26x26
					rVR.draw11RotFlipBitmap (_RenderLayer+1,  x-1, y-1, 0, false, _TextureIdOver, curSheetColor );			
			}

			if (showOutOfRangeSymbol)
			{				
				rVR.draw11RotFlipBitmap (_RenderLayer+1,  x, y, 0, false, rVR.getSystemTextureId(CViewRenderer::OutOfRangeTexture), CRGBA::White);
			}	
		}
		break;			
		case CCtrlSheetInfo::SheetType_OutpostBuilding:
			setupOutpostBuilding();
			if (_DispBackBmpId != -1)
				rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispBackBmpId, curSheetColor);
			if (_DispSheetBmpId != -1)
				rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispSheetBmpId, curSheetColor);
			if (_DispOverBmpId != -1)
				rVR.draw11RotFlipBitmap (_RenderLayer, x, y, 0, false, _DispOverBmpId, curSheetColor);
			displayCharBitmaps(_RenderLayer+2, x, y, curSheetColor);
			break;
		default:
			break;
	}

	if (showSelectionBorder)
	{
		if (!isDragged() || (isDragged() && _DuplicateOnDrag))
		{
			// draw selection border if this sheet is selected
			if (_SheetSelectionGroup != -1) // is this sheet selectable ?
			{   // yes, check if it is selected
				const CCtrlSheetSelection &css = CWidgetManager::getInstance()->getParser()->getCtrlSheetSelection();
				const CSheetSelectionGroup *ssg = css.getGroup(_SheetSelectionGroup);
				if (ssg)
				{
					// test is the selection group is active and if this item is selected
					if (ssg->isActive() && _CurrSelection == this)
					{
						// compute middle of the slot
						sint32 middleX = _XReal + (_WReal >> 1);
						sint32 middleY = _YReal + (_HReal >> 1);
						sint32 tw = ssg->getTextureWidth();
						sint32 th = ssg->getTextureHeight();
						// we are selected -> draw the selection border
						CRGBA color = ssg->getColor();
						if (ssg->isGlobalColorEnabled())
						{
							color.modulateFromColor(color, CWidgetManager::getInstance()->getGlobalColorForContent());
						}
						// decal layer because must drawn after Items/Brick in DXTC
						rVR.draw11RotFlipBitmap (_RenderLayer+1, middleX - (tw >> 1), middleY - (th >> 1), 0, false, ssg->getTextureIndex(), color);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
sint32 CDBCtrlSheet::drawNumber(sint32 x, sint32 y, sint32 wSheet, sint32 /* hSheet */, CRGBA color, sint32 value, bool rightAlign)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	sint32	wDigit= rVR.getFigurTextureW();
	sint32	hDigit= rVR.getFigurTextureH();

	sint32 totalWidth = 0;

	if (value > -1)
	{
		// compute start pos
		sint32	units = value;
		sint32	pos;
		if(rightAlign)
			pos= wSheet-wDigit;
		else
		{
			// compute number of digits to display
			pos= 0;
			uint	numDigits= 0;
			do
			{
				units = units / 10;
				numDigits++;
			}
			while (units != 0);
			// so pos is:
			pos= numDigits*wDigit - wDigit;
		}
		// display digits
		units = value;
		do
		{
			sint32 unitsID = rVR.getFigurTextureId (units % 10);
			// decal layer because must drawn after Items/Brick in DXTC
			rVR.drawRotFlipBitmap (_RenderLayer+2, x+pos, y, wDigit, hDigit, 0, false, unitsID, color);
			units = units / 10;
			pos-= wDigit;
			totalWidth += wDigit;
		}
		while (units != 0);
		return totalWidth;
	}
	return -1;
}

// ----------------------------------------------------------------------------
bool CDBCtrlSheet::handleEvent (const NLGUI::CEventDescriptor &event)
{
	if (CCtrlBase::handleEvent(event)) return true;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

		// Handle drag'n'drop
		if (CWidgetManager::getInstance()->getCapturePointerLeft() == this)
		{
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown && !isDragged())
			{
				_DragX = eventDesc.getX();
				_DragY = eventDesc.getY();
			}

			bool validClic = false;
			if (_DispSheetBmpId!=-1)
			{
				// Cannot drag if grayed (LOCKED or LATENT)!. Still can drag a shortcut
				if (asItemSheet() && asItemSheet()->Stackable > 1 && _UseQuantity)
				{
					validClic = isDraggable() && !isDragged() && (getQuantity() > 0);
					validClic = validClic && (!getItemWeared());
				}
				else
				{
					validClic = isDraggable() && !isDragged() && ((!getItemWeared()&&!getGrayed()) || isShortCut());
				}
			}
			if (_Type == SheetType_Macro)
			{
				validClic = isDraggable();
			}

			// posssibly check AH to see if really can draging
			if (validClic && _AHOnCanDrag != NULL)
			{
				_TempCanDrag= true;
				CAHManager::getInstance()->runActionHandler (_AHOnCanDrag, this, _AHCanDragParams);
				validClic= _TempCanDrag;
			}

			if (validClic)
			{
				if ((abs(_DragX-eventDesc.getX()) > 5) || (abs(_DragY-eventDesc.getY()) > 5))
				{
					_DeltaDragX= _DragX-(_XReal+1);
					_DeltaDragY= _DragY-(_YReal+1);
					if (_DeltaDragX > _WReal) _DeltaDragX = _WReal;
					if (_DeltaDragY > _HReal) _DeltaDragY = _HReal;
					setDragged( true );
					setDraggedSheet( this );

					if (_AHOnDrag != NULL)
					{
						CAHManager::getInstance()->runActionHandler (_AHOnDrag, this, _AHDragParams);
					}
				}
			}

			if (isDragged())
			{
				// If mouse left up, must end the Drag
				if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
				{
					bool handled = false;
					// get the ctrl under the drop
					const vector<CCtrlBase*> &rCUP = CWidgetManager::getInstance()->getCtrlsUnderPointer();
					CDBCtrlSheet *pCSdest = NULL;
					for (uint32 i = 0; i < rCUP.size(); ++i)
					{
						CCtrlBase *pCB = rCUP[i];
						if (pCB != this)
						{
							CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(pCB);
							pCSdest = pCS;
							if (pCSdest != NULL)
								break;
						}
					}

					// if ctrl exist
					if (pCSdest != NULL)
					{
						// dest have a drop action and have a drop request?
						if(pCSdest->_AHOnDrop != NULL && pCSdest->_AHOnCanDrop != NULL)
						{
							// test if can drop me on dest
							pCSdest->_CanDrop= false;
							string params = string("src=") + _Id;
							if (!pCSdest->_AHCanDropParams.empty())
							{
								if (CAHManager::getInstance()->getAHName(pCSdest->_AHOnCanDrop) == "lua")
								{
									params = pCSdest->_AHCanDropParams;
									strFindReplace(params, "%src", _Id);
								}
								else
								{
									string sTmp = pCSdest->_AHCanDropParams;
									params = sTmp + "|" + params;
								}
							}
							CAHManager::getInstance()->runActionHandler (pCSdest->_AHOnCanDrop, pCSdest, params);

							// Drop only if canDrop.
							if(pCSdest->_CanDrop)
							{
								// build params
								string params = string("src=") + _Id;
								if (!pCSdest->_AHDropParams.empty())
								{
									if (CAHManager::getInstance()->getAHName(pCSdest->_AHOnDrop) == "lua")
									{
										params = pCSdest->_AHDropParams;
										strFindReplace(params, "%src", _Id);
									}
									else
									{
										string sTmp = pCSdest->_AHDropParams;
										params = sTmp + "|" + params;// must copy 'drop' params at start because it could be the name of a procedure
									}
								}
								// call action
								CAHManager::getInstance()->runActionHandler (pCSdest->_AHOnDrop, pCSdest, params);
								handled = true;
							}
						}
					}
					else // If slot not found try to drop on a list
					{
						// get the list under the drop
						const vector<CInterfaceGroup*> &rGUP = CWidgetManager::getInstance()->getGroupsUnderPointer();
						CDBGroupListSheet *pList = NULL;
						CDBGroupListSheetText *pTextList = NULL;
						for (uint32 i = 0; i < rGUP.size(); ++i)
						{
							pList = dynamic_cast<CDBGroupListSheet*>(rGUP[i]);
							if (pList != NULL)
								if (pList->getCanDrop())
									break;

							pTextList = dynamic_cast<CDBGroupListSheetText*>(rGUP[i]);
							if (pTextList != NULL)
								if (pTextList->getCanDrop())
									break;
						}

						if ((pList != NULL) || (pTextList != NULL))
						{
							if (pList != NULL)
							if (pList->getCtrlSheetInfo()._AHOnDrop != NULL && pList->getCtrlSheetInfo()._AHOnCanDrop != NULL)
							{
								pList->setCanDrop(false);
								string params = string("src=") + _Id;
								if (!pList->getCtrlSheetInfo()._AHCanDropParams.empty())
								{
									if (CAHManager::getInstance()->getAHName(pList->getCtrlSheetInfo()._AHOnCanDrop) == "lua")
									{
										params = pList->getCtrlSheetInfo()._AHCanDropParams;
										strFindReplace(params, "%src", _Id);
									}
									else
									{
										string sTmp = pList->getCtrlSheetInfo()._AHCanDropParams;
										params = sTmp + "|" + params;
									}
								}
								CAHManager::getInstance()->runActionHandler (pList->getCtrlSheetInfo()._AHOnCanDrop, pList, params);

								// Drop only if canDrop.
								if(pList->getCanDrop())
								{
									// build params
									string params = string("src=") + _Id;
									if (!pList->getCtrlSheetInfo()._AHDropParams.empty())
									{
										if (CAHManager::getInstance()->getAHName(pList->getCtrlSheetInfo()._AHOnDrop) == "lua")
										{
											params = pList->getCtrlSheetInfo()._AHDropParams;
											strFindReplace(params, "%src", _Id);
										}
										else
										{
											string sTmp = pList->getCtrlSheetInfo()._AHDropParams;
											params = sTmp + "|" + params; // must copy 'drop' params at start because it could be the name of a procedure
										}
									}
									// call action
									CAHManager::getInstance()->runActionHandler (pList->getCtrlSheetInfo()._AHOnDrop, pList, params);
									handled = true;
								}
							}

							if (pTextList != NULL)
							if (pTextList->getCtrlSheetInfo()._AHOnDrop != NULL && pTextList->getCtrlSheetInfo()._AHOnCanDrop != NULL)
							{
								pTextList->setCanDrop(false);
								string params = string("src=") + _Id;
								if (!pTextList->getCtrlSheetInfo()._AHCanDropParams.empty())
								{
									string sTmp = pTextList->getCtrlSheetInfo()._AHCanDropParams;
									params = sTmp + "|" + params;
								}
								CAHManager::getInstance()->runActionHandler (pTextList->getCtrlSheetInfo()._AHOnCanDrop, pTextList, params);

								// Drop only if canDrop.
								if(pTextList->getCanDrop())
								{
									// build params
									string params = string("src=") + _Id;
									if (!pTextList->getCtrlSheetInfo()._AHDropParams.empty())
									{
										string sTmp = pTextList->getCtrlSheetInfo()._AHDropParams;
										params = sTmp + "|" + params; // must copy 'drop' params at start because it could be the name of a procedure
									}
									// call action
									CAHManager::getInstance()->runActionHandler (pTextList->getCtrlSheetInfo()._AHOnDrop, pTextList, params);
									handled = true;
								}
							}

						}
					}

					if (!handled && _AHOnCannotDrop != NULL )
					{
						CAHManager::getInstance()->runActionHandler (_AHOnCannotDrop, this, _AHCannotDropParams);
						handled = true;
					}

					// In all case, quit
					setDragged( false );
					setDraggedSheet( NULL );
					// In call case, end of drag => consider handled to not call another action
					return true;
				}
			}
		}

		// If we are dragging, no more event on us
		if(isDragged())
			return false; // true;

		// Mouse events that must be done over the control
		if (!((eventDesc.getX() >= _XReal) &&
			(eventDesc.getX() < (_XReal + _WReal))&&
			(eventDesc.getY() > _YReal) &&
			(eventDesc.getY() <= (_YReal+ _HReal))))
			return false;

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if (CWidgetManager::getInstance()->getCapturePointerLeft() != this)
				return false;

			// RunAction
			if(_AHOnLeftClick != NULL)
				CAHManager::getInstance()->runActionHandler (_AHOnLeftClick, this, _AHLeftClickParams);
			// Run Menu (if item is not being dragged)
			if (!_ListMenuLeft.empty() && dynamic_cast< CDBCtrlSheet* >( CCtrlDraggable::getDraggedSheet() ) == NULL)
			{
				if (getSheetId() != 0)
				{
					_CurrMenuSheet = this;
					CWidgetManager::getInstance()->enableModalWindow (this, _ListMenuLeft);
				}
			}
			// Always return true on LeftClick.
			return true;
		}

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown)
		{
			return true;
		}

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup)
		{
			bool	handled= false;
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if (CWidgetManager::getInstance()->getCapturePointerRight() != this)
				return false;

			// RunAction
			if(_AHOnRightClick != NULL)
			{
				handled= true;
				CAHManager::getInstance()->runActionHandler (_AHOnRightClick, this, _AHRightClickParams);
			}
			// Run Menu (if item is not being dragged)
			if (!_ListMenuRight.empty() || !_ListMenuRightEmptySlot.empty())
			{
				handled= true;
				// There must be no dragged sheet
				if( dynamic_cast< CDBCtrlSheet* >( CCtrlDraggable::getDraggedSheet() ) == NULL)
				{
					// if a macro, don't test if Sheet==0
					if ( isMacro() || getSheetId() != 0)
					{
						if(!_ListMenuRight.empty())
						{
							_CurrMenuSheet = this;
							CWidgetManager::getInstance()->enableModalWindow (this, _ListMenuRight);
						}
					}
					// if sheetId==0, then may open the other menu
					else
					{
						if(!_ListMenuRightEmptySlot.empty())
						{
							_CurrMenuSheet = this;
							CWidgetManager::getInstance()->enableModalWindow (this, _ListMenuRightEmptySlot);
						}
					}
				}
			}
			// If not handled here, ret to parent
			return handled;
		}

	}
	return false;
}


// ----------------------------------------------------------------------------
CCDBNodeBranch *CDBCtrlSheet::getRootBranch() const
{
	if (_SheetId.getNodePtr())
	{
		return _SheetId.getNodePtr()->getParent();
	}
	return NULL;
}


// ***************************************************************************
static void	swapDBProps(CCDBNodeLeaf *a, CCDBNodeLeaf *b)
{
	if(!a || !b)
		return;
	sint32	val= a->getValue32();
	a->setValue32(b->getValue32());
	b->setValue32(val);
}

// ***************************************************************************
void	CDBCtrlSheet::swapSheet(CDBCtrlSheet *other)
{
	sint32	sheetId= other->_SheetId.getSInt32();
	other->_SheetId.setSInt32(_SheetId.getSInt32());
	_SheetId.setSInt32(sheetId);
	// For Items try to swap all other infos
	if(_ActualType==SheetType_Item)
	{
		// quantity
		_Quantity.swap32(other->_Quantity);
		_Quality.swap32(other->_Quality);
		_NameId.swap32(other->_NameId);
		_Enchant.swap32(other->_Enchant);

		// swap the other props only if the DB exist in the 2 version. else no-op
		swapDBProps(_UserColor, other->_UserColor);
		swapDBProps(getItemLockedPtr(), other->getItemLockedPtr());
		swapDBProps(getItemWeightPtr(), other->getItemWeightPtr());
		swapDBProps(getItemInfoVersionPtr(), other->getItemInfoVersionPtr());
		swapDBProps(getItemRMClassTypePtr(), other->getItemRMClassTypePtr());
		swapDBProps(getItemRMFaberStatTypePtr(), other->getItemRMFaberStatTypePtr());
		swapDBProps(getItemPrerequisitValidPtr(), other->getItemPrerequisitValidPtr());
	}
}

// ***************************************************************************
void CDBCtrlSheet::setCurrSelection(CDBCtrlSheet *selected)
{
	_CurrSelection = selected;
	NLGUI::CDBManager::getInstance()->getDbProp("UI:SELECTED_ITEM_SHEET_ID:SHEET")->setValue64(selected ? selected->getSheetId() : 0);
	NLGUI::CDBManager::getInstance()->getDbProp("UI:SELECTED_ITEM_SHEET_ID:QUALITY")->setValue64(selected ? selected->getQuality() : 0);
	NLGUI::CDBManager::getInstance()->getDbProp("UI:SELECTED_ITEM_SHEET_ID:SLOT_TYPE")->setValue64(selected ? selected->getBehaviour() : 0);
	// set the selection group in the db
	NLGUI::CDBManager::getInstance()->getDbProp("UI:SELECTED_ITEM_SELECTION_GROUP")->setValue64(selected ? selected->getSelectionGroup() : -1);
}

// ***************************************************************************
const std::string &CDBCtrlSheet::getSelectionGroupAsString() const
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	const CCtrlSheetSelection &css = CWidgetManager::getInstance()->getParser()->getCtrlSheetSelection();
	const CSheetSelectionGroup *csg = css.getGroup(_SheetSelectionGroup);
	static const string emptyStr;
	return csg ? csg->getName() : emptyStr;
}

// ***************************************************************************
sint32 CDBCtrlSheet::getNonLockedQuantity() const
{
	sint32 nonLockedQt = getQuantity();
	CCDBNodeLeaf *lockedPtr = getItemLockedPtr();
	if (lockedPtr != NULL)
		nonLockedQt -= lockedPtr->getValue32();
	if (nonLockedQt < 0)
		nonLockedQt = 0;
	return nonLockedQt;
}

// ***************************************************************************
const CItemSheet *CDBCtrlSheet::asItemSheet() const
{
	updateActualType();
	if( _ActualType!=SheetType_Item )
		return NULL;

	CEntitySheet *sheet  = SheetMngr.get(CSheetId(getSheetId()));
	if (sheet && sheet->type() == CEntitySheet::ITEM)
	{
		return (CItemSheet *) sheet;
	}
	return NULL;
}

// ***************************************************************************
const CPactSheet *CDBCtrlSheet::asPactSheet() const
{
	updateActualType();
	if( _ActualType!=SheetType_Pact )
		return NULL;

	CEntitySheet *sheet  = SheetMngr.get(CSheetId(getSheetId()));
	if (sheet && sheet->type() == CEntitySheet::PACT)
	{
		return (CPactSheet *) sheet;
	}
	return NULL;
}

// ***************************************************************************
const CSBrickSheet *CDBCtrlSheet::asSBrickSheet() const
{
	updateActualType();
	if( _ActualType!=SheetType_SBrick )
		return NULL;

	CEntitySheet *sheet  = SheetMngr.get(CSheetId(getSheetId()));
	if (sheet && sheet->type() == CEntitySheet::SBRICK)
	{
		return (CSBrickSheet *) sheet;
	}
	return NULL;
}

// ***************************************************************************
const CSPhraseSheet	*CDBCtrlSheet::asSPhraseSheet() const
{
	updateActualType();
	if( _ActualType!=SheetType_SPhrase )
		return NULL;

	CEntitySheet *sheet  = SheetMngr.get(CSheetId(getSheetId()));
	if (sheet && sheet->type() == CEntitySheet::SPHRASE)
	{
		return (CSPhraseSheet *) sheet;
	}
	return NULL;
}

// ***************************************************************************
const COutpostBuildingSheet *CDBCtrlSheet::asOutpostBuildingSheet() const
{
	updateActualType();
	if (_ActualType != SheetType_OutpostBuilding)
		return NULL;

	CEntitySheet *sheet  = SheetMngr.get(CSheetId(getSheetId()));
	if (sheet && sheet->type() == CEntitySheet::OUTPOST_BUILDING)
	{
		return (COutpostBuildingSheet *) sheet;
	}
	return NULL;
}

// ***************************************************************************
void	CDBCtrlSheet::getContextHelp(ucstring &help) const
{
	if (getType() == CCtrlSheetInfo::SheetType_Skill)
	{
		// just show the name of the skill
		// the sheet id is interpreted as a skill enum
		help= STRING_MANAGER::CStringManagerClient::getSkillLocalizedName( (SKILLS::ESkills)_SheetId.getSInt32() );
	}
	else if(getType() == CCtrlSheetInfo::SheetType_Macro)
	{
		// TODO Find the name + params of the action
		help = _ContextHelp;
	}
	else if(getType() == CCtrlSheetInfo::SheetType_Item)
	{
		const CItemSheet	*item= asItemSheet();
		if(item)
		{
			if (item->Family == ITEMFAMILY::CRYSTALLIZED_SPELL || item->Family == ITEMFAMILY::JEWELRY || item->Family == ITEMFAMILY::ARMOR)
			{
				string luaMethodName = ( (item->Family == ITEMFAMILY::CRYSTALLIZED_SPELL) ? "updateCrystallizedSpellTooltip" : "updateBuffItemTooltip");
				CDBCtrlSheet *ctrlSheet = const_cast<CDBCtrlSheet*>(this);
				if ( ! getInventory().isItemInfoUpToDate(getInventory().getItemSlotId(ctrlSheet)))
				{
					// Prepare the waiter
					ControlSheetTooltipUpdater.ItemSheet= ctrlSheet->getSheetId();
					ControlSheetTooltipUpdater.LuaMethodName = luaMethodName;
					ControlSheetTooltipUpdater.ItemSlotId= getInventory().getItemSlotId(ctrlSheet);
					ControlSheetTooltipUpdater.CtrlSheet = ctrlSheet;

					// Add the waiter
					getInventory().addItemInfoWaiter(&ControlSheetTooltipUpdater);
				}

				help = ControlSheetTooltipUpdater.infoValidated(ctrlSheet, luaMethodName);

			}
			else
				help= getItemActualName();

		}
		else
			help= _ContextHelp;
	}
	else if(getType() == CCtrlSheetInfo::SheetType_Pact)
	{
		const CPactSheet	*item= asPactSheet();
		if(item)
		{
			sint32 quality = getQuality();
			if (quality >= (sint32) 0 && quality < (sint32) item->PactLose.size())
			{
				help= item->PactLose[quality].Name;
			}
			else
			{
				help= _ContextHelp;
			}
		}
		else
		{
			help= _ContextHelp;
		}
	}
	else if(getType() == CCtrlSheetInfo::SheetType_SBrick)
	{
		CSBrickManager	*pBM= CSBrickManager::getInstance();
		CSBrickSheet	*brick= pBM->getBrick(CSheetId(getSheetId()));
		if(brick)
			help= STRING_MANAGER::CStringManagerClient::getSBrickLocalizedName(brick->Id);
		else
			help= _ContextHelp;
	}
	else if(getType() == CCtrlSheetInfo::SheetType_SPhraseId)
	{
		sint32	phraseId= getSheetId();
		if (phraseId == 0)
		{
			help = ucstring();
		}
		else
		{
			// delegate setup of context he help ( & window ) to lua		
			CInterfaceManager *im = CInterfaceManager::getInstance();
			CLuaState *ls= CLuaManager::getInstance().getLuaState();
			{
				CLuaStackRestorer lsr(ls, 0);
				CSPhraseManager	*pPM= CSPhraseManager::getInstance();					
				_PhraseAdapter = new CSPhraseComAdpater;
				_PhraseAdapter->Phrase = pPM->getPhrase(phraseId);
				CLuaIHM::pushReflectableOnStack(*ls, _PhraseAdapter);
				ls->pushGlobalTable();	
				CLuaObject game(*ls);
				game = game["game"];		
				game.callMethodByNameNoThrow("updatePhraseTooltip", 1, 1);
				// retrieve result from stack
				help = ucstring();
				if (!ls->empty())
				{
					CLuaIHM::pop(*ls, help);
				}
				else
				{
					nlwarning("Ucstring result expected when calling 'game:updatePhraseTooltip', possible script error");
				}
			}
		}
		/*
		CSPhraseManager	*pPM= CSPhraseManager::getInstance();
		// The sheetId point to a phrase in the manager, get it
		sint32	phraseId= getSheetId();
		if(phraseId)
			help= pPM->getPhrase(phraseId).Name;
		else
			help= _ContextHelp;

		sint32 phraseSheetID = pPM->getSheetFromPhrase(pPM->getPhrase(phraseId));
		if (phraseSheetID != 0)
		{
			// is it a built-in phrase?
			ucstring desc = STRING_MANAGER::CStringManagerClient::getSPhraseLocalizedDescription(NLMISC::CSheetId(phraseSheetID));
			if (!desc.empty())
			{
				help += ucstring("\n\n@{CCCF}") + desc;
			}
		}
		*/
	}
	else if(getType() == CCtrlSheetInfo::SheetType_SPhrase)
	{
		CSPhraseSheet	*phrase= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(CSheetId(getSheetId())));
		if(phrase)
			help= STRING_MANAGER::CStringManagerClient::getSPhraseLocalizedName(phrase->Id);
		else
			help= _ContextHelp;
	}
	else if(getType() == CCtrlSheetInfo::SheetType_OutpostBuilding)
	{
		const COutpostBuildingSheet *outpost = asOutpostBuildingSheet();
		if (outpost)
			help = CStringManagerClient::getOutpostBuildingLocalizedName(CSheetId(_SheetId.getSInt32()));
		else
			help = _ContextHelp;
	}
}

// ***************************************************************************
void	CDBCtrlSheet::getContextHelpToolTip(ucstring &help) const
{
	// Special case for buff items and spell crystals, only for tooltips
	if (getType() == CCtrlSheetInfo::SheetType_Item)
	{
		const CItemSheet *item = asItemSheet();
		if (item)
		{
			if (item->Family == ITEMFAMILY::CRYSTALLIZED_SPELL 
				|| item->Family == ITEMFAMILY::JEWELRY || item->Family == ITEMFAMILY::ARMOR)
			{				
				string luaMethodName = (item->Family == ITEMFAMILY::CRYSTALLIZED_SPELL) ? "updateCrystallizedSpellTooltip" : "updateBuffItemTooltip";
				CDBCtrlSheet *ctrlSheet = const_cast<CDBCtrlSheet*>(this);
				if ( ! getInventory().isItemInfoUpToDate(getInventory().getItemSlotId(ctrlSheet)))
				{					
					// Prepare the waiter
					ControlSheetTooltipUpdater.ItemSheet= ctrlSheet->getSheetId();
					ControlSheetTooltipUpdater.LuaMethodName = luaMethodName;
					ControlSheetTooltipUpdater.ItemSlotId= getInventory().getItemSlotId(ctrlSheet);
					ControlSheetTooltipUpdater.CtrlSheet = ctrlSheet;
					
					// Add the waiter
					getInventory().addItemInfoWaiter(&ControlSheetTooltipUpdater);
				}
				
				help = ControlSheetTooltipUpdater.infoValidated(ctrlSheet, luaMethodName);
				return;
			}
		}
	}

	// Default
	getContextHelp(help);
}

// ***************************************************************************
bool	CDBCtrlSheet::canDropItem(CDBCtrlSheet *src) const
{
	if( src->getSheetId()==0 )
		return true;

	// If the dest or src is Grayed, cannot drop it
	if( src->getInventoryIndex() != INVENTORIES::exchange)
	{
		if (src->getGrayed() || getGrayed()) return false;
		if (src->getItemWeared() || getItemWeared()) return false;
	}

	// if no filter defined => OK.
	if( _ItemSlot== SLOTTYPE::UNDEFINED )
		return true;


	// Verify the item slot of the src
	CSheetId		sheetId(src->getSheetId());
	CEntitySheet	*pES = SheetMngr.get (sheetId);
	if ((pES != NULL) && (pES->type() == CEntitySheet::ITEM))
	{
		CItemSheet *pIS = (CItemSheet*)pES;

		// build the bitField for test.
		uint32	bf;
		bf= 1<<_ItemSlot;
		// special for right hand
		if( _ItemSlot== SLOTTYPE::RIGHT_HAND )
		{
			// Can put an object in right hand also if it is TWO_HANDS, or RIGHT_HAND_EXCLUSIVE
			bf|= 1<<SLOTTYPE::TWO_HANDS;
			bf|= 1<<SLOTTYPE::RIGHT_HAND_EXCLUSIVE;
		}

		// Look if one slot solution match.
		if( pIS->SlotBF & bf )
		{
			// Ok the object is compatible with the dest

			// Can put an object in left or right hand is dependent of other hand content
			if( _OtherHandItemFilter && (_ItemSlot == SLOTTYPE::LEFT_HAND || _ItemSlot == SLOTTYPE::RIGHT_HAND) )
			{
				// special for left hand
				if( _ItemSlot == SLOTTYPE::LEFT_HAND )
				{
					// If the item comes from right hand cant drop
					if (src->_ItemSlot == SLOTTYPE::RIGHT_HAND)
						return false;
					// get the item in the right hand
					CSheetId		sheetId(_OtherHandItemFilter->getSheetId());
					CEntitySheet	*pRightHandES = SheetMngr.get (sheetId);
					// if item present: must check if the right has a TWO_HANDS or RIGHT_HAND_EXCLUSIVE
					if ( pRightHandES != NULL && pRightHandES->type() == CEntitySheet::ITEM )
					{
						CItemSheet *pRightHandIS = (CItemSheet*)pRightHandES;
						if( pRightHandIS->hasSlot(SLOTTYPE::TWO_HANDS) ||
							pRightHandIS->hasSlot(SLOTTYPE::RIGHT_HAND_EXCLUSIVE) )
							return false;

						// if the current item we wants to drop is a dagger, check if right hand is a sword or a dagger
						if (pIS->ItemType == ITEM_TYPE::DAGGER)
						{
							if ((pRightHandIS->ItemType != ITEM_TYPE::DAGGER) &&
								(pRightHandIS->ItemType != ITEM_TYPE::SWORD))
								return false;
						}
					}
					else
					{
						// If nothing valid in right hand cant drop a dagger
						if (pIS->ItemType == ITEM_TYPE::DAGGER)
							return false;
					}
				}
				// special for right hand
				else
				{
					/*
					// If the right hand do not contains a two hand item
					bool bRightHandContainsTwoHandItem = false;
					CSheetId		sheetId(getSheetId());
					CEntitySheet	*pOwnES = SheetMngr.get (sheetId);
					// if item present: must check if the right has a TWO_HANDS or RIGHT_HAND_EXCLUSIVE
					if ( pOwnES != NULL && pOwnES->type() == CEntitySheet::ITEM )
					{
						CItemSheet *pOwnIS = (CItemSheet*)pOwnES;
						if( pOwnIS->hasSlot( SLOTTYPE::TWO_HANDS ) || pOwnIS->hasSlot( SLOTTYPE::RIGHT_HAND_EXCLUSIVE ) )
							bRightHandContainsTwoHandItem = true;
					}

					// if the LeftHand is not empty, and If the item we want to drop is a 2Hands item, CANNOT drop
					if (!bRightHandContainsTwoHandItem)
					if( _OtherHandItemFilter->getSheetId()!=0 )
					if( pIS->hasSlot( SLOTTYPE::TWO_HANDS ) || pIS->hasSlot( SLOTTYPE::RIGHT_HAND_EXCLUSIVE ) )
					{
						return false;
					}
					*/
				}
			}

			// Check if the ammo has the same type as the hand containing the weapon
			if( _OtherHandItemFilter && (_ItemSlot== SLOTTYPE::AMMO))
			{
				CSheetId		sheetId(_OtherHandItemFilter->getSheetId());
				CEntitySheet	*pESWeapon = SheetMngr.get (sheetId);
				if ( pESWeapon == NULL || pESWeapon->type() != CEntitySheet::ITEM )
					return false;
				CItemSheet *pISWeapon = (CItemSheet*)pESWeapon;
				if (pISWeapon->Family != ITEMFAMILY::RANGE_WEAPON)
					return false;
				if(pIS->Family != ITEMFAMILY::AMMO)
					return false;
				if (pISWeapon->RangeWeapon.Skill != pIS->Ammo.Skill)
					return false;
			}

			// ok, can drop!
			return true;
		}
		else
			return false;

	}

	// Default: OK
	return true;
}

// ***************************************************************************
void CDBCtrlSheet::updateActualType() const
{
	if (_Type == SheetType_Auto)
	{
		TSheetType	newType;
		if (isMission())
		{
			newType = SheetType_Mission;
		}
		else if (isSkill())
		{
			newType = SheetType_Skill;
		}
		else
		{
			// get type from sheet id
			CEntitySheet *es = SheetMngr.get(CSheetId(_SheetId.getSInt32()));
			if (!es)
			{
				newType = SheetType_Item;
			}
			else
			{
				switch(es->type())
				{
					case CEntitySheet::ITEM:  newType = SheetType_Item; break;
					case CEntitySheet::PACT:  newType = SheetType_Pact; break;
					case CEntitySheet::SBRICK: newType = SheetType_SBrick; break;
					case CEntitySheet::SPHRASE: newType = SheetType_SPhrase; break;
					case CEntitySheet::OUTPOST_BUILDING: newType = SheetType_OutpostBuilding; break;
					default:
						newType = SheetType_Item;
					break;
				}
			}
		}

		// if type changed, must reset some ctrl state (important for list sheet trade for instance else
		// new SPhrase can get aspect of old SItem (eg: Upper-Left MP Text and Redified color...)
		if(newType!=_ActualType)
		{
			_ActualType= newType;
			const_cast<CDBCtrlSheet*>(this)->_Grayed= false;
			const_cast<CDBCtrlSheet*>(this)->_Useable= true;
			const_cast<CDBCtrlSheet*>(this)->_OptString.clear();
			const_cast<CDBCtrlSheet*>(this)->resetCharBitmaps();
		}
	}
	else
	{
		_ActualType = _Type;
	}
}

// ***************************************************************************
void CDBCtrlSheet::setType(CCtrlSheetInfo::TSheetType type)
{
	_ActualType = _Type = type;
	_NeedSetup = true;
}

// ***************************************************************************
void CDBCtrlSheet::resetAllTexIDs()
{
	_DispSheetBmpId = -1;
	_DispBackBmpId = -1;
	_DispOverBmpId = -1;
	_DispOver2BmpId = -1;
	_DispQuality = -1;
	_DispQuantity= -1;
	_Stackable= 1;
	_IconW = 0;
	_IconH = 0;
}


// ***************************************************************************
CCtrlSheetInfo::TSheetType CDBCtrlSheet::getType() const
{
	if (_Type == SheetType_GuildFlag) return SheetType_GuildFlag;

	if (_Type != SheetType_Macro)
	{
		if (_LastSheetId != _SheetId.getSInt32())
		{
			updateActualType();
			_LastSheetId = _SheetId.getSInt32();
			_NeedSetup = true;
		}
		return _ActualType;
	}
	else
	{
		return SheetType_Macro;
	}
}

// ***************************************************************************
void CDBCtrlSheet::setBehaviour(TRADE_SLOT_TYPE::TTradeSlotType type)
{
	if (_HasTradeSlotType)
	{
		_TradeSlotType.setSInt32(type);
	}
}

// ***************************************************************************
void CDBCtrlSheet::copyAspect(CDBCtrlSheet *dest)
{
	dest->setType(getType());
	dest->setSheetId(getSheetId());
	if (!isSBrickOrSPhraseId() && !isMission())
	{
		if (getUseQuality())
		{
			dest->setUseQuality(true);
			if (_Quality.getNodePtr() != NULL)
				dest->setQuality(getQuality());
			dest->setReadQuantityFromSheetFlag(getReadQuantityFromSheetFlag());
		}
		else
		{
			dest->setUseQuality(false);
		}
		if (getUseQuantity())
		{
			dest->setUseQuantity(true);
			if (_Quantity.getNodePtr() != NULL)
				dest->setQuantity(getQuantity());
		}
		else
		{
			dest->setUseQuantity(false);
		}
		// copy color for items
		sint col = getItemColor();
		if (col != -1) dest->setItemColor(col);
		// copy weight
		dest->setItemWeight(getItemWeight());
		// copy nameId
		dest->setItemNameId(getItemNameId());
		// copy info version
		dest->setItemInfoVersion(getItemInfoVersion());
		// copy enchant info
		dest->setEnchant(getEnchant());
		// copy faber faber quality
		dest->setItemRMClassType(getItemRMClassType());
		// copy faber faber stat type
		dest->setItemRMFaberStatType(getItemRMFaberStatType());
		// copy faber faber stat type
		dest->setItemRMFaberStatType(getItemRMFaberStatType());
		// copy prerequisit valid flag
		dest->setItemPrerequisitValid(getItemPrerequisitValid());
	}
	// if brick, sphrase or sphraseId
	if(isSBrick() || isSPhrase() || isSPhraseId())
	{
		// must reset dest Useable/Redifyed and slotType (in case precedent type was Item!)
		dest->setGrayed(false);
		dest->_Useable= true;
		dest->_OptString.clear();
		dest->resetCharBitmaps();
	}
	// misc
	if (dest->_HasTradeSlotType) dest->setBehaviour(getBehaviour());
	dest->invalidateCoords();
	dest->initSheetSize();


	dest->_NeedSetup = true;
}

// ***************************************************************************
bool CDBCtrlSheet::sameAspect(CDBCtrlSheet *dest) const
{
	if( dest->getType() != getType() )
		return false;
	if( dest->getSheetId() != getSheetId() )
		return false;
	if (!isSBrickOrSPhraseId() && !isSPhrase())
	{
		/*
		if( dest->getUseQuality() != getUseQuality() )
			return false;
		if( getUseQuality() && _Quality.getNodePtr() != NULL )
		{
			if( dest->getQuality() != getQuality() )
				return false;
		}
		if( dest->getUseQuantity() != getUseQuantity() )
			return false;
		if( getUseQuantity() && _Quantity.getNodePtr() != NULL )
		{
			if( dest->getQuantity() != getQuantity() )
				return false;
		}
		if( dest->getEnchant() != getEnchant() )
			return false;
		if( dest->getRMClassType() != getRMClassType() )
			return false;
		if( dest->getRMFaberStatType() != getRMFaberStatType() )
			return false;
		*/
		return false; // nico v : Items can be different because of their item info. So can't conclude about equality.
		              // default to false for safety
	}
	if (dest->_HasTradeSlotType)
	{
		if( dest->getBehaviour() != getBehaviour() )
			return false;
	}

	return true;
}

// ***************************************************************************
CDBCtrlSheet::TSheetCategory CDBCtrlSheet::getSheetCategory() const
{
	if (isSkill()) return Skill;
	if (getType() == SheetType_Pact) return Pact;
	if (getType() == SheetType_GuildFlag) return GuildFlag;
	if (getType() == SheetType_Mission) return Mission;
	if (isSPhrase()) return Phrase;
	return Item;
}

// ***************************************************************************
bool CDBCtrlSheet::isMission() const
{
	CCDBNodeBranch *root = getRootBranch();
	if (!root) return false;
	CCDBNodeLeaf *node = dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("ICON"), false));
	return node != NULL;
}

// ***************************************************************************
void	CDBCtrlSheet::setupInit()
{
	_SetupInit= true;

	// Init _OtherHandItemFilter (with the opti string)
	if( !_OptString.empty() )
	{
		// typically replace "handl" with "handr" or vice versa
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CInterfaceElement	*pElt = CWidgetManager::getInstance()->getElementFromId (_Id, _OptString);
		CDBCtrlSheet		*pOtherHand = dynamic_cast<CDBCtrlSheet*>(pElt);
		if( !pOtherHand  || pOtherHand ->getType() != CCtrlSheetInfo::SheetType_Item )
		{
			nlwarning("%s: other_hand_slot error", _Id.c_str());
		}
		_OtherHandItemFilter= pOtherHand;
	}
}

// ***************************************************************************
IListSheetBase					*CDBCtrlSheet::getListSheetParent() const
{
	IListSheetBase *parent = dynamic_cast<IListSheetBase *>(_Parent);
	// ListSheetTrade double parent sons (intermeidate _List)
	if(!parent && _Parent)
	{
		parent = dynamic_cast<IListSheetBase *>(_Parent->getParent());
	}
	return parent;
}

// ***************************************************************************
sint CDBCtrlSheet::getIndexInParent() const
{
	IListSheetBase *parent= getListSheetParent();
	if (!parent) return -1;
	return parent->getIndexOf(this);
}

// ***************************************************************************
void CDBCtrlSheet::readFromMacro(const CMacroCmd &mc)
{
	if (_Type != SheetType_Macro) return;
	CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

	_DispBackBmpId = (mc.BitmapBack == 0xFF) ? -1 : pMCM->getTexIdBack(mc.BitmapBack);

	_DispOverBmpId = (mc.BitmapIcon == 0xFF) ? -1 : pMCM->getTexIdIcon(mc.BitmapIcon);

	_DispOver2BmpId = (mc.BitmapOver == 0xFF) ? -1 : pMCM->getTexIdOver(mc.BitmapOver);

	_OptString = mc.DispText;
	_NeedSetup = true;
	_MacroID = mc.ID;
}

// ***************************************************************************
void CDBCtrlSheet::writeToMacro(CMacroCmd &mc)
{
	uint i = 0;
	if (_Type != SheetType_Macro) return;
	CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

	// Find back
	if (_DispBackBmpId == -1)
	{
		mc.BitmapBack = 0xFF;
	}
	else
	{
		i = 0;
		while (i != 255)
		{
			if (pMCM->getTexIdBack(i) == _DispBackBmpId) break;
			i++;
		}
		if (i == 255) nlwarning("cannot find back id for macro");
		mc.BitmapBack = i;
	}

	// Find Icon
	if (_DispOverBmpId == -1)
	{
		mc.BitmapIcon = 0xFF;
	}
	else
	{
		i = 0;
		while (i != 255)
		{
			if (pMCM->getTexIdIcon(i) == _DispOverBmpId) break;
			i++;
		}
		if (i == 255) nlwarning("cannot find icon id for macro");
		mc.BitmapIcon = i;
	}

	// Find over
	if (_DispOver2BmpId == -1)
	{
		mc.BitmapOver = 0xFF;
	}
	else
	{
		i = 0;
		while (i != 255)
		{
			if (pMCM->getTexIdOver(i) == _DispOver2BmpId) break;
			i++;
		}
		if (i == 255) nlwarning("cannot find over id for macro");
		mc.BitmapOver = i;
	}

	mc.DispText = _OptString;
	mc.ID = _MacroID;

}

// ***************************************************************************
void CDBCtrlSheet::setMacroBack(uint8 nb)
{
	if (_Type != SheetType_Macro) return;
	CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
	_DispBackBmpId = (nb == 0xFF) ? -1 : pMCM->getTexIdBack(nb);
}

// ***************************************************************************
void CDBCtrlSheet::setMacroIcon(uint8 nb)
{
	if (_Type != SheetType_Macro) return;
	CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
	_DispOverBmpId = (nb == 0xFF) ? -1 : pMCM->getTexIdIcon(nb);
}

// ***************************************************************************
void CDBCtrlSheet::setMacroOver(uint8 nb)
{
	if (_Type != SheetType_Macro) return;
	CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
	_DispOver2BmpId = (nb == 0xFF) ? -1 : pMCM->getTexIdOver(nb);
}

// ***************************************************************************
void CDBCtrlSheet::setMacroText(const std::string &mcText)
{
	_OptString = mcText;
	_NeedSetup = true;
}

// ***************************************************************************
bool CDBCtrlSheet::isSheetValid() const
{
	// get the actual type
	TSheetType	type= getType();

	sint32	sid = 0;
	bool validSheet = false;

	if ((type != SheetType_Macro) && (type != SheetType_Skill))
	{
		sid = getSheetId();
		validSheet = (sid!=0);
	}
	// different test according to type
	switch(type)
	{
		// brick OK?
		// always for macros and skills enums
		case SheetType_Macro:
		case SheetType_Skill:
			validSheet= true;
			break;
		case SheetType_GuildFlag:
			// sid == name, quality == icon if both are != 0 then its a valid guild flag
			validSheet= (sid!=0)&&(_Quality.getSInt32() != 0);
			break;
		// SBrick OK?
		case SheetType_SBrick:
			validSheet= validSheet && CSBrickManager::getInstance()->getBrick(CSheetId((uint32)sid));
			break;
		case SheetType_SPhraseId:
			// Suppose phrase always valid but if 0.
			break;
		// Same for Items, Pact, and SPhrase
		default:
			validSheet= validSheet && SheetMngr.get(CSheetId((uint32)sid));
			validSheet= validSheet && (!_ItemWeared);
		break;
	};

	// TODO_BRICK: SPhrase / SPhraseId????

	return validSheet;
}

// ***************************************************************************
// GUILD_FLAG
// ***************************************************************************

// ***************************************************************************
CRGBA CDBCtrlSheet::getGuildColor1() const
{
	CRGBA col = CRGBA::White;
	if (_Type != SheetType_GuildFlag) return col;
	uint64 nGuildIcon = _Quality.getSInt64();
	return CGuildManager::iconGetColor1(nGuildIcon);
}

// ***************************************************************************
CRGBA CDBCtrlSheet::getGuildColor2() const
{
	CRGBA col=CRGBA::White;
	if (_Type != SheetType_GuildFlag) return col;
	uint64 nGuildIcon = _Quality.getSInt64();
	return CGuildManager::iconGetColor2(nGuildIcon);
}

// ***************************************************************************
sint32 CDBCtrlSheet::getGuildBack() const
{
	sint32 back = 0;
	if (_Type != SheetType_GuildFlag) return back;
	uint64 nGuildIcon = _Quality.getSInt64();
	return CGuildManager::iconGetBack(nGuildIcon);
}

// ***************************************************************************
sint32 CDBCtrlSheet::getGuildSymbol() const
{
	sint32 symb = 0;
	if (_Type != SheetType_GuildFlag) return symb;
	uint64 nGuildIcon = _Quality.getSInt64();
	return CGuildManager::iconGetSymbol(nGuildIcon);
}

// ***************************************************************************
bool CDBCtrlSheet::getInvertGuildSymbol() const
{
	if (_Type != SheetType_GuildFlag) return false;
	uint64 nGuildIcon = _Quality.getSInt64();
	return CGuildManager::iconGetInvertSymbol(nGuildIcon);
}

// ***************************************************************************
void CDBCtrlSheet::setGuildColor1(CRGBA col)
{
	if (_Type != SheetType_GuildFlag) return;
	// Clean up bits
	uint64 nGuildIcon = _Quality.getSInt64();
	CGuildManager::iconSetColor1(nGuildIcon, col);
	_Quality.setSInt64(nGuildIcon);
}

// ***************************************************************************
void CDBCtrlSheet::setGuildColor2(CRGBA col)
{
	if (_Type != SheetType_GuildFlag) return;
	// Clean up bits
	uint64 nGuildIcon = _Quality.getSInt64();
	CGuildManager::iconSetColor2(nGuildIcon, col);
	_Quality.setSInt64(nGuildIcon);
}

// ***************************************************************************
void CDBCtrlSheet::setGuildBack(sint32 n)
{
	if (_Type != SheetType_GuildFlag) return;
	// Clean up bits
	uint64 nGuildIcon = _Quality.getSInt64();
	CGuildManager::iconSetBack(nGuildIcon, (uint8)n);
	_Quality.setSInt64(nGuildIcon);
}

// ***************************************************************************
void CDBCtrlSheet::setGuildSymbol(sint32 n)
{
	if (_Type != SheetType_GuildFlag) return;
	// Clean up bits
	uint64 nGuildIcon = _Quality.getSInt64();
	CGuildManager::iconSetSymbol(nGuildIcon, (uint8)n);
	_Quality.setSInt64(nGuildIcon);
}

// ***************************************************************************
void CDBCtrlSheet::setInvertGuildSymbol(bool b)
{
	if (_Type != SheetType_GuildFlag) return;
	// Clean up bits
	uint64 nGuildIcon = _Quality.getSInt64();
	CGuildManager::iconSetInvertSymbol(nGuildIcon, b);
	_Quality.setSInt64(nGuildIcon);
}

// ***************************************************************************
void CDBCtrlSheet::setSlot(const std::string &textureName)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	_DispSlotBmpId = rVR.getTextureIdFromName (textureName);
	rVR.getTextureSizeFromId (_DispSlotBmpId, _W, _H);
	_DrawSlot = true;
}

// ***************************************************************************
// SBrick
// ***************************************************************************

// ***************************************************************************
bool		CDBCtrlSheet::isSPhraseIdMemory() const
{
	if(!isSPhraseId())
		return false;

	// test if mem match
	if( 0 == _DbBranchName.compare(0, PHRASE_DB_MEMORY.size(), PHRASE_DB_MEMORY) )
		return true;
	return false;
}

// ***************************************************************************
bool		CDBCtrlSheet::isMacroMemory() const
{
	if(!isMacro())
		return false;

	// test if mem match
	if( 0 == _DbBranchName.compare(0, PHRASE_DB_MEMORY.size(), PHRASE_DB_MEMORY) )
		return true;
	return false;
}

// ***************************************************************************
uint8 CDBCtrlSheet::getItemInfoVersion() const
{
	CCDBNodeLeaf *node = getItemInfoVersionPtr();
	return node ? (uint8) node->getValue8() : 0;
}

// ***************************************************************************
CCDBNodeLeaf *CDBCtrlSheet::getItemInfoVersionPtr() const
{
	CCDBNodeBranch *root = getRootBranch();
	if (!root) return NULL;
	return dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("INFO_VERSION"), false));
}

// ***************************************************************************
void CDBCtrlSheet::setItemInfoVersion(uint8 infoVersion)
{
	CCDBNodeLeaf *node = getItemInfoVersionPtr();
	if (node) node->setValue8((sint8) infoVersion);
}

// ***************************************************************************
CCDBNodeLeaf *CDBCtrlSheet::getItemWeightPtr() const
{
	CCDBNodeBranch *root = getRootBranch();
	if (!root) return NULL;
	return dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("WEIGHT"), false));
}

// ***************************************************************************
uint16 CDBCtrlSheet::getItemWeight() const
{
	CCDBNodeLeaf *node = getItemWeightPtr();
	return node ? (uint16) node->getValue16() : 0;
}

// ***************************************************************************
void CDBCtrlSheet::setItemWeight(uint16 weight)
{
	CCDBNodeLeaf *node = getItemWeightPtr();
	if (node) node->setValue16((sint16) weight);
}

// ***************************************************************************
CCDBNodeLeaf *CDBCtrlSheet::getItemLockedPtr() const
{
	CCDBNodeBranch *root = getRootBranch();
	if (!root) return NULL;
	return dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("LOCKED"), false));
}

// ***************************************************************************
uint16	CDBCtrlSheet::getItemLocked() const
{
	CCDBNodeLeaf *node = getItemLockedPtr();
	if (!node) return -1;
	return (sint) node->getValue32();
}

// ***************************************************************************
void CDBCtrlSheet::setItemLocked(uint16	lock)
{
	CCDBNodeLeaf *node = getItemLockedPtr();
	if (!node) return;
	node->setValue32(lock);
}

// ***************************************************************************
sint32 CDBCtrlSheet::getItemPrice() const
{
	CCDBNodeLeaf *node = getItemPricePtr();
	if (!node) return 0;
	return node->getValue32();
}

// ***************************************************************************
CCDBNodeLeaf *CDBCtrlSheet::getItemPricePtr() const
{
	CCDBNodeBranch *root = getRootBranch();
	if (!root) return NULL;
	return dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("PRICE"), false));
}

// ***************************************************************************
void CDBCtrlSheet::setItemPrice(sint32 price)
{
	CCDBNodeLeaf *node = getItemPricePtr();
	if (!node) return;
	node->setValue32(price);
}


// ***************************************************************************
sint32 CDBCtrlSheet::getItemResaleFlag() const
{
	CCDBNodeLeaf *node = getItemResaleFlagPtr();
	if (!node) return 0;
	return node->getValue32();
}

// ***************************************************************************
CCDBNodeLeaf *CDBCtrlSheet::getItemResaleFlagPtr() const
{
	CCDBNodeBranch *root = getRootBranch();
	if (!root) return NULL;
	return dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("RESALE_FLAG"), false));
}

// ***************************************************************************
void CDBCtrlSheet::setItemResaleFlag(sint32 rf)
{
	CCDBNodeLeaf *node = getItemResaleFlagPtr();
	if (!node) return;
	node->setValue32(rf);
}

// ***************************************************************************
bool CDBCtrlSheet::getLockedByOwner() const
{
	return (getItemResaleFlag() == BOTCHATTYPE::ResaleKOLockedByOwner);
}

// ***************************************************************************
bool CDBCtrlSheet::canOwnerLock() const
{
	return (NULL != getItemResaleFlagPtr());
}

// ***************************************************************************
sint32 CDBCtrlSheet::getItemSellerType() const
{
	CCDBNodeLeaf *node = getItemSellerTypePtr();
	if (!node) return 0;
	return node->getValue32();
}

// ***************************************************************************
CCDBNodeLeaf *CDBCtrlSheet::getItemSellerTypePtr() const
{
	CCDBNodeBranch *root = getRootBranch();
	if (!root) return NULL;
	return dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("SELLER_TYPE"), false));
}

// ***************************************************************************
void CDBCtrlSheet::setItemSellerType(sint32 rf)
{
	CCDBNodeLeaf *node = getItemSellerTypePtr();
	if (!node) return;
	node->setValue32(rf);
}

// ***************************************************************************
RM_CLASS_TYPE::TRMClassType CDBCtrlSheet::getItemRMClassType() const
{
	CCDBNodeLeaf *node = getItemRMClassTypePtr();
	return (RM_CLASS_TYPE::TRMClassType) (node ? node->getValue32() : 0);
}

// ***************************************************************************
void CDBCtrlSheet::setItemRMClassType(sint32 fq)
{
	CCDBNodeLeaf *node = getItemRMClassTypePtr();
	if (!node) return;
	node->setValue32(fq);
}

// ***************************************************************************
RM_FABER_STAT_TYPE::TRMStatType CDBCtrlSheet::getItemRMFaberStatType() const
{
	CCDBNodeLeaf *node = getItemRMFaberStatTypePtr();
	return (RM_FABER_STAT_TYPE::TRMStatType) (node ? node->getValue32() : 0);
}

// ***************************************************************************
void CDBCtrlSheet::setItemRMFaberStatType(sint32 fss)
{
	CCDBNodeLeaf *node = getItemRMFaberStatTypePtr();
	if (!node) return;
	node->setValue32(fss);
}

// ***************************************************************************
bool CDBCtrlSheet::getItemPrerequisitValid() const
{
	CCDBNodeLeaf *node = getItemPrerequisitValidPtr();
	return (bool) (node ? node->getValueBool() : true);
}

// ***************************************************************************
CCDBNodeLeaf *CDBCtrlSheet::getItemPrerequisitValidPtr() const
{
	CCDBNodeBranch *root = getRootBranch();
	if (!root) return NULL;
	return dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("PREREQUISIT_VALID"), false));
}

// ***************************************************************************
void CDBCtrlSheet::setItemPrerequisitValid(bool prv)
{
	CCDBNodeLeaf *node = getItemPrerequisitValidPtr();
	if (!node) return;
	node->setValueBool(prv);
}

// ***************************************************************************
void CDBCtrlSheet::initArmourColors()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	for(uint col = 0; col < RM_COLOR::NumColors; ++col)
	{
		_ArmourColor[col] = CRGBA::White;
		std::string defineName= "armour_color_" + toString(col);
		std::string	colVal= CWidgetManager::getInstance()->getParser()->getDefine(defineName);
		if(!colVal.empty())
			_ArmourColor[col] = convertColor(colVal.c_str());
	}
}


// ***************************************************************************
ucstring CDBCtrlSheet::getItemActualName() const
{
	const CItemSheet *pIS= asItemSheet();
	if(!pIS)
		return ucstring();
	else
	{
		ucstring ret;
		// If NameId not 0, get from StringManager
		uint32	nameId= getItemNameId();
		if(nameId)
		{
			STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
			pSMC->getDynString(nameId, ret);
		}
		// else get standard localized version
		else
		{
			ret = STRING_MANAGER::CStringManagerClient::getItemLocalizedName(pIS->Id);
		}

		if (pIS->Family == ITEMFAMILY::SCROLL_R2)
		{
			const R2::TMissionItem *mi = R2::getEditor().getPlotItemInfos(getSheetId());
			if (mi) return mi->Name;
		}
		// if item is not a mp, append faber_quality & faber_stat_type
		// Don't append quality and stat type for Named Items!!!
		if (!nameId &&
			(pIS->Family == ITEMFAMILY::ARMOR ||
			pIS->Family == ITEMFAMILY::MELEE_WEAPON ||
			pIS->Family == ITEMFAMILY::RANGE_WEAPON ||
			pIS->Family == ITEMFAMILY::AMMO ||
			pIS->Family == ITEMFAMILY::SHIELD ||
			pIS->Family == ITEMFAMILY::JEWELRY)
			)
		{
			// get description string for item format
			std::string formatID = getItemRMFaberStatType() != RM_FABER_STAT_TYPE::Unknown ? "uihelpItemFaberPrefixAndSuffix" : "uihelpItemFaberPrefix";
			ucstring format;
			if (!CI18N::hasTranslation(formatID))
			{
				format = ucstring("%p %n %s"); // not found, uses default string
			}
			else
			{
				format = CI18N::get(formatID);
			}
			// suffix
			strFindReplace(format, ucstring("%p"), RM_CLASS_TYPE::toLocalString(getItemRMClassType()));
			// name
			strFindReplace(format, ucstring("%n"), ret);
			// prefix
			strFindReplace(format, ucstring("%s"), CI18N::get(toString("mpstatItemQualifier%d", (int) getItemRMFaberStatType()).c_str()));


			ret = format;
		}
		return ret;
	}
}

// ***************************************************************************
void	CDBCtrlSheet::updateArmourColor(sint8 col)
{
	// bkup index cache (bkup -1 too)
	_ArmourColorIndex= col;

	if(_ArmourColorIndex>=0 && _ArmourColorIndex<=7)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CViewRenderer		&rVR= *CViewRenderer::getInstance();

		// if the BMP have not been correctly setuped
		if(!_ArmourColorBmpOk)
		{
			_ArmourColorBmpOk= true;

			if (_DispOverBmpId != -1)
			{
				_DispOver2BmpId = _DispOverBmpId;
			}
			std::string iconName = rVR.getTextureNameFromId(_DispSheetBmpId);
			std::string maskName = CFile::getFilenameWithoutExtension(iconName) + "_mask." + CFile::getExtension(iconName);
			_DispOverBmpId = rVR.getTextureIdFromName (maskName);
		}

		// new true color
		_PackedArmourColor = _ArmourColor[col].getPacked();
	}
}

// ***************************************************************************
bool	CDBCtrlSheet::checkItemRequirement()
{
	if(getType()!=SheetType_Item)
		return true;

	// If quality not used, well, cannot check item requirement.
	if(!_UseQuality)
		return true;

	// NB: we cannot test directly _Useable, because callers typically call this method BEFORE updateCoords()

	// If this is not the same sheet/quality, need to resetup charac requirement
/*
	sint32 sheet = _SheetId.getSInt32();
	if (_LastSheetId != sheet || _NeedSetup || _Quality.getSInt32()!=_DispQuality)
	{
		// NB: don't update cache, leave this feature to updateCoords()
		// Special Item requirement
		updateItemCharacRequirement(sheet);
	}

	// at each frame, must test for Redifyed (player carac does not met item requirement)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	bool retVal = pIM->isItemCaracRequirementMet(_ItemCaracReqType, _ItemCaracReqValue);

	if (retVal && _ItemSheet)
	{
		if (_ItemSheet->RequiredCharac != CHARACTERISTICS::Unknown && _ItemSheet->RequiredCharacLevel>0)
			retVal = pIM->isItemCaracRequirementMet(_ItemSheet->RequiredCharac, _ItemSheet->RequiredCharacLevel);

		if (retVal && _ItemSheet->RequiredSkillLevel > 0 && _ItemSheet->RequiredSkill != SKILLS::unknown)
			_Useable = CSkillManager::getInstance()->checkBaseSkillMetRequirement(_ItemSheet->RequiredSkill, _ItemSheet->RequiredSkillLevel);
	}
	return retVal;
*/
	return _Useable = _PrerequisitValid.getBool();
}


bool NoOpForCCtrlSheetInfo_Serial = false; // Prevent an assert in CDBCtrlSheet::serial (set externally) for a very specific case.
										   // The only case where an empty implementation is ok is for r2 in edition mode
										   // when building in scene interface for entities (the CDBCtrlSheet are hidden then ...!)										  /

// ***************************************************************************
void CDBCtrlSheet::serial(NLMISC::IStream &f)
{
	CCtrlBase::serial(f);
	if (NoOpForCCtrlSheetInfo_Serial) return;	// no-op for now
	nlassert(0); // !! IMPLEMENT ME !!
}

// ***************************************************************************
std::string CDBCtrlSheet::getContextHelpWindowName() const
{
	if (getType() == CCtrlSheetInfo::SheetType_SPhraseId)
	{
		return "action_context_help";
	}
	if (getType() == CCtrlSheetInfo::SheetType_Item)
	{
		const CItemSheet	*item= asItemSheet();
		if(item)
		{
			if (item->Family == ITEMFAMILY::CRYSTALLIZED_SPELL)
			{
				return "crystallized_spell_context_help";
			}
			else if (item->Family == ITEMFAMILY::JEWELRY || item->Family == ITEMFAMILY::ARMOR)
			{
				return "buff_item_context_help";
			}
		}
	}
	return CCtrlBase::getContextHelpWindowName();
}


// ***************************************************************************
void CDBCtrlSheet::setRegenTickRange(const CTickRange &tickRange)
{
	_RegenTickRange = tickRange;
}

// ***************************************************************************
void CDBCtrlSheet::startNotifyAnim()
{
	_NotifyAnimEndTime = T1 + NOTIFY_ANIM_MS_DURATION;
}





