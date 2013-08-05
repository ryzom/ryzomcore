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
#include "sphrase_manager.h"
#include "interface_manager.h"
#include "../client_sheets/sphrase_sheet.h"
#include "../sheet_manager.h"
#include "../string_manager_client.h"
#include "sbrick_manager.h"
#include "skill_manager.h"
#include "inventory_manager.h"
#include "../user_entity.h"
#include "game_share/memorization_set_types.h"
#include "../client_cfg.h"
#include "../net_manager.h"
#include "nel/misc/algo.h"
#include "../client_sheets/success_table_sheet.h"
#include "dbctrl_sheet.h"
#include "macrocmd_manager.h"
#include "game_share/rolemaster_flags.h"
#include "game_share/people.h"
#include "nel/gui/lua_ihm.h"
#include "../time_client.h"


using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

// Context help
extern void contextHelp (const std::string &help);


// ***************************************************************************
// define this for deubg purpose only
//#define PHRASE_DEBUG_VERBOSE


// ***************************************************************************
const	std::string		PhraseMemoryViewNextAction= "ui:interface:gestionsets:shortcuts:view_next_action";
const	std::string		PhraseMemoryViewCycleAction= "ui:interface:gestionsets:shortcuts:view_cycle_action";
const	std::string		PhraseMemoryViewSlotBase= "ui:interface:gestionsets:shortcuts:s";
const	std::string		PhraseMemoryCtrlBase= "ui:interface:gestionsets:shortcuts:s";
const	std::string		PhraseMemoryAltCtrlBase= "ui:interface:gestionsets2:header_closed:shortcuts:s";

const	std::string		PhraseMemoryPhraseMenu= "ui:interface:cm_memory_phrase";
const	std::string		PhraseMemoryPhraseAction= "cast_phrase_or_create_new";
const	std::string		PhraseMemoryMacroMenu= "ui:interface:cm_memory_macro";
const	std::string		PhraseMemoryMacroAction= "cast_macro";


// ***************************************************************************
CSPhraseManager		*CSPhraseManager::_Instance= NULL;

// ***************************************************************************
SKILLS::ESkills		getRightHandItemSkill();
uint32				getRightHandEffectiveLevel();

// ***************************************************************************
void CSPhraseManager::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ***************************************************************************
CSPhraseManager::CSPhraseManager()
{
	reset();
	for(uint i=0;i<NumSuccessTable;i++)
		_SuccessTableSheet[i]= NULL;
	_RegenTickRangeTouched = true;
}

// ***************************************************************************
CSPhraseManager::~CSPhraseManager()
{
	reset();
}

// ***************************************************************************
void			CSPhraseManager::initInGame()
{
	if(_InitInGameDone)
		return;

	static bool registerClassDone= false;


	_InitInGameDone= true;

	// Init Database values.
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	i;
	_BookDbLeaves.resize(PHRASE_MAX_BOOK_SLOT, NULL);
	for(i=0;i<PHRASE_MAX_BOOK_SLOT;i++)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_BOOK + ":" + toString(i) + ":PHRASE");
		node->setValue32(0);
		_BookDbLeaves[i]= node;
	}
	_MemoryDbLeaves.resize(PHRASE_MAX_MEMORY_SLOT, NULL);
	_MemoryAltDbLeaves.resize(PHRASE_MAX_MEMORY_SLOT, NULL);

	for(i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_MEMORY + ":" + toString(i) + ":PHRASE");
		node->setValue32(0);
		_MemoryDbLeaves[i]= node;
		CCDBNodeLeaf	*node_alt= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_MEMORY_ALT + ":" + toString(i) + ":PHRASE");
		node_alt->setValue32(0);
		_MemoryAltDbLeaves[i]= node_alt;
	}

	// Progression Db leaves
	nlctassert( NumProgressType == sizeof(PHRASE_DB_PROGRESSION)/sizeof(PHRASE_DB_PROGRESSION[0]) );
	for(uint j=0;j<NumProgressType;j++)
	{
		_ProgressionDbSheets[j].resize(PHRASE_MAX_PROGRESSION_SLOT, NULL);
		_ProgressionDbLocks[j].resize(PHRASE_MAX_PROGRESSION_SLOT, NULL);
		_ProgressionDbLevels[j].resize(PHRASE_MAX_PROGRESSION_SLOT, NULL);
	}
	for(i=0;i<PHRASE_MAX_PROGRESSION_SLOT;i++)
	{
		for(uint j=0;j<NumProgressType;j++)
		{
			// SHEET
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_PROGRESSION[j] + ":" + toString(i) + ":SHEET");
			node->setValue32(0);
			_ProgressionDbSheets[j][i]= node;
			// LEVEL
			node= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_PROGRESSION[j] + ":" + toString(i) + ":LEVEL");
			node->setValue32(0);
			_ProgressionDbLevels[j][i]= node;
			// LOCKED
			node= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_PROGRESSION[j] + ":" + toString(i) + ":LOCKED");
			node->setValue32(0);
			_ProgressionDbLocks[j][i]= node;
		}
	}

	// init the UI Next Execute slot
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_EXECUTE_NEXT);
		node->setValue32(0);
		_NextExecuteLeaf= node;
		node= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_EXECUTE_NEXT_IS_CYCLIC);
		node->setValue32(0);
		_NextExecuteIsCyclicLeaf= node;
	}
	// Init BotChat leaves
	_BotChatPhraseSheetLeaves.resize(PHRASE_MAX_BOTCHAT_SLOT, NULL);
	_BotChatPhrasePriceLeaves.resize(PHRASE_MAX_BOTCHAT_SLOT, NULL);
	for(i=0;i<PHRASE_MAX_BOTCHAT_SLOT;i++)
	{
		CCDBNodeLeaf	*nodeSheet= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_BOTCHAT+ ":" + toString(i) + ":SHEET");
		CCDBNodeLeaf	*nodePrice= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_BOTCHAT+ ":" + toString(i) + ":PRICE");
		_BotChatPhraseSheetLeaves[i]= nodeSheet;
		_BotChatPhrasePriceLeaves[i]= nodePrice;
	}

	// and so update book and memory db
	updateBookDB();
	updateMemoryDBAll();

	// Load the success table here
	loadSuccessTable();

	// compute and the progression phrase, and update DB
	computePhraseProgression();

	_EnchantWeaponMainBrick = NLMISC::CSheetId("bsxea10.sbrick");

	// build map that gives its description for each built-in phrase
	// slow test on all sheets here ...
	const CSheetManager::TEntitySheetMap &sm = SheetMngr.getSheets();
	sint32 result= 0;
	CSPhraseCom tmpPhrase;
	for(CSheetManager::TEntitySheetMap::const_iterator it = sm.begin(); it != sm.end(); ++it)
	{
		if (it->second.EntitySheet && it->second.EntitySheet->Type == CEntitySheet::SPHRASE)
		{						
			const_cast<CSPhraseManager *>(this)->buildPhraseFromSheet(tmpPhrase, it->first.asInt());
			_PhraseToSheet[tmpPhrase] = it->first.asInt();
		}
	}
}


// ***************************************************************************
void			CSPhraseManager::updateMemoryBar()
{
	if(!_InitInGameDone)
		return;

	updateMemoryDBAll();
	updateAllMemoryCtrlState();
}


// ***************************************************************************
void			CSPhraseManager::setPhraseInternal(uint32 slot, const CSPhraseCom &phrase, bool lock, bool updateDB)
{
	// don't allow slot too big. don't allow set the 0 slot.
	if(slot>PHRASE_MAX_ID || slot==0)
		return;

	// enlargePhraseClient
	if(slot>=_PhraseClient.size())
		_PhraseClient.resize(slot+1);

	// set the phrase
	_PhraseMap[slot]= phrase;
	// increment the phrase version.
	_PhraseClient[slot].Version++;
	// BotChat lock?
	_PhraseClient[slot].Lock= lock;

	// For Free Slot Mgt.
	_MaxSlotSet= max(_MaxSlotSet, slot);

	// update the book, if necessary
	if( updateDB && !lock && slot >= BookStartSlot )
		updateBookDB();
}

// ***************************************************************************
void			CSPhraseManager::setPhrase(uint32 slot, const CSPhraseCom &phrase, bool lock)
{
	// set the phrase and update the DB
	setPhraseInternal(slot, phrase, lock, true);
}

// ***************************************************************************
void			CSPhraseManager::setPhraseNoUpdateDB(uint32 slot, const CSPhraseCom &phrase)
{
	// set the phrase but don't update the DB (NB: no phrase lock)
	setPhraseInternal(slot, phrase, false, false);
}


// ***************************************************************************
void			CSPhraseManager::erasePhrase(uint32 slot)
{
	if(slot>=_PhraseClient.size())
		return;

	_PhraseMap.erase(slot);
	_PhraseClient[slot].Version= -1;
	_PhraseClient[slot].Lock= false;

	// make this slot available for allocation
	_FreeSlots.push_back(slot);

	// update the book.
	updateBookDB();

	// if the phrase erased was currently executed, must stop it
	if(slot==_CurrentExecutePhraseIdCycle || slot==_CurrentExecutePhraseIdNext)
	{
		if(slot==_CurrentExecutePhraseIdNext)
		{
			_CurrentExecuteLineNext= -1;
			_CurrentExecuteSlotNext= -1;
			_CurrentExecutePhraseIdNext= 0;
		}
		if(slot==_CurrentExecutePhraseIdCycle)
		{
			_CurrentExecuteLineCycle= -1;
			_CurrentExecuteSlotCycle= -1;
			_CurrentExecutePhraseIdCycle= 0;
		}

		// update execution display
		updateExecutionDisplay();
	}

}

// ***************************************************************************
const CSPhraseCom	&CSPhraseManager::getPhrase(uint32 slot) const
{
	TPhraseMap::const_iterator	it= _PhraseMap.find(slot);
	if(it==_PhraseMap.end())
		return CSPhraseCom::EmptyPhrase;
	else
		return it->second;
}

// ***************************************************************************
sint32			CSPhraseManager::getPhraseVersion(uint32 slot) const
{
	if(slot>=_PhraseClient.size())
		return -1;

	return _PhraseClient[slot].Version;
}

// ***************************************************************************
void			CSPhraseManager::unlockPhrase(uint32 slot)
{
	TPhraseMap::const_iterator	it= _PhraseMap.find(slot);
	if(it!=_PhraseMap.end())
	{
		// unlcok the phrase
		_PhraseClient[slot].Lock= false;
		// and so update the book.
		updateBookDB();
	}
}


// ***************************************************************************
void			CSPhraseManager::receiveBotChatConfirmBuy(uint16 phraseId, bool confirm)
{
	// if confirm => unlock
	if(confirm)
	{
		// first unlock it.
		unlockPhrase(phraseId);

		// Then try to auto memorize it.
		const	CSPhraseCom	&phrase= getPhrase(phraseId);
		if(!phrase.empty())
		{
			CSBrickManager	*pBM= CSBrickManager::getInstance();
			CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
			if(rootBrick)
			{
				// Must save to the first Memory slot available
				for(uint dstMemoryLine=0; dstMemoryLine<MEM_SET_TYPES::NumMemories; dstMemoryLine++)
				{
					// Find a free slot to write.
					sint	dstMemorySlot= -1;
					for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
					{
						// if slot is free
						if( !isMemorizedMacro(dstMemoryLine, i) &&
							getMemorizedPhrase(dstMemoryLine, i)==0 )
						{
							dstMemorySlot= i;
							break;
						}
					}

					// If free slot found in this memory line
					if(dstMemorySlot>=0)
					{
						// memorize
						memorizePhrase(dstMemoryLine, dstMemorySlot, phraseId);

						// Send Server Info
						sendMemorizeToServer(dstMemoryLine, dstMemorySlot, phraseId);

						// stop!
						break;
					}
				}

				// Ugly: must update ALL the memoryBar in case of dstMemoryLine is the selected one
				updateAllMemoryCtrlState();
			}
		}

		// Context help
		contextHelp ("action_book");
	}
	// else delete slot
	else
		erasePhrase(phraseId);
}


// ***************************************************************************
void				CSPhraseManager::forgetPhrase(uint32 memoryLine, uint32 memorySlot)
{
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return;
	// if the slot eihter don't exist, abort
	if(memoryLine>=_Memories.size())
		return;
	// if the slot is not a phrase
	if(!_Memories[memoryLine].Slot[memorySlot].isPhrase())
		return;

	// update memory
	_Memories[memoryLine].Slot[memorySlot].IsMacro= false;
	_Memories[memoryLine].Slot[memorySlot].Id= 0;

	// must update DB?
	if((sint32)memoryLine==_SelectedMemoryDB)
	{
		// update the db
		updateMemoryDBSlot(memorySlot);
		// update the ctrl state
		updateMemoryCtrlState(memorySlot);

		// If there is an execution running on this slot, no more display it
		if(_CurrentExecuteSlotNext==(sint32)memorySlot || _CurrentExecuteSlotCycle==(sint32)memorySlot)
			updateExecutionDisplay();
	}
}

// ***************************************************************************
uint32				CSPhraseManager::getMemorizedPhrase(uint32 memoryLine, uint32 memorySlot) const
{
	if(memoryLine>=_Memories.size())
		return 0;
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return 0;
	if(!_Memories[memoryLine].Slot[memorySlot].isPhrase())
		return 0;

	return _Memories[memoryLine].Slot[memorySlot].Id;
}

// ***************************************************************************
void				CSPhraseManager::memorizePhrase(uint32 memoryLine, uint32 memorySlot, uint32 slot)
{
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return;
	if(slot>=_PhraseClient.size())
		return;
	// Can memorize only a phrase of the book
	if(slot<BookStartSlot)
		return;

	// first force forget old mem slot
	forgetPhrase(memoryLine, memorySlot);

	// then memorize new one.
	TPhraseMap::const_iterator	it= _PhraseMap.find(slot);
	if(it==_PhraseMap.end())
		return;

	// enlarge memory if needed
	if(memoryLine>=_Memories.size())
		_Memories.resize(memoryLine+1);

	// update memory
	_Memories[memoryLine].Slot[memorySlot].IsMacro= false;
	_Memories[memoryLine].Slot[memorySlot].Id= slot;

	// must update DB?
	if((sint32)memoryLine==_SelectedMemoryDB)
	{
		// update the DB
		updateMemoryDBSlot(memorySlot);
		// update the ctrl state
		updateMemoryCtrlState(memorySlot);

		// If there is an execution running with this action, maybe re-display it
		if(_CurrentExecutePhraseIdNext==slot || _CurrentExecutePhraseIdCycle==slot)
			updateExecutionDisplay();
	}
}

void CSPhraseManager::selectMemoryLineDBalt(sint32 memoryLine)
{
	if(memoryLine<0)
		memoryLine= -1;
	
	if(_SelectedMemoryDBalt!=memoryLine)
	{
		_SelectedMemoryDBalt= memoryLine;
		// since memory selection changes then must update all the DB and the Ctrl states
		
		updateMemoryDBAll();
		updateAllMemoryCtrlState();
		updateAllMemoryCtrlRegenTickRange();
		// must update also the execution views
		updateExecutionDisplay();
	}
}

// ***************************************************************************
void				CSPhraseManager::selectMemoryLineDB(sint32 memoryLine)
{
	if(memoryLine<0)
		memoryLine= -1;
	if(_SelectedMemoryDB!=memoryLine)
	{
		_SelectedMemoryDB= memoryLine;
		// since memory selection changes then must update all the DB and the Ctrl states
		updateMemoryDBAll();
		updateAllMemoryCtrlState();
		updateAllMemoryCtrlRegenTickRange();
		// must update also the execution views
		updateExecutionDisplay();
	}
}

// ***************************************************************************
void		CSPhraseManager::updateMemoryDBAll()
{
	// If DB not inited, no-op
	if(!_InitInGameDone)
		return;

	if(_SelectedMemoryDB==-1 || _SelectedMemoryDB>=(sint32)_Memories.size())
	{
		for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
		{
			_MemoryDbLeaves[i]->setValue32(0);
		}
	}
	else
	{
		for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
		{
			CMemorySlot		&slot= _Memories[_SelectedMemoryDB].Slot[i];
			if(!slot.isPhrase())
				_MemoryDbLeaves[i]->setValue32(0);
			else
				_MemoryDbLeaves[i]->setValue32(slot.Id);
		}
	}

	if(_SelectedMemoryDBalt == -1 || _SelectedMemoryDBalt>=(sint32)_Memories.size())
	{
		for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
		{
			_MemoryAltDbLeaves[i]->setValue32(0);
		}
	}
	else
	{
		// Always update alt gestionsets
		for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
		{
			CMemorySlot		&slotAlt= _Memories[_SelectedMemoryDBalt].Slot[i];
			if(!slotAlt.isPhrase())
				_MemoryAltDbLeaves[i]->setValue32(0);
			else
				_MemoryAltDbLeaves[i]->setValue32(slotAlt.Id);
		}
	}
}

// ***************************************************************************
void		CSPhraseManager::updateMemoryDBSlot(uint32 memorySlot)
{
	// If DB not inited, no-op
	if(!_InitInGameDone)
		return;

	if(_SelectedMemoryDB==-1 || _SelectedMemoryDB>=(sint32)_Memories.size())
		return;

	if(memorySlot<PHRASE_MAX_MEMORY_SLOT)
	{
		CMemorySlot		&slot= _Memories[_SelectedMemoryDB].Slot[memorySlot];
		if(!slot.isPhrase())
			_MemoryDbLeaves[memorySlot]->setValue32(0);
		else
			_MemoryDbLeaves[memorySlot]->setValue32(slot.Id);
		
		CMemorySlot		&slotAlt= _Memories[_SelectedMemoryDBalt].Slot[memorySlot];

		if(!slotAlt.isPhrase())
			_MemoryAltDbLeaves[memorySlot]->setValue32(0);
		else
			_MemoryAltDbLeaves[memorySlot]->setValue32(slotAlt.Id);
	}
}

// ***************************************************************************
bool				CSPhraseManager::isPhraseCastable(uint32 sheetId) const
{
	return isPhraseCastable(dynamic_cast<CSPhraseSheet*>(SheetMngr.get(NLMISC::CSheetId(sheetId))));
}

// ***************************************************************************
bool				CSPhraseManager::isPhraseCastable(CSPhraseSheet *phraseSheet) const
{
	bool	ok = false;
	if(phraseSheet)
	{
		// castable
		if(phraseSheet->Castable)
		{
			// check Charac Buying (compatibility)
			if(!isPhraseCharacBuying(phraseSheet))
				ok= true;
		}
	}

	return ok;
}

