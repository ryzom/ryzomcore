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


#include "nel/gui/action_handler.h"
#include "../motion/user_controls.h"
#include "../view.h"
#include "../misc.h"
#include "../input.h"
#include "../client_cfg.h"
#include "../actions_client.h"
#include "../sheet_manager.h"
#include "interface_manager.h"
#include "dbctrl_sheet.h"
#include "dbgroup_build_phrase.h"
#include "nel/gui/group_container.h"
#include "sphrase_manager.h"
#include "sbrick_manager.h"
#include "nel/gui/ctrl_button.h"
#include "../user_entity.h"
#include "skill_manager.h"
#include "inventory_manager.h"
#include "game_share/memorization_set_types.h"
#include "action_handler_help.h"
#include "bot_chat_page_all.h"
#include "bot_chat_page_trade.h"
#include "../net_manager.h"
#include "../entities.h"
#include "macrocmd_manager.h"
#include "nel/gui/group_menu.h"
#include "nel/gui/group_tree.h"

extern CSheetManager SheetMngr;

using namespace std;
using namespace NLMISC;



// ***************************************************************************
void	launchPhraseComposition(bool creation);
const	std::string		PhraseComposition="ui:interface:phrase_composition";
const	std::string		PhraseCompositionGroup="ui:interface:phrase_composition:header_opened";
const	std::string		PhraseMemoryCtrlBase= "ui:interface:gestionsets:shortcuts:s";
const	std::string		PhraseMemoryAltCtrlBase= "ui:interface:gestionsets2:header_closed:shortcuts:s";


// **********************************************************************************************************
// debug update the action bar
void	debugUpdateActionBar()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_TSTART")->setValue64(NetMngr.getCurrentServerTick());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_TEND")->setValue64(NetMngr.getCurrentServerTick()+30);
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// PHRASE BOOK EDITION
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
class CHandlerPhraseEdit : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSDst == NULL || !pCSDst->isSPhraseId())
			return;

		// get the phrase id.
		sint32	id= pCSDst->getSPhraseId();

		// Nb: Yoyo: this slot can come from eiter the Action Book, or the Memorized sentence

		// set the phrase to edit
		CSPhraseManager		*pPM= CSPhraseManager::getInstance();
		pPM->CompositionPhraseId= id;
		pPM->CompositionPhraseMemoryLineDest= -1;
		pPM->CompositionPhraseMemorySlotDest= -1;

		// Launch the composition window
		launchPhraseComposition(false);
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseEdit, "phrase_edit" );


// ***************************************************************************
class CHandlerPhraseNew : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		// Get A free slot to edit (not allocated).
		CSPhraseManager		*pPM= CSPhraseManager::getInstance();
		// if possible
		if(pPM->hasFreeSlot())
		{
			// Set the phrase to edit to 0, to indicate a new phrase. CANNOT "ALLOCATE" THE ID NOW, else may
			// crash if buy a botchatphrase, while editing a new phrase!!!!
			pPM->CompositionPhraseId= 0;

			// If the caller is ctrl sheet (may be a standard button else), and if comes from memory
			CDBCtrlSheet	*pCSDst= dynamic_cast<CDBCtrlSheet*>(pCaller);
			if(pCSDst && pCSDst->isSPhraseId() && pCSDst->isSPhraseIdMemory())
			{
				// then will auto-memorize it in this slot
				if (pCSDst->isShortCut())
					pPM->CompositionPhraseMemoryLineDest= pPM->getSelectedMemoryLineDB();
				else
					pPM->CompositionPhraseMemoryLineDest= 0;

				pPM->CompositionPhraseMemorySlotDest= pCSDst->getIndexInDB();
			}
			// else no auto memorize
			else
			{
				pPM->CompositionPhraseMemoryLineDest= -1;
				pPM->CompositionPhraseMemorySlotDest= -1;
			}

			// Launch the composition window
			launchPhraseComposition(true);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseNew, "phrase_new" );


// ***************************************************************************
class CHandlerPhraseValidate : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		// NB: The user can validate only if the server was OK.

		// get our father build sentence
		CInterfaceGroup *parent = pCaller->getParent();
		if (parent)
		{
			CSPhraseManager		*pPM= CSPhraseManager::getInstance();

			CDBGroupBuildPhrase	*buildGroup= dynamic_cast<CDBGroupBuildPhrase*>(parent->getParent());
			if(!buildGroup)
				return;

			// is this an action creation or action edition?
			bool	actionCreation= pPM->CompositionPhraseId==0;

			// For a new created phrase, the CompositionPhraseId is 0, allocate it now
			if(!pPM->CompositionPhraseId)
			{
				pPM->CompositionPhraseId= pPM->allocatePhraseSlot();
				// fail?
				if(!pPM->CompositionPhraseId)
					return;
			}

			// build the edited phrase
			CSPhraseCom		newPhrase;
			buildGroup->buildCurrentPhrase(newPhrase);

			// the new slot of the phrase
			uint32	slotId= pPM->CompositionPhraseId;

			// Action Creation case.
			if(actionCreation)
			{
				// Append in the Phrase Manager.
				pPM->setPhrase(slotId, newPhrase);

				// inform the server of our book change, with the edited name
				pPM->sendLearnToServer(slotId);

				// if this action creation requires an auto "memorize in slot"
				sint32	memoryLine= pPM->CompositionPhraseMemoryLineDest;
				sint32	memorySlot= pPM->CompositionPhraseMemorySlotDest;
				if( memoryLine>=0 &&
					memorySlot>=0 &&
					memorySlot<PHRASE_MAX_MEMORY_SLOT)
				{
					// then memorize auto, client and server
					pPM->memorizePhrase(memoryLine, memorySlot, slotId);
					pPM->sendMemorizeToServer(memoryLine, memorySlot, slotId);
				}
			}
			// Action edition case
			else
			{
				// Replace in the Phrase Manager.
				pPM->setPhrase(slotId, newPhrase);

				// inform the server of our book change, with the edited name
				pPM->sendLearnToServer(slotId);

				// Then, auto-re_memorize all after, changing Ctrl Gray States if needed, and sending msg to server
				pPM->rememorizeAllThatUsePhrase(slotId);
			}

			// Close the Composition Window
			pCaller->getRootWindow()->setActive(false);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseValidate, "phrase_validate" );


// ***************************************************************************
class CHandlerPhraseValidateOnEnter : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		// Test if the OK control is valid.
		CGroupContainer	*pGC= dynamic_cast<CGroupContainer*>(pCaller);
		if(!pGC)	return;
		if(!pGC->getHeaderOpened()) return;
		CDBGroupBuildPhrase	*buildPhrase= dynamic_cast<CDBGroupBuildPhrase*>(pGC->getHeaderOpened());
		if(!buildPhrase) return;
		CCtrlBaseButton *button= buildPhrase->getValidateButton();

		// Ok, button found. test if active and not frozen.
		if( button && button->getActive() && !button->getFrozen() )
		{
			// Act as if the player click on this button
			CInterfaceManager	*pIM = CInterfaceManager::getInstance();
			CAHManager::getInstance()->runActionHandler(button->getActionOnLeftClick(), button, button->getParamsOnLeftClick() );
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseValidateOnEnter, "phrase_validate_on_enter");


