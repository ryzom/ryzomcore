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



#ifndef RZ_DBCTRL_SHEET_H
#define RZ_DBCTRL_SHEET_H

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
// client
#include "nel/gui/reflect.h"
#include "nel/gui/ctrl_draggable.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/action_handler.h"
#include "sphrase_manager.h"
// game share
#include "game_share/brick_types.h"
#include "game_share/trade_slot_type.h"
#include "game_share/skills.h"
#include "game_share/slot_types.h"
#include "game_share/rm_family.h"
//
#include "../time_client.h"


class CItemSheet;
class CPactSheet;
class CDBCtrlSheet;
class CMacroCmd;
class IListSheetBase;
class CSBrickSheet;
class CSPhraseSheet;
class COutpostBuildingSheet;

namespace NLGUI
{
	class CViewRenderer;
}


// ***************************************************************************
/** Common info for CDBCtrlSheet and CDBGroupListSheet
 */
class CCtrlSheetInfo
{
public:

	enum TSheetType
	{
		// Items. sheetId points to a CItemSheet via SheetMngr
		SheetType_Item,
		// A pact
		SheetType_Pact,
		// A skill
		SheetType_Skill,
		// Special : the type of the sheet is automatically deduced from its sheet, it drawn like an item
		SheetType_Auto,
		// A macro (ie:'open window inventory + dock bag1')
		SheetType_Macro,
		// Flag of a guild
		SheetType_GuildFlag,
		// A mission
		SheetType_Mission,
		// New Sabrina Brick
		SheetType_SBrick,
		// New Sabrina Phrase. Index in the PhraseBook
		SheetType_SPhraseId,
		// New Sabrina Phrase. Complete phrase used for bot chat sentence (sheet id represent a CSPhraseSheet)
		SheetType_SPhrase,
		// A teleport location (just the slot bitmap)
		SheetType_Teleport_Location,
		// A guild teleport type
		SheetType_ElevatorDestination,
		// An outpost building
		SheetType_OutpostBuilding,
	};

	CCtrlSheetInfo();

	enum TBrickSheetSize
	{
		BrickSheetWidth= 24,
		BrickSheetHeight= 24,
	};

public:


	TSheetType			_Type;

	// If it is a brick or a spell, give all the brick type this slot accepts.
	// Each bit is for the associated BRICK_TYPE::EBrickType enum (unnknown bit 0 etc...)
	// Default is ALL (ie 0xFFFFFFFF)
	uint32				_BrickTypeBitField;

	// display.
	sint32				_DispNoSheetBmpId;	// Texture to display when no sheet (==0)
	sint32				_SheetSelectionGroup; // group for sheet selection, or -1 if none

	// Events
	IActionHandler		*_AHOnLeftClick;
	CStringShared		_AHLeftClickParams;
	IActionHandler		*_AHOnRightClick;
	CStringShared		_AHRightClickParams;
	//
	IActionHandler		*_AHOnCanDrag;
	CStringShared		_AHCanDragParams;
	IActionHandler		*_AHOnDrag;
	CStringShared		_AHDragParams;
	IActionHandler		*_AHOnCanDrop;
	CStringShared		_AHCanDropParams;
	IActionHandler		*_AHOnDrop;
	CStringShared		_AHDropParams;
	IActionHandler		*_AHOnCannotDrop;
	CStringShared		_AHCannotDropParams;

	CStringShared		_ListMenuLeft;
	CStringShared		_ListMenuRight;
	CStringShared		_ListMenuRightEmptySlot;

	//
	bool				_InterfaceColor        : 1; // Color given by the interface ?
	bool				_UseQuantity	       : 1; // is the quantity read and displayed ?
	bool				_ReadQuantityFromSheet : 1; // Read quantity from sheet rather than from database
	bool				_UseQuality            : 1; // is the quality read and displayed ?
	bool				_DisplayItemQuality    : 1; // Do we have to display the quality for the item (false for Cosmetics and Teleport and if _UseQuality==fasle)?
	bool                _DuplicateOnDrag       : 1; // when dragged, the item is shown twice : one version at the mouse position.
	                                             // and another in the source slot. Useful for items to buy that are in infinite quantity.
	bool				_AutoGrayed            : 1; // if true then gray the ctrlSheeet if: 1/ Items: Is the Item Locked. 2/ Bricks: is the brick Latent.
	bool                _HasTradeSlotType      : 1; // true is the SLOT_TYPE DB field should be taken in account