// ***************************************************************************
bool				CSPhraseManager::isPhraseCharacBuying(uint32 sheetId) const
{
	return isPhraseCharacBuying(dynamic_cast<CSPhraseSheet*>(SheetMngr.get(NLMISC::CSheetId(sheetId))));
}

// ***************************************************************************
bool				CSPhraseManager::isPhraseCharacBuying(class CSPhraseSheet *phraseSheet) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	if(!phraseSheet || phraseSheet->Bricks.empty())
		return false;

	// check brick
	CSBrickSheet	*brickSheet= pBM->getBrick(phraseSheet->Bricks[0]);
	// if not a charac buying phrase
	if(brickSheet && BRICK_FAMILIES::isCharacBuyFamily(brickSheet->BrickFamily) )
		return true;

	return false;
}

// ***************************************************************************
uint32				CSPhraseManager::allocatePhraseSlot()
{
	if(_FreeSlots.empty())
	{
		// if too big, fail.
		if(_MaxSlotSet>=PHRASE_MAX_ID)
			return 0;

		// get a free slot
		return _MaxSlotSet+1;
	}
	else
	{
		uint32	val= _FreeSlots.back();
		_FreeSlots.pop_back();
		return val;
	}
}

// ***************************************************************************
bool				CSPhraseManager::hasFreeSlot() const
{
	// if no free slot and too big, fail.
	if(_FreeSlots.empty())
	{
		if(_MaxSlotSet>=PHRASE_MAX_ID)
			return false;
	}

	return true;
}

// ***************************************************************************
void				CSPhraseManager::setBookFilter(BRICK_TYPE::EBrickType brickTypeFilter, SKILLS::ESkills skillFilter)
{
	// set the filter
	_BookBrickTypeFilter = brickTypeFilter;
	_BookSkillFitler = skillFilter;

	// the DB book may change
	updateBookDB();
	// the DB progression may change
	updatePhraseProgressionDB();
}

// ***************************************************************************
bool				CSPhraseManager::matchBookSkillFilter(const CSPhraseCom &phrase) const
{
	// Special: if book Filter is unknown, then always match!
	if(_BookBrickTypeFilter==BRICK_TYPE::UNKNOWN && _BookSkillFitler==SKILLS::unknown)
		return true;

	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSkillManager	*pSM= CSkillManager::getInstance();

	// must have a valid rootBrick. NB: it is an error, but decide to show it (return true)
	if(phrase.empty())
		return true;
	CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
	if(!rootBrick)
		return true;

	// If decide to filter by brickType
	if(_BookBrickTypeFilter!=BRICK_TYPE::UNKNOWN)
	{
		if( BRICK_FAMILIES::brickType(rootBrick->BrickFamily) == _BookBrickTypeFilter)
			return true;
	}
	// else filter by Skill
	else
	{
		// Special for powers: always true. \todo yoyo: may change this feature
		if(rootBrick->isSpecialPower())
		{
			// Skill Filter must be at least combat or magic
			return pSM->areSkillOnSameBranch(_BookSkillFitler, SKILLS::SM) ||
				pSM->areSkillOnSameBranch(_BookSkillFitler, SKILLS::SF);
		}
		// Special for combat
		else if(rootBrick->isCombat())
		{
			// For combat, it is best to show "All Actions that can be used with the weapon associated to the skill"
			if(skillCompatibleWithCombatPhrase(_BookSkillFitler, phrase.Bricks))
				return true;
		}
		// suppose same for magic, craft, and forage
		else
		{
			// run all the phrase Bricks, if only one match (eg important for magic multi spells), it is ok!
			for(uint i=0;i<phrase.Bricks.size();i++)
			{
				CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
				if(brick)
				{
					// if the filter is an ancestor, ok! (NB: if brick skill unknown then it will fail!)
					if(pSM->isSkillAncestor(_BookSkillFitler, brick->getSkill()))
						return true;
				}
			}
		}
	}

	// no match!
	return false;
}

// ***************************************************************************
void				CSPhraseManager::updateBookDB()
{
	// If DB not inited, no-op
	if(!_InitInGameDone)
		return;

	// Fill All the book.
	TPhraseMap::const_iterator	it= _PhraseMap.begin();
	sint	numBookFill= (sint)_PhraseMap.size();
	sint	i= 0;
	while(i<numBookFill)
	{
		// if the slot is not from the book, then don't display it
		// if the slot is Locked (BotChat confirm wait), then don't display it too
		// if the slot does not match the filter, then don't display it too
		if(it->first<BookStartSlot ||
			_PhraseClient[it->first].Lock ||
			!matchBookSkillFilter(it->second) )
		{
			numBookFill--;
		}
		else
		{
			// fill with the phrase id
			_BookDbLeaves[i]->setValue32(it->first);
			// next mem fill
			i++;
		}

		// if no more place on book, stop
		if(i>=PHRASE_MAX_BOOK_SLOT)
		{
			numBookFill= PHRASE_MAX_BOOK_SLOT;
			break;
		}

		// next map entry.
		it++;
	}

	// reset old no more used to empty
	for(i=numBookFill;i<(sint)_LastBookNumDbFill;i++)
	{
		_BookDbLeaves[i]->setValue32(0);
	}

	// update cache
	_LastBookNumDbFill= numBookFill;
}

// ***************************************************************************
void				CSPhraseManager::buildPhraseFromSheet(CSPhraseCom &phrase, sint32 sheetId)
{
	CSPhraseSheet	*phraseSheet= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(CSheetId(sheetId)));
	if(phraseSheet)
	{
		// get localized Name
		phrase.Name= STRING_MANAGER::CStringManagerClient::getSPhraseLocalizedName(CSheetId(sheetId));
		// Build bricks
		phrase.Bricks.clear();
		for(uint i=0;i<phraseSheet->Bricks.size();i++)
			phrase.Bricks.push_back(phraseSheet->Bricks[i]);
	}
	else
	{
		phrase= CSPhraseCom::EmptyPhrase;
	}
}

// ***************************************************************************
bool				CSPhraseManager::isPhraseNextExecuteCounterSync() const
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	sint32	srvVal= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_COUNTER_NEXT)->getValue32();
	return srvVal==(sint32)_PhraseNextExecuteCounter;
}

// ***************************************************************************
bool				CSPhraseManager::isPhraseCycleExecuteCounterSync() const
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	sint32	srvVal= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_COUNTER_CYCLE)->getValue32();
	return srvVal==(sint32)_PhraseCycleExecuteCounter;
}

// ***************************************************************************
void				CSPhraseManager::sendMemorizeToServer(uint32 memoryLine, uint32 memorySlot, uint32 phraseId)
{
	// Local simulation
	if(ClientCfg.Local)
	{
		extern void	debugUpdateActionBar();
		debugUpdateActionBar();
	}
	// Send the memorize to server
	else
	{
		CBitMemStream out;
		const string sMsg = "PHRASE:MEMORIZE";
		if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			CSPhraseCom		phrase= getPhrase(phraseId);
			// free Band; don't send name
			phrase.Name.clear();

			uint8	memoryId= (uint8)memoryLine;
			uint8	slotId= (uint8)memorySlot;
			uint16	pid= (uint16)phraseId;
			out.serial(memoryId);
			out.serial(slotId);
			out.serial(pid);
			out.serial(phrase);
			NetMngr.push(out);
			//nlinfo("impulseCallBack : %s %d %d %d (phrase) sent", sMsg.c_str(), memoryId, slotId, pid);
		}
		else
			nlwarning("impulseCallBack : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
void				CSPhraseManager::sendForgetToServer(uint32 memoryLine, uint32 memoryIndex)
{
	// Local simulation
	if(ClientCfg.Local)
	{
	}
	// Send the forget to server.
	else
	{
		CBitMemStream out;
		const string sMsg = "PHRASE:FORGET";
		if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			//serial the sentence memorized index
			uint8	memoryId= (uint8)memoryLine;
			uint8	slotId= (uint8)memoryIndex;
			out.serial( memoryId );
			out.serial( slotId );
			NetMngr.push(out);
			//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), memoryId, slotId);
		}
		else
			nlwarning("impulseCallBack : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
void				CSPhraseManager::reset()
{
	_PhraseMap.clear();
	_PhraseClient.clear();
	_FreeSlots.clear();
	_Memories.clear();

	// Default alloc.
	_PhraseClient.resize(1024);

	_InitInGameDone= false;

	_SelectedMemoryDB= -1;
	_SelectedMemoryDBalt = _SelectedMemoryDB;
	// NB: slot under 2 can't be taken.
	_MaxSlotSet= BookStartSlot-1;
	_LastBookNumDbFill= 0;
	_PhraseNextExecuteCounter= 0;
	_PhraseCycleExecuteCounter= 0;
	_CurrentExecuteLineNext= -1;
	_CurrentExecuteSlotNext= -1;
	_CurrentExecutePhraseIdNext= 0;
	_CurrentExecuteLineCycle= -1;
	_CurrentExecuteSlotCycle= -1;
	_CurrentExecutePhraseIdCycle= 0;
	_PhraseDebugEndNextAction= 0;
	_PhraseDebugEndCyclicAction= 0;
	_UserIndoor = true;
	_BookSkillFitler = SKILLS::unknown;
	_BookBrickTypeFilter = BRICK_TYPE::UNKNOWN;

	_BookDbLeaves.clear();
	for(uint i=0;i<NumProgressType;i++)
	{
		_ProgressionDbSheets[i].clear();
		_ProgressionDbLevels[i].clear();
		_ProgressionDbLocks[i].clear();
		_LastProgressionNumDbFill[i]= 0;
	}
	_MemoryDbLeaves.clear();
	_MemoryAltDbLeaves.clear();
	_NextExecuteLeaf= NULL;
	_NextExecuteIsCyclicLeaf= NULL;

	_EquipInvalidationEnd= 0;
	_CurrentServerTick= 0;

	CompositionPhraseMemoryLineDest= 0;
	CompositionPhraseMemoryLineDest= -1;
	CompositionPhraseMemorySlotDest= -1;

	// remove phrase progression update
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSkillManager	*pSM= CSkillManager::getInstance();
	pBM->removeBrickLearnedCallback(&_ProgressionUpdate);
	pSM->removeSkillChangeCallback(&_ProgressionUpdate);

	_TotalMalusEquipLeaf = NULL;
}

// ***************************************************************************
bool CSPhraseManager::isPhraseKnown(const CSPhraseCom &phrase) const
{
	for(TPhraseMap::const_iterator it = _PhraseMap.begin(); it != _PhraseMap.end(); ++it)
	{
		if (it->second == phrase) return true;
	}
	return false;
}

// ***************************************************************************
ucstring	CSPhraseManager::formatMalus(sint base, sint malus)
{
	if(malus)
		return toString("@{F80F}%d@{FFFF}  (%d)", base+malus, base);
	else
		return toString(base);
}

// ***************************************************************************
ucstring	CSPhraseManager::formatMalus(float base, float malus)
{
	if(malus)
		return toString("@{F80F}%.1f@{FFFF}  (%.1f)", base+malus, base);
	else
		return toString("%.1f", base);
}

// ***************************************************************************
string	CSPhraseManager::formatBonusMalus(sint32 base, sint32 mod)
{
	string str;
	if( mod == 0 )
	{
		str = toString("@{FFFF}") + toString(base);
	}
	else
		if( mod > 0 ) // bonus
		{
			str = "@{0F0F}" + toString( base + mod )
				+ "@{FFFF}("
				+ toString( base )
				+ "@{0F0F} + "
				+ toString( mod )
				+ "@{FFFF})";
		}
		else
		{
			str = "@{E42F}" + toString( base + mod )
				+ "@{FFFF}("
				+ toString( base )
				+ "@{E42F} - "
				+ toString( mod )
				+ "@{FFFF})";
		}
	return str;
}

// ***************************************************************************
void CSPhraseManager::buildPhraseDesc(ucstring &text, const CSPhraseCom &phrase, uint32 phraseSheetId, bool wantRequirement, const std::string &specialPhraseFormat)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	text.erase();

	// castable action?
	bool	castable= phraseSheetId==0 || isPhraseCastable(phraseSheetId);


	// For true phraseId, or castable .sphrase only
	if(castable)
	{
		// **** Get the format from EnergyType
		CSBrickSheet	*rootBrick= NULL;
		if(phrase.Bricks.size())
			rootBrick= pBM->getBrick(phrase.Bricks[0]);
		if(rootBrick)
		{
			static const string		compoId= "composition";
			static const ucstring	compoTag("%compostart");
			bool	isComposition= specialPhraseFormat==compoId;

			// if format not given by user, auto select it.
			if(specialPhraseFormat.empty() || isComposition)
			{
				if(rootBrick->isCombat())
					text= CI18N::get("uihelpPhraseCombatFormat");
				else if(rootBrick->isMagic())
					text= CI18N::get("uihelpPhraseMagicFormat");
				else if(rootBrick->isFaber())
					text= CI18N::get("uihelpPhraseCraftFormat");
				else if(rootBrick->isSpecialPower())
					text= CI18N::get("uihelpPhraseSpecialPowerFormat");
				else if(rootBrick->isForageExtraction())
					text= CI18N::get("uihelpPhraseForageExtractionFormat");
				else if(rootBrick->isProcEnchantment())
					text= CI18N::get("uihelpPhraseProcEnchantment");
				else
					text= CI18N::get("uihelpPhraseOtherFormat");

				// if composition, cut the text before the tag (including)
				if(isComposition)
				{
					ucstring::size_type	pos= text.find(compoTag);
					if(pos!=ucstring::npos)
						text.erase(0, pos+compoTag.size());
				}
				// else just clear the tag
				else
				{
					strFindReplace(text, compoTag, ucstring() );
				}
			}
			else
				text= CI18N::get(specialPhraseFormat);
		}
		else
			return;

		// **** Phrase info basics
		// replace name
		strFindReplace(text, "%name", phrase.Name);
		// replace Sabrina Cost and credit.
		uint32	cost, credit;
		pBM->getSabrinaCom().getPhraseCost(phrase.Bricks, cost, credit);
		strFindReplace(text, "%cost", toString(cost));
		strFindReplace(text, "%credit", toString(credit));

		// for combat, fill weapon compatibility
		ucstring	weaponRestriction;
		bool		usableWithMelee;
		bool		usableWithRange;
		if(rootBrick && rootBrick->isCombat())
		{
			getCombatWeaponRestriction(weaponRestriction, phrase, usableWithMelee, usableWithRange);
			strFindReplace(text, "%wcomp", weaponRestriction);
		}

		// for Magic, fill Spell Level, and Magic Resist Type
		if(rootBrick && rootBrick->isMagic())
		{
			// Spell Level
			uint32		spellLevel= getSpellLevel(phrase.Bricks);
			strFindReplace(text, "%mglvl", toString(spellLevel));

			// ResistMagic. May have mutliple because of double spell
			bool		resistMagic[RESISTANCE_TYPE::NB_RESISTANCE_TYPE];
			getResistMagic(resistMagic, phrase.Bricks);
			bool		first= true;
			ucstring	resList;
			for(uint i=0;i<RESISTANCE_TYPE::NB_RESISTANCE_TYPE;i++)
			{
				if(resistMagic[i])
				{
					if(first)
						first= false;
					else
						resList+= ", ";
					resList+= CI18N::get("rs"+ RESISTANCE_TYPE::toString((RESISTANCE_TYPE::TResistanceType)i) );
				}
			}
			// If have some resist
			if(!resList.empty())
			{
				ucstring	fmt= CI18N::get("uihelpPhraseMagicResist");
				strFindReplace(fmt, "%t", resList);
				strFindReplace(text, "%magicresist", fmt);
			}
			else
			{
				strFindReplace(text, "%magicresist", ucstring());
			}

		}

		// **** Compute Phrase Elements from phrase
		// get the current action malus (0-100)
		uint32	totalActionMalus= 0;
		CCDBNodeLeaf *actMalus = _TotalMalusEquipLeaf ? &*_TotalMalusEquipLeaf
			: &*(_TotalMalusEquipLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:TOTAL_MALUS_EQUIP", false));
		
		// root brick must not be Power or aura, because Action malus don't apply to them
		// (ie leave 0 ActionMalus for Aura or Powers
		if(actMalus && !rootBrick->isSpecialPower())
			totalActionMalus= actMalus->getValue32();
		// Get the stats of the phrase, with malus due to item weared
		sint	success= getPhraseSuccessRate(phrase);
		float	castTime= 0, castTimeMalus= 0;
		sint	range= 0, rangeMalus= 0;
		sint	hpCost= 0, hpCostMalus= 0;
		sint	enCost= 0, enCostMalus= 0;

		getPhraseCastTime(phrase, totalActionMalus, castTime, castTimeMalus);
		getPhraseMagicRange(phrase, totalActionMalus, range, rangeMalus);
		getPhraseHpCost(phrase, totalActionMalus, hpCost, hpCostMalus);

		if(rootBrick->isCombat())
			getPhraseStaCost(phrase, totalActionMalus, enCost, enCostMalus);
		else if(rootBrick->isMagic())
			getPhraseSapCost(phrase, totalActionMalus, enCost, enCostMalus);
		else
			getPhraseFocusCost(phrase, totalActionMalus, enCost, enCostMalus);

		sint32 successModifier = 0;
		CCDBNodeLeaf * nodeSM = NULL;
		if(rootBrick->isCombat())
		{
			// if phrase can be used with in melee and range we choose which one to display according to hand weapon family
			if( usableWithMelee && usableWithRange )
			{
				CInventoryManager *inv = CInventoryManager::getInstance();
				if(inv)
				{
					uint32 rightHandSheet = inv->getRightHandItemSheet();
					if( inv->isRangeWeaponItem(rightHandSheet) )
					{
						nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:RANGE", false);
					}
					else
					{
						nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:MELEE", false);
					}
				}
			}
			else
			// phrase usable only in melee fight
			if( usableWithMelee )
			{
				nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:MELEE", false);
			}
			else
			// phrase usable only in range fight
			if( usableWithRange )
			{
				nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:RANGE", false);
			}
		}
		else
		if(rootBrick->isMagic())
		{
			nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:MAGIC", false);
		}
		if(nodeSM)
		{
			successModifier = nodeSM->getValue32();
		}
		string successStr = formatBonusMalus(success, successModifier);

		// **** assemble in text
		strFindReplace(text, "%success", toString(successStr));
		strFindReplace(text, "%duration", formatMalus(castTime, castTimeMalus) );
		strFindReplace(text, "%energy_cost", formatMalus(enCost, enCostMalus) );
		strFindReplace(text, "%hp_cost", formatMalus(hpCost, hpCostMalus) );
		// special range and "self"
		if(range==0)
			strFindReplace(text, "%range", CI18N::get("uihelpPhraseRangeSelf"));
		else
		{
			ucstring	fmt= CI18N::get("uihelpPhraseRangeMeters");
			strFindReplace(fmt, "%dist", formatMalus(range, rangeMalus));
			strFindReplace(text, "%range", fmt);
		}

		// **** special Forage extraction
		if(rootBrick->isForageExtraction())
		{
			// Choose the fmt text
			ucstring	fmt= getForageExtractionPhraseEcotypeFmt(phrase);

			// Replace forage success rate in any ecotype
			successModifier = 0;
			sint32 commonModifier = 0;
			nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:0:FORAGE", false);
			if(nodeSM)
			{
				commonModifier = nodeSM->getValue32();
			}
			//desert
			success= getForageExtractionPhraseSuccessRate(phrase, SKILLS::SHFDAEM);
			nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:1:FORAGE", false);
			if(nodeSM) successModifier = nodeSM->getValue32();
			if( successModifier == 0 )
				successModifier = commonModifier;
			successStr = formatBonusMalus(success, successModifier);
			successModifier = 0;
			strFindReplace(fmt, "%suc_desert", toString(successStr));

			//forest
			success= getForageExtractionPhraseSuccessRate(phrase, SKILLS::SHFFAEM);
			nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:2:FORAGE", false);
			if(nodeSM) successModifier = nodeSM->getValue32();
			if( successModifier == 0 )
				successModifier = commonModifier;
			successStr = formatBonusMalus(success, successModifier);
			successModifier = 0;
			strFindReplace(fmt, "%suc_forest", successStr);

			// lake
			success= getForageExtractionPhraseSuccessRate(phrase, SKILLS::SHFLAEM);
			nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:3:FORAGE", false);
			if(nodeSM) successModifier = nodeSM->getValue32();
			if( successModifier == 0 )
				successModifier = commonModifier;
			successStr = formatBonusMalus(success, successModifier);
			successModifier = 0;
			strFindReplace(fmt, "%suc_lake", successStr);

			// jungle
			success= getForageExtractionPhraseSuccessRate(phrase, SKILLS::SHFJAEM);
			nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:4:FORAGE", false);
			if(nodeSM) successModifier = nodeSM->getValue32();
			if( successModifier == 0 )
				successModifier = commonModifier;
			successStr = formatBonusMalus(success, successModifier);
			successModifier = 0;
			strFindReplace(fmt, "%suc_jungle", successStr);

			//prime roots
			success= getForageExtractionPhraseSuccessRate(phrase, SKILLS::SHFPAEM);
			nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:6:FORAGE", false);
			if(nodeSM) successModifier = nodeSM->getValue32();
			if( successModifier == 0 )
				successModifier = commonModifier;
			successStr = formatBonusMalus(success, successModifier);
			successModifier = 0;
			strFindReplace(fmt, "%suc_prime", successStr);

			// replace the forage succes
			strFindReplace(text, "%suc_forage", fmt);
		}
	}
	// **** not castable case
	else
	{
		// special for charac buying
		if(isPhraseCharacBuying(phraseSheetId))
			text= CI18N::get("uihelpPhraseCharacteristic");
		else
			text= CI18N::get("uihelpPhraseNotCastableFormat");
	}

	// **** Special .sphrase description
	if(phraseSheetId)
	{
		// get the text
		ucstring	desc(STRING_MANAGER::CStringManagerClient::getSPhraseLocalizedDescription(CSheetId(phraseSheetId)));
		if(desc.empty())
			strFindReplace(text, "%desc", ucstring());
		else
		{
			// append an \n before, for clearness
			desc= ucstring("\n") + desc;
			// append \n at end if not done
			if(desc[desc.size()-1]!='\n')
				desc+= '\n';
			// replace in format
			strFindReplace(text, "%desc", desc);
		}
	}
	else
	{
		strFindReplace(text, "%desc", ucstring());
	}

	// **** Special .sphrase requirement
	if(phraseSheetId && wantRequirement)
	{
		ucstring	reqText;
		reqText= CI18N::get("uihelpPhraseRequirementHeader");

		// replace the skill point cost
		strFindReplace(reqText, "%sp", toString(getCurrentPhraseSheetPrice(phraseSheetId)));
		strFindReplace(reqText, "%basesp", toString(getBasePhraseSheetPrice(phraseSheetId)));

		// build the phrase requirement from this sheetId
		TPhraseReqSkillMap::const_iterator	it= _PhraseReqSkillMap.find(phraseSheetId);
		if(it!=_PhraseReqSkillMap.end())
		{
			const CReqSkillFormula	&formula= it->second;

			// from this formula, build the requirement tex
			ucstring	textForm;
			formula.getInfoText(textForm);
			reqText+= textForm;
		}

		// replace in text
		strFindReplace(text, "%req", reqText);
	}
	else
		strFindReplace(text, "%req", "");
}


// ***************************************************************************
sint				CSPhraseManager::getPhraseSuccessRate(const CSPhraseCom &phrase)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSkillManager	*pSM= CSkillManager::getInstance();

	if(phrase.Bricks.size()==0)
		return 0;
	CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
	if(!rootBrick)
		return 0;

	// get the Skill value (according to phrase type etc...)
	TSuccessTable	sTable;
	sint			skillValue= 0;
	if(rootBrick->isCombat())
	{
		// retrieve the current Skill of the Item in RightHand
		SKILLS::ESkills		skill= getRightHandItemSkill();

		// Get its value
		skillValue= pSM->getSkillValueMaxBranch(skill);

		// use the combat table
		sTable= STCombat;
	}
	else if(rootBrick->isMagic())
	{
		uint32	numSkill= 0;
		skillValue= 0;

		// Must take a mean of all skills
		for(uint i=0;i<phrase.Bricks.size();i++)
		{
			CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
			if(brick)
			{
				SKILLS::ESkills		skill= brick->getSkill();
				if(skill!=SKILLS::unknown)
				{
					skillValue+= pSM->getSkillValueMaxBranch(skill);
					numSkill++;
				}
			}
		}

		// The mean!
		if(numSkill)
			skillValue/= numSkill;

		// use one of the magic table
		if(rootBrick->ActionNature==ACTNATURE::CURATIVE_MAGIC)
			sTable= STCurativeMagic;
		else
			// should be ACTNATURE::OFFENSIVE_MAGIC !!
			sTable= STOffensiveMagic;
	}
	// Craft: false, but return value not used
	else if(rootBrick->isFaber())
	{
		skillValue= pSM->getSkillValueMaxBranch(rootBrick->getSkill());
		sTable= STCraft;
	}
	// Forage Prospect: always success
	else if(rootBrick->isForageProspection())
	{
		return 100;
	}
	// Forage Extract: false, but return value not used
	else if(rootBrick->isForageExtraction())
	{
		skillValue= pSM->getSkillValueMaxBranch(rootBrick->getSkill());
		sTable= STExtract;
	}
	// Default
	else
	{
		// Default: take skill value of the root
		skillValue= pSM->getSkillValueMaxBranch(rootBrick->getSkill());
		// well, it's bug time!
		sTable= STCombat;
	}

	// return the success rate with this skillValue
	return getPhraseSuccessRate(sTable, phrase, skillValue, INT_MAX);
}

// ***************************************************************************
sint				CSPhraseManager::getCraftPhraseSuccessRate(const CSPhraseCom &phrase, SKILLS::ESkills skill, uint minMpLevel)
{
	CSkillManager	*pSM= CSkillManager::getInstance();

	if(phrase.Bricks.size()==0)
		return 0;

	// take skill value of the skill
	sint	skillValue= pSM->getBestSkillValue(skill);

	// return the sr according to this skill
	return getPhraseSuccessRate(STCraft, phrase, skillValue, minMpLevel);
}

// ***************************************************************************
sint				CSPhraseManager::getForageExtractionPhraseSuccessRate(const CSPhraseCom &phrase, SKILLS::ESkills skill)
{
	CSkillManager	*pSM= CSkillManager::getInstance();

	if(phrase.Bricks.size()==0)
		return 0;

	// take skill value of the skill
	sint	skillValue= pSM->getSkillValueMaxBranch(skill);

	// return the sr according to this skill
	return getPhraseSuccessRate(STExtract, phrase, skillValue, INT_MAX);
}

// ***************************************************************************
void				CSPhraseManager::getPhraseSapCost(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &cost, sint &costMalus)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	cost= (sint)getPhraseSumBrickProp(phrase, pBM->SapPropId, true);
	// compute malus (positive)
	costMalus= (cost * (totalActionMalus))/100;
}

// ***************************************************************************
void				CSPhraseManager::getPhraseStaCost(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &cost, sint &costMalus)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	cost= (sint)getPhraseSumBrickProp(phrase, pBM->StaPropId, true);
	// TODO: combat special case
	// compute malus (positive)
	costMalus= (cost * (totalActionMalus))/100;
}

// ***************************************************************************
void				CSPhraseManager::getPhraseFocusCost(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &cost, sint &costMalus)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	cost= (sint)getPhraseSumBrickProp(phrase, pBM->FocusPropId, true);
	// compute malus (positive)
	costMalus= (cost * (totalActionMalus))/100;
}