// ***************************************************************************
void	launchPhraseComposition(bool creation)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Launch the composition window
	CGroupContainer		*window= dynamic_cast<CGroupContainer*>( CWidgetManager::getInstance()->getElementFromId(PhraseComposition) );
	if(window)
	{
		CDBGroupBuildPhrase	*buildSentenceTarget= dynamic_cast<CDBGroupBuildPhrase*>( CWidgetManager::getInstance()->getElementFromId(PhraseCompositionGroup) );
		// if found
		if(buildSentenceTarget)
		{
			// enable the window
			window->setActive(true);

			// Set the Text of the Window
			if(creation)
				window->setUCTitle(CI18N::get("uiPhraseCreate"));
			else
				window->setUCTitle(CI18N::get("uiPhraseEdit"));

			// clear the sentence for a New Phrase creation.
			buildSentenceTarget->clearBuildingPhrase();

			// copy phrase to edit
			CSPhraseManager		*pPM= CSPhraseManager::getInstance();
			// get the edited (or empty if new) phrase.
			const CSPhraseCom	&phrase= pPM->getPhrase(pPM->CompositionPhraseId);
			// Start the composition
			buildSentenceTarget->startComposition(phrase);

			/** if edition, avoid changing the root type, it behaves better
			 */
			// Default, no filter (important for creation)
			if(creation)
			{
				// all root are possible => no filter
				buildSentenceTarget->setRootBrickTypeFilter(BRICK_TYPE::UNKNOWN);
			}
			// if edition
			else
			{
				if(phrase.empty())
				{
					buildSentenceTarget->setRootBrickTypeFilter(BRICK_TYPE::UNKNOWN);
				}
				else
				{
					CSBrickManager		*pBM= CSBrickManager::getInstance();
					const CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
					// can select only bricks of the same type
					if(rootBrick)
						buildSentenceTarget->setRootBrickTypeFilter(BRICK_FAMILIES::brickType(rootBrick->BrickFamily));
					// maybe data error: filter nothing
					else
						buildSentenceTarget->setRootBrickTypeFilter(BRICK_TYPE::UNKNOWN);
				}
			}
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// PHRASE COMPOSITION
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// **********************************************************************************************************
/**	Called when the user select a brick from the list of possible bricks
 */
class CHandlerPhraseValidateBrick : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager	*pIM = CInterfaceManager::getInstance();

		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(!pCSSrc || !pCSSrc->isSBrick())
			return;

		// get the selected brick
		const CSBrickSheet		*brick= pCSSrc->asSBrickSheet();
		if(brick==NULL)
			return;

		// and validate the composition
		// Standard Case: selection for action composition
		if(BrickType != FaberPlan)
		{
			nlassert(BuildPhraseGroup);
			switch(BrickType)
			{
			case Root:			BuildPhraseGroup->validateRoot(brick);	break;
			case OtherMain:		BuildPhraseGroup->validateMain(Index, brick);	break;
			case Param:			BuildPhraseGroup->validateParam(Index, ParamIndex, brick);	break;
			case NewOpCredit:	BuildPhraseGroup->validateNewOpCredit(brick);	break;
			default: break;
			}
		}
		// Special Case: selection for faber plan
		else
		{
			extern void		validateFaberPlanSelection(CSBrickSheet *itemPlanBrick);
			validateFaberPlanSelection(const_cast<CSBrickSheet*>(brick));
		}

		// And hide the modal
		CWidgetManager::getInstance()->disableModalWindow();
	}
public:
	enum	TType	{Root, OtherMain, Param, NewOpCredit, FaberPlan};
	static TType					BrickType;
	static uint						Index;
	static uint						ParamIndex;
	static CDBGroupBuildPhrase		*BuildPhraseGroup;
};
REGISTER_ACTION_HANDLER( CHandlerPhraseValidateBrick, "phrase_validate_brick" );
CHandlerPhraseValidateBrick::TType	CHandlerPhraseValidateBrick::BrickType;
uint						CHandlerPhraseValidateBrick::Index;
uint						CHandlerPhraseValidateBrick::ParamIndex;
CDBGroupBuildPhrase			*CHandlerPhraseValidateBrick::BuildPhraseGroup= NULL;


// ***************************************************************************
class CHandlerPhraseSelectMainBrick : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get the type and index in the current build sentence
		uint index;
		fromString(getParam(Params, "index"), index);
		CHandlerPhraseValidateBrick::Index= index;
		if(index==0)
			CHandlerPhraseValidateBrick::BrickType= CHandlerPhraseValidateBrick::Root;
		else
			CHandlerPhraseValidateBrick::BrickType= CHandlerPhraseValidateBrick::OtherMain;

		// get our father build sentence
		CDBGroupBuildPhrase	*buildGroup= dynamic_cast<CDBGroupBuildPhrase*>(pCaller->getParent()->getParent());
		if(!buildGroup)
			return;
		// setup the validation
		CHandlerPhraseValidateBrick::BuildPhraseGroup= buildGroup;

		// build the list of possible bricks
		if(index==0)
			buildGroup->fillSelectionRoot();
		else
			buildGroup->fillSelectionMain(index);

		// launch the modal
		CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId( CDBGroupBuildPhrase::BrickSelectionModal ) );
		if(group)
		{
			// enable the modal
			CWidgetManager::getInstance()->enableModalWindow(pCaller, group);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseSelectMainBrick, "phrase_select_main_brick" );


// ***************************************************************************
class CHandlerPhraseSelectParamBrick : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get the type and index in the current build sentence
		CHandlerPhraseValidateBrick::BrickType= CHandlerPhraseValidateBrick::Param;
		uint index, paramIndex;
		fromString(getParam(Params, "index"), index);
		CHandlerPhraseValidateBrick::Index= index;
		fromString(getParam(Params, "param_index"), paramIndex);
		CHandlerPhraseValidateBrick::ParamIndex= paramIndex;

		// get our father build sentence
		CDBGroupBuildPhrase	*buildGroup= dynamic_cast<CDBGroupBuildPhrase*>(pCaller->getParent()->getParent());
		if(!buildGroup)
			return;
		// setup the validation
		CHandlerPhraseValidateBrick::BuildPhraseGroup= buildGroup;

		// build the list of possible bricks
		buildGroup->fillSelectionParam(index, paramIndex);

		// launch the modal
		CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId( CDBGroupBuildPhrase::BrickSelectionModal ) );
		if(group)
		{
			// enable the modal
			CWidgetManager::getInstance()->enableModalWindow(pCaller, group);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseSelectParamBrick, "phrase_select_param_brick" );