	bool				_BrickOverable         : 1;	// if Type is Brick, display like a button (because LeftClickable).

	bool				_DragCopy			   : 1; // true if support copy drag

	bool				_ForceItemBackGroundGeneric : 1;

	// For an Item only, this tells what item type this slot accept. Undefined means ALL
	SLOTTYPE::TSlotType	_ItemSlot;


public:
	bool parseCtrlInfo(xmlNodePtr cur, CInterfaceGroup * parentGroup);

	// Context menu accessor/ One for each button
	void setListMenuLeft (const std::string &cm) { _ListMenuLeft = cm; }
	void setListMenuRight (const std::string &cm) { _ListMenuRight = cm; }
	void setListMenuBoth (const std::string &cm) { _ListMenuLeft= _ListMenuRight= cm; }
	const std::string &getListMenuLeft () { return _ListMenuLeft; }
	const std::string &getListMenuRight () { return _ListMenuRight; }
	const std::string &getListMenuRightEmptySlot () {return _ListMenuRightEmptySlot;}
};

// ***************************************************************************
/**
 * Class representing base for items that are described in Georges forms
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2002
 */
class CDBCtrlSheet : public CCtrlDraggable, protected CCtrlSheetInfo
{
public:
	DECLARE_UI_CLASS(CDBCtrlSheet)

	// Release fucking statics
	static void release ();

	enum TSheetCategory { Item = 0, Pact, Skill, GuildFlag, Mission, Phrase, DontKnow };
public:

	CDBCtrlSheet(const TCtorParam &param);
	~CDBCtrlSheet();

	virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);

	virtual void updateCoords();
	virtual void draw();
	void drawSheet (sint32 scrX, sint32 scrY, bool draging, bool showSelectionBorder = true);

	virtual bool handleEvent (const NLGUI::CEventDescriptor &event);

	void setActionOnLeftClick (const std::string &ActionHandlerName) { _AHOnLeftClick = CAHManager::getInstance()->getAH(ActionHandlerName, _AHLeftClickParams); }
	void setActionOnRightClick (const std::string &ActionHandlerName) { _AHOnRightClick = CAHManager::getInstance()->getAH(ActionHandlerName, _AHRightClickParams); }
	void setActionOnDrop (const std::string &ActionHandlerName) { _AHOnDrop = CAHManager::getInstance()->getAH(ActionHandlerName, _AHDropParams); }
	void setActionOnCanDrop (const std::string &ActionHandlerName) { _AHOnCanDrop = CAHManager::getInstance()->getAH(ActionHandlerName, _AHCanDropParams); }
	void setParamsOnLeftClick (const std::string &ParamsHandlerName) { _AHLeftClickParams = ParamsHandlerName; }
	void setParamsOnRightClick (const std::string &ParamsHandlerName) { _AHRightClickParams = ParamsHandlerName; }
	void setParamsOnDrop (const std::string &ParamsHandlerName) { _AHDropParams = ParamsHandlerName; }
	void setParamsOnCanDrop (const std::string &ParamsHandlerName) { _AHCanDropParams = ParamsHandlerName; }

	const std::string &getActionOnLeftClick () const { return CAHManager::getInstance()->getAHName(_AHOnLeftClick); }
	const std::string &getActionOnRightClick () const { return CAHManager::getInstance()->getAHName(_AHOnRightClick); }
	const std::string &getActionOnDrop () const { return CAHManager::getInstance()->getAHName(_AHOnDrop); }
	const std::string &getActionOnCanDrop () const { return CAHManager::getInstance()->getAHName(_AHOnCanDrop); }
	const std::string &getParamsOnLeftClick () const { return _AHLeftClickParams; }
	const std::string &getParamsOnRightClick () const { return _AHRightClickParams; }
	const std::string &getParamsOnDrop () const { return _AHDropParams; }
	const std::string &getParamsOnCanDrop () const { return _AHCanDropParams; }

	void setListMenuLeft (const std::string &cm) { CCtrlSheetInfo::setListMenuLeft(cm); }
	void setListMenuRight (const std::string &cm) { CCtrlSheetInfo::setListMenuRight(cm); }
	void setListMenuBoth (const std::string &cm) { CCtrlSheetInfo::setListMenuBoth(cm); }
	const std::string &getListMenuLeft () { return CCtrlSheetInfo::getListMenuLeft(); }
	const std::string &getListMenuRight () { return CCtrlSheetInfo::getListMenuRight(); }
	const std::string &getListMenuRightEmptySlot () {return CCtrlSheetInfo::getListMenuRightEmptySlot();}

	void	setCanDrop (bool cd) { _CanDrop = cd; }
	bool	getCanDrop () const { return _CanDrop; }
	sint32	getDeltaDragX() {return _DeltaDragX;}
	sint32	getDeltaDragY() {return _DeltaDragY;}
	// For "oncandrag" action handlers only, which would want to avoid the drag
	void	setTempCanDrag(bool cd) {_TempCanDrag= cd;}
	// called when a setCapturePointerLeft(NULL) is made for instance

	CCtrlSheetInfo::TSheetType getType () const;
	void  setType (CCtrlSheetInfo::TSheetType type);

	// Swap the content with another ctrl_sheet (debug): SheetId, Quantity and Quality
	void	swapSheet(CDBCtrlSheet *other);
	void	setSheetId(sint32 val) {_SheetId.setSInt32(val);}
	void	setQuality(sint32 val) {_Quality.setSInt32(val);}
	void	setQuantity(sint32 val) {_Quantity.setSInt32(val);}
	void	setItemNameId(uint32 val) {_NameId.setSInt32(val);}
	void	setEnchant(sint32 val) {_Enchant.setSInt32(val);}

	// get sheet info.
	sint32	getSheetId() const { return (sint32)_SheetId.getSInt32(); }
	sint32	getQuality() const { return _UseQuality ? (sint32)_Quality.getSInt32() : 0; }
	sint32	getEnchant() const { return _Enchant.getSInt32(); }
	sint32	getQuantity() const { return (sint32)_Quantity.getSInt32(); }
	uint32	getItemNameId() const { return (uint32)_NameId.getSInt32();}
	// New Stack Size
	sint32	getStackable() const { return (_Stackable>1) ? 999 : 1; }

	// get non locked quantity (can be zero)
	sint32 getNonLockedQuantity() const;

	const CItemSheet  *asItemSheet() const;
	const CPactSheet *asPactSheet() const;
	const CSBrickSheet *asSBrickSheet() const;
	const CSPhraseSheet	*asSPhraseSheet() const;
	const COutpostBuildingSheet *asOutpostBuildingSheet() const;

	// Operation on the link between the sheet and database. the string is the branch root of SHEET....
	std::string getSheet() const;
	void	    setSheet (const std::string &dbBranchId);
	void		setSheetFast( const std::string &dbParentBranchId, int sheetNum, int slotNum );

	REFLECT_EXPORT_START(CDBCtrlSheet, CCtrlDraggable)
		REFLECT_STRING("sheet", getSheet, setSheet);
		REFLECT_RGBA("color", getSheetColor, setSheetColor);
		REFLECT_RGBA("color1", getGuildColor1, setGuildColor1);
		REFLECT_RGBA("color2", getGuildColor2, setGuildColor2);
		REFLECT_SINT32("back", getGuildBack, setGuildBack);
		REFLECT_SINT32("symbol", getGuildSymbol, setGuildSymbol);
		REFLECT_BOOL("invert_symbol", getInvertGuildSymbol, setInvertGuildSymbol);
		REFLECT_BOOL("can_drop", getCanDrop, setCanDrop);
		REFLECT_STRING ("left_click", getActionOnLeftClick, setActionOnLeftClick);
		REFLECT_STRING ("right_click", getActionOnRightClick, setActionOnRightClick);
		REFLECT_STRING ("left_click_params", getParamsOnLeftClick, setParamsOnLeftClick);
		REFLECT_STRING ("right_click_params", getParamsOnRightClick, setParamsOnRightClick);
		REFLECT_STRING ("on_drop", getActionOnDrop, setActionOnDrop);
		REFLECT_STRING ("on_drop_params", getParamsOnDrop, setParamsOnDrop);
		REFLECT_STRING ("on_can_drop", getActionOnCanDrop, setActionOnCanDrop);
		REFLECT_STRING ("on_can_drop_params", getParamsOnCanDrop, setParamsOnCanDrop);
		REFLECT_LUA_METHOD("getDraggedSheet",	luaGetDraggedSheet)
		REFLECT_LUA_METHOD("getHpBuff",	luaGetHpBuff)
		REFLECT_LUA_METHOD("getSapBuff",	luaGetSapBuff)
		REFLECT_LUA_METHOD("getFocusBuff",	luaGetFocusBuff)
		REFLECT_LUA_METHOD("getStaBuff",	luaGetStaBuff)
		REFLECT_LUA_METHOD("getName",		luaGetName)
		REFLECT_LUA_METHOD("getCreatorName", luaGetCreatorName)
		REFLECT_LUA_METHOD("waitInfo", luaWaitInfo)
		REFLECT_LUA_METHOD("buildCrystallizedSpellListBrick",		luaBuildCrystallizedSpellListBrick)
	REFLECT_EXPORT_END

	int luaGetDraggedSheet(CLuaState &ls);
	int luaGetHpBuff(CLuaState &ls);
	int luaGetSapBuff(CLuaState &ls);
	int luaGetFocusBuff(CLuaState &ls);
	int luaGetStaBuff(CLuaState &ls);
	int luaGetName(CLuaState &ls);
	int luaGetCreatorName(CLuaState &ls);
	int luaWaitInfo(CLuaState &ls);
	int luaBuildCrystallizedSpellListBrick(CLuaState &ls);

	// hardcode creation. User must setup other CtrlBase value (parent etc...)
	void	initSheet(const std::string &dbValue, const CCtrlSheetInfo &ctrlInfo);
	void	initSheetFast( const std::string &dbParentBranchId, int sheetNum, int slotNum,
		const CCtrlSheetInfo &ctrlInfo );

	// get the selection group for this ctrl sheet, or -1 if none
	sint                   getSelectionGroup() const { return _SheetSelectionGroup; }
	// get the selection group for this ctrl sheet as a string, or "" if none
	const std::string	  &getSelectionGroupAsString() const;

	// get the currently selected sheet (only one at a time)
	static CDBCtrlSheet   *getCurrSelection() { return _CurrSelection; }

	// set the currently selected sheet.
	static	void		   setCurrSelection(CDBCtrlSheet *selected);

	// get the root branch containing the properties for that sheet
	NLMISC::CCDBNodeBranch        *getRootBranch() const;

	/** If the branch in setSheet(branch) is of the form ...:# (where # is a number), return this number.
	 *	The value is hence modified by setSheet(). return 0 if not of this form.
	 */
	uint32	getIndexInDB() const {return _IndexInDB;}
	/** If the branch in setSheet(branch) is of the form ...:#:# (where # are numbers), return this number.
	 *	The value is hence modified by setSheet(). return 0 if not of this form.
	 */
	uint32	getSecondIndexInDB() const {return _SecondIndexInDB;}

	// synonym for getSecondIndexInDB().
	uint32	getInventoryIndex() const {return getSecondIndexInDB();}

	// determine the inventory slot from the database branch id
	static uint getInventorySlot( const std::string &dbBranchId );

	// Get the last dropped sheet. The pointer is only valid during the call of the event handler
	//static CDBCtrlSheet	  *getDraggedSheet() { return _LastDraggedSheet; }

	/* Get the last selected sheet that have been draged or right clicked (should be use by menu to get their caller)
	 * It is used by the item actions like destroy, move etc..
	 */
	static CDBCtrlSheet	  *getCurrSelSheet() { return _CurrMenuSheet; }
	static void			   setCurrSelSheet(CDBCtrlSheet *sheet) { _CurrMenuSheet = sheet; }


	/// \name SBrick / SPhrase
	// @{
	// true if the ctrl_sheet is a Sabrina brick
	bool					isSBrick() const	{return getType() ==SheetType_SBrick;}
	// true if magic sentence is a Sabrina phrase
	bool					isSPhraseId() const {return getType()==SheetType_SPhraseId;}
	// true if magic sentence is a Sabrina phrase
	bool					isSPhrase() const {return getType()==SheetType_SPhrase;}
	// true if brick or magic sentence
	bool					isSBrickOrSPhraseId() const
	{
		CCtrlSheetInfo::TSheetType	type= getType();
		return type==SheetType_SBrick || type==SheetType_SPhraseId;
	}
	bool					isSPhraseIdMemory() const;
	// special for macro and Memories
	bool					isMacroMemory() const;
	// return the phrase slot from database.
	sint32					getSPhraseId() const;
	/// true if it is a shortcut
	bool					isShortCut() const {return _ShortCut;}
	// @}


	/// Setup the alpha of the sheet drawn
	void					setSheetColor(NLMISC::CRGBA color) {_SheetColor= color;}
	NLMISC::CRGBA			getSheetColor() const {return _SheetColor;}

	/// Special ContextHelp for ctrl sheet.
	virtual void			getContextHelp(ucstring &help) const;

	virtual void			getContextHelpToolTip(ucstring &help) const;


	/** true if an item of another ctrlSheet can be dropped on this slot.
	 *	also return true if src is 0, or if _ItemSlot==UNDEFINED
	 */
	bool					canDropItem(CDBCtrlSheet *src) const;

	/// Force the item/brick to be grayed. NB: no effect if "auto_grayed" is true
	void					setGrayed(bool state) { _Grayed= state;}
	bool					getGrayed() const
	{
		// If The Item sheet or phrase is 0, assume never grayed
		if(_SheetId.getNodePtr() && getSheetId() == 0)
			return false;
		// NB: if a macro, getNodePtr()==NULL
		else
			return _Grayed;
	}
	// Additional gray for items. The item is finally grayed if one of this
	void					setItemWeared(bool state) {_ItemWeared= state;}
	bool					getItemWeared() const { return _ItemWeared;}
	void					setItemBeastGrayed(bool state) {_ItemBeastGrayed= state;}
	bool					getItemBeastGrayed() const { return _ItemBeastGrayed;}

	void					setUseQuality(bool use) { if(use!=_UseQuality) { _UseQuality= use; _NeedSetup= true;} }
	void					setUseQuantity(bool use) { if(use!=_UseQuantity) { _UseQuantity= use; _NeedSetup= true;} }
	bool                    getUseQuality() const { return _UseQuality; }
	bool                    getUseQuantity() const { return _UseQuantity; }
	//
	void					setReadQuantityFromSheetFlag(bool on) { _ReadQuantityFromSheet = on; }
	bool					getReadQuantityFromSheetFlag() const { return _ReadQuantityFromSheet; }

	// test if the sheet is a skill sheet
	bool					isSkill() const { return _HasTradeSlotType ? _TradeSlotType.getSInt32() == TRADE_SLOT_TYPE::Skill : false; }

	// test if the sheet is a mission sheet
	bool					isMission() const;

	// If the sheet is a skill, just get it
	SKILLS::ESkills         getSkill() const { return isSkill() ? (SKILLS::ESkills) getSheetId() : SKILLS::unknown; }


	// set special behaviour
	void							setBehaviour(TRADE_SLOT_TYPE::TTradeSlotType type);
	TRADE_SLOT_TYPE::TTradeSlotType getBehaviour() const { return _HasTradeSlotType ? (TRADE_SLOT_TYPE::TTradeSlotType) _TradeSlotType.getSInt32() : TRADE_SLOT_TYPE::StandardBehaviour; }

	SLOTTYPE::TSlotType	getItemSlot() { return _ItemSlot; }
	void setItemSlot(SLOTTYPE::TSlotType slot) { _ItemSlot = slot; }
	void setTextureNoItem(sint32 nTxID) { _DispNoSheetBmpId = nTxID; }
	sint32 getTextureNoItem() { return _DispNoSheetBmpId; }

	// copy the aspect of this sheet (not the handlers)
	void							copyAspect(CDBCtrlSheet *dest);
	// true if same aspects.
	bool							sameAspect(CDBCtrlSheet *dest) const;

	// get the sheet category
	TSheetCategory					getSheetCategory() const;

	// get the ListSheet parent. NB: different code from CDBGroupListSheet and CDBGroupListSheetTrade. NULL if not found
	IListSheetBase					*getListSheetParent() const;

	// get the index of this sheet in its parent list, or -1 if there's no such list
	sint							getIndexInParent() const;

	// get the 'LOCKED' field in the db
	NLMISC::CCDBNodeLeaf		*getLockValuePtr() { return _GrayedLink; }

	/// \name Macro
	// @{
	void			readFromMacro(const CMacroCmd &mc);	// Macro To ControlSheet
	void			writeToMacro(CMacroCmd &mc);			// ControlSheet To Macro
	void			setMacroBack(uint8 nb);
	void			setMacroIcon(uint8 nb);
	void			setMacroOver(uint8 nb);
	void			setMacroText(const std::string &mcText);
	sint32			getMacroId() const {return _MacroID;}
	bool			isMacro() const {return getType()==SheetType_Macro;}
	// @}

	/** According to ActualType, return true if the sheet is valid. eg: getSheetId()!=0 and brick exist
	 *	for sheet type brucks. Always for Macros and skills.
	 */
	bool							isSheetValid() const;

	/// \name Guild Flag
	// @{
	NLMISC::CRGBA	getGuildColor1() const;
	NLMISC::CRGBA	getGuildColor2() const;
	sint32			getGuildBack() const;
	sint32			getGuildSymbol() const;
	bool			getInvertGuildSymbol() const;

	void setGuildColor1(NLMISC::CRGBA col);
	void setGuildColor2(NLMISC::CRGBA col);
	void setGuildBack(sint32 n);
	void setGuildSymbol(sint32 n);
	void setInvertGuildSymbol(bool b);
	// @}

	/// \name For teleport location
	// @{
	void setSlot(const std::string &textureName);
	void initSheetSize();
	// @}

	NLMISC::CCDBNodeLeaf *getSlotType() const { return _TradeSlotType.getNodePtr(); }

	// get item weight
	uint16 getItemWeight() const;
	NLMISC::CCDBNodeLeaf *getItemWeightPtr() const;
	// set item weight
	void setItemWeight(uint16 weight);

	// get item info version
	uint8 getItemInfoVersion() const;
	NLMISC::CCDBNodeLeaf *getItemInfoVersionPtr() const;
	// set item info version
	void setItemInfoVersion(uint8 infoVersion);

	// get item Locked state
	uint16 getItemLocked() const;
	NLMISC::CCDBNodeLeaf *getItemLockedPtr() const;
	// set item locked state
	void setItemLocked(uint16 lock);

	// get item PRICE. 0 if no DB
	sint32 getItemPrice() const;
	NLMISC::CCDBNodeLeaf *getItemPricePtr() const;
	// set item PRICE
	void setItemPrice(sint32 price);

	// get item RESALE_FLAG. 0 if no DB
	sint32 getItemResaleFlag() const;
	NLMISC::CCDBNodeLeaf *getItemResaleFlagPtr() const;
	// set item RESALE_FLAG
	void setItemResaleFlag(sint32 rf);

	// get item locked by owner
	bool getLockedByOwner() const;

	// true if the inventory supports owner locking
	bool canOwnerLock() const;

	// get item SELLER_TYPE. 0 if no DB
	sint32 getItemSellerType() const;
	NLMISC::CCDBNodeLeaf *getItemSellerTypePtr() const;
	// set item SELLER_TYPE
	void setItemSellerType(sint32 rf);

	// get item FABER_QUALITY. 0 if no DB
	RM_CLASS_TYPE::TRMClassType getItemRMClassType() const;
	NLMISC::CCDBNodeLeaf *getItemRMClassTypePtr() const {return _ItemRMClassType;}
	// set item FABER_QUALITY
	void setItemRMClassType(sint32 fq);

	// get item FABER_STAT_TYPE. 0 if no DB
	RM_FABER_STAT_TYPE::TRMStatType getItemRMFaberStatType() const;
	NLMISC::CCDBNodeLeaf *getItemRMFaberStatTypePtr() const {return _ItemRMFaberStatType;}
	// set item FABER_STAT_TYPE
	void setItemRMFaberStatType(sint32 fss);

	// get item PREREQUISIT_VALID. true of no DB
	bool getItemPrerequisitValid() const;
	NLMISC::CCDBNodeLeaf *getItemPrerequisitValidPtr() const;
	// set item PREREQUISIT_VALID
	void setItemPrerequisitValid(bool prv);

	// get color index of item (return -1 if no color available)
	sint32	getItemColor() const {if(_UserColor) return _UserColor->getValue32(); else return -1;}
	// set item color (if possible)
	void	setItemColor(sint32 val) {if(_UserColor) _UserColor->setValue32(val);}

	// Get the Actual item name. Localized version of SheetId, or given by server through NAMEID.
	ucstring getItemActualName() const;

	/// true if support drag copy (with CTRL). action handler has to check control.
	bool	canDragCopy() const {return _DragCopy;}

	// special for items, call it at initInGame()
	static void initArmourColors();

	// true if an item that respect Carac requirement. NB: still return true if not an item
	bool	checkItemRequirement();

	virtual void serial(NLMISC::IStream &f);

	// From CCtrlBase, for phrases, we use a special, enhanced tooltip window
	virtual std::string getContextHelpWindowName() const;

	// For auras, powers, etc. set the range of ticks during which regen occurs
	void	setRegenTickRange(const CTickRange &tickRange);
	const CTickRange &getRegenTickRange() const { return _RegenTickRange; }
	
	// start notify anim (at the end of regen usually)
	void	startNotifyAnim();