// ***************************************************************************
void				CSPhraseManager::getPhraseHpCost(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &cost, sint &costMalus)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	cost= (sint)getPhraseSumBrickProp(phrase, pBM->HpPropId, true);
	// compute malus (positive)
	costMalus= (cost * (totalActionMalus))/100;
}

// ***************************************************************************
void				CSPhraseManager::getPhraseCastTime(const CSPhraseCom &phrase, uint32 totalActionMalus, float &castTime, float &castTimeMalus)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	castTime= 0;
	castTimeMalus= 0;

	if(phrase.Bricks.size()==0)
		return;
	CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
	if(!rootBrick)
		return;

	// for all bricks get the max "min and Max" cast time, and get the sum of time credit
	uint	sumCastTimeCredit= 0;
	uint	sabrinaCost= 0;
	float	sabrinaRelativeCost = 1.0f;
	uint	minCastTime= 0, maxCastTime= 0;
	uint	i;
	for(i=0;i<phrase.Bricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
		if(brick)
		{
			// For cast time, take the max
			minCastTime= max(minCastTime, (uint)brick->MinCastTime);
			maxCastTime= max(maxCastTime, (uint)brick->MaxCastTime);

			if(brick->SabrinaCost>0)
				sabrinaCost+= brick->SabrinaCost;

			if( brick->SabrinaRelativeCost>0.f)
				sabrinaRelativeCost+= brick->SabrinaRelativeCost;

			for(uint j=0;j<brick->Properties.size();j++)
			{
				if(brick->Properties[j].PropId == pBM->CastTimePropId)
				{
					sumCastTimeCredit+= abs(brick->SabrinaCost);
				}
			}
		}
	}

	// TODO yoyo: may change in future?
	if(rootBrick->isFaber())
		minCastTime= maxCastTime= 20;

	// Code from Server!!
	float castingTimeFactor;
	if(sabrinaCost)
		castingTimeFactor = (float)sumCastTimeCredit/(float)(sabrinaCost*sabrinaRelativeCost);
	else
		castingTimeFactor = (float)sumCastTimeCredit;
	clamp(castingTimeFactor, 0.f, 1.f);
	// Check Min >= Max
	if(maxCastTime < minCastTime)
		maxCastTime = minCastTime;

	// Compute the casting time.
	castTime= (float)(maxCastTime-minCastTime)*castingTimeFactor+(float)minCastTime;

	// Compute the malus (positive value)
	castTimeMalus= (float)(maxCastTime-minCastTime)*castingTimeFactor*totalActionMalus/100;

	// In bricks, save in (1/10sec)
	castTime/= 10;
	castTimeMalus/= 10;
}

// ***************************************************************************
void				CSPhraseManager::getPhraseMagicRange(const CSPhraseCom &phrase, uint32 totalActionMalus, sint &range, sint &rangeMalus)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	range= 0;
	rangeMalus= 0;

	if(phrase.Bricks.size()==0)
		return;
	CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
	if(!rootBrick)
		return;

	// for all bricks get the min "min and Max" range, and get the sum of range credit
	uint	sumRangeCredit= 0;
	uint	sabrinaCost= 0;
	float	sabrinaRelativeCost= 1.f;
	uint	minRange= 255, maxRange= 255;
	uint	i;
	for(i=0;i<phrase.Bricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
		if(brick)
		{
			// For range, take the min
			minRange= min(minRange, (uint)brick->MinRange);
			maxRange= min(maxRange, (uint)brick->MaxRange);

			if(brick->SabrinaCost>0)
				sabrinaCost+= brick->SabrinaCost;

			if(brick->SabrinaRelativeCost>0.f)
				sabrinaRelativeCost+= brick->SabrinaRelativeCost;

			for(uint j=0;j<brick->Properties.size();j++)
			{
				if(brick->Properties[j].PropId == pBM->RangePropId)
				{
					// For Range, must add the Creidt Value of the prop
					sumRangeCredit+= (uint)fabs(brick->Properties[j].Value);
				}
			}
		}
	}

	// Code from Server!!
	float rangeFactor;
	if(sabrinaCost)
		rangeFactor = (float)sumRangeCredit/(float)(sabrinaCost*sabrinaRelativeCost);
	else
		rangeFactor = (float)sumRangeCredit;
	clamp(rangeFactor, 0.f, 1.f);
	// Check Min >= Max
	if(maxRange < minRange)
		maxRange = minRange;
	// Compute the range. the more the credit, the lesser the range
	float fRange = (float)(maxRange-minRange)*(1-rangeFactor)+(float)minRange;

	// Compute the range, with malus
	float fRangeWithMalus = (float)(maxRange-minRange)*(1-rangeFactor)*100/(100+totalActionMalus)+(float)minRange;

	// In bricks, save in meters
	range= (sint)fRange;
	rangeMalus= (sint)fRangeWithMalus - range;
}

// ***************************************************************************
float				CSPhraseManager::getPhraseSumBrickProp(const CSPhraseCom &phrase, uint propId, bool doAbs)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	float	sum= 0.f;
	for(uint i=0;i<phrase.Bricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
		if(brick)
		{
			// test all properties
			for(uint j=0;j<brick->Properties.size();j++)
			{
				if(brick->Properties[j].PropId==propId)
				{
					if(doAbs)
						sum+= (float)fabs(brick->Properties[j].Value);
					else
						sum+= brick->Properties[j].Value;
				}
				else if(propId==CSBrickManager::getInstance()->StaPropId && brick->Properties[j].PropId==CSBrickManager::getInstance()->StaWeightFactorId)
				{
					CInterfaceManager *im = CInterfaceManager::getInstance();
					if (!_ServerUserDefaultWeightHandsLeaf) _ServerUserDefaultWeightHandsLeaf = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:DEFAULT_WEIGHT_HANDS");
					uint32 weight = (uint32)(&*_ServerUserDefaultWeightHandsLeaf)->getValue32() / 10; // weight must be in dg here
					CDBCtrlSheet *ctrlSheet = dynamic_cast<CDBCtrlSheet *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:gestionsets:hands:handr"));
					if (ctrlSheet)
					{
						const CItemSheet *itemSheet = ctrlSheet->asItemSheet();
						if (itemSheet && (itemSheet->Family == ITEMFAMILY::MELEE_WEAPON || itemSheet->Family == ITEMFAMILY::RANGE_WEAPON))
						{
							weight = (uint32) ctrlSheet->getItemWeight();
						}
					}
					ctrlSheet = dynamic_cast<CDBCtrlSheet *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:gestionsets:hands:handl"));
					if (ctrlSheet)
					{
						const CItemSheet *itemSheet = ctrlSheet->asItemSheet();
						if (itemSheet && (itemSheet->Family == ITEMFAMILY::MELEE_WEAPON /*|| itemSheet->Family == ITEMFAMILY::AMMO*/))
						{
							weight = (weight + (uint32) ctrlSheet->getItemWeight()) /*/ 2*/;
						}
					}

//					float fweight = (float)weight / DB_WEIGHT_SCALE;
					float value = brick->Properties[j].Value;
					sint32 value2 = sint32(brick->Properties[j].Value2);

					if(doAbs)
						sum+= (float)fabs(value * weight / DB_WEIGHT_SCALE + value2);
					else
						sum+= brick->Properties[j].Value * weight / DB_WEIGHT_SCALE + brick->Properties[j].Value2;
				}
			}
		}
	}

	return sum;
}


// ***************************************************************************
bool CSPhraseManager::isExecutionInvalidCauseTempInv() const
{
	CTempInvManager *pTIM = CTempInvManager::getInstance();
	// Never invalidate action if we are in forage mode
	if (pTIM->getMode() == TEMP_INV_MODE::Forage)
		return false;
	// Invalid action if the temp inv is opened
	return CTempInvManager::getInstance()->isOpened();
}

// ***************************************************************************
sint	CSPhraseManager::getSuccessRate(TSuccessTable st, sint level, bool partialSuccess)
{
	nlassert(st>=0 && st<NumSuccessTable);
	if(_SuccessTableSheet[st])
	{
		sint	bestProba= 50;
		sint	bestLevelDiff= INT_MAX;

		for(uint i=0;i<_SuccessTableSheet[st]->SuccessTable.size();i++)
		{
			sint	levelDiff= level - _SuccessTableSheet[st]->SuccessTable[i].RelativeLevel;
			levelDiff= abs(levelDiff);
			if(levelDiff<bestLevelDiff)
			{
				bestLevelDiff= levelDiff;
				if(partialSuccess)
					bestProba= _SuccessTableSheet[st]->SuccessTable[i].PartialSuccessProbability;
				else
					bestProba= _SuccessTableSheet[st]->SuccessTable[i].SuccessProbability;
			}
		}

		return bestProba;
	}
	else
		return 0;
}

// ***************************************************************************
void	CSPhraseManager::loadSuccessTable()
{
	// load all the table from packed sheet
	_SuccessTableSheet[STCombat]= dynamic_cast<CSuccessTableSheet*>(SheetMngr.get(CSheetId("fight_phrase.succes_chances_table")));
	_SuccessTableSheet[STOffensiveMagic]= dynamic_cast<CSuccessTableSheet*>(SheetMngr.get(CSheetId("offensive_magic.succes_chances_table")));
	_SuccessTableSheet[STCurativeMagic]= dynamic_cast<CSuccessTableSheet*>(SheetMngr.get(CSheetId("curative_magic.succes_chances_table")));
	_SuccessTableSheet[STCraft]= dynamic_cast<CSuccessTableSheet*>(SheetMngr.get(CSheetId("craft.succes_chances_table")));
	_SuccessTableSheet[STExtract]= dynamic_cast<CSuccessTableSheet*>(SheetMngr.get(CSheetId("extracting.succes_chances_table")));
	_SuccessTableSheet[STResistMagic]= dynamic_cast<CSuccessTableSheet*>(SheetMngr.get(CSheetId("magic_resist.succes_chances_table")));
	_SuccessTableSheet[STResistMagicLink]= dynamic_cast<CSuccessTableSheet*>(SheetMngr.get(CSheetId("magic_resist_link.succes_chances_table")));
	_SuccessTableSheet[STDodgeParry]= dynamic_cast<CSuccessTableSheet*>(SheetMngr.get(CSheetId("dodge_parry.succes_chances_table")));
}