// ***************************************************************************
class CHandlerPhraseSelectNewBrick : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get the type and index in the current build sentence
		CHandlerPhraseValidateBrick::BrickType= CHandlerPhraseValidateBrick::NewOpCredit;
		string	typeStr= getParam(Params, "type");

		// get our father build sentence
		CDBGroupBuildPhrase	*buildGroup= dynamic_cast<CDBGroupBuildPhrase*>(pCaller->getParent()->getParent());
		if(!buildGroup)
			return;
		// setup the validation
		CHandlerPhraseValidateBrick::BuildPhraseGroup= buildGroup;

		// build the list of possible bricks
		if(typeStr=="optional")
			buildGroup->fillSelectionNewOp();
		else if(typeStr=="credit")
			buildGroup->fillSelectionNewCredit();

		// launch the modal
		CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId( CDBGroupBuildPhrase::BrickSelectionModal ) );
		if(group)
		{
			// enable the modal
			CWidgetManager::getInstance()->enableModalWindow(pCaller, group);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseSelectNewBrick, "phrase_select_new_brick" );


// ***************************************************************************
class CHandlerPhraseChangeName : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		// The BuildPhrase should be the grandParent
		CDBGroupBuildPhrase	*buildPhrase= dynamic_cast<CDBGroupBuildPhrase*>(pCaller->getParent()->getParent());
		if(!buildPhrase)
			return;
		// update the edited icon
		buildPhrase->updateSpellView();
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseChangeName, "phrase_change_name");


// ***************************************************************************
/*
 *	Special For Faber Plan selection
 */
class CHandlerPhraseFaberSelectPlan : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Launch the modal to select the faber plan
		extern void		fillFaberPlanSelection(const std::string &brickDB, uint maxSelection);
		fillFaberPlanSelection(CDBGroupBuildPhrase::BrickSelectionDB, CDBGroupBuildPhrase::MaxSelection);

		// setup the validation
		CHandlerPhraseValidateBrick::BuildPhraseGroup= NULL;
		CHandlerPhraseValidateBrick::BrickType= CHandlerPhraseValidateBrick::FaberPlan;

		// launch the modal
		CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId( CDBGroupBuildPhrase::BrickSelectionModal ) );
		if(group)
		{
			// enable the modal
			CWidgetManager::getInstance()->enableModalWindow(pCaller, group);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseFaberSelectPlan, "phrase_faber_select_plan" );




// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// PHRASE MEMORISATION / EXECUTION / MISC
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static void	updateAllSPhraseInfo()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// update all info windows
	CInterfaceHelp::updateWindowSPhraseTexts();
	// If the composition is opened, refresh
	CInterfaceGroup			*pIG= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:phrase_composition"));
	if(pIG && pIG->getActive())
	{
		CDBGroupBuildPhrase		*buildPhrase= dynamic_cast<CDBGroupBuildPhrase*>(pIG->getGroup("header_opened"));
		if(buildPhrase)
			buildPhrase->updateAllDisplay();
	}
	// update bot chat
	if(BotChatPageAll && BotChatPageAll->Trade)
		BotChatPageAll->Trade->updateSPhraseBuyDialog();
}

// **********************************************************************************************************
/** Called when the Item in Right Hand change
 */
class CHandlerPhraseUpdateFromHand : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CSPhraseManager		*pPM = CSPhraseManager::getInstance();

		// **** Update the grayed state of all memory ctrls
		pPM->updateAllMemoryCtrlState();

		// **** Update misc Action Infos related to items weared
		updateAllSPhraseInfo();
	}
};
REGISTER_ACTION_HANDLER (CHandlerPhraseUpdateFromHand, "phrase_update_from_hand");


// **********************************************************************************************************
/** Called when the Item in equip change (Action Malus due to armor)
 */
class CHandlerPhraseUpdateFromActionMalus : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// **** Update misc Action Infos related to items weared
		updateAllSPhraseInfo();
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseUpdateFromActionMalus, "phrase_update_from_action_malus");


// **********************************************************************************************************
/** drag'n'drop: cannot drag PhraseSheet that are not castable
 */
class CHandlerCanDragPhrase : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CSPhraseManager		*pPM= CSPhraseManager::getInstance();

		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(!pCSSrc || !pCSSrc->isSPhrase())
			return;

		// can drag only if memorizable
		pCSSrc->setTempCanDrag(pPM->isPhraseCastable(pCSSrc->getSheetId()));
	}
};
REGISTER_ACTION_HANDLER (CHandlerCanDragPhrase, "phrase_can_drag_castable");


// **********************************************************************************************************
/** drag'n'drop: true if the src ctrlSheet is a Phrase and the dest is in memory. Or if both are in memory
 */
class CHandlerCanMemorizePhraseOrMacro : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM = CInterfaceManager::getInstance();
		CSPhraseManager		*pPM = CSPhraseManager::getInstance();
		CSBrickManager		*pBM = CSBrickManager::getInstance();
		CMacroCmdManager	*pMM = CMacroCmdManager::getInstance();

		string src = getParam(Params, "src");
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		// can be a phrase id (comes from memory), a phraseSheet (comes from progression), or a macro,
		if (pCSSrc->isSPhraseId() || pCSSrc->isMacro() || pCSSrc->isSPhrase())
		if (pCSDst->isSPhraseId() || pCSDst->isMacro())
		{
			// check if incoming phrase ok
			if(pCSSrc->isSPhraseId())
			{
				// get the src phrase.
				const CSPhraseCom	&phrase= pPM->getPhrase(pCSSrc->getSPhraseId());
				if(!phrase.empty())
				{
					// Get the RootBrick of the Phrase
					CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[0]);
					if(brick)
						pCSDst->setCanDrop (true);
				}
			}
			// can be a phrase sheet
			else if(pCSSrc->isSPhrase())
			{
				CSPhraseSheet	*phrase= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(NLMISC::CSheetId(pCSSrc->getSheetId())));
				if(phrase && !phrase->Bricks.empty())
				{
					// Get the RootBrick of the Phrase
					CSBrickSheet	*brick= pBM->getBrick(phrase->Bricks[0]);
					if(brick)
						pCSDst->setCanDrop (true);
				}
			}
			// check if incoming macro ok
			else
			{
				// get the macro
				const CMacroCmd		*macroCmd= pMM->getMacroFromMacroID(pCSSrc->getMacroId());
				if(macroCmd)
					pCSDst->setCanDrop(true);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerCanMemorizePhraseOrMacro, "can_memorize_phrase_or_macro");



class CHandlerPhraseMemoryCopy : public IActionHandler
{
public:
	static bool haveLastPhraseElement;
	static bool isMacro;
	static sint32 sPhraseId;
	static sint32 macroId;

	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CDBCtrlSheet	*ctrl= dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(ctrl && ctrl->isSPhraseIdMemory())
		{
			haveLastPhraseElement = true;
			isMacro = ctrl->isMacro();
			sPhraseId = ctrl->getSPhraseId();
			macroId = ctrl->getMacroId();

			string mode = getParam(Params, "mode"); //default mode is copy
			if (mode == "cut") //need delete src
			{
				CAHManager::getInstance()->runActionHandler("forget_phrase_or_macro", ctrl);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseMemoryCopy, "phrase_memory_copy");
bool CHandlerPhraseMemoryCopy::haveLastPhraseElement = false;
bool CHandlerPhraseMemoryCopy::isMacro = false;
sint32 CHandlerPhraseMemoryCopy::sPhraseId = 0;
sint32 CHandlerPhraseMemoryCopy::macroId = 0;


// **********************************************************************************************************
// debug update the action bar
extern void	debugUpdateActionBar();

// **********************************************************************************************************
/** Memorize a combat Brick into a Memory slot.
 *	Called when the user drop a combat brick on a spell slot.
 */
class CHandlerMemorizePhraseOrMacro : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params);
	void memorizePhraseOrMacro(uint dstMemoryIndex, bool isMacro, sint32 phraseId, sint32 macroId);
	void memorizePhraseSheet(uint dstMemoryIndex, uint32 sheetId);
};
REGISTER_ACTION_HANDLER( CHandlerMemorizePhraseOrMacro, "memorize_phrase_or_macro");