protected:

	void setupItem();
	void setupPact();
	void setupMacro();
	void setupGuildFlag();
	void setupMission();
	void setupSBrick();
	void setupSPhrase();
	void setupSPhraseId();
	void setupOutpostBuilding();
	// optSheet is for special faber
	void setupDisplayAsSBrick(sint32 sheet, sint32 optSheet= 0);
	// setup icon from phrases
	void setupDisplayAsPhrase(const std::vector<NLMISC::CSheetId> &bricks, const ucstring &phraseName);

	// draw a number and returns the width of the drawn number
	sint32 drawNumber(sint32 x, sint32 y, sint32 wSheet, sint32 hSheet, NLMISC::CRGBA color, sint32 value, bool rightAlign=true);


protected:

	// Root branch of the DB
	std::string			_DbBranchName;

	// items db entries
	CInterfaceProperty	_SheetId;
	CInterfaceProperty	_NameId;
	CInterfaceProperty	_Quantity;
	CInterfaceProperty	_Quality;
	CInterfaceProperty	_TradeSlotType;
	CInterfaceProperty	_Enchant;
	CInterfaceProperty  _PrerequisitValid;
	CInterfaceProperty	_Worned; // if true means that item is worned (red cross, no longer usable unless it's a tool)

	// As node leaf for backward compatibilities
	NLMISC::CCDBNodeLeaf		*_ItemRMClassType;
	NLMISC::CCDBNodeLeaf		*_ItemRMFaberStatType;

	mutable sint32		_LastSheetId;

	/// Display
	sint32				_DispSlotBmpId;		// Display slot bitmap id
	sint32				_DispSelSlotId;

	sint32				_DispBackBmpId;		// Back Icon
	sint32				_DispSheetBmpId;	// Main Icon
	sint32				_DispOverBmpId;		// Over Icon
	sint32				_DispOver2BmpId;	// Over Icon N0 2 for bricks / items. Useful for items when _DispOverBmpId is used to paint user color on the item.

	// Level Brick or Quality
	union
	{
		sint32				_DispQuality;
		sint32				_DispLevel;
	};
	// Quantity for items
	sint32				_DispQuantity;
	sint32				_Stackable;

	sint32				_TextureIdOver;

	uint32				_IndexInDB;
	uint32				_SecondIndexInDB;

	// For SBrick only. Display level?
	bool		_MustDisplayLevel	: 1;

	/// Events

	bool		_CanDrop			: 1;
	bool		_Over				: 1;

	/// Is a TaskBar shortcut
	bool		_ShortCut			: 1;

	/// Draw the slot ?
	bool		_DrawSlot			: 1;

	/// Is the Item/Brick as to be grayed?
	bool		_ItemBeastGrayed	: 1; // This is an item in a pack animal / beast, that is unavailable
	bool		_ItemWeared			: 1; // Item weared (for bags item)
	bool		_Grayed				: 1;
	bool		_Useable			: 1; // if false, means more than grayed: "red-ed" we cannot get or use it. SBrick, SPhrase and SItem

	/// Init after parse()
	bool			_SetupInit		: 1;

	/// true if need a full setup in setupItem() ...
	mutable bool	_NeedSetup		: 1;

	// Temp for oncandrag AH
	bool			_TempCanDrag	: 1;

	// For ArmourColorFromDB
	bool			_ArmourColorFromDB	: 1;

	// True if the Armour Color Bitmaps have been setuped
	bool			_ArmourColorBmpOk	: 1;

	// Bkup of Armour Color index. -1 or 0..7
	sint8			_ArmourColorIndex;

	CTickRange		_RegenTickRange;

	/// D'n'd
	sint32		_DragX, _DragY;
	sint32		_DeltaDragX, _DeltaDragY;
	sint32      _IconW, _IconH;

	// Global color of the control.
	NLMISC::CRGBA		_SheetColor;

	// Special colors (item/bricks)
	NLMISC::CRGBA		_IconBackColor;
	NLMISC::CRGBA		_IconColor;
	NLMISC::CRGBA		_IconOverColor;
	NLMISC::CRGBA		_IconOver2Color;
	// Item only: Special armour color. black(0) if is not an armour
	uint32				_PackedArmourColor;


	// For an Item only. Useful for LeftHand Filtering: must have a pointer to the right hand
	CDBCtrlSheet		*_OtherHandItemFilter;

	// This String is optional and usage dependent for Item, Macro, or Sentence
	std::string			_OptString;

	NLMISC::CCDBNodeLeaf		*_GrayedLink;

	// Macro or sentence String compiled as texture Ids and positions, from the _OptString.
	struct	CCharBitmap
	{
		sint32	X,Y;
		sint32	Id;
	};
	std::vector<CCharBitmap> _CharBitmaps;

	// Macro Id
	sint32				_MacroID;

	// Guild Flag
	NL3D::UTextureFile *_GuildBack;
	NL3D::UTextureFile *_GuildSymb;
	static NL3D::UMaterial _GuildMat;
	bool _UseGuildIcon;
	NLMISC::CSheetId _GuildIcon;

	// SPhrase version
	sint32				_LastPhraseVersion;

	// special for items : colour of armour
	static NLMISC::CRGBA _ArmourColor[8];

	// special for items. Carac Requirement
	CHARACTERISTICS::TCharacteristics	_ItemCaracReqType;
	sint32								_ItemCaracReqValue;

	// Special for Armour
	NLMISC::CCDBNodeLeaf		*_UserColor;

	// keep pointer on item sheet
	const CItemSheet	*_ItemSheet;

	// unique persistent phrase exposed to lua script
	static NLMISC::CSmartPtr<CSPhraseComAdpater> _PhraseAdapter;

	sint64		_NotifyAnimEndTime;