// ***************************************************************************
void	CSPhraseManager::update()
{
	// Update brick state ?
	if (UserEntity)
	{
		bool userIndoor = UserEntity->indoor();
		if (_UserIndoor != userIndoor)
		{
			_UserIndoor = userIndoor;
			updateAllMemoryCtrlState();
		}
	}
}

// ***************************************************************************
sint	CSPhraseManager::getPhraseSuccessRate(TSuccessTable st, const CSPhraseCom &phrase, sint skillValue, uint minMpLevel)
{
	if(phrase.empty())
		return 0;

	CSBrickManager	*pBM= CSBrickManager::getInstance();

	// get the Sabrina cost
	uint32	costSum, creditSum;
	pBM->getSabrinaCom().getPhraseCost(phrase.Bricks, costSum, creditSum);
	// get the max cost of all brick
	uint32	costMax= pBM->getSabrinaCom().getPhraseMaxBrickCost(phrase.Bricks);

	// Special For combat: maybe replace the costSum
	CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
	if(rootBrick && rootBrick->isCombat())
	{
		// this to avoid problem with the default attack which has a 0 cost...
		costSum= max(costSum, getRightHandEffectiveLevel());
	}


	// New Formula to get the Entry in Difficulty table
	sint	deltaLevel;
	// special for craft (credit is actualy the level of item crafted)
	if(st==STCraft)
		deltaLevel= skillValue - min((sint)creditSum,(sint)minMpLevel);
	// general case
	else
		deltaLevel= skillValue + (sint)creditSum - (sint)costSum - (sint)costMax;

	return getSuccessRate(st, deltaLevel);
}

// ***************************************************************************
void	CSPhraseManager::updateExecutionDisplay()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// **** test if have to draws the icons, and try to recover if sphrase forgeted or removed.

	// do we have to display icons?
	bool	displayCycle= _CurrentExecuteSlotCycle>=0 && _SelectedMemoryDB==_CurrentExecuteLineCycle;
	bool	displayNext= _CurrentExecuteSlotNext>=0 && _SelectedMemoryDB==_CurrentExecuteLineNext;

	// if yes, verify that the actions under the icons are relevant
	if(displayCycle)
	{
		// if the memory slot is different from the action executed
		if(_Memories[_CurrentExecuteLineCycle].Slot[_CurrentExecuteSlotCycle].Id != _CurrentExecutePhraseIdCycle)
		{
			// fails!
			displayCycle= false;

			// try to find in the action bar, this action (in case of DragAndDrop for instance)
			for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
			{
				if(_Memories[_CurrentExecuteLineCycle].Slot[i].Id == _CurrentExecutePhraseIdCycle)
				{
					// change the slot number
					_CurrentExecuteSlotCycle= i;
					// ok and stop
					displayCycle= true;
					break;
				}
			}
		}
	}

	// if yes, verify that the actions under the icons are relevant
	if(displayNext)
	{
		// if the memory slot is different from the action executed
		if(_Memories[_CurrentExecuteLineNext].Slot[_CurrentExecuteSlotNext].Id != _CurrentExecutePhraseIdNext)
		{
			// fails!
			displayNext= false;

			// try to find in the action bar, this action (in case of DragAndDrop for instance)
			for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
			{
				if(_Memories[_CurrentExecuteLineNext].Slot[i].Id == _CurrentExecutePhraseIdNext)
				{
					// change the slot number
					_CurrentExecuteSlotNext= i;
					// ok and stop
					displayNext= true;
					break;
				}
			}
		}
	}


	// **** display the icons in the action bar

	// Actually, if the NextAction is the same as the cycle one, hide next one
	if(displayCycle && displayNext &&
		_CurrentExecuteLineNext == _CurrentExecuteLineCycle &&
		_CurrentExecuteSlotNext == _CurrentExecuteSlotCycle )
		displayNext= false;

	// DisplayCycleSelectionOnActionBar
	CInterfaceElement	*viewCycle= CWidgetManager::getInstance()->getElementFromId(PhraseMemoryViewCycleAction);
	if(viewCycle)
	{
		CInterfaceElement	*ctrl= NULL;
		if(displayCycle)
			ctrl= CWidgetManager::getInstance()->getElementFromId(PhraseMemoryViewSlotBase + toString(_CurrentExecuteSlotCycle));
		if(displayCycle && ctrl)
		{
			viewCycle->setParentPos(ctrl);
			viewCycle->setActive(true);
			viewCycle->invalidateCoords();
		}
		else
		{
			viewCycle->setActive(false);
		}
	}

	// DisplayNextSelectionOnActionBar
	CInterfaceElement	*viewNext= CWidgetManager::getInstance()->getElementFromId(PhraseMemoryViewNextAction);
	if(viewNext)
	{
		CInterfaceElement	*ctrl= NULL;
		if(displayNext)
			ctrl= CWidgetManager::getInstance()->getElementFromId(PhraseMemoryViewSlotBase + toString(_CurrentExecuteSlotNext));
		if(displayNext && ctrl)
		{
			viewNext->setParentPos(ctrl);
			viewNext->setActive(true);
			viewNext->invalidateCoords();
		}
		else
		{
			viewNext->setActive(false);
		}
	}


	// **** Display Next Action on Slot in PLAYER window , whatever the selected memory line is
	if(_CurrentExecutePhraseIdCycle>0 || _CurrentExecutePhraseIdNext>0)
	{
		// get the next phrase
		uint	nextPhrase;
		if(_CurrentExecutePhraseIdNext>0)
			nextPhrase= _CurrentExecutePhraseIdNext;
		else
			nextPhrase= _CurrentExecutePhraseIdCycle;

		// display this slot
		_NextExecuteLeaf->setValue32(nextPhrase);

		// display or not the widget "this is a cyclic action"
		_NextExecuteIsCyclicLeaf->setValue32(_CurrentExecutePhraseIdNext==0 || _CurrentExecutePhraseIdNext==_CurrentExecutePhraseIdCycle);
	}
	else
	{
		_NextExecuteLeaf->setValue32(0);
		_NextExecuteIsCyclicLeaf->setValue32(0);
	}
}

// ***************************************************************************
void	CSPhraseManager::clientExecute(uint32 memoryLine, uint32 memorySlot, bool cyclic)
{
	if(cyclic)
	{
		// Inc Local Counter For Server/Client synchronisation
		_PhraseCycleExecuteCounter++;
		_PhraseCycleExecuteCounter&=PHRASE_EXECUTE_COUNTER_MASK;
		// what slot set?
		_CurrentExecuteLineCycle= memoryLine;
		_CurrentExecuteSlotCycle= memorySlot;
		_CurrentExecutePhraseIdCycle= getMemorizedPhrase(memoryLine, memorySlot);
	}
	else
	{
		// Inc Local Counter For Server/Client synchronisation
		_PhraseNextExecuteCounter++;
		_PhraseNextExecuteCounter&=PHRASE_EXECUTE_COUNTER_MASK;
		// what slot set?
		_CurrentExecuteLineNext= memoryLine;
		_CurrentExecuteSlotNext= memorySlot;
		_CurrentExecutePhraseIdNext= getMemorizedPhrase(memoryLine, memorySlot);
	}

	// update the Action Execution view
	updateExecutionDisplay();

	// Local simulation
	if(ClientCfg.Local)
	{
		if(cyclic)
			_PhraseDebugEndCyclicAction= T1 + 8000;
		else
			_PhraseDebugEndNextAction= T1 + 2000;
	}
}

// ***************************************************************************
void	CSPhraseManager::cancelClientExecute(bool cyclic)
{
	if(cyclic)
	{
		// Dec Local Counter For Server/Client synchronisation
		_PhraseCycleExecuteCounter--;
		_PhraseCycleExecuteCounter&=PHRASE_EXECUTE_COUNTER_MASK;
		// reset
		_CurrentExecuteLineCycle= -1;
		_CurrentExecuteSlotCycle= -1;
		_CurrentExecutePhraseIdCycle= 0;
	}
	else
	{
		// Dec Local Counter For Server/Client synchronisation
		_PhraseNextExecuteCounter--;
		_PhraseNextExecuteCounter&=PHRASE_EXECUTE_COUNTER_MASK;
		// what slot set?
		_CurrentExecuteLineNext= -1;
		_CurrentExecuteSlotNext= -1;
		_CurrentExecutePhraseIdNext= 0;
	}

	// update the Action Execution view
	updateExecutionDisplay();
}


// ***************************************************************************
void	CSPhraseManager::sendExecuteToServer(uint32 memoryLine, uint32 memorySlot, bool cyclic)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Local simulation
	if(ClientCfg.Local)
	{
		if(cyclic)
			pIM->displaySystemInfo("PhraseCycleCasted:"+toString(memoryLine)+"_"+toString(memorySlot), "CHK");
		else
			pIM->displaySystemInfo("PhraseNextCasted:"+toString(memoryLine)+"_"+toString(memorySlot), "CHK");

		extern void	debugUpdateActionBar();
		debugUpdateActionBar();
	}
	// Send the execute to server.
	else
	{
		// before, append the execution counter to the list of ACK to wait
		appendCurrentToAckExecute(cyclic);

		// send msg
		CBitMemStream out;
		const	char	*msg;
		if(cyclic)
			msg= "PHRASE:EXECUTE_CYCLIC";
		else
			msg= "PHRASE:EXECUTE";
		if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
		{
			//serial the sentence memorized index
			uint8	memoryId= (uint8)memoryLine;
			uint8	slotId= (uint8)memorySlot;
			out.serial( memoryId );
			out.serial( slotId );
			NetMngr.push(out);
		}
		else
			nlinfo("unknown message %s", msg);
	}
}

// ***************************************************************************
void	CSPhraseManager::executeCraft(uint32 memoryLine, uint32 memorySlot, uint32 planSheetId,
	std::vector<CFaberMsgItem> &mpItemPartList, std::vector<CFaberMsgItem> specificItemList)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// **** Do also a clientExecute to display the correct craft action (NB: not cyclic!)
	clientExecute(memoryLine, memorySlot, false);

	// **** Then do a special sendExecuteToServer
	// Local simulation
	if(ClientCfg.Local)
	{
		pIM->displaySystemInfo("PhraseCraftCasted:"+toString(memoryLine)+"_"+toString(memorySlot), "CHK");
		extern void	debugUpdateActionBar();
		debugUpdateActionBar();
	}
	// Send the execute to the server
	else
	{
		// before, append the execution counter to the list of ACK to wait
		appendCurrentToAckExecute(false);

		// send msg
		CBitMemStream out;
		const	char	*msg;
		msg= "PHRASE:EXECUTE_FABER";
		if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
		{
			// Serialize the faberPlan first.
			out.serial( planSheetId );
			// Serial the sentence memorized index
			uint8	memoryId= (uint8)memoryLine;
			uint8	slotId= (uint8)memorySlot;
			out.serial( memoryId );
			out.serial( slotId );
			// Then serial the list of Mp index
			out.serialCont( mpItemPartList );
			out.serialCont( specificItemList );
			NetMngr.push(out);
		}
		else
			nlinfo("unknown message %s", msg);
	}

}


// ***************************************************************************
void	CSPhraseManager::executeCristalize(uint32 memoryLine, uint32 memorySlot)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// **** Increment counter client side (like an execution) (no cylic)
	clientExecute(memoryLine, memorySlot, false);

	// **** special sendExecuteToServer
	if(ClientCfg.Local)
	{
		pIM->displaySystemInfo("PhraseCristalize:"+toString(memoryLine)+"_"+toString(memorySlot), "CHK");
		extern void	debugUpdateActionBar();
		debugUpdateActionBar();
	}
	else
	{
		// before, append the execution counter to the list of ACK to wait
		appendCurrentToAckExecute(false);

		// send msg
		CBitMemStream out;
		const string sMsg = "PHRASE:CRISTALIZE";
		if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			//serial the sentence memorized index
			uint8	memoryId= (uint8)memoryLine;
			uint8	slotId= (uint8)memorySlot;
			out.serial( memoryId );
			out.serial( slotId );
			NetMngr.push(out);
		}
		else
			nlinfo("unknown message %s", sMsg.c_str());
	}
}


// ***************************************************************************
void	CSPhraseManager::executeDefaultAttack()
{
	// Try to find a "default attack" in the memories, and prefer launch it instead
	uint32	memoryLine, memorySlot;
	if(findDefaultAttack(memoryLine, memorySlot))
	{
		clientExecute(memoryLine, memorySlot, true);
		sendExecuteToServer(memoryLine, memorySlot, true);
	}
	else
	{
		// Set the database in local.
		if(ClientCfg.Local)
		{
			IngameDbMngr.setProp("Entities:E0:P" + toString(CLFECOMMON::PROPERTY_MODE), MBEHAV::COMBAT);	// Mode
			UserEntity->updateVisualProperty(0, CLFECOMMON::PROPERTY_MODE);
		}
		else
		{
			// Attack MSG.
			const string msgName = "COMBAT:DEFAULT_ATTACK";
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
				NetMngr.push(out);
			else
				nlwarning("UE::attack: unknown message named '%s'.", msgName.c_str());
		}
	}
}


// ***************************************************************************
void	CSPhraseManager::appendCurrentToAckExecute(bool cyclic)
{
	CAckExecuteEntry	entry;

	// wait an ack from server
	entry.State= CAckExecuteEntry::WaitAck;
	if(cyclic)
	{
		entry.Counter= _PhraseCycleExecuteCounter;
		entry.MemoryLine= _CurrentExecuteLineCycle;
		entry.MemorySlot= _CurrentExecuteSlotCycle;
		entry.PhraseId= _CurrentExecutePhraseIdCycle;
		_AckExecuteCycleList.push_back(entry);
#ifdef	PHRASE_DEBUG_VERBOSE
		nlinfo("ACK-ADD: CYCLIC. counter=%d. size=%d.", entry.Counter, _AckExecuteCycleList.size());
#endif
	}
	else
	{
		entry.Counter= _PhraseNextExecuteCounter;
		entry.MemoryLine= _CurrentExecuteLineNext;
		entry.MemorySlot= _CurrentExecuteSlotNext;
		entry.PhraseId= _CurrentExecutePhraseIdNext;
		_AckExecuteNextList.push_back(entry);
#ifdef	PHRASE_DEBUG_VERBOSE
		nlinfo("ACK-ADD: NEXT. counter=%d. size=%d.", entry.Counter, _AckExecuteNextList.size());
#endif
	}

	// avoid any problems if server is too laggy. flush the list
	while(_AckExecuteCycleList.size()>PHRASE_EXECUTE_COUNTER_MASK+1)
		_AckExecuteCycleList.pop_front();
	while(_AckExecuteNextList.size()>PHRASE_EXECUTE_COUNTER_MASK+1)
		_AckExecuteNextList.pop_front();
}


// ***************************************************************************
void	CSPhraseManager::resetAckExecuteList(bool cyclic)
{
	if(cyclic)
	{
		_AckExecuteCycleList.clear();
#ifdef	PHRASE_DEBUG_VERBOSE
		nlinfo("ACK-RESET: CYCLIC.");
#endif
	}
	else
	{
		_AckExecuteNextList.clear();
#ifdef	PHRASE_DEBUG_VERBOSE
		nlinfo("ACK-RESET: NEXT.");
#endif
	}
}


// ***************************************************************************
void	CSPhraseManager::receiveAckExecuteFromServer(bool cyclic, uint counterValue, bool ok)
{
	// get the approriated list
	TAckExecuteList		&ackList= cyclic?_AckExecuteCycleList:_AckExecuteNextList;

	// **** if it's a validation
	if(ok)
	{
		// try to find the element in list
		TAckExecuteList::iterator	it= ackList.begin();
		for(;it!=ackList.end();it++)
		{
			if(it->Counter==counterValue)
			{
				// found, mark
				it->State= CAckExecuteEntry::OK;
				// erase all elements before it
				ackList.erase(ackList.begin(), it);
				break;
			}
		}
		// NB: if not found, noop (NB: possible for instance if msg arrives after DB update, or if order
		// of ACK is not good)
#ifdef	PHRASE_DEBUG_VERBOSE
		nlinfo("ACK-RECEIVE-OK: %s. counter=%d. size=%d", cyclic?"CYCLIC":"NEXT", counterValue, ackList.size());
#endif
	}
	// **** server cancelation
	else
	{
		// try to find the element in list
		bool	found= false;
		TAckExecuteList::iterator	it= ackList.begin();
		for(;it!=ackList.end();it++)
		{
			if(it->Counter==counterValue)
			{
				// found, mark as KO
				it->State= CAckExecuteEntry::KO;
				found= true;
				break;
			}
		}
		// NB: if not found, noop (NB: possible for instance if msg arrives after DB update, or if order
		// of ACK is not good)


		// if element succesfuly marked, must find which action we have to fall back
		if(found)
		{
			// find the first element in list (start from end) that is not in KO state (ie WaitAck or OK)
			bool				foundFallBack= false;
			CAckExecuteEntry	fallBack;
			TAckExecuteList::reverse_iterator	it= ackList.rbegin();
			for(;it!=ackList.rend();it++)
			{
				if(it->State!=CAckExecuteEntry::KO)
				{
					foundFallBack= true;
					fallBack= *it;
					break;
				}
			}

			// if no fallback found (the list contains only KO)
			if(!foundFallBack)
			{
				// setup an empty slot
				fallBack.MemoryLine= -1;
				fallBack.MemorySlot= -1;
				fallBack.PhraseId= 0;
			}

			// replace the Next or Cycle action view, with this fallback slot.
			if(cyclic)
			{
				_CurrentExecuteLineCycle= fallBack.MemoryLine;
				_CurrentExecuteSlotCycle= fallBack.MemorySlot;
				_CurrentExecutePhraseIdCycle= fallBack.PhraseId;
			}
			else
			{
				_CurrentExecuteLineNext= fallBack.MemoryLine;
				_CurrentExecuteSlotNext= fallBack.MemorySlot;
				_CurrentExecutePhraseIdNext= fallBack.PhraseId;
			}

			// update views
			updateExecutionDisplay();
		}

#ifdef	PHRASE_DEBUG_VERBOSE
		nlinfo("ACK-RECEIVE-KO: %s. counter=%d. size=%d", cyclic?"CYCLIC":"NEXT", counterValue, ackList.size());
#endif
	}
}