void CHandlerMemorizePhraseOrMacro::execute (CCtrlBase *pCaller, const string &Params)
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	CSPhraseManager		*pPM = CSPhraseManager::getInstance();

	string src = getParam(Params, "src");
	CDBCtrlSheet *pCSSrc;
	CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
	
	// NB: THIS IS UGLY BUT WORKS BECAUSE Memory ctrls are first initialized as SPhrase (branchname init)

	// type check
	if (pCSDst == NULL) return;
	// The dest must be a memory or a macro memory
	if (!pCSDst->isSPhraseIdMemory() && !pCSDst->isMacroMemory())	return;
	// get the memory line and memory index
	sint32	dstMemoryLine= pPM->getSelectedMemoryLineDB();
	uint	dstMemoryIndex= pCSDst->getIndexInDB();

	bool	srcIsMacro;
	sint32	srcPhraseId;
	sint32	srcMacroId;

	bool dstIsMacro= pCSDst->isMacro();
	sint32 dstPhraseId= pCSDst->getSPhraseId();
	sint32 dstMacroId= pCSDst->getMacroId();

	if ((src.empty()) && (CHandlerPhraseMemoryCopy::haveLastPhraseElement))
	{		
		// get the slot ids from save
		srcIsMacro= CHandlerPhraseMemoryCopy::isMacro;
		srcPhraseId= CHandlerPhraseMemoryCopy::sPhraseId;
		srcMacroId= CHandlerPhraseMemoryCopy::macroId;

		// if a phrase
		if(!srcIsMacro)
		{
			// \toto yoyo: There is a Network BUG which prevents us from simply doing {Forget(),Delete()} Old phrase,
			// then {Learn(),Memorize()}  new one (Messages are shuffled).
			// Instead, replace the old phrase if needed
			uint16	newPhraseId= (uint16)pPM->getMemorizedPhraseIfLastOrNewSlot(dstMemoryLine, dstMemoryIndex);
			if(!newPhraseId)
				return;

			// set it, copy from srcPhraseId
			pPM->setPhrase(newPhraseId, pPM->getPhrase(srcPhraseId));

			// send learn to server
			pPM->sendLearnToServer(newPhraseId);

			// memorize the new phrase
			memorizePhraseOrMacro(dstMemoryIndex, srcIsMacro, newPhraseId, srcMacroId);
		}
	}
	else
	{
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		
		// type check
		if (pCSSrc == NULL) return;
		// The src must be a phraseid, a phrasesheet, or a macro droped
		if (!pCSSrc->isSPhraseId() && !pCSSrc->isSPhrase() && !pCSSrc->isMacro()) return;

		// get the slot ids.
		srcIsMacro= pCSSrc->isMacro();
		srcPhraseId= pCSSrc->getSPhraseId();
		srcMacroId= pCSSrc->getMacroId();
		

		// If the src comes not from a memory
		if(!pCSSrc->isSPhraseIdMemory() && !pCSSrc->isMacroMemory())

		{
			// if the src is a phrase sheet
			if(pCSSrc->isSPhrase())
			{
				// learn and memorize this phrase
				memorizePhraseSheet(dstMemoryIndex, pCSSrc->getSheetId());
			}
			else
			{
				// We may replace a phrase with a macro => must delete the phrase under us
				if(srcIsMacro)
					pPM->fullDeletePhraseIfLast(dstMemoryLine, dstMemoryIndex);

				// memorize the phrase or macro
				memorizePhraseOrMacro(dstMemoryIndex, srcIsMacro, srcPhraseId, srcMacroId);
			}
		}
		// Else the src is a memory too
		else
		{
			// if Drag copy => this is a copy!
			if(pCSSrc->canDragCopy() &&	pIM->testDragCopyKey())
			{
				// if a phrase
				if(!srcIsMacro)
				{
					// \toto yoyo: There is a Network BUG which prevents us from simply doing {Forget(),Delete()} Old phrase,
					// then {Learn(),Memorize()}  new one (Messages are shuffled).
					// Instead, replace the old phrase if needed
					uint16	newPhraseId= (uint16)pPM->getMemorizedPhraseIfLastOrNewSlot(dstMemoryLine, dstMemoryIndex);
					if(!newPhraseId)
						return;

					// set it, copy from srcPhraseId
					pPM->setPhrase(newPhraseId, pPM->getPhrase(srcPhraseId));

					// send learn to server
					pPM->sendLearnToServer(newPhraseId);

					// memorize the new phrase
					memorizePhraseOrMacro(dstMemoryIndex, srcIsMacro, newPhraseId, srcMacroId);
				}
				else
				{
					// We may replace a phrase with a macro => must delete the phrase under us
					pPM->fullDeletePhraseIfLast(dstMemoryLine, dstMemoryIndex);

					// memorize the macro (still a reference)
					memorizePhraseOrMacro(dstMemoryIndex, srcIsMacro, srcPhraseId, srcMacroId);
				}
			}
			// else this is a swap!
			else
			{
				// if the dest exist, swap
				if(dstPhraseId || dstIsMacro)
				{
					// get the memory index for src
					uint	srcMemoryIndex= pCSSrc->getIndexInDB();

					// memorize dst into src
					memorizePhraseOrMacro(srcMemoryIndex, dstIsMacro, dstPhraseId, dstMacroId);
					// memorize src into dst
					memorizePhraseOrMacro(dstMemoryIndex, srcIsMacro, srcPhraseId, srcMacroId);
				}
				// else, it's a move
				else
				{
					// copy
					memorizePhraseOrMacro(dstMemoryIndex, srcIsMacro, srcPhraseId, srcMacroId);

					// forget src (after shorctut change!)
					CAHManager::getInstance()->runActionHandler("forget_phrase_or_macro", pCSSrc);
				}
			}
		}
	}
}


// memorize a spell
void CHandlerMemorizePhraseOrMacro::memorizePhraseOrMacro(uint memoryIndex, bool isMacro, sint32 phraseId, sint32 macroId)
{
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();

	sint32	memoryLine= pPM->getSelectedMemoryLineDB();
	if(memoryLine<0)
		return;

	if(isMacro)
	{
		pPM->memorizeMacro(memoryLine, memoryIndex, macroId);
	}
	else
	{
		// memorize in local
		pPM->memorizePhrase(memoryLine, memoryIndex, phraseId);
		// Send the Server msg
		pPM->sendMemorizeToServer(memoryLine, memoryIndex, phraseId);
	}
}