private:
	mutable TSheetType			_ActualType;

	static		CDBCtrlSheet *_CurrSelection;
	static		CDBCtrlSheet *_CurrMenuSheet;
private:
	void		updateActualType() const;
	void		updateIconSize();
	void		resetAllTexIDs();
	void		setupInit();

	void		setupCharBitmaps(sint32 maxW, sint32 maxLine, sint32 maxWChar= 1000, bool topDown= false);
	void		resetCharBitmaps();
	void		displayCharBitmaps(sint32 rdrLayer, sint32 x, sint32 y, NLMISC::CRGBA color);

	// special for items
	void		updateItemCharacRequirement(sint32 sheetId);

	// update armour color, and cache
	void		updateArmourColor(sint8 col);

	// setup sheet DB. _DbBranchName must be ok, and _SecondIndexInDb and _IndexInDb as well
	void	    setupSheetDbLinks ();

	
	// 'regen' rendering
	// convert from uv coordinates in the [0, 1] x [0, 1] range to screen coords
	inline void uvToScreen(float x, float y, NLMISC::CVector &screenPos, uint texSize) const;
	// from an angle in the [0, 1] range, return both uv & screen coords for that angle 
	// (angle is projected on the side of rectangle of the 'regen' texture)
	void buildPieCorner(float angle, NLMISC::CUV &uv, NLMISC::CVector &pos, uint texSize) const;
	// from a start and end angle in the [0, 1] range, build the set of uv mapped triangles necessary
	// to display that 'pie'
	// NB : output are indeed quads, at the time ofthis writing, uv mappedtri randering not available ...
	// so we turn them into quad for conveniency ...
	uint buildPie(NLMISC::CQuadUV *quv, float startAngle, float endAngle, uint texSize);

	// gelper to draw the notify animation
	void drawRotatedQuad(CViewRenderer &vr, float angle, float scale, uint renderLayer, uint32 textureId, sint32 texWidth, sint32 texHeight);

};

/** User type (used with expression system of the interface, see interface_expr.h, that contains a pointer to a CDBCtrlSheet
  */
struct CDBCtrlSheetPtrUserType : public CInterfaceExprUserType
{
  CDBCtrlSheet *Sheet; // pointer to a sheet
  // ctor
  CDBCtrlSheetPtrUserType(CDBCtrlSheet *sheet = NULL) : Sheet(sheet) {}
  // from CInterfaceExprUserType
  virtual CInterfaceExprUserType *clone() const { return new CDBCtrlSheetPtrUserType(*this); }
};


#endif // RZ_DBCTRL_SHEET_H

/* End of dbctrl_sheet.h */