// ***************************************************************************
/** Called when the database counter change
 */
class CHandlerPhraseCounterUpdate : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CSPhraseManager		*pPM = CSPhraseManager::getInstance();

		// Cyclic Cast?
		bool	cyclic;
		cyclic= getParam(Params, "cyclic") == "1" ;

		if(cyclic)
		{
			// if synchronized
			if(pPM->isPhraseCycleExecuteCounterSync())
			{
				// reset the CycleAckList
				pPM->resetAckExecuteList(true);

				// reset slot played.
				pPM->_CurrentExecuteLineCycle= -1;
				pPM->_CurrentExecuteSlotCycle= -1;
				pPM->_CurrentExecutePhraseIdCycle= 0;

				// update views
				pPM->updateExecutionDisplay();
			}
		}
		else
		{
			// if synchronized
			if(pPM->isPhraseNextExecuteCounterSync())
			{
				// reset the NextAckList
				pPM->resetAckExecuteList(false);

				// reset slot played.
				pPM->_CurrentExecuteLineNext= -1;
				pPM->_CurrentExecuteSlotNext= -1;
				pPM->_CurrentExecutePhraseIdNext= 0;

				// update views
				pPM->updateExecutionDisplay();
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseCounterUpdate, "phrase_counter_update");


// ***************************************************************************
class CHandlerPhraseDebugClient : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if(ClientCfg.Local)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			CSPhraseManager		*pPM = CSPhraseManager::getInstance();

			// simulate server response for Action
			if(T1>pPM->_PhraseDebugEndNextAction)
			{
				// copy counter
				NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_COUNTER_NEXT)->setValue32(pPM->_PhraseNextExecuteCounter);
			}
			if(T1>pPM->_PhraseDebugEndCyclicAction)
			{
				// copy counter
				NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_COUNTER_CYCLE)->setValue32(pPM->_PhraseCycleExecuteCounter);
			}

			sint64 st= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CURRENT_SERVER_TICK")->getValue64();
			sint64 stEnd= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_TEND")->getValue64();
			if(stEnd && st>stEnd+20)
				NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_TEND")->setValue64(0);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseDebugClient, "phrase_debug_client");


// ***************************************************************************
void CSPhraseManager::setEquipInvalidation(sint64 serverTick, sint invalidTickTime)
{
	// if the item takes no time to invalidate, no-op!
	if(invalidTickTime<0)
	{
		return;
	}

	sint64 tempTick = serverTick + invalidTickTime;
	if(tempTick==0)
	{
		_EquipInvalidationEnd = 0;
	}
	else
	{
		_EquipInvalidationEnd= max(_EquipInvalidationEnd, tempTick);
	}
}

// ***************************************************************************
void CSPhraseManager::updateEquipInvalidation(sint64 serverTick)
{
	bool	precState= isExecutionInvalidCauseEquip();

	// set new server tick
	_CurrentServerTick= serverTick;

	// if the state has changed, must update the ctrl states!
	if(precState != isExecutionInvalidCauseEquip())
	{
		updateAllMemoryCtrlState();
	}
}

// ***************************************************************************
void CSPhraseManager::updateAllActionRegen()
{	
	if (_RegenTickRangeTouched)
	{
		TTicks startTime = CTime::getPerformanceTime();
		updateAllMemoryCtrlRegenTickRange();		
		_RegenTickRangeTouched = false;
		TTicks endTime = CTime::getPerformanceTime();		
		//nldebug("***** %d ms for CSPhraseManager::updateAllActionRegen", (int) (1000 * CTime::ticksToSecond(endTime - startTime)));	
	}	
}

// ***************************************************************************
bool CSPhraseManager::isExecutionInvalidCauseEquip() const
{
	return _EquipInvalidationEnd > _CurrentServerTick;
}

// ***************************************************************************
void CSPhraseManager::sendDeleteToServer(uint32 slot)
{
	if(!ClientCfg.Local)
	{
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("PHRASE:DELETE", out))
		{
			uint16 slotId= (uint16)slot;
			out.serial(slotId);
			NetMngr.push(out);
		}
		else
			nlwarning(" unknown message name 'PHRASE:DELETE");
	}
}

// ***************************************************************************
void CSPhraseManager::sendLearnToServer(uint32 slot)
{
	CSPhraseCom		phrase= getPhrase(slot);
	if(phrase.empty())
		return;

	if(!ClientCfg.Local)
	{
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("PHRASE:LEARN", out))
		{
			uint16 slotId= (uint16)slot;
			out.serial(slotId);
			out.serial(phrase);
			NetMngr.push(out);
		}
		else
			nlwarning(" unknown message name 'PHRASE:LEARN");
	}
}

// ***************************************************************************
void				CSPhraseManager::fullDelete(uint32 slot)
{
	// auto-forget all before.
	forgetAllThatUsePhrase(slot);

	// erase it.
	erasePhrase(slot);

	// send the erase to server
	sendDeleteToServer(slot);
}

// ***************************************************************************
uint				CSPhraseManager::countAllThatUsePhrase(uint32 slot)
{
	if(slot==0)
		return 0;

	// for all memory slot
	uint	count= 0;
	for(uint i=0;i<_Memories.size();i++)
	{
		for(uint j=0;j<PHRASE_MAX_MEMORY_SLOT;j++)
		{
			if( _Memories[i].Slot[j].isPhrase() &&
				_Memories[i].Slot[j].Id==slot )
			{
				count++;
			}
		}
	}

	return count;
}

// ***************************************************************************
void				CSPhraseManager::forgetAllThatUsePhrase(uint32 slot, bool sendMsgOnly)
{
	if(slot==0)
		return;

	bool someForgetedOnClient= false;

	// for all memory slot
	for(uint i=0;i<_Memories.size();i++)
	{
		for(uint j=0;j<PHRASE_MAX_MEMORY_SLOT;j++)
		{
			if( _Memories[i].Slot[j].isPhrase() &&
				_Memories[i].Slot[j].Id==slot )
			{
				// forget the phrase localy, if wanted
				if(!sendMsgOnly)
				{
					forgetPhrase(i, j);
					someForgetedOnClient= true;
				}

				// forget the phrase to server
				// TODO_DAVID_PHRASE_CLEAN: remove JUST the line below
				sendForgetToServer(i, j);
			}
		}
	}

	// easy: update all memory ctrl states, if some forgeted on client
	if(someForgetedOnClient)
		updateAllMemoryCtrlState();
}

// ***************************************************************************
void				CSPhraseManager::rememorizeAllThatUsePhrase(uint32 slot)
{
	bool someMemorized= false;

	// for all memory slot
	for(uint i=0;i<_Memories.size();i++)
	{
		for(uint j=0;j<PHRASE_MAX_MEMORY_SLOT;j++)
		{
			if( _Memories[i].Slot[j].isPhrase() &&
				_Memories[i].Slot[j].Id==slot )
			{
				// re-memorize the phrase, need server only
				// TODO_DAVID_PHRASE_CLEAN: remove JUST the line below
				sendMemorizeToServer(i, j, slot);

				// for updating client gray states
				someMemorized= true;
			}
		}
	}

	// easy: update all memory ctrl states, if some re-learned
	if(someMemorized)
		updateAllMemoryCtrlState();
}


// ***************************************************************************
CSheetId	getRightHandItem()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CSheetId	item;
	// get the RightHand bag index
	sint32	itemSlot= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:HAND:0:INDEX_IN_BAG")->getValue32();
	// if something in hand
	if(itemSlot>0)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:BAG:"+toString(itemSlot-1) +":SHEET", false);
		if(node)
			item= node->getValue32();
	}

	return item;
}

// ***************************************************************************
// retrieve the current Skill of the Item in RightHand
SKILLS::ESkills		getRightHandItemSkill()
{
	SKILLS::ESkills		itemSkill= SKILLS::unknown;

	CSheetId	item= getRightHandItem();

	if(item.asInt()!=0)
	{
		// get the ItemSheet.
		CItemSheet	*itemSheet= dynamic_cast<CItemSheet*>(SheetMngr.get(item));
		if(itemSheet)
			itemSkill= itemSheet->getRequiredSkill();
	}
	else
	{
		// Empty Hand => Hand skill
		itemSkill= SKILLS::SFMCAH;
	}
	return itemSkill;
}

// ***************************************************************************
// retrieve the current CraftToolType in RightHand
TOOL_TYPE::TCraftingToolType	getRightHandCraftToolType()
{
	TOOL_TYPE::TCraftingToolType	toolType= TOOL_TYPE::Unknown;

	CSheetId	item= getRightHandItem();

	if(item.asInt()!=0)
	{
		// get the ItemSheet.
		CItemSheet	*itemSheet= dynamic_cast<CItemSheet*>(SheetMngr.get(item));
		if(itemSheet && itemSheet->Family == ITEMFAMILY::CRAFTING_TOOL)
			toolType= itemSheet->Tool.CraftingToolType;
	}

	return toolType;
}

// ***************************************************************************
uint32	getRightHandEffectiveLevel()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CSkillManager		*pSM= CSkillManager::getInstance();

	uint32	effectiveLevel= 0;

	// **** get the right hand item skill value
	SKILLS::ESkills		rhSkill= getRightHandItemSkill();
	effectiveLevel= pSM->getSkillValueMaxBranch(rhSkill);

	// **** get the right hand item 'required skill'
	// get the RightHand bag index
	sint32	itemSlot= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:HAND:0:INDEX_IN_BAG")->getValue32();
	// if something in hand
	if(itemSlot>0)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:BAG:"+toString(itemSlot-1) +":QUALITY", false);
		if(node)
			// if the right hand item quality is less than our skill value, take it....
			effectiveLevel= min(effectiveLevel, (uint32)node->getValue32());
	}
	// else awlays take the bare hand skill level (above)

	return effectiveLevel;
}

// ***************************************************************************
static sint	getRightHandEnchantValue()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	sint ret= 0;
	// get the RightHand bag index
	sint32	itemSlot= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:HAND:0:INDEX_IN_BAG")->getValue32();
	// if something in hand
	if(itemSlot>0)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:BAG:"+toString(itemSlot-1) +":ENCHANT", false);
		if(node)
			ret= node->getValue32();
	}

	return ret;
}

// ***************************************************************************
void	CSPhraseManager::updateMemoryCtrlRegenTickRange(uint memorySlot, CDBCtrlSheet	*ctrl)
{
	//
	sint32	memoryLine;
	if (ctrl->isShortCut())
		memoryLine = getSelectedMemoryLineDB();
	else
		memoryLine = getSelectedMemoryAltLineDB();
	if (memoryLine < 0)
		return;
	sint32	phraseId= getMemorizedPhrase(memoryLine, memorySlot);
	//
	if(phraseId)
	{
		const CSPhraseCom &phrase = getPhrase(phraseId);
		CTickRange rtr = getRegenTickRange(phrase);
		if ((_RegenPhrases.find(phraseId) != _RegenPhrases.end()) && rtr.isEmpty() && !ctrl->getRegenTickRange().isEmpty())
		{
			ctrl->startNotifyAnim(); // anim to state that the phraseis available again
			_RegenPhrases.erase(_RegenPhrases.find(phraseId));
		}
		if (!rtr.isEmpty())
			_RegenPhrases.insert(phraseId);
		ctrl->setRegenTickRange(rtr);
	}
}

// ***************************************************************************
uint64 CSPhraseManager::getPhraseRequiredFlags(const CSPhraseCom &phrase)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	uint64 phraseFlags = 0;
	for(uint i=0;i<phrase.Bricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
		if(brick)
			phraseFlags|= brick->BrickRequiredFlags;
	}
	return phraseFlags;
}

// ***************************************************************************
CTickRange CSPhraseManager::getRegenTickRange(const CSPhraseCom &phrase) const
{
	CTickRange tickRange;
	if(!phrase.empty())
	{
		CSBrickManager	*pBM= CSBrickManager::getInstance();
		// Get the RootBrick of the Phrase
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[0]);
		if(brick)
		{
			if (brick->isSpecialPower())
			{
				uint64 phraseFlags = getPhraseRequiredFlags(phrase);				
				for(uint b = 0; b < 64; ++b)
				{
					if (phraseFlags & (uint64(1) << b))
					{
						CTickRange newTR = getRegenTickRange(b);
						if (!newTR.isEmpty())
						{
							if (tickRange.StartTick == tickRange.EndTick)
							{
								tickRange = newTR;
							}
							else
							{
								tickRange.extend(newTR);
							}
						}
					}
				}
			}
		}
	}
	return tickRange;
}


// ***************************************************************************
NLMISC::TGameCycle CSPhraseManager::getPowerDisableTime(const CSPhraseCom &phrase) const
{
	float regenTime = 0;
	if(!phrase.empty())
	{
		CSBrickManager	*pBM= CSBrickManager::getInstance();
		for (uint brickIndex = 0; brickIndex < phrase.Bricks.size(); ++brickIndex)
		{
			// Get the RootBrick of the Phrase
			CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[brickIndex]);
			if(brick)
			{
				for (uint prop = 0; prop < brick->Properties.size(); ++prop)
				{
					std::string::size_type endPos = brick->Properties[prop].Text.find(":");
					if (endPos != std::string::npos)
					{
						std::string propName = brick->Properties[prop].Text.substr(0, endPos);
						if (nlstricmp(propName, "SP_RECAST_TIME") == 0)
						{
							regenTime = std::max(regenTime, brick->Properties[prop].Value);
						}
					}
				}
			}
		}
	}
	return (NLMISC::TGameCycle) (regenTime * 10.f);
}


// ***************************************************************************
NLMISC::TGameCycle CSPhraseManager::getRegenTime(const CSPhraseCom &phrase) const
{
	CTickRange tickRange = getRegenTickRange(phrase);
	sint left = LastGameCycle - tickRange.StartTick;
	return (uint) std::max(0, left);
}

// ***************************************************************************
NLMISC::TGameCycle CSPhraseManager::getTotalRegenTime(const CSPhraseCom &phrase) const
{
	CTickRange tickRange = getRegenTickRange(phrase);
	return (tickRange.EndTick - tickRange.StartTick);
}


// ***************************************************************************
void	CSPhraseManager::updateMemoryCtrlState(uint memorySlot, CDBCtrlSheet	*ctrl, SKILLS::ESkills itemSkill)
{
	/*
		Update Gray State of Memory Ctrls.
		Can Execute only if skill of the phrase and skill of the selected Weapon are compatible.
		Eg: can execute a "FyrosAxe" phrase on a FyrosAxe Weapon (skills on same branch)
			can execute a "Combat" phrase on a FyrosAxe Weapon (skills on same branch)
			can't execute a "MatisAxe" phrase on a FyrosAxe Weapon (skills not on same branch)
		Additionaly, a Magic Phrase is always executable
	*/

	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	CSBrickManager		*pBM = CSBrickManager::getInstance();
	CSkillManager		*pSM = CSkillManager::getInstance();

	uint	memoryLine;
	// get the slot info
	if (ctrl->isShortCut()) // No memoryLine defined
		memoryLine= getSelectedMemoryLineDB();
	else
		memoryLine= getSelectedMemoryAltLineDB();
	bool	newIsMacro= isMemorizedMacro(memoryLine, memorySlot);
	sint32	macroId= getMemorizedMacro(memoryLine, memorySlot);
	sint32	phraseId= getMemorizedPhrase(memoryLine, memorySlot);
	CMemorySlot		*memSlot= NULL;
	if(memoryLine<_Memories.size() && memorySlot<PHRASE_MAX_MEMORY_SLOT)
		memSlot= &_Memories[memoryLine].Slot[memorySlot];

	// update the ctrl
	if(ctrl && (ctrl->isSPhraseId() || ctrl->isMacro()) )
	{
		// if the new ctrl is a phrase
		if(!newIsMacro)
		{
			// **** if ctrl is macro, change type
			if(ctrl->isMacro())
			{
				ctrl->setType(CCtrlSheetInfo::SheetType_SPhraseId);
				ctrl->setListMenuRight(PhraseMemoryPhraseMenu);
				ctrl->setActionOnLeftClick(PhraseMemoryPhraseAction);
			}

			// **** get the related ctrl skill
			SKILLS::ESkills		phraseRootSkill= SKILLS::unknown;
			bool				isGlobalMemPhrase= false;
			bool				isFaberPhrase= false;
			bool				isHarvestPhrase= false;
			bool				isCombatPhrase= false;
			bool				isProcEnchantmentPhrase= false;
			bool				isSpecialPowerPhrase= false;
			if(phraseId)
			{
				const CSPhraseCom	&phrase= getPhrase(phraseId);
				phraseRootSkill= getPhraseRootSkill(phrase.Bricks);
				if(!phrase.empty())
				{
					// Get the RootBrick of the Phrase
					CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[0]);
					if(brick)
					{
						// Magic or SpecialPower Root?
						isGlobalMemPhrase= brick->isMagic() || brick->isSpecialPower();
						isSpecialPowerPhrase= brick->isSpecialPower();
						isFaberPhrase= brick->isFaber();
						isHarvestPhrase= brick->isHarvest();
						isCombatPhrase= brick->isCombat();
						isProcEnchantmentPhrase= brick->isProcEnchantment();
					}
				}
			}


			// **** gray ctrl if not compatibles
			bool	valid= false;
			if( isGlobalMemPhrase || pSM->areSkillOnSameBranch(phraseRootSkill, itemSkill) )
				valid= true;

			// *** Combat: the item must be compatible with the phrase
			if( valid && isCombatPhrase && !skillCompatibleWithCombatPhrase(itemSkill, getPhrase(phraseId).Bricks) )
				valid= false;

			// *** Power: item enchant.
			if( valid && isSpecialPowerPhrase && !skillCompatibleWithSpecialPowerPhrase(itemSkill, getPhrase(phraseId).Bricks) )
				valid= false;

			// *** Harvet : not in indoor
			if (isHarvestPhrase && UserEntity->indoor())
				valid= false;

			// **** Faber special: valid only if good ToolType
			if(valid && itemSkill == SKILLS::SC && isFaberPhrase)
			{
				valid= false;

				TOOL_TYPE::TCraftingToolType	cttHand= getRightHandCraftToolType();
				// Get The FaberPlan
				const CSPhraseCom	&phrase= getPhrase(phraseId);
				if(!phrase.empty())
				{
					// Ok if the item built can be build with the tool in hand
					valid= pBM->getSabrinaCom().getPhraseFaberPlanToolType(phrase.Bricks) == cttHand;
				}
			}

			// **** Proc Enchantment: valid only if the item in right hand is enchanted
			if( isProcEnchantmentPhrase )
			{
				valid= false;
				sint	enchantValue= getRightHandEnchantValue();
				// dec because 0 == not enchanted, and 1 == enchanted but no more charge
				enchantValue--;
				// if some charge left
				if(enchantValue>=1)
					valid= true;
			}

			// *** check if need an item in hands
			if( valid && isCombatPhrase && !phraseUsableWithEmptyHands( getPhrase(phraseId).Bricks) )
			{
				if( getRightHandItem() == CSheetId::Unknown )
					valid= false;
			}


			// **** special: valid only according to BRICK_TICK_RANGE dbProp.
			if(valid)
			{
				uint64	currentFlags;
				if (isSpecialPowerPhrase)
				{
					currentFlags = ~(uint64(0));
					//Power Flag
					for (uint powerIndex = 32; powerIndex < 64; ++powerIndex)
					{
						if (!getRegenTickRange(powerIndex).isEmpty())
						{
							currentFlags &= ~(uint64(1) << powerIndex);
						}
					}
				}
				else
				{
					currentFlags = uint64(0);
					//CombatEvent Flag
					for (uint powerIndex = 0; powerIndex < 32; ++powerIndex)
					{
						if (!getRegenTickRange(powerIndex).isEmpty())
						{
							currentFlags |= uint64(1) << powerIndex;
						}
					}
				}

				// Build the phrase required bricks flags
				const CSPhraseCom	&phrase= getPhrase(phraseId);
				uint64	phraseFlags = getPhraseRequiredFlags(phrase);
				// if required flags are not present in database, invalid
				if( (currentFlags & phraseFlags) != phraseFlags )
				{
					valid= false;
				}
			}




			// **** valid only if the user is not equiping himself
			if(isExecutionInvalidCauseEquip())
			{
				valid= false;
			}

			// Check if the temporary inventory is opened
			if(isExecutionInvalidCauseTempInv())
			{
				valid= false;
			}

			// **** update state
			ctrl->setGrayed(!valid);
		}
		else
		{
			bool	neadReadMacro= false;
			nlassert(memSlot);

			// **** if ctrl is macro, change type
			if(ctrl->isSPhraseId())
			{
				ctrl->setType(CCtrlSheetInfo::SheetType_Macro);
				ctrl->setListMenuRight(PhraseMemoryMacroMenu);
				ctrl->setActionOnLeftClick(PhraseMemoryMacroAction);
				neadReadMacro= true;
			}

			// macro changed?
			if(ctrl->getMacroId() != macroId)
				neadReadMacro= true;

			// slot dirty?
			if(memSlot->IsMacroVisualDirty)
			{
				neadReadMacro= true;
				// clean
				memSlot->IsMacroVisualDirty= false;
			}

			// change macro
			if(neadReadMacro)
			{
				CMacroCmdManager	*pMM= CMacroCmdManager::getInstance();
				const CMacroCmd 	*macroCmd= pMM->getMacroFromMacroID(macroId);
				if(macroCmd)
					ctrl->readFromMacro(*macroCmd);
			}

			// **** set gray state
			ctrl->setGrayed(false);
		}
	}
	_RegenTickRangeTouched = true;
}