// memorize a default spell
void CHandlerMemorizePhraseOrMacro::memorizePhraseSheet(uint memoryIndex, uint32 sheetId)
{
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();

	sint32	memoryLine= pPM->getSelectedMemoryLineDB();
	if(memoryLine<0)
		return;

	// this should have been checked in CanDrag
	if(!pPM->isPhraseCastable(sheetId))
		return;
	// build the com phrase
	CSPhraseCom		phraseCom;
	pPM->buildPhraseFromSheet(phraseCom, sheetId);
	if(phraseCom.empty())
		return;


	// **** first learn this phrase
	// \toto yoyo: There is a Network BUG which prevents us from simply doing {Forget(),Delete()} Old phrase,
	// then {Learn(),Memorize()}  new one (Messages are shuffled).
	// Instead, replace the old phrase if needed
	uint16	newPhraseId= (uint16)pPM->getMemorizedPhraseIfLastOrNewSlot(memoryLine, memoryIndex);
	if(!newPhraseId)
		return;

	// set it
	pPM->setPhrase(newPhraseId, phraseCom);

	// send learn to server
	pPM->sendLearnToServer(newPhraseId);


	// **** memorize
	// memorize in local
	pPM->memorizePhrase(memoryLine, memoryIndex, newPhraseId);
	// Send the Server msg
	pPM->sendMemorizeToServer(memoryLine, memoryIndex, newPhraseId);
}


// **********************************************************************************************************
/** Forget a Phrase from a Memory slot
 *	Called when the user choose to free a spell memory slot
 */
class CHandlerForgetPhraseOrMacro : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CSPhraseManager		*pPM = CSPhraseManager::getInstance();

		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSDst == NULL) return;
		if (!pCSDst->isSPhraseIdMemory() && !pCSDst->isMacroMemory())
			return;

		// Ok, the user try to forget a phrase slot.
		sint32	memoryLine= pPM->getSelectedMemoryLineDB();
		if(memoryLine<0)
			return;

		// get the memory index
		uint	memoryIndex= pCSDst->getIndexInDB();

		if(pCSDst->isMacro())
		{
			pPM->forgetMacro(memoryLine, memoryIndex);
		}
		else
		{
			// forget in local.
			pPM->forgetPhrase(memoryLine, memoryIndex);
			// Server com.
			pPM->sendForgetToServer(memoryLine, memoryIndex);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerForgetPhraseOrMacro, "forget_phrase_or_macro");


// **********************************************************************************************************
/** Forget a Phrase from a Memory slot
 *	Called when the user choose to free a spell memory slot
 */
class CHandlerDeletePhraseOrForgetMacro : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string & Params)
	{
		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSDst == NULL) return;
		if (!pCSDst->isSPhraseIdMemory() && !pCSDst->isMacroMemory())
			return;

		// get the memory index
		uint memoryIndex = pCSDst->getIndexInDB();

		bool isMacro = pCSDst->isMacro();
		sint32 phraseId = pCSDst->getSPhraseId();

		// build params string
		string sParams;
		sParams.append("memoryIndex=");
		sParams.append(toString(memoryIndex));
		sParams.append("|isMacro=");
		sParams.append(toString(isMacro));
		sParams.append("|phraseId=");
		sParams.append(toString(phraseId));
		if (!Params.empty())
		{
			sParams.append("|");
			sParams.append(Params);
		}

		// Ask if ok before
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQDeleteAction"), "do_delete_phrase_or_forget_macro", sParams);
	}
};
REGISTER_ACTION_HANDLER( CHandlerDeletePhraseOrForgetMacro, "delete_phrase_or_forget_macro");


// **********************************************************************************************************
/** Forget a Phrase from a Memory slot
 *	Called when the user confirmed that he want to free a spell memory slot
 */
class CHandlerDoDeletePhraseOrForgetMacro : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		// Ok, the user try to forget a phrase slot
		CSPhraseManager	*pPM = CSPhraseManager::getInstance();

		sint32 memoryLine = pPM->getSelectedMemoryLineDB();
		if (memoryLine<0)
			return;

		// get params
		uint memoryIndex;
		fromString(getParam(Params, "memoryIndex"), memoryIndex);
		bool isMacro;
		fromString(getParam(Params, "isMacro"),isMacro);
		sint32 phraseId;
		fromString(getParam(Params, "phraseId"),phraseId);

		if (isMacro)
		{
			pPM->forgetMacro(memoryLine, memoryIndex);
		}
		else
		{
			// do all the thing to delete properly this phrase
			pPM->fullDelete(phraseId);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerDoDeletePhraseOrForgetMacro, "do_delete_phrase_or_forget_macro");


// **********************************************************************************************************
/** Cast a spell from a memory slot (combat or magic)
 *	Called when the user click on a memory slot. Can be called also from CAHRunShortcut, or from
 *	MoveToAction system (in CSPhraseManager)
 */
