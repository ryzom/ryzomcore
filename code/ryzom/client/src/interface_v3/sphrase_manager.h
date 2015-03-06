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

#ifndef RY_SPHRASE_MANAGER_H
#define RY_SPHRASE_MANAGER_H

#include "nel/misc/types_nl.h"
#include "game_share/sphrase_com.h"
#include "game_share/skills.h"
#include "game_share/brick_types.h"
#include "game_share/resistance_type.h"
#include "req_skill_formula.h"
#include "brick_learned_callback.h"
#include "skill_change_callback.h"
#include "trade_common.h"
#include "nel/gui/interface_element.h"
#include "../time_client.h"


// ***************************************************************************
const	std::string		PHRASE_DB_BOOK="UI:PHRASE:BOOK";
const	std::string		PHRASE_DB_PROGRESSION[2]= {"UI:PHRASE:PROGRESS_ACTIONS", "UI:PHRASE:PROGRESS_UPGRADES"};
const	std::string		PHRASE_DB_MEMORY="UI:PHRASE:MEMORY";
const	std::string		PHRASE_DB_MEMORY_ALT="UI:PHRASE:MEMORY_ALT";
const	std::string		PHRASE_DB_EXECUTE_NEXT="UI:PHRASE:EXECUTE_NEXT:PHRASE";
const	std::string		PHRASE_DB_EXECUTE_NEXT_IS_CYCLIC="UI:PHRASE:EXECUTE_NEXT:ISCYCLIC";
const	std::string		PHRASE_DB_BOTCHAT="LOCAL:TRADING";
#define	PHRASE_MAX_BOOK_SLOT		512
#define	PHRASE_MAX_PROGRESSION_SLOT	512
#define	PHRASE_MAX_MEMORY_SLOT		20
#define PHRASE_MAX_BOTCHAT_SLOT		TRADE_MAX_ENTRIES
// u16 only for client/server com.
#define	PHRASE_MAX_ID				65535

// For phrase execution counter
#define	PHRASE_EXECUTE_COUNTER_MASK		255
const	std::string		PHRASE_DB_COUNTER_NEXT="SERVER:EXECUTE_PHRASE:NEXT_COUNTER";
const	std::string		PHRASE_DB_COUNTER_CYCLE="SERVER:EXECUTE_PHRASE:CYCLE_COUNTER";


// TODO_OPTIM: too big test for the list_sheet* each frame with 512 entries!!!

class	CSuccessTableSheet;
namespace NLMISC{
class	CCDBNodeLeaf;
}


/** Special helper class for lua export : enclose a phrase into an object accessible to lua
  * This way we can setup the phrase tooltip in lua code instead of hardcoded C++ stuff letting
  * the possibility to the user to customize it to his liking.
  */
class CSPhraseComAdpater : public CReflectableRefPtrTarget
{
public:
	CSPhraseCom		Phrase;
	REFLECT_EXPORT_START(CSPhraseComAdpater, CInterfaceElement)
		REFLECT_LUA_METHOD("getCastTime",	luaGetCastTime)
		REFLECT_LUA_METHOD("getCastRange",  luaGetCastRange)
		REFLECT_LUA_METHOD("getHpCost",		luaGetHpCost)
		REFLECT_LUA_METHOD("getSapCost",	luaGetSapCost)
		REFLECT_LUA_METHOD("getFocusCost",	luaGetFocusCost)
		REFLECT_LUA_METHOD("getStaCost",	luaGetStaCost)
		REFLECT_LUA_METHOD("getName",		luaGetName)
		REFLECT_LUA_METHOD("getDesc",		luaGetDesc)
		REFLECT_LUA_METHOD("getSuccessRate",		luaGetSuccessRate)
		REFLECT_LUA_METHOD("isMagicPhrase", luaIsMagicPhrase)
		REFLECT_LUA_METHOD("isCombatPhrase", luaIsCombatPhrase)
		REFLECT_LUA_METHOD("isPowerPhrase", luaIsPowerPhrase)
		REFLECT_LUA_METHOD("getRegenTime", luaGetRegenTime)
		REFLECT_LUA_METHOD("getTotalRegenTime", luaGetTotalRegenTime)
		REFLECT_LUA_METHOD("getPowerDisableTime", luaGetPowerDisableTime)
	REFLECT_EXPORT_END
	int luaGetCastTime(CLuaState &ls);
	int luaGetCastRange(CLuaState &ls);
	int luaGetHpCost(CLuaState &ls);
	int luaGetSapCost(CLuaState &ls);
	int luaGetSuccessRate(CLuaState &ls);
	int luaGetFocusCost(CLuaState &ls);
	int luaGetStaCost(CLuaState &ls);
	int luaGetName(CLuaState &ls);
	int luaGetDesc(CLuaState &ls);
	int luaIsMagicPhrase(CLuaState &ls);
	int luaIsCombatPhrase(CLuaState &ls);
	int luaIsPowerPhrase(CLuaState &ls);
	int luaGetRegenTime(CLuaState &ls);
	int luaGetTotalRegenTime(CLuaState &ls);
	int luaGetPowerDisableTime(CLuaState &ls);
};