// ***************************************************************************
void	CSPhraseManager::updateAllMemoryCtrlState()
{
	TTicks startTime = CTime::getPerformanceTime();
	// get the item skill
	SKILLS::ESkills itemSkill= getRightHandItemSkill();

	// Update All Memory Ctrls
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	// For All the selected Memory Ctrls.
	for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
	{
		// Get the ctrl
		CDBCtrlSheet	*ctrl= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(PhraseMemoryCtrlBase + toString(i)) );
		if(ctrl)
		{
			// update the valid state.
			updateMemoryCtrlState(i, ctrl, itemSkill);
		}
		CDBCtrlSheet	*ctrlAlt= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(PhraseMemoryAltCtrlBase + toString(i)) );
		if(ctrlAlt)
			updateMemoryCtrlState(i, ctrlAlt, itemSkill);
	}
	TTicks endTime = CTime::getPerformanceTime();
	//nldebug("***** %d ms for CSPhraseManager::updateAllMemoryCtrlState", (int) (1000 * CTime::ticksToSecond(endTime - startTime)));	
}

// ***************************************************************************
void	CSPhraseManager::updateAllMemoryCtrlRegenTickRange()
{
	for(uint i=0;i<PHRASE_MAX_MEMORY_SLOT;i++)
	{
		updateMemoryCtrlRegenTickRange(i);
	}
}


// ***************************************************************************
CDBCtrlSheet	*CSPhraseManager::getMemorySlotCtrl(uint memorySlot)
{
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return NULL;

	// Get the ctrl
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	return dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(PhraseMemoryCtrlBase + toString(memorySlot)));	
}

// ***************************************************************************
CDBCtrlSheet	*CSPhraseManager::getMemoryAltSlotCtrl(uint memorySlot)
{
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return NULL;

	// Get the ctrl
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	return dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(PhraseMemoryAltCtrlBase + toString(memorySlot)));	
}

// ***************************************************************************
void	CSPhraseManager::updateMemoryCtrlState(uint memorySlot)
{	
	CDBCtrlSheet	*ctrl= getMemorySlotCtrl(memorySlot);
	CDBCtrlSheet	*ctrlAlt= getMemoryAltSlotCtrl(memorySlot);
	if(ctrl)
	{
		// update the valid state.
		updateMemoryCtrlState(memorySlot, ctrl, getRightHandItemSkill());
	}
	if(ctrlAlt)
	{
		// update the valid state.
		updateMemoryCtrlState(memorySlot, ctrlAlt, getRightHandItemSkill());
	}
}

// ***************************************************************************
void	CSPhraseManager::updateMemoryCtrlRegenTickRange(uint memorySlot)
{
	CDBCtrlSheet	*ctrl= getMemorySlotCtrl(memorySlot);
	CDBCtrlSheet	*ctrlAlt= getMemoryAltSlotCtrl(memorySlot);
	if(ctrl)
	{
		// update the valid state.
		updateMemoryCtrlRegenTickRange(memorySlot, ctrl);
	}
	if(ctrlAlt)
	{
		// update the valid state.
		updateMemoryCtrlRegenTickRange(memorySlot, ctrlAlt);
	}
}

// ***************************************************************************
SKILLS::ESkills		CSPhraseManager::getPhraseRootSkill(const std::vector<NLMISC::CSheetId> &phraseBricks) const
{
	if(phraseBricks.empty())
		return SKILLS::unknown;

	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*rootBrick= pBM->getBrick(phraseBricks[0]);
	if(!rootBrick)
		return SKILLS::unknown;
	else
		return rootBrick->getSkill();
}

// ***************************************************************************
bool		CSPhraseManager::skillCompatibleWithCombatPhrase(SKILLS::ESkills compareSkill, const std::vector<NLMISC::CSheetId> &phraseBricks) const
{
	// special case for hand to hand due to possible brick restriction
	if( compareSkill == SKILLS::SFMCAHM )
	{
		if( !phraseUsableWithEmptyHands(phraseBricks) )
		{
			return false;
		}
	}

	CSkillManager	*pSM= CSkillManager::getInstance();

	// **** Build the "phrase skill compatible" formula.
	CReqSkillFormula	phraseFormula= buildCombatPhraseSkillFormula(phraseBricks);

	// **** test if one of the ored skill match
	std::list<CReqSkillFormula::CSkillValueAnd>::iterator	it(phraseFormula.OrSkills.begin()),
		end(phraseFormula.OrSkills.end());
	for(;it!=end;it++)
	{
		CReqSkillFormula::CSkillValueAnd	skillAnd= *it;
		// if the and is not a single value, then no chance to match (eg: SFR&SFM won't match any compareSkill)
		if(skillAnd.AndSkills.size()==1)
		{
			// ok if the skill to compare is more restrictive (or equal) to this skill
			if(pSM->isSkillAncestor(skillAnd.AndSkills.begin()->Skill, compareSkill))
				return true;
		}
	}

	return false;
}

// ***************************************************************************
bool		CSPhraseManager::skillCompatibleWithSpecialPowerPhrase(SKILLS::ESkills compareSkill, const std::vector<NLMISC::CSheetId> &phraseBricks) const
{
	CSkillManager	*pSM= CSkillManager::getInstance();

	bool hasEnchantWeapon = false;
	vector<NLMISC::CSheetId>::const_iterator itBrick, itBrickEnd = phraseBricks.end();
	for (itBrick=phraseBricks.begin(); itBrick!=itBrickEnd; ++itBrick)
	{
		NLMISC::CSheetId const& brick = *itBrick;
		if (brick==_EnchantWeaponMainBrick)
			hasEnchantWeapon = true;
	}

	// If phrase enchants a weapon
	if (hasEnchantWeapon)
	{
		// check we have a melee weapon in hand
		if (pSM->isSkillAncestor(SKILLS::SFM, compareSkill) && compareSkill!=SKILLS::SFMCAH)
			return true;
		else
			return false;
	}

	return true;
}


// ***************************************************************************
bool		CSPhraseManager::phraseUsableWithEmptyHands(const std::vector<NLMISC::CSheetId> &phraseBricks) const
{
	vector<NLMISC::CSheetId>::const_iterator itBrick, itBrickEnd = phraseBricks.end();
	for (itBrick=phraseBricks.begin(); itBrick!=itBrickEnd; ++itBrick)
	{
		CSBrickSheet	*brick= CSBrickManager::getInstance()->getBrick(*itBrick);
		if(brick)
		{
			if( !brick->UsableWithEmptyHands )
				return false;
		}
	}
	return true;
}


// ***************************************************************************
// ***************************************************************************
// MACROS
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void	CSPhraseManager::memorizeMacro(uint32 memoryLine, uint32 memorySlot, uint32 macroId)
{
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return;

	// enlarge memory if needed
	if(memoryLine>=_Memories.size())
		_Memories.resize(memoryLine+1);

	// if the slot is a phrase
	if(_Memories[memoryLine].Slot[memorySlot].isPhrase())
	{
		// forget old phrase slot, both client and server
		forgetPhrase(memoryLine, memorySlot);
		sendForgetToServer(memoryLine, memorySlot);
	}

	// update memory
	_Memories[memoryLine].Slot[memorySlot].IsMacro= true;
	_Memories[memoryLine].Slot[memorySlot].Id= macroId;

	// must update DB?
	if((sint32)memoryLine==_SelectedMemoryDB)
	{
		// update the DB
		updateMemoryDBSlot(memorySlot);
		// update the ctrl state
		updateMemoryCtrlState(memorySlot);
	}
}

// ***************************************************************************
void	CSPhraseManager::forgetMacro(uint32 memoryLine, uint32 memorySlot)
{
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return;
	// if the slot eihter don't exist, abort
	if(memoryLine>=_Memories.size())
		return;
	// if the slot is not a macro
	if(!_Memories[memoryLine].Slot[memorySlot].isMacro())
		return;

	// update memory. NB: revert to Phrase mode is important
	_Memories[memoryLine].Slot[memorySlot].IsMacro= false;
	_Memories[memoryLine].Slot[memorySlot].Id= 0;

	// must update DB?
	if((sint32)memoryLine==_SelectedMemoryDB)
	{
		// update the db
		updateMemoryDBSlot(memorySlot);
		// update the ctrl state
		updateMemoryCtrlState(memorySlot);
	}
}

// ***************************************************************************
uint32	CSPhraseManager::getMemorizedMacro(uint32 memoryLine, uint32 memorySlot) const
{
	if(memoryLine>=_Memories.size())
		return 0;
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return 0;
	if(!_Memories[memoryLine].Slot[memorySlot].isMacro())
		return 0;

	return _Memories[memoryLine].Slot[memorySlot].Id;
}

// ***************************************************************************
bool	CSPhraseManager::isMemorizedMacro(uint32 memoryLine, uint32 memorySlot) const
{
	if(memoryLine>=_Memories.size())
		return 0;
	if(memorySlot>=PHRASE_MAX_MEMORY_SLOT)
		return 0;

	return _Memories[memoryLine].Slot[memorySlot].isMacro();
}

// ***************************************************************************
void	CSPhraseManager::updateMacroShortcuts(sint32 macroId)
{
	// dirt any macro visual
	for(uint i=0;i<_Memories.size();i++)
	{
		for(uint j=0;j<PHRASE_MAX_MEMORY_SLOT;j++)
		{
			CMemorySlot		&memSlot= _Memories[i].Slot[j];
			if(memSlot.isMacro() && memSlot.Id==(uint32)macroId)
				memSlot.IsMacroVisualDirty= true;
		}
	}

	// update the visual
	updateAllMemoryCtrlState();
}

// ***************************************************************************
void	CSPhraseManager::deleteMacroShortcuts(sint32 macroId)
{
	// forget any macro that use this slot
	for(uint i=0;i<_Memories.size();i++)
	{
		for(uint j=0;j<PHRASE_MAX_MEMORY_SLOT;j++)
		{
			CMemorySlot		&memSlot= _Memories[i].Slot[j];
			if(memSlot.isMacro() && memSlot.Id==(uint32)macroId)
				forgetMacro(i, j);
		}
	}
}


// ***************************************************************************
struct CMacroMemSerialSlot
{
	uint8	MemoryLine;
	uint8	MemorySlot;
	uint32	MacroId;

	void	serial(NLMISC::IStream &f)
	{
		f.serial(MemoryLine);
		f.serial(MemorySlot);
		f.serial(MacroId);
	}
};

void	CSPhraseManager::serialMacroMemory(NLMISC::IStream &f)
{
	// load
	if(f.isReading())
	{
		CMacroCmdManager	*pMM= CMacroCmdManager::getInstance();

		// read the list of slot
		vector<CMacroMemSerialSlot>		macroList;
		f.serialCont(macroList);

		// for each one, test if the macro really exist, then set
		for(uint i=0;i<macroList.size();i++)
		{
			CMacroMemSerialSlot		slot= macroList[i];
			if(pMM->getMacroFromMacroID(slot.MacroId))
				memorizeMacro(slot.MemoryLine, slot.MemorySlot, slot.MacroId);
		}
	}
	// write
	else
	{
		// create the list of macros that resides in the memory
		vector<CMacroMemSerialSlot>		macroList;

		// for all mems, test if macro, then append
		for(uint i=0;i<_Memories.size();i++)
		{
			for(uint j=0;j<PHRASE_MAX_MEMORY_SLOT;j++)
			{
				CMemorySlot		&memSlot= _Memories[i].Slot[j];
				if(memSlot.isMacro())
				{
					CMacroMemSerialSlot	slot;
					slot.MemoryLine= i;
					slot.MemorySlot= j;
					slot.MacroId= memSlot.Id;
					macroList.push_back(slot);
				}
			}
		}

		// then serial
		f.serialCont(macroList);
	}
}


// ***************************************************************************
// ***************************************************************************
// BOTCHAT
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
uint32			CSPhraseManager::getCurrentPhraseSheetPrice(uint32 phraseSheetId)
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	uint	price= 0;

	// get the SPhraseSheet
	CSPhraseSheet	*phraseSheet= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(CSheetId(phraseSheetId)));
	if(phraseSheet)
	{
		// Append the price of all bricks
		for(uint j=0;j<phraseSheet->Bricks.size();j++)
		{
			// add the price only if the brick is not already learned
			if(!pBM->isBrickKnown(phraseSheet->Bricks[j]))
			{
				CSBrickSheet	*brick= pBM->getBrick(phraseSheet->Bricks[j]);
				if(brick)
				{
					price+= brick->SPCost;
				}
			}
		}
	}

	return price;
}

// ***************************************************************************
uint32				CSPhraseManager::getBasePhraseSheetPrice(uint32 phraseSheetId)
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	uint	price= 0;

	// get the SPhraseSheet
	CSPhraseSheet	*phraseSheet= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(CSheetId(phraseSheetId)));
	if(phraseSheet)
	{
		// Append the price of all bricks
		for(uint j=0;j<phraseSheet->Bricks.size();j++)
		{
			// add the price either if the brick is not already learned
			CSBrickSheet	*brick= pBM->getBrick(phraseSheet->Bricks[j]);
			if(brick)
			{
				price+= brick->SPCost;
			}
		}
	}

	return price;
}