class CHandlerCastPhrase : public IActionHandler
{
public:
	static	sint64						LastHitTime;
	static	sint						LastIndex;

public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager	*pIM = CInterfaceManager::getInstance();
		CSPhraseManager		*pPM = CSPhraseManager::getInstance();
		CSBrickManager		*pBM= CSBrickManager::getInstance();

		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSDst == NULL) return;
		if (!pCSDst->isSPhraseIdMemory())
			return;

		// Can cast only if not Latent (ie grayed) !!
		if (pCSDst->getGrayed())
			return;

		// Ok, the user try to cast a phrase slot.
		sint32	memoryLine;
		if (pCSDst->isShortCut())
			memoryLine = pPM->getSelectedMemoryLineDB();
		else
			memoryLine = pPM->getSelectedMemoryAltLineDB();
		if(memoryLine<0)
			return;

		// get the memory index
		uint	memoryIndex= pCSDst->getIndexInDB();

		// Check If the phrase exist
		sint	phraseId= pCSDst->getSPhraseId();
		if(!phraseId)
			return;
		const CSPhraseCom	&phraseCom= pPM->getPhrase(phraseId);
		if(phraseCom.empty())
			return;
		CSBrickSheet	*rootBrick= pBM->getBrick(phraseCom.Bricks[0]);
		if(!rootBrick)
			return;

		// **** Check If the phrase is a Craft Phrase.
		if( rootBrick->isFaber() )
		{
			extern void		launchFaberCastWindow(sint32 , uint , CSBrickSheet *);
			launchFaberCastWindow(memoryLine, memoryIndex, rootBrick);

			// Cancel any moveTo, because don't want to continue reaching the prec entity
			UserEntity->resetAnyMoveTo();
		}
		// **** Else standart cast
		else
		{
			// ****  Cyclic Cast? (dblclick)
			bool	cyclic;
			// Manage "DblHit"
			uint dbclickDelay = CWidgetManager::getInstance()->getUserDblClickDelay();
			// if success to "dblclick"
			if(LastIndex==(sint)memoryIndex && T1<=LastHitTime+dbclickDelay )
				cyclic= true;
			else
				cyclic= false;
			// for next hit
			LastHitTime= T1;
			LastIndex= memoryIndex;

			// can't cast magic phrase while moving
			if( rootBrick->isMagic() )
			{
				if( UserControls.isMoving() || (UserEntity && UserEntity->follow() && UserEntity->speed()!=0.0) )
				{
					// display "you can't cast while moving"
					CInterfaceManager	*pIM= CInterfaceManager::getInstance();
					ucstring msg = CI18N::get("msgNoCastWhileMoving");
					string cat = getStringCategory(msg, msg);
					pIM->displaySystemInfo(msg, cat);
					return;
				}
			}

			// Can't cyclic cast Magic, SpecialPower or Harvest phrases (forage_extraction are auto)
			if( rootBrick->isMagic() || rootBrick->isHarvest() || rootBrick->isSpecialPower() )
				cyclic= false;

			// auto cyclic for forage extraction
			if( rootBrick->isForageExtraction() )
				cyclic= true;

			// even for extraction, if AvoidCyclic in any bricks, no cyclic!
			if( pPM->avoidCyclicForPhrase(phraseCom) )
				cyclic= false;

			// **** Launch the cast
			// Cast only if their is a target, or if it is not a combat action
			CEntityCL	*target = EntitiesMngr.entity(UserEntity->targetSlot());
			if(target || !rootBrick->isCombat())
			{
				// combat (may moveTo before) ?
				if(rootBrick->isCombat())
				{
					if( !UserEntity->canEngageCombat() )
						return;

					UserEntity->executeCombatWithPhrase(target, memoryLine, memoryIndex, cyclic);
				}
				// else can cast soon!
				else if ( rootBrick->isForageExtraction() && (! UserEntity->isRiding()) ) // if mounted, send directly to server (without moving) to receive the error message
				{
					// Yoyo: TEMP if a target selected, must be a forage source
					if(!target || target->isForageSource())
					{
						// Cancel any follow
						UserEntity->disableFollow();
						// reset any moveTo also (if target==NULL, moveToExtractionPhrase() and therefore resetAnyMoveTo() not called)
						// VERY important if previous MoveTo was a SPhrase MoveTo (because cancelClientExecute() must be called)
						UserEntity->resetAnyMoveTo();

						// Move to targetted source
						if ( target )
							UserEntity->moveToExtractionPhrase( target->slot(), MaxExtractionDistance, memoryLine, memoryIndex, cyclic );

						// start client execution
						pPM->clientExecute(memoryLine, memoryIndex, cyclic);

						if ( ! target )
						{
							// inform Server of phrase cast
							pPM->sendExecuteToServer(memoryLine, memoryIndex, cyclic);
						}
					}
				}
				else
				{
					// Cancel any moveTo(), because don't want to continue reaching the prec entity
					// VERY important if previous MoveTo was a SPhrase MoveTo (because cancelClientExecute() must be called)
					UserEntity->resetAnyMoveTo();

					// start client execution: NB: start client execution even if it
					pPM->clientExecute(memoryLine, memoryIndex, cyclic);

					// inform Server of phrase cast
					pPM->sendExecuteToServer(memoryLine, memoryIndex, cyclic);
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerCastPhrase, "cast_phrase");
sint64	CHandlerCastPhrase::LastHitTime= 0;
sint	CHandlerCastPhrase::LastIndex= -1;


// ***************************************************************************
/** Called when user hit the 1.2.3.4.5..... key
 */
class CAHRunShortcut : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		sint	shortcut;
		fromString(Params, shortcut);
		if (shortcut>=0 && shortcut <= 2*RYZOM_MAX_SHORTCUT)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();

			// get the control
			CInterfaceElement	*elm;
			if (shortcut < RYZOM_MAX_SHORTCUT)
				elm = CWidgetManager::getInstance()->getElementFromId(PhraseMemoryCtrlBase + toString(shortcut) );
			else
				elm = CWidgetManager::getInstance()->getElementFromId(PhraseMemoryAltCtrlBase + toString(shortcut-RYZOM_MAX_SHORTCUT) );
			CDBCtrlSheet		*ctrl= dynamic_cast<CDBCtrlSheet*>(elm);
			if(ctrl)
			{
				// run the standard cast case.
				if(ctrl->isMacro())
					CAHManager::getInstance()->runActionHandler("cast_macro", ctrl);
				else
					CAHManager::getInstance()->runActionHandler("cast_phrase", ctrl);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHRunShortcut, "run_shortcut");


// ***************************************************************************
/** Called when the user click on a memory slot. Different AH called if the slot is empty or full
*/
class CHandlerCastPhraseOrCreateNew : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM = CInterfaceManager::getInstance();

		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSDst == NULL) return;
		if (!pCSDst->isSPhraseIdMemory())
			return;

		// get the phrase id under it
		sint	phraseId= pCSDst->getSPhraseId();
		// if a phrase is on this slot, just cast the phrase
		if(phraseId)
		{
			CAHManager::getInstance()->runActionHandler("cast_phrase", pCaller, Params);
		}
		// else open the RightMenuEmpty, to have "NewAction"
		else
		{
			string	menu= pCSDst->getListMenuRightEmptySlot();
			// opens only if no dragged sheet
			if( !menu.empty() && CDBCtrlSheet::getDraggedSheet()==NULL )
			{
				// opens the menu
				CDBCtrlSheet::setCurrSelSheet(pCSDst);
				CWidgetManager::getInstance()->enableModalWindow (pCSDst, menu);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerCastPhraseOrCreateNew, "cast_phrase_or_create_new");


// ***************************************************************************
/** Called to get info on Phrase Link
 */
class CHandlerPhraseLinkCtrlRClick : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		CInterfaceGroup	*parent= pCaller->getParent();
		if(parent)
		{
			// Get the Brother CtrlSheet
			CDBCtrlSheet	*ctrl= dynamic_cast<CDBCtrlSheet*>(parent->getCtrl("ctrl_phrase"));
			if(ctrl)
			{
				CAHManager::getInstance()->runActionHandler(ctrl->getActionOnRightClick(), ctrl, ctrl->getParamsOnRightClick());
			}
		}

	}
};
REGISTER_ACTION_HANDLER (CHandlerPhraseLinkCtrlRClick, "phrase_link_ctrl_rclick");


// ***************************************************************************
/** Called to cancel a Phrase link
 */
class CHandlerPhraseLinkStop : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get the link index
		uint8 index;
		fromString(Params, index);
		// Get the link counter. Used to verify that the client cancel the correct link according to server
		uint8	counter= 0;
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:EXECUTE_PHRASE:LINK:%d:COUNTER", index), false);
		if(node)
			counter= node->getValue8();

		// Send msg to server
		if (!ClientCfg.Local)
		{
			NLMISC::CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream("PHRASE:CANCEL_LINK", out))
			{
				out.serial(index);
				out.serial(counter);
				NetMngr.push(out);
			}
			else
			{
				nlwarning(" unknown message name '%s'", "PHRASE:CANCEL_LINK");
			}
		}
		else
		{
			// debug:
			pIM->displaySystemInfo( toString("PHRASE:CANCEL_LINK %d, %d", index, counter) );
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseLinkStop, "phrase_link_stop");


// ***************************************************************************
/** Called to cancel a Phrase link
 */
class CHandlerPhraseCancelCast: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Send msg to server
		if (!ClientCfg.Local)
		{
			UserEntity->cancelAllPhrases();
		}
		else
		{
			// debug:
			pIM->displaySystemInfo( ucstring("PHRASE:CANCEL_ALL") );
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseCancelCast, "phrase_cancel_cast");