// ***************************************************************************
/**
 * Singleton to Get/Set Sabrina Phrase in SpellBook / Memory.
 *	NB: you MUST create it (getInstance()), before loading of ingame.xmls.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CSPhraseManager
{
public:
	static	CSPhraseManager		*getInstance()
	{
		if(!_Instance)
			_Instance= new CSPhraseManager;
		return _Instance;
	}

	// release singleton
	static void releaseInstance();

	// The Slot 0 of the Book indicate "NoPhrase".
	// The Slot 1 indicate the Edited Phrase for phrase composition, and is not "really" a part of the book.
	enum	{EditionSlot= 1, BookStartSlot= 2};

	/// To call when The DB inGame is setup. Else, no write is made to it before. (NB: DB is updated here)
	void				initInGame();

	/// Reset
	void				reset();

	/// Call juste once when interface is built
	void				updateMemoryBar();

	/// Call once per pass
	void				update();

	// \name Book Edition
	// @{
	/** set a phrase on a slot.
	 *	\param lock special for BotChat phrase buy. when true, the phrase is never written in the book DB
	 *		the server has to unlock it (or delete if not confirmed)
	 *	NO MSG SENT
	 */
	void				setPhrase(uint32 slot, const CSPhraseCom &phrase, bool lock= false);

	/** Same as setPhrase() but do not update the DB of book now
	 *	User can use this if lot of phrase must be chaned in one time.
	 *	User must call updateBookDB() after setting all those phrases.
	 *	Nb: suppose lock==false.
	 */
	void				setPhraseNoUpdateDB(uint32 slot, const CSPhraseCom &phrase);

	/// update Action Book Database. You need only to use it in conjunction with setPhraseNoUpdateDB()
	void				updateBookDB();

	/// erase a phrase on a slot. NO MSG SENT
	void				erasePhrase(uint32 slot);

	/// get a phrase for a slot. Empty Phrase returned if don't exist. O(logN)
	const CSPhraseCom	&getPhrase(uint32 slot) const;

	/// get the number of memory lines.
	uint32				getNbMemoryLines() const { return (uint32)_Memories.size(); }

	/** Allocate a Free Slot (for NEW phrase setup). 0 is not a free slot
	 *	NB: the slot is not still correctly filled(), and getPhrase() will return Empty. use setPhrase() just after
	 *	\return 0 if not possible (eg too big)
	 */
	uint32				allocatePhraseSlot();

	/// true if a free slot is available (don't allocate)
	bool				hasFreeSlot() const;

	/// get a phrase version for a slot. -1 if phrase not present. O(1)
	sint32				getPhraseVersion(uint32 slot) const;

	// Receive a BotChat phrase Buy confirmation: NB: memorize it (and server send) if possible
	void				receiveBotChatConfirmBuy(uint16 phraseId, bool confirm);

	// test if the given phrase is known
	bool				isPhraseKnown(const CSPhraseCom &phrase) const;

	/** Filter the BOOK DB with this brickType or skill.
	 *	 NB: if brickTypeFilter is UNKNOWN then filtered by skill
	 *	 il filterSkill is unknown, then not filtered (all match). This is the default.
	 */
	void				setBookFilter(BRICK_TYPE::EBrickType brickTypeFilter, SKILLS::ESkills skillFilter);

	/// get the book skill filter
	SKILLS::ESkills		getBookSkillFilter() const {return _BookSkillFitler;}

	/// Send the PHRASE:DELETE message to the server
	void				sendDeleteToServer(uint32 slot);

	/// Send the PHRASE:LEARN message to the server
	void				sendLearnToServer(uint32 slot);

	/// This is a clean full delete: forget all phrase before, erase the phrase, send msg to server
	void				fullDelete(uint32 slot);

	/** Do it only if the phrase id is only used in this memory slot
	 *	This is a clean full delete: forget all phrase before, erase the phrase, send msg to server
	 */
	void				fullDeletePhraseIfLast(uint32 memoryLine, uint32 memorySlot);

	// @}


	// \name Memory Edition
	// @{

	/// Memorize a phrase (no MSG send)
	void				memorizePhrase(uint32 memoryLine, uint32 memorySlot, uint32 slot);
	/// Forget a phrase (no MSG send). NB: no-op if it is a macro
	void				forgetPhrase(uint32 memoryLine, uint32 memorySlot);
	/// Get the phrase memorized. 0 if not found or if it is a macro
	uint32				getMemorizedPhrase(uint32 memoryLine, uint32 memorySlot) const;

	/// Memorize a macro (no MSG send). if was a phrase, forget first (client and server)
	void				memorizeMacro(uint32 memoryLine, uint32 memorySlot, uint32 macroId);
	/// Forget a macro. NB: no-op if it is a phrase
	void				forgetMacro(uint32 memoryLine, uint32 memorySlot);
	/// Get the phrase memorized. 0 if not found or if it is a macro
	uint32				getMemorizedMacro(uint32 memoryLine, uint32 memorySlot) const;
	/// return true if it is a memorized macro. false if empty or if it is a phrase
	bool				isMemorizedMacro(uint32 memoryLine, uint32 memorySlot) const;
	/// To Be called when the macro visual changes
	void				updateMacroShortcuts(sint32 macroId);
	/// To be called when the Id changes
	void				deleteMacroShortcuts(sint32 macroId);

	/// Only one memory line is displayed in the Memory DB. if -1, erased.
	void				selectMemoryLineDB(sint32 memoryLine);
	void				selectMemoryLineDBalt(sint32 memoryLine);
	/// get the selected memory line.
	sint32				getSelectedMemoryLineDB() const {return _SelectedMemoryDB;}
	sint32				getSelectedMemoryAltLineDB() const {return _SelectedMemoryDBalt;}
	/// Common Method to send the Memorize msg to server
	void				sendMemorizeToServer(uint32 memoryLine, uint32 memorySlot, uint32 phraseId);
	/// Common Method to send the Forget msg to server
	void				sendForgetToServer(uint32 memoryLine, uint32 memorySlot);

	/// count all memories that use this phrase
	uint				countAllThatUsePhrase(uint32 slot);
	/// Forget all Memories that use this phrase slot, AND send appropriate message to server. if sendMsgOnly==true, only the msg is sent to server (no modification to local)
	void				forgetAllThatUsePhrase(uint32 slot, bool sendMsgOnly= false);
	/// Rememorize all memories that use this slot
	void				rememorizeAllThatUsePhrase(uint32 slot);

	/// Update all Ctrl State of the action Bar, according to macro/itemInRightHand/available states
	void				updateAllMemoryCtrlState();

	/// Load or save the macro
	void				serialMacroMemory(NLMISC::IStream &f);

	/// is this phrase sheet can be memorized / casted?
	bool				isPhraseCastable(uint32 sheetId) const;
	bool				isPhraseCastable(class CSPhraseSheet *phraseSheet) const;
	bool				isPhraseCharacBuying(uint32 sheetId) const;
	bool				isPhraseCharacBuying(class CSPhraseSheet *phraseSheet) const;

	/** return getMemorizedPhrase(memoryLine, memoryIndex), only if this phrase is only used in this memory slot
     *	else return allocatePhraseSlot()
	 */
	uint32				getMemorizedPhraseIfLastOrNewSlot(uint32 memoryLine, uint32 memoryIndex);

	// @}


	// \name Phrase composition
	// @{
	// The phrase edited/created. If 0, it may be a phrase newly created, but not still assigned.
	uint32				CompositionPhraseId;
	// For phrase created only. If not -1, then at creation, will be auto-memorized to this slot
	sint32				CompositionPhraseMemoryLineDest;
	sint32				CompositionPhraseMemorySlotDest;


	/// For BotChat learning, build a phrase from .sphrase sheetId
	void				buildPhraseFromSheet(CSPhraseCom &phrase, sint32 sheetId);

	/** For a given phrase, see if there's a matching sheet, or 0 else.
	  * NB : O(1) algo with respect to the number of phrase sheets
	  */
	sint32				getSheetFromPhrase(const CSPhraseCom &phrase) const;

	// @}


	// \name Phrase Execution
	// @{
	/// Start execution from Client. Only the bitmaps are displayed. A sendExecuteToServer() or a cancelClientExecution() must follow for each clientExecute
	void				clientExecute(uint32 memoryLine, uint32 memorySlot, bool cyclic);
	/// Common Method to send the execution msg to server
	void				sendExecuteToServer(uint32 memoryLine, uint32 memorySlot, bool cyclic);
	/// Cancel the client execution (case when used with Move in combat for instance)
	void				cancelClientExecute(bool cyclic);
	/// Execute a craft action (after selecting the plan and MPs), on both client and server side
	void				executeCraft(uint32 memoryLine, uint32 memorySlot, uint32 planSheetId,
		std::vector<CFaberMsgItem> &mpItemPartList, std::vector<CFaberMsgItem> specificItemList);
	/// Execute a cristalize action on both client and server side
	void				executeCristalize(uint32 memoryLine, uint32 memorySlot);
	/** Execute a default attack, on both client and server side.
	 *	NB: try first to launch a standard action "default attack" if found,
	 *	calling clientExecute() and sendExecuteToServer()
	 */
	void				executeDefaultAttack();
	/// Search the default attack like on client. false if not found. NB: find preferly in current memoryLine
	bool				findDefaultAttack(uint32 &memoryLine, uint32 &memorySlot);

	uint				getPhraseNextExecuteCounter() const {return _PhraseNextExecuteCounter;}
	uint				getPhraseCycleExecuteCounter() const {return _PhraseCycleExecuteCounter;}

	/// Server Execution ACK
	void				receiveAckExecuteFromServer(bool cyclic, uint counterValue, bool ok);

	/// get the Next PhraseId Executed (0 if none)
	uint32				getNextExecutePhraseId() const {return _CurrentExecutePhraseIdNext;}
	/// get the Cycle PhraseId Executed (0 if none)
	uint32				getCycleExecutePhraseId() const {return _CurrentExecutePhraseIdCycle;}

	// @}

	// \name Misc
	// @{
	/** Build the Text description of a phrase (spell). NB: this is a TagFormated text
	 *	\phraseSheetId not 0 if comes from a .sphrase Sheet => used to show progression info, and display phrase description
	 *  \specialPhraseFormat if empty, format is auto selected. if "composition", same but the text is cut under the %compostart tag.
	 *		else take directly this format.
	 */
	void				buildPhraseDesc(ucstring &text, const CSPhraseCom &phrase, uint32 phraseSheetId, bool wantRequirement, const std::string &specialPhraseFormat= std::string());
	// Get the Phrase Success Rate %
	sint				getPhraseSuccessRate(const CSPhraseCom &phrase);
	// Get the Phrase Success Rate %. Manually gives the Skill to do the comparison (for craft)
	sint				getCraftPhraseSuccessRate(const CSPhraseCom &phrase, SKILLS::ESkills skill, uint minMpLevel, sint successModifier);
	// Get the Phrase Success Rate %. Manually gives the Skill to do the comparison (for Forage Extraction)
	sint				getForageExtractionPhraseSuccessRate(const CSPhraseCom &phrase, SKILLS::ESkills skill);
	// return the fmt according to forage terrain specializing
	ucstring			getForageExtractionPhraseEcotypeFmt(const CSPhraseCom &phrase);
	// Get the Phrase Sap Cost
	void				getPhraseSapCost(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &cost, sint &costMalus);
	// Get the Phrase Sta Cost
	void				getPhraseStaCost(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &cost, sint &costMalus);
	// Get the Phrase Focus Cost
	void				getPhraseFocusCost(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &cost, sint &costMalus);
	// Get the Phrase Hp Cost
	void				getPhraseHpCost(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &cost, sint &costMalus);
	// Get the Phrase Cast Time
	void				getPhraseCastTime(const CSPhraseCom &phrase, uint32 totalActionMalus, float &castTime, float &castTimeMalus);
	// Get the Phrase Range
	void				getPhraseMagicRange(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &range, sint &rangeMalus);
	// For all bricks, get the sum of the propId property
	float				getPhraseSumBrickProp(const CSPhraseCom &phrase, uint propId, bool doAbs=false);
	/// If the temporary inventory is opened so we cant execute action
	bool				isExecutionInvalidCauseTempInv() const;
	/// build a list of bricks that are needed in order to learn this phrase. NB: any brick already contained in the phrase are removed
	void				buildPhraseBrickRequirement(uint32 phraseSheetId, std::vector<NLMISC::CSheetId> &bricks);
	/// check if all bricks of phrase meet fame requirement
	bool				fameRequirementOk(uint32 phraseSheetId);
	/// true if interesting to list the bricks of this phrase in help
	bool				allowListBrickInHelp(const CSPhraseCom &phrase) const;
	/// return the combat restriction text (empty if not combat)
	void				getCombatWeaponRestriction(ucstring &text, const CSPhraseCom &phrase, bool& usableWithMelee, bool& usableWithRange);
	void				getCombatWeaponRestriction(ucstring &text, sint32 phraseSheetId, bool& usableWithMelee, bool& usableWithRange);
	// return true if any of the Bricks contains AvoidCyclic==true (the phrase cannot be cyclic)
	bool				avoidCyclicForPhrase(const CSPhraseCom &phrase) const;
	bool				avoidCyclicForPhrase(sint32 phraseSheetId) const;
	// Get total regeneration time for a power/aura phrase, in tick (0 if not a power/aura)
	NLMISC::TGameCycle	getPowerDisableTime(const CSPhraseCom &phrase) const;
	// Get current time left for a power/aura phrase, in ticks (0 if not a power/aura)
	NLMISC::TGameCycle	getRegenTime(const CSPhraseCom &phrase) const;
	// Get total time span necessary for a power/aura phrase to be available again (in ticks)
	NLMISC::TGameCycle	getTotalRegenTime(const CSPhraseCom &phrase) const;

	/// Lookup a SuccessTable and return chance
	enum	TSuccessTable
	{
		STCombat= 0,
		STOffensiveMagic,
		STCurativeMagic,
		STCraft,
		STExtract,
		STResistMagic,
		STResistMagicLink,
		STDodgeParry,
		NumSuccessTable
	};
	sint				getSuccessRate(TSuccessTable st, sint level, bool partialSuccess= false);
	// @}

	// \name Phrase Execution invalidation cause of Equip
	// @{
	void				setEquipInvalidation(sint64 serverTick, sint invalidTickTime);
	void				updateEquipInvalidation(sint64 serverTick);
	bool				isExecutionInvalidCauseEquip() const;
	// @}



	// \name BotChat special
	// @{
	// get the current skill point price of a phrase sheet
	uint32				getCurrentPhraseSheetPrice(uint32 phraseSheetId);
	// get the base skill point price of a phrase sheet (ie don't check if bricks learned or not)
	uint32				getBasePhraseSheetPrice(uint32 phraseSheetId);
	// update bot chat prices in the range [start, end[
	void				updateBotChatPhrasePrice(uint start, uint end);

	// From a phrase required level, retrieve its section (for interface listing)
	uint32				getPhraseSectionFromLevel(uint32 level);
	// inverse
	void				getPhraseLevelFromSection(uint32 section, uint32 &minLevel, uint32 &maxLevel);
	// For book display of section
	void				getPhraseSectionBoundFromSkillFilter(sint &minSectionId, sint &maxSectionId);

	// Client Side Rolemaster phrase generation. return the number of db node filled
	uint				fillRoleMasterGenericPhrase(uint32 rmFlags, uint32 rmRace);

	// @}


	/// true if the compareSkill is compatible with this combat phrase, eg more restrictive for instance
	bool				skillCompatibleWithCombatPhrase(SKILLS::ESkills compareSkill, const std::vector<NLMISC::CSheetId> &phraseBricks) const;
	bool				skillCompatibleWithSpecialPowerPhrase(SKILLS::ESkills compareSkill, const std::vector<NLMISC::CSheetId> &phraseBricks) const;

	/// true if phrase is usable with empty hands
	bool				phraseUsableWithEmptyHands(const std::vector<NLMISC::CSheetId> &phraseBricks) const;

	// get total action malus
	uint32				getTotalActionMalus(const CSPhraseCom &phrase) const;

	void updateAllActionRegen();

	void touchRegenTickRangeFlag() { _RegenTickRangeTouched = true; }

	static uint64				getPhraseRequiredFlags(const CSPhraseCom &phrase);
	