// ***************************************************************************
void			CSPhraseManager::updateBotChatPhrasePrice(uint start, uint end)
{
	end= min(end, (uint)PHRASE_MAX_BOTCHAT_SLOT);
	start= min(start, end);

	// For all phrase to test
	for(uint i=start;i<end;i++)
	{
		if(_BotChatPhraseSheetLeaves[i] && _BotChatPhrasePriceLeaves[i])
		{
			uint	price= getCurrentPhraseSheetPrice(_BotChatPhraseSheetLeaves[i]->getValue32());

			// set the price
			_BotChatPhrasePriceLeaves[i]->setValue32(price);
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// PROGRESSION
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void				CSPhraseManager::updatePhraseProgressionDB()
{
	// If DB not inited, no-op
	if(!_InitInGameDone)
		return;

	CSBrickManager		*pBM= CSBrickManager::getInstance();

	// Fill All the progression db.
	uint	numProgressFill= 0;
	if(_BookSkillFitler!=SKILLS::unknown)
		numProgressFill= (uint)_ProgressionPhrases[_BookSkillFitler].Phrases.size();
	uint	i;
	uint	progressIndex[NumProgressType];
	for(i=0;i<NumProgressType;i++)
	{
		progressIndex[i]= 0;
	}

	// for all phrases to fill
	for(i=0;i<numProgressFill;i++)
	{
		CPhraseProgressInfo		&ppi= _ProgressionPhrases[_BookSkillFitler].Phrases[i];

		// choose the progression where to write to
		uint	progressType;
		if(isPhraseCastable(ppi.SheetId))
			progressType= ActionProgress;
		else
			progressType= UpgradeProgress;
		uint	&dbIndex= progressIndex[progressType];


		// if no more place in db, skip this phrase
		if(dbIndex>=PHRASE_MAX_PROGRESSION_SLOT)
		{
			continue;
		}


		// is this phrase known?
		bool	known= true;
		CSPhraseSheet	*phrase= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(NLMISC::CSheetId(ppi.SheetId)));
		if(phrase)
		{
			// if only one brick is not known, the phrase is not known
			for(uint j=0;j<phrase->Bricks.size();j++)
			{
				if(!pBM->isBrickKnown(phrase->Bricks[j]))
				{
					known= false;
					break;
				}
			}
		}


		// if show, but only if full learnt, skip it if not fully learnt
		if(phrase->ShowInAPOnlyIfLearnt && !known)
		{
			continue;
		}


		// fill with the phrase id
		_ProgressionDbSheets[progressType][dbIndex]->setValue32(ppi.SheetId);
		// fill the level info
		_ProgressionDbLevels[progressType][dbIndex]->setValue32(ppi.Level);

		// if known, ungray
		if(known)
			_ProgressionDbLocks[progressType][dbIndex]->setValue32(0);
		else
		{
			// just gray if all requirement are met in order to learn it
			if(phraseRequirementOk(ppi.SheetId))
				_ProgressionDbLocks[progressType][dbIndex]->setValue32(1);
			// else redify
			else
				_ProgressionDbLocks[progressType][dbIndex]->setValue32(2);
		}

		// increment the db index
		dbIndex++;
	}


	// for each progression type
	for(uint pt= 0;pt<NumProgressType;pt++)
	{
		// reset old no more used to empty
		for(i=progressIndex[pt];i<_LastProgressionNumDbFill[pt];i++)
		{
			_ProgressionDbSheets[pt][i]->setValue32(0);
		}
		// update cache
		_LastProgressionNumDbFill[pt]= progressIndex[pt];
	}

}

// ***************************************************************************
class CPhraseSortEntry
{
public:
	CSPhraseManager::CPhraseProgressInfo	ProgressInfo;
	// Key sort
	uint32		Section;
	bool		Castable;
	uint32		Type;
	uint32		Icon;
	ucstring	Text;

	bool	operator<(const CPhraseSortEntry &pse) const
	{
		// sort by Section
		if(Section!=pse.Section)
			return Section<pse.Section;
		// take first castable (invert sens)
		if(Castable!=pse.Castable)
			return Castable>pse.Castable;
		// sort by type
		if(Type!=pse.Type)
			return Type<pse.Type;
		// sort by Icon (same type possible for effect bricks)
		if(Icon!=pse.Icon)
			return Icon<pse.Icon;
		// sort by text cost for same spell
		return Text<pse.Text;
	}
};

// ***************************************************************************
void				CSPhraseManager::computePhraseProgression()
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSkillManager	*pSM= CSkillManager::getInstance();

	// progression
	sint							skillReqLevel[SKILLS::NUM_SKILLS];
	std::vector<SKILLS::ESkills>	skillsToInsert;
	skillsToInsert.reserve(SKILLS::NUM_SKILLS);
	memset(skillReqLevel, 0xFF, SKILLS::NUM_SKILLS*sizeof(sint));

	// **** For each Phrase Sheet, compute its related progression
	CSheetManager::TEntitySheetMap::const_iterator	it;
	for(it=SheetMngr.getSheets().begin();it!=SheetMngr.getSheets().end();it++)
	{
		if(it->second.EntitySheet->Type==CEntitySheet::SPHRASE)
		{
			const CSPhraseSheet	*phrase= safe_cast<const CSPhraseSheet*>(it->second.EntitySheet);

#ifdef	PHRASE_DEBUG_VERBOSE
			static uint	pscount= 0;
			nlinfo("**** PROG PHRASE: %4d, %s", pscount++, phrase->Id.toString().c_str());
#endif

			// if not shown in ActionProgression window, skip it
			if(!phrase->ShowInActionProgression)
				continue;

			// if not valid, skip it
			if(!phrase->isValid())
				continue;

			// **** for this phrase, build the required formula, with and without credits
			CReqSkillFormula	skillFormulaProg;	// For progression: No Credits
			CReqSkillFormula	skillFormulaFull;	// For info: full, With Credits
			// For all bricks
			uint	i;
			for(i=0;i<phrase->Bricks.size();i++)
			{
				CSBrickSheet	*brick= pBM->getBrick(phrase->Bricks[i]);
				if(brick)
				{
					// the brick formula is an OR of all skill required
					CReqSkillFormula	brickFormula;

					// get all its required Skill
					for(uint j=0;j<brick->RequiredSkills.size();j++)
					{
						CSkillValue		sv;
						sv.Skill= brick->RequiredSkills[j].Skill;
						sv.Value= brick->RequiredSkills[j].Value;
						brickFormula.orV(sv);
					}

					// if not empty
					if(!brickFormula.empty())
					{
						// AND with phrase requirement formula
						skillFormulaFull.andV(brickFormula);
						// if the brick is not a credit
						if(!brick->isCredit())
							skillFormulaProg.andV(brickFormula);
					}
				}
			}

			/* if the skillFormulaProg is empty(), and if the full formula is not, it is because
				this phrase is a Credit upgrade!!! we DO WANT see it in the skill tree

				Additionnaly, for special case of "after critical hit 3" for instance,
				where the credit is more restrictive than the options, take the full formula instead.
				Detect this case simply if the full formula is made of only ONE SkillValue
			*/
			if(skillFormulaProg.empty() || skillFormulaFull.singleValue())
				skillFormulaProg= skillFormulaFull;

#ifdef	PHRASE_DEBUG_VERBOSE
			skillFormulaFull.log("FULL");
			skillFormulaProg.log("PROG");
#endif


			// **** For phrase info, store the Full formula
			_PhraseReqSkillMap[phrase->Id.asInt()]= skillFormulaFull;


			// **** For phrase progression filter
			skillsToInsert.clear();
			// parse all the required skill, both AND and OR
			std::list<CReqSkillFormula::CSkillValueAnd>::const_iterator		itSVA;
			for(itSVA= skillFormulaProg.OrSkills.begin();itSVA!=skillFormulaProg.OrSkills.end();itSVA++)
			{
				std::list<CSkillValue>::const_iterator		itSV;
				for(itSV= itSVA->AndSkills.begin();itSV!=itSVA->AndSkills.end();itSV++)
				{
					const CSkillValue	&sv= *itSV;
					if(sv.Skill!=SKILLS::unknown)
					{
						// get the actual Skill associated to the skillValue (with no sheet data error,
						// it should be == sv.Skill).
						SKILLS::ESkills		skill= sv.Skill;
						uint32				value= sv.Value;
						// eg (SME,10) is incorrect (SME range is [20,50]) => take instead (SM,10)
						while(value<pSM->getMinSkillValue(skill))
						{
							SKILLS::ESkills		sp= pSM->getParent(skill);
							if(sp==SKILLS::unknown)
								break;
							else
								skill= sp;
						}

						/* in case of (SM,25) (eg arise with auras), it means that for example,
							(SMA,25), or (SMB,25) or (SMC,25) are required.
							Then we have to add this progression in all those skills!
							=> insert in those skills only!
						*/
						insertProgressionSkillRecurs(skill, value, skillReqLevel, skillsToInsert);
					}
				}
			}

			// then parse the skill to insert list
			for(i=0;i<skillsToInsert.size();i++)
			{
				SKILLS::ESkills		skill= skillsToInsert[i];

				// append this phrase to this progression line
				CPhraseProgressInfo	ppi;
				ppi.SheetId= phrase->Id.asInt();
				ppi.Level= skillReqLevel[skill];
				_ProgressionPhrases[skill].Phrases.push_back(ppi);

				// clean the level, for next phrase
				skillReqLevel[skill]= -1;
			}
		}
	}

	// **** Sort each phrase sheet by type/cost
	// For all skills
	std::vector<CPhraseSortEntry>	phraseSortEntry;
	for(uint i=0;i<SKILLS::NUM_SKILLS;i++)
	{
		CPhraseProgression	&pprog= _ProgressionPhrases[i];
		uint	j;

		// for all phrase of this skills, fill sort struct
		phraseSortEntry.resize(pprog.Phrases.size());
		for(j=0;j<phraseSortEntry.size();j++)
		{
			CPhraseSortEntry	&pse= phraseSortEntry[j];
			pse.ProgressInfo= pprog.Phrases[j];

			// get the phrase best display brick
			CSPhraseSheet		*phraseSheet= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(CSheetId(pse.ProgressInfo.SheetId)));
			NLMISC::CSheetId	brickSheet;
			if(phraseSheet)
				brickSheet= pBM->getSabrinaCom().getPhraseBestDisplayBrick(phraseSheet->Bricks);

			// take first the castable phrase
			pse.Castable= isPhraseCastable(phraseSheet);

			// sort by section
			pse.Section= getPhraseSectionFromLevel(pse.ProgressInfo.Level);

			// get the family of this brick as second sort key
			CSBrickSheet	*brick= pBM->getBrick(brickSheet);
			if(brick)
				pse.Type= brick->BrickFamily;
			else
				pse.Type= 0;

			// get the icon of this brick as third sort key
			if(brick)
				pse.Icon= (uint32)brick->IdIcon;
			else
				pse.Icon= 0;

			// get the phrase Text as 4th sort key
			pse.Text= STRING_MANAGER::CStringManagerClient::getSPhraseLocalizedName(CSheetId(pse.ProgressInfo.SheetId));
			// avoid mutliple sapce problem
			strFindReplace(pse.Text, " ", "");
			// replace each number with 001 format. strlwr
			for(uint k=0;k<pse.Text.size();k++)
			{
				if(pse.Text[k] < 256 && isalpha(pse.Text[k]))
					pse.Text[k]= tolower(pse.Text[k]);
				else if(pse.Text[k] < 256 && isdigit(pse.Text[k]))
				{
					uint32	number= 0;
					uint32	start= k;
					// get the whole number
					for(;k<pse.Text.size() && isdigit(pse.Text[k]);k++)
					{
						number*=10;
						number+= pse.Text[k] - '0';
					}
					// format, and replace in string
					ucstring	newNumber= toString("%3d", number);
					pse.Text.replace(start, k-start, newNumber);
					// and skip this number
					k= start + (uint)newNumber.size();
				}
			}
		}

		// sort
		sort(phraseSortEntry.begin(), phraseSortEntry.end());

		// write result
		for(j=0;j<phraseSortEntry.size();j++)
		{
			pprog.Phrases[j]= phraseSortEntry[j].ProgressInfo;
		}
	}

	// **** append a callback to update the progression when a brick is learned
	pBM->appendBrickLearnedCallback(&_ProgressionUpdate);
	pSM->appendSkillChangeCallback(&_ProgressionUpdate);
}

// ***************************************************************************
enum	{MaxSkillValue= 250};
void				CSPhraseManager::insertProgressionSkillRecurs(SKILLS::ESkills skill, uint32 value, sint *skillReqLevel, std::vector<SKILLS::ESkills>	&skillsToInsert)
{
	CSkillManager	*pSM= CSkillManager::getInstance();
	const std::vector<SKILLS::ESkills> &children= pSM->getChildren(skill);

	// Yoyo: special case for Skill value >250  (disabled bricks)....
	// Don't insert them
	if(value>MaxSkillValue)
		return;

	// if the value is under the max value of this skill, it's ok
	// Or special case for the MAX 250 value of skills, ie if the skill has no more children
	if(value < pSM->getMaxSkillValue(skill) || children.empty())
	{
		// if not used, insert this skill in the "skill used in this phrase progression"
		if(skillReqLevel[skill]==-1)
		{
			skillReqLevel[skill]= value;
			skillsToInsert.push_back(skill);
		}
		// else maximize the level required for this phrase
		else
		{
			skillReqLevel[skill]= max(skillReqLevel[skill], (sint)value);
		}
	}
	// else recurs to children
	else
	{
		for(uint i=0;i<children.size();i++)
		{
			insertProgressionSkillRecurs(children[i], value, skillReqLevel, skillsToInsert);
		}
	}
}

// ***************************************************************************
uint32				CSPhraseManager::getPhraseSectionFromLevel(uint32 level)
{
	uint32	ret= level/5;
	ret= min(ret, (uint32)((MaxSkillValue-1)/5));
	return ret;
}

// ***************************************************************************
void				CSPhraseManager::getPhraseLevelFromSection(uint32 section, uint32 &minLevel, uint32 &maxLevel)
{
	minLevel= section*5;
	maxLevel= (section+1)*5-1;
	if(maxLevel==MaxSkillValue-1)
		maxLevel= MaxSkillValue;
}

// ***************************************************************************
void				CSPhraseManager::buildPhraseBrickRequirement(uint32 phraseSheetId, std::vector<CSheetId> &bricks)
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();
	bricks.clear();

	CSPhraseSheet	*phraseSheet= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(CSheetId(phraseSheetId)));

	if(phraseSheet)
	{
		uint	i;

		// first get all bricks contained in this phrase
		set<CSheetId>	brickSet;
		for(i=0;i<phraseSheet->Bricks.size();i++)
		{
			CSBrickSheet	*brick= pBM->getBrick(phraseSheet->Bricks[i]);
			if(brick)
				brickSet.insert(brick->Id);
		}

		// For all bricks of the phrase.
		for(i=0;i<phraseSheet->Bricks.size();i++)
		{
			CSBrickSheet	*brick= pBM->getBrick(phraseSheet->Bricks[i]);
			if(brick)
			{
				// Special case: For "Use Item enchantment" phrase, don't describe requirement
				if(brick->isProcEnchantment())
				{
					bricks.clear();
					return;
				}

				// For all its required bricks
				for(uint j=0;j<brick->RequiredBricks.size();j++)
				{
					/* if the required brick is not in the current phrase, then it is required
						E.g: if a phrase has brickA and brickB, and if brickB require brickA
						then learning the phrase finally don't require brickA! (else won't be possible to learn it
						if this is the only phrase that offers brickA for instance)
					*/
					if( brickSet.find(brick->RequiredBricks[j])==brickSet.end() )
					{
						bricks.push_back(brick->RequiredBricks[j]);
					}
				}
			}
		}
	}
}

// ***************************************************************************
bool				CSPhraseManager::fameRequirementOk(uint32 phraseSheetId)
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	CSPhraseSheet	*phraseSheet= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(CSheetId(phraseSheetId)));

	if(phraseSheet)
	{
		uint	i;

		// first get all bricks contained in this phrase
		set<CSheetId>	brickSet;
		for(i=0;i<phraseSheet->Bricks.size();i++)
		{
			CSBrickSheet	*brick= pBM->getBrick(phraseSheet->Bricks[i]);
			if(brick)
				brickSet.insert(brick->Id);
		}

		// For all bricks of the phrase.
		for(i=0;i<phraseSheet->Bricks.size();i++)
		{
			CSBrickSheet	*brick= pBM->getBrick(phraseSheet->Bricks[i]);
			if(brick)
			{
				// get player fame for this faction and check with min value needed
				if( brick->FactionIndex != CStaticFames::INVALID_FACTION_INDEX )
				{

					//TODO

				}
			}
		}
		return true;
	}

	return false;
}

// ***************************************************************************
bool				CSPhraseManager::phraseRequirementOk(uint32 phraseSheetId)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();


	// **** check skill requirement for this phrase
	TPhraseReqSkillMap::const_iterator	it= _PhraseReqSkillMap.find(phraseSheetId);
	if(it!=_PhraseReqSkillMap.end())
	{
		const CReqSkillFormula	&formula= it->second;

		if(!formula.evaluate())
			return false;
	}


	// **** check fame requirement
	if( !fameRequirementOk(phraseSheetId) )
	{
		return false;
	}


	// **** check brick requirement
	std::vector<CSheetId>	bricks;
	buildPhraseBrickRequirement(phraseSheetId, bricks);

	// For all required bricks
	for(uint i=0;i<bricks.size();i++)
	{
		// if the required brick is not learned, fail
		if( !pBM->isBrickKnown(bricks[i]) )
			return false;
	}


	// ok all requirement met
	return true;
}


// ***************************************************************************
bool				CSPhraseManager::allowListBrickInHelp(const CSPhraseCom &phrase) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
	// broken phrase
	if(!rootBrick)
		return false;

	// don't list bricks for Charac upgrades
	if(BRICK_FAMILIES::isCharacBuyFamily(rootBrick->BrickFamily))
		return false;

	// don't list bricks for "use item enchant" phrase
	if(rootBrick->isProcEnchantment())
		return false;

	// common case
	return true;
}

// ***************************************************************************
void				CSPhraseManager::getCombatWeaponRestriction(ucstring &text, const CSPhraseCom &phrase, bool& usableWithMelee, bool& usableWithRange)
{
	text.clear();

	if(phrase.empty())
		return;

	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
	if(rootBrick && rootBrick->isCombat())
	{
		// **** Build the "phrase skill compatible" formula.
		CReqSkillFormula	phraseFormula= buildCombatPhraseSkillFormula(phrase.Bricks);

		// **** build the dorted array of ored skills
		std::vector<SKILLS::ESkills>	skills;
		std::list<CReqSkillFormula::CSkillValueAnd>::iterator	it(phraseFormula.OrSkills.begin()),
			end(phraseFormula.OrSkills.end());
		for(;it!=end;it++)
		{
			CReqSkillFormula::CSkillValueAnd	skillAnd= *it;
			// if the and is not a single value, then no chance to match (eg: SFR&SFM won't match any compareSkill)
			if(skillAnd.AndSkills.size()==1)
			{
				SKILLS::ESkills		skill= skillAnd.AndSkills.begin()->Skill;
				if(skill!=SKILLS::unknown)
					skills.push_back(skill);
			}
		}
		// sort by skill index. this is the convention, to be sure that we don't search "uiawrSFM_SFR"
		// while "uiawrSFR_SFM" is defined in en.uxt....
		std::sort(skills.begin(), skills.end());

		// used to know what weapon's family can be used with this phrase
		usableWithMelee = false;
		usableWithRange = false;

		// **** build the CI18N name
		std::string		idName= "uiawr";
		for(uint i=0;i<skills.size();i++)
		{
			string skillName = SKILLS::toString(skills[i]);
			idName+= skillName;
			if(i<skills.size()-1)
				idName+= "_";

			if( skillName.find("SFM") != std::string::npos )
				usableWithMelee = true;
			if( skillName.find("SFR") != std::string::npos )
				usableWithRange = true;
		}

		// no restriction at all means everything possible
		if( !usableWithMelee && !usableWithRange )
		{
			usableWithMelee = true;
			usableWithRange = true;
		}

		// **** If found with this combination version
		if(CI18N::hasTranslation(idName))
			text= CI18N::get(idName);
		// The combination don't exist. just list each skill
		else
		{
			for(uint i=0;i<skills.size();i++)
			{
				text+= CI18N::get("uiawr" + SKILLS::toString(skills[i]));
				if(i<skills.size()-1)
					text+= CI18N::get("uiPhraseWRSeparator");
			}
		}
	}
}

// ***************************************************************************
void				CSPhraseManager::getCombatWeaponRestriction(ucstring &text, sint32 phraseSheetId, bool& usableWithMelee, bool& usableWithRange)
{
	CSPhraseCom	phrase;
	buildPhraseFromSheet(phrase, phraseSheetId);
	getCombatWeaponRestriction(text, phrase, usableWithMelee, usableWithRange);
}