// ***************************************************************************
/// Called when one of the BRICK_TICK_RANGE has changed
class CHandlerPhraseUpdateAllMemoryRegenTickRange : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *, const string &)
	{
		CSPhraseManager	*pPM= CSPhraseManager::getInstance();
		pPM->updateAllMemoryCtrlState();
		pPM->touchRegenTickRangeFlag();
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseUpdateAllMemoryRegenTickRange, "phrase_update_all_memory_ctrl_regen_tick_range");


// ***************************************************************************
/// Called when we right click on a brick in the memories
class CHandlerPhraseCheckCanCristalize: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		const string sCristalizePath = "ui:interface:cm_memory_phrase:cri";
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CSPhraseManager *pPM = CSPhraseManager::getInstance();
		CSBrickManager *pBM = CSBrickManager::getInstance();
		CInterfaceElement *pCristalizeMenuOption = CWidgetManager::getInstance()->getElementFromId(sCristalizePath);

		if (pCristalizeMenuOption == NULL) return;
		// The default is to not display the cristalize menu option
		pCristalizeMenuOption->setActive(false);
		if (pCaller == NULL) return;

		// Get the interface control sheet

		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCS == NULL) return;
		if (!pCS->isSPhraseIdMemory()) return;

		// If its a phrase id in memory then get the phrase

		const CSPhraseCom &phrase = pPM->getPhrase(pCS->getSheetId());
		if (phrase.empty()) return;

		// If the phrase is not empty get the root to known if its a magic phrase or not
		// And if its a magic phrase display the cristalize menu option

		CSBrickSheet *pBrick = pBM->getBrick(phrase.Bricks[0]);
		if (pBrick != NULL)
		{
			if (pBrick->isMagic())
			{
				pCristalizeMenuOption->setActive(true);

				// Disable the cristalize item if the player has a forage in progress
				CViewTextMenu* vtm = dynamic_cast<CViewTextMenu*>(pCristalizeMenuOption);
				if ( vtm )
				{
					CTempInvManager *tim = CTempInvManager::getInstance();
					bool isForageInProgress = tim && tim->isOpened() && (tim->getMode() == TEMP_INV_MODE::Forage);
					vtm->setGrayed( isForageInProgress );
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseCheckCanCristalize, "phrase_check_can_cristalize");

// ***************************************************************************
/// Called after the cm_memory_phrase menu has been opened on a magic phrase
class CHandlerPhraseCristalize: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CSPhraseManager		*pPM = CSPhraseManager::getInstance();

		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSDst == NULL) return;
		if (!pCSDst->isSPhraseIdMemory())
			return;

		// Can cast only if not Latent (ie grayed) !!
		if (pCSDst->getGrayed())
			return;

		// Ok, the user try to cast a phrase slot.
		sint32	memoryLine;
		if (pCSDst->isShortCut())
			memoryLine = pPM->getSelectedMemoryLineDB();
		else
			memoryLine = 0;
		if(memoryLine<0)
			return;

		// get the memory index
		uint	memoryIndex= pCSDst->getIndexInDB();

		// execute both client and server
		pPM->executeCristalize(memoryLine, memoryIndex);
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseCristalize, "phrase_cristalize");

// ***************************************************************************
class CHandlerPhraseBookSkillFilter : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CSPhraseManager		*pPM= CSPhraseManager::getInstance();

		// If the param is a BrickType Filter...
		string	btFilter= getParam(Params, "bt");
		if(!btFilter.empty())
		{
			BRICK_TYPE::EBrickType	bt= BRICK_TYPE::toBrickType(btFilter);
			if(bt!=BRICK_TYPE::UNKNOWN)
				pPM->setBookFilter(bt, SKILLS::unknown);
		}
		// else it is a skill filter
		else
		{
			sint	index;
			fromString(Params, index);
			if(index>=0 && index<SKILLS::NUM_SKILLS)
				pPM->setBookFilter(BRICK_TYPE::UNKNOWN, (SKILLS::ESkills)index);
			else
				pPM->setBookFilter(BRICK_TYPE::UNKNOWN, SKILLS::unknown);
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseBookSkillFilter, "phrase_book_skill_filter");

// ***************************************************************************
class CHandlerPhraseSelectMemory : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		string expr = getParam (Params, "value");
		CInterfaceExprValue value;
		if (CInterfaceExpr::eval(expr, value, NULL))
		{
			if (!value.toInteger())
			{
				nlwarning("<CHandlerPhraseSelectMemory:execute> expression doesn't evaluate to a numerical value");
			}
			else
			{
				CSPhraseManager		*pPM= CSPhraseManager::getInstance();
				sint	val= (sint32)value.getInteger();
				clamp(val, 0, MEM_SET_TYPES::NumMemories-1);
				pPM->selectMemoryLineDB(val);
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseSelectMemory, "phrase_select_memory");

class CHandlerPhraseSelectMemory2 : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		string expr = getParam (Params, "value");
		CInterfaceExprValue value;
		if (CInterfaceExpr::eval(expr, value, NULL))
		{
			if (!value.toInteger())
			{
				nlwarning("<CHandlerPhraseSelectMemory:execute> expression doesn't evaluate to a numerical value");
			}
			else
			{
				CSPhraseManager		*pPM= CSPhraseManager::getInstance();
				sint	val= (sint32)value.getInteger();
				clamp(val, 0, MEM_SET_TYPES::NumMemories-1);
				pPM->selectMemoryLineDBalt(val);
			}
		}
	}
};

REGISTER_ACTION_HANDLER(CHandlerPhraseSelectMemory2, "phrase_select_memory_2");

// ***************************************************************************
class CHandlerPhraseSelectShortcutBar : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:PHRASE:SELECT_MEMORY", false);
		if(node)
		{
			sint32 val;
			fromString(Params, val);
			node->setValue32(val);
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerPhraseSelectShortcutBar, "select_shortcut_bar");


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// HELP / LINKS
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// This expr is used only for define in phrase.xml.
DECLARE_INTERFACE_CONSTANT(getPhraseBrickSelectionMax, CDBGroupBuildPhrase::MaxSelection)


// ***************************************************************************
// Get the UC name of a phraseId
static DECLARE_INTERFACE_USER_FCT(getSPhraseName)
{
	if (args.size() > 0)
	{
		if(!args[0].toInteger())
			return false;
		sint	sphraseId= (sint)args[0].getInteger();
		CSPhraseManager		*pPM= CSPhraseManager::getInstance();
		result.setUCString(pPM->getPhrase(sphraseId).Name);
		return true;
	}
	else
	{
		return false;
	}
}
REGISTER_INTERFACE_USER_FCT("getSPhraseName", getSPhraseName)