// ****************
private:
	/// Constructor
	CSPhraseManager();
	~CSPhraseManager();
	static	CSPhraseManager		*_Instance;

	typedef	std::map<uint32, CSPhraseCom>	TPhraseMap;
	typedef	TPhraseMap::iterator			ItPhraseMap;
	typedef	std::map<CSPhraseCom, uint32>	TPhraseToSheet; // map each phrase to its sheet id
	// Map of All Phrase. Contains the Book + some system phrase (1: the Edition Phrase)
	TPhraseMap								_PhraseMap;
	TPhraseToSheet							_PhraseToSheet;
	std::set<sint32>						_RegenPhrases;	

	// timeouts for the power / auras bricks
	CTickRange								_BrickRegenRange[64];

	// Shortcut To Phrases Leaves
	std::vector<NLMISC::CCDBNodeLeaf*>				_BookDbLeaves;
	std::vector<NLMISC::CCDBNodeLeaf*>				_MemoryDbLeaves;
	std::vector<NLMISC::CCDBNodeLeaf*>				_MemoryAltDbLeaves;
	NLMISC::CCDBNodeLeaf							*_NextExecuteLeaf;
	NLMISC::CCDBNodeLeaf							*_NextExecuteIsCyclicLeaf;

	// extra client data
	struct	CPhraseClient
	{
		sint32		Version;
		bool		Lock;
		CPhraseClient()
		{
			Version= 0;
			Lock= false;
		}
	};
	std::vector<CPhraseClient>				_PhraseClient;
	uint32									_MaxSlotSet;
	std::vector<uint32>						_FreeSlots;

	bool									_RegenTickRangeTouched;

	// Memory
	struct	CMemorySlot
	{
		// Is this a macro
		bool	IsMacro;
		// Is his visual dirty?
		bool	IsMacroVisualDirty;
		// Macro or PhraseId
		uint32	Id;
		CMemorySlot()
		{
			IsMacro= false;
			IsMacroVisualDirty= false;
			Id= 0;
		}

		// suppose Id==0 is valid for a macro, but not for a phrase
		bool	isEmpty() const
		{
			return Id==0 && IsMacro==false;
		}
		// Macro possible only if IsMacro==true
		bool	isMacro() const
		{
			return IsMacro;
		}
		// phrase only if not macro, and Id!=0
		bool	isPhrase() const
		{
			return !IsMacro && Id!=0;
		}
	};
	struct	CMemoryLine
	{
		CMemorySlot		Slot[PHRASE_MAX_MEMORY_SLOT];
	};
	std::vector<CMemoryLine>	_Memories;
	sint32						_SelectedMemoryDB;
	sint32						_SelectedMemoryDBalt;

	void		updateMemoryDBAll();
	void		updateMemoryDBSlot(uint32 memorySlot);

	// The number of entries setuped to not 0
	uint						_LastBookNumDbFill;
	// The Book filter.
	SKILLS::ESkills				_BookSkillFitler;
	BRICK_TYPE::EBrickType		_BookBrickTypeFilter;
	// true if the phrase is "compatbile" with _BookSkillFitler
	bool		matchBookSkillFilter(const CSPhraseCom &phrase) const;

	uint32						_PhraseNextExecuteCounter;
	uint32						_PhraseCycleExecuteCounter;

	bool						_InitInGameDone;

	/// unlock a phrase (for BotChat Phrase buy confirmation)
	void				unlockPhrase(uint32 slot);

	// Phrase compute related
	CSuccessTableSheet	*_SuccessTableSheet[NumSuccessTable];
	void				loadSuccessTable();

	// The user was indoor
	bool				_UserIndoor;

	// called by getPhraseSuccessRate() public
	sint				getPhraseSuccessRate(TSuccessTable st, const CSPhraseCom &phrase, sint skillValue, uint minMpLevel);

	// real stuff
	void				setPhraseInternal(uint32 slot, const CSPhraseCom &phrase, bool lock, bool updateDB);

	// If the local counter and server counter are same, then view will be hid.
	bool				isPhraseNextExecuteCounterSync() const;
	bool				isPhraseCycleExecuteCounterSync() const;

	// setup by AH. -1 if not activated
	sint32				_CurrentExecuteLineNext;
	sint32				_CurrentExecuteSlotNext;
	uint32				_CurrentExecutePhraseIdNext;
	sint32				_CurrentExecuteLineCycle;
	sint32				_CurrentExecuteSlotCycle;
	uint32				_CurrentExecutePhraseIdCycle;
	// For debug only. setuped by sendExecuteToServer() in local mode only
	sint64				_PhraseDebugEndNextAction;
	sint64				_PhraseDebugEndCyclicAction;

	friend class CHandlerPhraseCounterUpdate;
	friend class CHandlerPhraseDebugClient;

	// Equip invalidation
	sint64				_EquipInvalidationEnd;
	sint64				_CurrentServerTick;

	// update the execution views
	void				updateExecutionDisplay();

	// update the memory ctrl
	void	updateMemoryCtrlState(uint memorySlot, class CDBCtrlSheet *ctrl, SKILLS::ESkills itemSkill);
	// update the ith memory ctrl in the action bar
	void	updateMemoryCtrlState(uint memorySlot);

	// Shortcut To PhraseSheets Leaves in BotChat
	std::vector<NLMISC::CCDBNodeLeaf*>				_BotChatPhraseSheetLeaves;
	std::vector<NLMISC::CCDBNodeLeaf*>				_BotChatPhrasePriceLeaves;

	// For phrase compatibility with enchant weapon special power
	NLMISC::CSheetId						_EnchantWeaponMainBrick;

	// \name Phrase sheet progression
	// @{
	enum	TProgressType
	{
		ActionProgress= 0,
		UpgradeProgress,

		NumProgressType
	};
	std::vector<NLMISC::CCDBNodeLeaf*>				_ProgressionDbSheets[NumProgressType];
	std::vector<NLMISC::CCDBNodeLeaf*>				_ProgressionDbLevels[NumProgressType];
	std::vector<NLMISC::CCDBNodeLeaf*>				_ProgressionDbLocks[NumProgressType];

	// For each skill, which phrase are learned when the skill is gained
	class CPhraseProgressInfo
	{
	public:
		uint32		SheetId;
		uint32		Level;
	};
	friend class CPhraseSortEntry;
	class CPhraseProgression
	{
	public:
		// All the phrase that can be learned at this level, anbd the required level of the skill
		std::vector<CPhraseProgressInfo>		Phrases;
	};
	CPhraseProgression			_ProgressionPhrases[SKILLS::NUM_SKILLS];

	// For each phrase SheetId, gives the required Skill formula (for help)
	typedef	std::map<uint32, CReqSkillFormula>		TPhraseReqSkillMap;
	TPhraseReqSkillMap			_PhraseReqSkillMap;

	// For db fill
	uint						_LastProgressionNumDbFill[NumProgressType];
	// For db update when a brick is learned
	class CProgressionUpdate : public IBrickLearnedCallback, public ISkillChangeCallback
	{
	public:
		virtual	void	onBrickLearned()
		{
			CSPhraseManager	*pPM= CSPhraseManager::getInstance();
			pPM->updatePhraseProgressionDB();
		}
		virtual	void	onSkillChange()
		{
			CSPhraseManager	*pPM= CSPhraseManager::getInstance();
			pPM->updatePhraseProgressionDB();
		}
	};
	friend class CProgressionUpdate;
	CProgressionUpdate			_ProgressionUpdate;

	// methods
	void				updatePhraseProgressionDB();
	// called at initInGame
	void				computePhraseProgression();
	void				insertProgressionSkillRecurs(SKILLS::ESkills skill, uint32 value, sint *skillReqLevel, std::vector<SKILLS::ESkills>	&skillsToInsert);

	mutable NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _TotalMalusEquipLeaf;

	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _ServerUserDefaultWeightHandsLeaf;

	// @}

	/// return the skill of the root
	SKILLS::ESkills		getPhraseRootSkill(const std::vector<NLMISC::CSheetId> &phraseBricks) const;

	/// return true if all the requirement in order to learn this prhase are met.
	bool				phraseRequirementOk(uint32 phraseSheetId);


	/// Execution ACK reception
	// @{
	struct CAckExecuteEntry
	{
		enum	TState
		{
			WaitAck= 0,
			OK,
			KO
		};
		TState	State;
		uint32	Counter;
		sint32	MemoryLine;
		sint32	MemorySlot;
		uint32	PhraseId;
	};
	typedef		std::deque<CAckExecuteEntry>	TAckExecuteList;
	TAckExecuteList			_AckExecuteCycleList;
	TAckExecuteList			_AckExecuteNextList;

	void	appendCurrentToAckExecute(bool cyclic);
	void	resetAckExecuteList(bool cyclic);

	// @}

	ucstring	formatMalus(sint base, sint malus);
	ucstring	formatMalus(float base, float malus);
	std::string		formatBonusMalus(sint32 base, sint32 mod);

	// Special for combat: Build the "phrase skill compatible" formula
	// NB: Use a ReqSkillFormula, but don't use the value part, cause no requirement is made on it
	CReqSkillFormula	buildCombatPhraseSkillFormula(const std::vector<NLMISC::CSheetId> &phraseBricks) const;

	// For Magic and Magic Staff
	uint32		getSpellLevel(const std::vector<NLMISC::CSheetId> &phraseBricks) const;

	// For Magic
	void		getResistMagic(bool resistMagic[RESISTANCE_TYPE::NB_RESISTANCE_TYPE], const std::vector<NLMISC::CSheetId> &phraseBricks);

	void	updateAllMemoryCtrlRegenTickRange();
	void	updateMemoryCtrlRegenTickRange(uint memorySlot, CDBCtrlSheet	*ctrl);
	void	updateMemoryCtrlRegenTickRange(uint memorySlot);

	CDBCtrlSheet	*getMemorySlotCtrl(uint memorySlot);
	CDBCtrlSheet	*getMemoryAltSlotCtrl(uint memorySlot);

	CTickRange getRegenTickRange(const CSPhraseCom &phrase) const;

	NLMISC::CCDBNodeLeaf	*getRegenTickRangeDbLeaf(uint powerIndex) const;
	// get regen tick range for a specific power, from the database
	CTickRange getRegenTickRange(uint powerIndex) const;
public:
	// tmp for test : set regen tick range locally
	void setRegenTickRange(uint powerIndex, const CTickRange &tickRange);
};

#endif // NL_SPHRASE_MANAGER_H

/* End of sphrase_manager.h */