// ***************************************************************************
CReqSkillFormula	CSPhraseManager::buildCombatPhraseSkillFormula(const std::vector<NLMISC::CSheetId> &phraseBricks) const
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();
	CReqSkillFormula	phraseFormula;

	// default: work with any Skill Fight weapon (ie all... :) )
	phraseFormula.andV(CSkillValue(SKILLS::SF));

	// for all bricks not Unknown
	for(uint i=0;i<phraseBricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phraseBricks[i]);
		if(brick && brick->getSkill()!=SKILLS::unknown)
		{
			CReqSkillFormula	brickFormula;
			for(uint j=0;j<brick->UsedSkills.size();j++)
			{
				brickFormula.orV(CSkillValue(brick->UsedSkills[j]));
			}

			// and with the phraseFormula
			phraseFormula.andV(brickFormula);
		}
	}

	return phraseFormula;
}

// ***************************************************************************
uint32		CSPhraseManager::getSpellLevel(const std::vector<NLMISC::CSheetId> &phraseBricks) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	sint32	maxCost= 0;
	for(uint i=0;i<phraseBricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phraseBricks[i]);
		if(brick && !brick->isCredit())
		{
			maxCost= max(maxCost, brick->SabrinaCost);
		}
	}

	return maxCost;
}

// ***************************************************************************
void		CSPhraseManager::getResistMagic(bool resistMagic[RESISTANCE_TYPE::NB_RESISTANCE_TYPE], const std::vector<NLMISC::CSheetId> &phraseBricks)
{
	// reset
	for(uint i=0;i<RESISTANCE_TYPE::NB_RESISTANCE_TYPE;i++)
	{
		resistMagic[i]= false;
	}

	// set
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	for(uint i=0;i<phraseBricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phraseBricks[i]);
		if(brick && brick->MagicResistType!=RESISTANCE_TYPE::None)
		{
			resistMagic[brick->MagicResistType]= true;
		}
	}
}

// ***************************************************************************
uint32		CSPhraseManager::getMemorizedPhraseIfLastOrNewSlot(uint32 memoryLine, uint32 memoryIndex)
{
	// \toto yoyo: There is a Network BUG which prevents us from simply doing {Forget(),Delete()} Old,
	// then {Learn(),Memorize()}  new (Messages are shuffled).

	// If the memory slot has a Phrase
	uint32	oldPhraseId= getMemorizedPhrase(memoryLine, memoryIndex);
	if(oldPhraseId)
	{
		// if this is the last referenced, then return this phrase id
		if(countAllThatUsePhrase(oldPhraseId)==1)
		{
			return oldPhraseId;
		}
	}

	// else get New Free Slot
	return allocatePhraseSlot();
}

// ***************************************************************************
void		CSPhraseManager::fullDeletePhraseIfLast(uint32 memoryLine, uint32 memorySlot)
{
	uint32	phraseId= getMemorizedPhrase(memoryLine, memorySlot);
	// not a phrase? no-op
	if(!phraseId)
		return;

	// more than 1 use this slot? => no problem (will be forgeted/replaced)
	if(countAllThatUsePhrase(phraseId)>1)
		return;

	// else delete this phrase (server etc....)
	fullDelete(phraseId);
}


// ***************************************************************************
ucstring	CSPhraseManager::getForageExtractionPhraseEcotypeFmt(const CSPhraseCom &phrase)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	static	string	tspecString= "FG_ECT_SPC: ";

	// Test all brick, if find a ecotype specialisation, return fmt
	for(uint i=0;i<phrase.Bricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
		// if a forage extraction brick
		if(brick)
		{
			// Parse all properties of the brick, if FG_ECT_SPC, this is a terrain speciliasing
			for(uint j=0;j<brick->Properties.size();j++)
			{
				const string	&text= brick->Properties[j].Text;
				if(text.compare(0, tspecString.size(), tspecString)==0)
				{
					// extract the  associated terrain
					string	terran= text.substr(tspecString.size(), string::npos);
					terran= string("uihelpPhraseForageSuccess")+terran;
					// try to get the associated I18N
					if(CI18N::hasTranslation(terran))
						return CI18N::get(terran);
				}
			}
		}
	}

	// Failure, display all
	return CI18N::get("uihelpPhraseForageSuccessAll");
}

// ***************************************************************************
bool		CSPhraseManager::findDefaultAttack(uint32 &memoryLine, uint32 &memorySlot)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	if(_Memories.empty())
		return false;

	// **** Search in all memory lines, but try the currently selected one
	std::deque<uint>	memoryLineSort;
	memoryLineSort.resize(_Memories.size());
	uint32	i;
	for(i=0;i<memoryLineSort.size();i++)
		memoryLineSort[i]= i;
	// remove and push_front the selected memory line
	if(_SelectedMemoryDB>=0 && _SelectedMemoryDB<(sint)memoryLineSort.size())
	{
		memoryLineSort.erase(memoryLineSort.begin()+_SelectedMemoryDB);
		memoryLineSort.push_front(_SelectedMemoryDB);
	}

	// **** Parse all memories
	for(i=0;i<memoryLineSort.size();i++)
	{
		CMemoryLine		&memLine= _Memories[memoryLineSort[i]];
		for(uint j=0;j<PHRASE_MAX_MEMORY_SLOT;j++)
		{
			if(memLine.Slot[j].isPhrase())
			{
				const CSPhraseCom &phrase= getPhrase(memLine.Slot[j].Id);
				// The phrase must have only one brick: the default attack brick
				if( phrase.Bricks.size() == 1 )
				{
					CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[0]);
					// combat root brick?
					if( brick && brick->isRoot() && brick->isCombat() )
					{
						// default attack!
						memoryLine= memoryLineSort[i];
						memorySlot= j;
						return true;
					}
				}
			}
		}
	}

	return false;
}

// ***************************************************************************
bool		CSPhraseManager::avoidCyclicForPhrase(const CSPhraseCom &phrase) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	for(uint i=0;i<phrase.Bricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
		if(brick && brick->AvoidCyclic)
			return true;
	}

	return false;
}

// ***************************************************************************
bool		CSPhraseManager::avoidCyclicForPhrase(sint32 phraseSheetId) const
{
	return avoidCyclicForPhrase(getPhrase(phraseSheetId));
}


// ***************************************************************************
uint		CSPhraseManager::fillRoleMasterGenericPhrase(uint32 rmFlags, uint32 rmRace)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	uint	destIndex= 0;
	uint	numFill= 0;

	// not initialized => abort
	if(_BotChatPhraseSheetLeaves.empty())
		return 0;

	// *** For all phrase sheets
	CSheetManager::TEntitySheetMap::const_iterator	it;
	for(it=SheetMngr.getSheets().begin();it!=SheetMngr.getSheets().end();it++)
	{
		if(it->second.EntitySheet->Type==CEntitySheet::SPHRASE)
		{
			// *** Filter RoleMaster and basics
			const CSPhraseSheet	*phrase= safe_cast<const CSPhraseSheet*>(it->second.EntitySheet);
			// Not valid phrase are not sellable (some brick no more exist)
			if(!phrase->isValid())
				continue;
			// ShowInAPOnlyIfLearnt are typically special mission phrase, skip them
			if(phrase->ShowInAPOnlyIfLearnt)
				continue;
			// test rmFlags against phrase name
			if( !ROLEMASTER_FLAGS::canSellPhrase(rmFlags, phrase->Id.toString()) )
				continue;
			// test race (test all bricks)
			bool	raceOk= true;
			uint	i;
			for(i=0;i<phrase->Bricks.size();i++)
			{
				CSBrickSheet	*brick= pBM->getBrick(phrase->Bricks[i]);
				// if the brick exist, not a Common (no restriction), and not equal to rolemaster race,
				// then can't sell this phrase
				if( brick && brick->CivRestriction!=EGSPD::CPeople::Common &&
					brick->CivRestriction!=(EGSPD::CPeople::TPeople)rmRace )
				{
					raceOk= false;
					break;
				}
			}
			// if some brick failed, skip
			if(!raceOk)
				continue;


			// *** Filter not fully known phrases
			bool	fullKnown= true;
			for(i=0;i<phrase->Bricks.size();i++)
			{
				CSBrickSheet	*brick= pBM->getBrick(phrase->Bricks[i]);
				// if the brick exist, and not known then OK, all are not known
				if(brick && !pBM->isBrickKnown(phrase->Bricks[i]))
				{
					fullKnown= false;
					break;
				}
			}
			// if all bricks of this phrase are known, then don't need to sell!
			if(fullKnown)
				continue;


			// *** Filter phrases that are ok with requirement
			if(!phraseRequirementOk(phrase->Id.asInt()))
				continue;


			// *** All filter pass, add this phrase
			if(_BotChatPhraseSheetLeaves[destIndex])
				_BotChatPhraseSheetLeaves[destIndex]->setValue32(phrase->Id.asInt());

			// next
			destIndex++;
			// full DB filled?
			if(destIndex==_BotChatPhraseSheetLeaves.size())
				break;
		}
	}

	// nb really filled
	numFill= destIndex;

	// *** Fill others with 0!
	for(;destIndex<_BotChatPhraseSheetLeaves.size();destIndex++)
	{
		if(_BotChatPhraseSheetLeaves[destIndex])
			_BotChatPhraseSheetLeaves[destIndex]->setValue32(0);
	}

	return numFill;
}


// ***************************************************************************
void		CSPhraseManager::getPhraseSectionBoundFromSkillFilter(sint &minSectionId, sint &maxSectionId)
{
	CSkillManager	*pSM= CSkillManager::getInstance();
	SKILLS::ESkills	filter= getBookSkillFilter();
	// get its skill bound (eg: 20-50)
	sint	minSkillValue= pSM->getMinSkillValue(filter);
	sint	maxSkillValue= pSM->getMaxSkillValue(filter);
	minSkillValue= max(minSkillValue, 0);
	maxSkillValue= max(maxSkillValue, 0);
	// get its related section id bound (eg: 3-10)
	minSectionId= minSkillValue/5 -1;
	// NB: 50/5==10 is already exclusive (I mean all actions of DefensiveMagic Skill are 49 max)
	maxSectionId= maxSkillValue/5;
}

// ***************************************************************************
sint32 CSPhraseManager::getSheetFromPhrase(const CSPhraseCom &phrase) const
{		
	TPhraseToSheet::const_iterator it = _PhraseToSheet.find(phrase);		
	if (it == _PhraseToSheet.end()) return 0;
	return it->second;	
}

// ***************************************************************************
uint32 CSPhraseManager::getTotalActionMalus(const CSPhraseCom &phrase) const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	uint32	totalActionMalus= 0;
	CCDBNodeLeaf *actMalus = _TotalMalusEquipLeaf ? &*_TotalMalusEquipLeaf
		: &*(_TotalMalusEquipLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:TOTAL_MALUS_EQUIP", false));
	// root brick must not be Power or aura, because Action malus don't apply to them
	// (ie leave 0 ActionMalus for Aura or Powers
	if (!phrase.Bricks.empty())
	{
		CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
		if(actMalus && !rootBrick->isSpecialPower())
			totalActionMalus= actMalus->getValue32();	
	}
	return totalActionMalus;
}


// ***************************************************************************
CCDBNodeLeaf	*CSPhraseManager::getRegenTickRangeDbLeaf(uint powerIndex) const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();	
	CCDBNodeLeaf	*dbLeaf = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:FLAGS:BRICK_TICK_RANGE:%d:TICK_RANGE", (int) powerIndex), false);		
	return dbLeaf;
}

// ***************************************************************************
CTickRange CSPhraseManager::getRegenTickRange(uint powerIndex) const
{
	CTickRange tickRange;	
	CCDBNodeLeaf	*dbLeaf = getRegenTickRangeDbLeaf(powerIndex);
	if (dbLeaf)
	{		
		tickRange.StartTick = (NLMISC::TGameCycle) dbLeaf->getValue32();
		tickRange.EndTick = (NLMISC::TGameCycle) (((uint64) dbLeaf->getValue64()) >> 32);
	}	
	return tickRange;
}

// ***************************************************************************
void CSPhraseManager::setRegenTickRange(uint powerIndex, const CTickRange &tickRange)
{	
	CCDBNodeLeaf	*dbLeaf = getRegenTickRangeDbLeaf(powerIndex);
	if (dbLeaf)
	{	
		uint64 value = uint64(tickRange.StartTick) | (uint64(tickRange.EndTick) << 32);
		dbLeaf->setValue64(value);
	}		
}


// ***************************************************************************
int CSPhraseComAdpater::luaGetCastTime(CLuaState &ls)
{
	if (Phrase.Bricks.empty())
	{
		ls.push((double) 0);
		return 1;
	}
	CSPhraseManager *pPM = CSPhraseManager::getInstance();	
	float castTime;
	float castTimeMalus;
	pPM->getPhraseCastTime(Phrase, pPM->getTotalActionMalus(Phrase), castTime, castTimeMalus);
	ls.push((double) (castTime + castTimeMalus));
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaGetCastRange(CLuaState &ls)
{
	if (Phrase.Bricks.empty())
	{
		ls.push((double) 0);
		return 1;
	}
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	sint range;
	sint  rangeMalus;
	pPM->getPhraseMagicRange(this->Phrase, pPM->getTotalActionMalus(Phrase), range, rangeMalus);
	ls.push((double) (range + rangeMalus));
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaGetHpCost(CLuaState &ls)
{
	if (Phrase.Bricks.empty())
	{
		ls.push((double) 0);
		return 1;
	}
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	sint hpCost;
	sint hpCostMalus;
	pPM->getPhraseHpCost(this->Phrase, pPM->getTotalActionMalus(Phrase), hpCost, hpCostMalus);
	ls.push((double) (hpCost + hpCostMalus));
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaGetSapCost(CLuaState &ls)
{
	if (Phrase.Bricks.empty())
	{
		ls.push((double) 0);
		return 1;
	}
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	sint sapCost;
	sint sapCostMalus;
	pPM->getPhraseSapCost(this->Phrase, pPM->getTotalActionMalus(Phrase), sapCost, sapCostMalus);
	ls.push((double) (sapCost + sapCostMalus));
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaGetSuccessRate(CLuaState &ls)
{
	if (Phrase.Bricks.empty())
	{
		ls.push((double) 0);
		return 1;
	}
	CSPhraseManager *pPM = CSPhraseManager::getInstance();		;
	ls.push((double) pPM->getPhraseSuccessRate(this->Phrase));
	return 1;
}


// ***************************************************************************
int CSPhraseComAdpater::luaGetFocusCost(CLuaState &ls)
{
	if (Phrase.Bricks.empty())
	{
		ls.push((double) 0);
		return 1;
	}
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	sint focusCost;
	sint focusCostMalus;
	pPM->getPhraseFocusCost(this->Phrase, pPM->getTotalActionMalus(Phrase), focusCost, focusCostMalus);
	ls.push((double) (focusCost + focusCostMalus));
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaGetStaCost(CLuaState &ls)
{
	if (Phrase.Bricks.empty())
	{
		ls.push((double) 0);
		return 1;
	}
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	sint staCost;
	sint staCostMalus;
	pPM->getPhraseStaCost(this->Phrase, pPM->getTotalActionMalus(Phrase), staCost, staCostMalus);
	ls.push((double) (staCost + staCostMalus));
	return 1;
}


// ***************************************************************************
int CSPhraseComAdpater::luaGetName(CLuaState &ls)
{
	CLuaIHM::push(ls, this->Phrase.Name);
	return 1;
}


// ***************************************************************************
int CSPhraseComAdpater::luaGetDesc(CLuaState &ls)
{
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	sint32 phraseSheetID = pPM->getSheetFromPhrase(Phrase);
	if (phraseSheetID != 0)
	{
		// is it a built-in phrase?
		ucstring desc(STRING_MANAGER::CStringManagerClient::getSPhraseLocalizedDescription(NLMISC::CSheetId(phraseSheetID)));
		if (!desc.empty())
		{
			CLuaIHM::push(ls, desc);
			return 1;
		}
	}
	return 0;
}

// ***************************************************************************
int CSPhraseComAdpater::luaIsCombatPhrase(CLuaState &ls)
{
	if(Phrase.empty())
	{
		ls.push(false);
		return 1;
	}
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*rootBrick= pBM->getBrick(Phrase.Bricks[0]);
	if(!rootBrick)
	{
		ls.push(false);
		return 1;
	}
	ls.push(rootBrick->isCombat());
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaIsMagicPhrase(CLuaState &ls)
{
	if(Phrase.empty())
	{
		ls.push(false);
		return 1;
	}
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*rootBrick= pBM->getBrick(Phrase.Bricks[0]);
	if(!rootBrick)
	{
		ls.push(false);
		return 1;
	}
	ls.push(rootBrick->isMagic());
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaIsPowerPhrase(CLuaState &ls)
{
	if(Phrase.empty())
	{
		ls.push(false);
		return 1;
	}
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*rootBrick= pBM->getBrick(Phrase.Bricks[0]);
	if(!rootBrick)
	{
		ls.push(false);
		return 1;
	}
	ls.push(rootBrick->isSpecialPower());
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaGetRegenTime(CLuaState &ls)
{	
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	ls.push((double) pPM->getRegenTime(Phrase));
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaGetTotalRegenTime(CLuaState &ls)
{
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	ls.push((double) pPM->getTotalRegenTime(Phrase));
	return 1;
}

// ***************************************************************************
int CSPhraseComAdpater::luaGetPowerDisableTime(CLuaState &ls)
{
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	ls.push((double) pPM->getPowerDisableTime(Phrase));
	return 1;
}

//////////////
// COMMANDS //
//////////////

#ifdef NL_DEBUG

NLMISC_COMMAND(regenTickRange, "Emulate regen tick range locally for a memory slot)", "")
{
	if (args.size() != 1 && args.size() != 2) return false;
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	uint32 phraseId;
	NLMISC::fromString(args[0], phraseId);
	const CSPhraseCom	&phrase = pPM->getPhrase(phraseId);
	uint64 powerBitField = CSPhraseManager::getPhraseRequiredFlags(phrase);
	TGameCycle duration;
	if (args.size() >= 2)
	{
		NLMISC::fromString(args[1], duration);
	}
	else
	{
		duration = pPM->getPowerDisableTime(phrase);
	}
	for (uint b = 0; b < 64; ++b)
	{
		if (powerBitField & (uint64(1) << b))
		{
			pPM->setRegenTickRange(b, CTickRange(LastGameCycle, LastGameCycle + duration));
		}
	}
	pPM->touchRegenTickRangeFlag();
	return true;
}

NLMISC_COMMAND(forceUpdatePhrases, "Force to update the display of phrases", "")
{
	if (!args.empty()) return false;
	CSPhraseManager *pPM = CSPhraseManager::getInstance();
	pPM->touchRegenTickRangeFlag();
	pPM->updateAllMemoryCtrlState();
	pPM->updateAllActionRegen();
	return true;
}

#endif