// ***************************************************************************
// Get the tooltip of a Combat Weapon restriction brick
class CHandlerCombatRestrictTooltip : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CDBCtrlSheet	*ctrlSheet= dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(!ctrlSheet)
			return;

		ucstring	str(STRING_MANAGER::CStringManagerClient::getSBrickLocalizedName(CSheetId(ctrlSheet->getSheetId())));

		// According to locked state
		if(ctrlSheet->getGrayed())
			strFindReplace(str, "%comp", CI18N::get("uittPhraseCombatRestrictKO"));
		else
			strFindReplace(str, "%comp", CI18N::get("uittPhraseCombatRestrictOK"));

		CWidgetManager::getInstance()->setContextHelpText(str);
	}
};
REGISTER_ACTION_HANDLER( CHandlerCombatRestrictTooltip, "phrase_combat_restrict_tooltip");


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// BOT CHAT
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	phraseBotChatBuyBySheet(NLMISC::CSheetId sheetId, uint16 phraseId)
{
	uint32	sheetNum= sheetId.asInt();
	if (!ClientCfg.Local)
	{
		NLMISC::CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("PHRASE:BUY_SHEET", out))
		{
			out.serial(sheetNum);
			out.serial(phraseId);
			NetMngr.push(out);
		}
		else
		{
			nlwarning(" unknown message name '%s'", "PHRASE:BUY_SHEET");
		}
	}
	else
	{
		// debug:
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		pIM->displaySystemInfo( toString("PHRASE:BUY_SHEET => ") + toString(phraseId) );

		// **** Server emulation
		// learn all bricks of this phrase
		CSPhraseSheet	*phrase= dynamic_cast<CSPhraseSheet*>(SheetMngr.get(sheetId));
		if(phrase)
		{
			CSBrickManager	*pBM= CSBrickManager::getInstance();
			// For all bricks of this phrase
			for(uint i=0;i<phrase->Bricks.size();i++)
			{
				CSBrickSheet	*brick= pBM->getBrick(phrase->Bricks[i]);
				if(brick)
				{
					// force learn it.
					CCDBNodeLeaf * node= pBM->getKnownBrickBitFieldDB(brick->BrickFamily);
					if(node)
					{
						uint64	flags= node->getValue64();
						flags|= uint64(1)<<(brick->IndexInFamily-1);
						node->setValue64(flags);
					}
				}
			}
		}
		// ack phrase learn
		CSPhraseManager	*pPM= CSPhraseManager::getInstance();
		if(phraseId)
			pPM->receiveBotChatConfirmBuy(phraseId, true);
		// synchronize
		uint	counter= pIM->getLocalSyncActionCounter();
		NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:COUNTER")->setValue32(counter);
		NLGUI::CDBManager::getInstance()->getDbProp("SERVER:EXCHANGE:COUNTER")->setValue32(counter);
		NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TARGET:CONTEXT_MENU:COUNTER")->setValue32(counter);
		NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:COUNTER")->setValue32(counter);
	}
}

// ***************************************************************************
void	phraseBotChatBuyActionByIndex(uint8 index, uint16 phraseId)
{
	if (!ClientCfg.Local)
	{
		// send the selection to the server
		NLMISC::CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("PHRASE:BUY", out))
		{
			out.serial(index);
			out.serial(phraseId);
			NetMngr.push(out);
		}
		else
			nlwarning(" unknown message name 'PHRASE:BUY");
	}
	else
	{
		// debug:
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		pIM->displaySystemInfo("PHRASE:BUY => " + toString(phraseId) );
	}
}

// ***************************************************************************
/* Called from CConfirmBuyItem action handler
 */
void	phraseBuyBotChat(CDBCtrlSheet *ctrl, uint8 index, bool useBuySheetMsg)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSPhraseManager		*pSM= CSPhraseManager::getInstance();

	if(!ctrl->isSPhrase())
		return;
	// Get the SPhrase
	const CSPhraseSheet	*sp= ctrl->asSPhraseSheet();
	if(!sp || sp->Bricks.empty())
		return;

	// Verify that the phrase is not a special "CHARACTERISTIC BUYING" phrase
	bool	isCharacBuying= false;
	CSBrickSheet	*brickSheet= pBM->getBrick(CSheetId(sp->Bricks[0]));
	if(brickSheet && BRICK_FAMILIES::isCharacBuyFamily(brickSheet->BrickFamily) )
		isCharacBuying= true;

	// confirm the buy
	{
		// default: not append to the manager
		uint16	phraseId= 0;

		// Append the phrase to the manager. ONLY if this phrase is castable
		if(sp->Castable && !isCharacBuying)
		{
			CSPhraseCom		phraseCom;
			pSM->buildPhraseFromSheet(phraseCom, ctrl->getSheetId());
			if(phraseCom.empty())
				return;

			// get the new phraseId
			phraseId= (uint16)pSM->allocatePhraseSlot();

			// Since we are not sure this slot work, lock it!
			pSM->setPhrase(phraseId, phraseCom, true);
		}

		// Special for LOCAL:USER:SKILL_POINTS_   update
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		pIM->incLocalSyncActionCounter();

		// Send msg to server
		if(useBuySheetMsg)
			phraseBotChatBuyBySheet(sp->Id, phraseId);
		else
			phraseBotChatBuyActionByIndex(index, phraseId);
	}
}

// ***************************************************************************
// Command for Debug BotChat ACK.
NLMISC_COMMAND(phraseComfirmBuy, "Debug: confirm a phrase BotChat buy", "")
{
	if(args.size() != 2)	return false;

	uint phraseId;
	fromString(args[0], phraseId);

	bool confirm;
	fromString(args[1], confirm);

	CSPhraseManager		*pSM= CSPhraseManager::getInstance();
	pSM->receiveBotChatConfirmBuy(phraseId, confirm);

	return true;
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// MACRO
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
class CHandlerCastMacro : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CMacroCmdManager	*pMM = CMacroCmdManager::getInstance();

		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSDst == NULL) return;
		if (!pCSDst->isMacroMemory())
			return;

		// Can cast only if not grayed
		if (pCSDst->getGrayed())
			return;

		// execute the macro id
		pMM->executeID(pCSDst->getMacroId());
	}
};
REGISTER_ACTION_HANDLER( CHandlerCastMacro, "cast_macro");


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// MISC
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
class CHandlerPhraseMemoryBeforeMenu : public IActionHandler
{
public:
	static	uint32 LastPhraseIdMenu;

	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet	*ctrl= dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(ctrl && ctrl->isSPhraseIdMemory())
		{
			LastPhraseIdMenu= ctrl->getSPhraseId();
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerPhraseMemoryBeforeMenu, "phrase_memory_before_menu");
uint32 CHandlerPhraseMemoryBeforeMenu::LastPhraseIdMenu= 0;


// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(isPhraseMenuNotExecuting)
{
	bool	ok= true;

	// if the PhraseId for this menu is equal to either cyclic action, or next action, fails
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();
	if(CHandlerPhraseMemoryBeforeMenu::LastPhraseIdMenu!=0)
	{
		if(pPM->getCycleExecutePhraseId()==CHandlerPhraseMemoryBeforeMenu::LastPhraseIdMenu)
			ok= false;
		if(pPM->getNextExecutePhraseId()==CHandlerPhraseMemoryBeforeMenu::LastPhraseIdMenu)
			ok= false;
	}

	result.setBool(ok);
	return true;
}
REGISTER_INTERFACE_USER_FCT("isPhraseMenuNotExecuting", isPhraseMenuNotExecuting)

