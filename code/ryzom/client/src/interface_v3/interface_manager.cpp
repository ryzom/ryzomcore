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

// Memory
#include <memory>

#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/algo.h"

#include "nel/net/tcp_sock.h"

// Globals
#include "interface_manager.h"
#include "input_handler_manager.h"
#include "interface_config.h"
#include "task_bar_manager.h"
#include "guild_manager.h"
#include "../client_cfg.h"
#include "encyclopedia_manager.h"
// Expr
#include "interface_expr.h"
#include "register_interface_elements.h"
// Action / Observers
#include "action_handler.h"
#include "interface_observer.h"
#include "interface_anim.h"
#include "interface_ddx.h"
#include "action_handler_help.h"
#include "action_handler_item.h"
// View
#include "view_bitmap.h"
//#include "view_bitmap_progress.h"
#include "view_bitmap_faber_mp.h"
#include "view_bitmap_combo.h"
#include "view_text.h"
// Ctrl
#include "ctrl_scroll.h"
#include "ctrl_button.h"
#include "ctrl_text_button.h"
// DBCtrl
#include "dbctrl_sheet.h"
// Group
#include "group_list.h"
#include "group_menu.h"
#include "group_container.h"
#include "group_modal.h"
#include "group_editbox.h"
#include "group_in_scene_bubble.h"
#include "group_skills.h"
#include "group_compas.h"

// Misc
#include "../input.h"
#include "bot_chat_manager.h"
#include "bot_chat_page_all.h"
#include "chat_displayer.h"
#include "skill_manager.h"
#include "../sound_manager.h"
#include "../actions.h"
#include "../actions_client.h"

#include "../weather_manager_client.h"
#include "../weather.h"

#include "../user_entity.h"
#include "../motion/user_controls.h"
#include "people_interraction.h"
#include "macrocmd_manager.h"
#include "inventory_manager.h"

#include "../connection.h" // needed for loading config file (PlayerSelectedFileName)

#include "sbrick_manager.h"
#include "sphrase_manager.h"
#include "bar_manager.h"

#include "../continent_manager.h"
#include "../entity_cl.h"
#include "../login.h"

#include "../sheet_manager.h"				// for emotes
#include "../global.h"						// for emotes
#include "../entity_animation_manager.h"	// for emotes
#include "../net_manager.h"				// for emotes
#include "../client_chat_manager.h"		// for emotes
#include "../entities.h"

#include "chat_text_manager.h"
#include "../npc_icon.h"

#include "lua_helper.h"
#include "lua_ihm.h"

#include "add_on_manager.h"

#include "game_share/r2_share_itf.h"

#include "../time_client.h"

#include "../r2/editor.h"
#include "../r2/dmc/client_edition_module.h"

#include "../bg_downloader_access.h"

using namespace NLMISC;

extern CClientChatManager   ChatMngr;
extern CContinentManager ContinentMngr;
extern CStringMapper *_UIStringMapper;
extern bool IsInRingSession;
extern CEventsListener EventsListener;

namespace R2
{
	extern bool ReloadUIFlag;
}

// ***************************************************************************
/*
	Version 11: - Dyn chat in user tab
	Version 10: - Last screen resolution serialisation
	Version  9: UI_DB_SAVE_VERSION system
	Version  8:	- serialInSceneBubbleInfo (for ignore context help)
	Version  7:	- serialMacroMemory
	Version  6:	DEPRECATED - Info about friend/ignore list
	Version  5:	- Info Windows pos
	Version  4:	- User landmark serialisation
	Version  3:	- Added a Hack for CInterfaceConfig version miss
	Version  2:	- TaskBar serialisation
	Version  1:	- people interraction
	Version  0:	- base version
*/
#define	ICFG_STREAM_VERSION	11

#ifdef AJM_DEBUG_TRACK_INTERFACE_GROUPS
	#define	FOREACH(__itvar,__conttype,__contvar)	\
	for (__conttype::iterator __itvar(__contvar.begin()),__itvar##end(__contvar.end()); __itvar!=__itvar##end; ++__itvar)
#endif

// ***************************************************************************

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// ------------------------------------------------------------------------------------------------

extern NL3D::UDriver *Driver;
extern bool loginFinished;
// Edit actions
CActionsManager EditActions;

CInterfaceManager * CInterfaceManager::_Instance = NULL;

CChatDisplayer * ChatDisplayer = NULL;


void initActions();
void uninitActions();

// ------------------------------------------------------------------------------------------------
string	CInterfaceManager::_CtrlLaunchingModalId= "ctrl_launch_modal";


// ------------------------------------------------------------------------------------------------
static const uint DOUBLE_CLICK_MIN = 50;
static const uint DOUBLE_CLICK_MAX = 750;

///\todo nico: remove this dummy displayer
NLMISC::CLog	g_log;


////////////
// GLOBAL //
////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Update_Frame_Events )

// ------------------------------------------------------------------------------------------------
// AJM TEMP TEMP TEMP TEMP
#ifdef AJM_DEBUG_TRACK_INTERFACE_GROUPS

// track creation of an interface group
void CInterfaceManager::DebugTrackGroupsCreated( CInterfaceGroup *pIG )
{
	// add to set
	_DebugTrackGroupSet.insert( pIG );

	// add to map and increment create counter
	_DebugTrackGroupMap[pIG] = _DebugTrackGroupCreateCount++;
}

// track destruction of an interface group
void CInterfaceManager::DebugTrackGroupsDestroyed( CInterfaceGroup *pIG )
{
	// lookup id of the group being destroyed (for debugging)
	int foo = DebugTrackGroupsGetId( pIG );

	// remove from set
	setInterfaceGroupPtr::iterator it = _DebugTrackGroupSet.find( pIG );
	if( it == _DebugTrackGroupSet.end() )
	{
		nldebug( "AJM DEBUG: Interface Group %x Destroyed twice", pIG );
		return;
	}
	_DebugTrackGroupSet.erase( pIG );

	// remove from map and increment destroy counter
	_DebugTrackGroupMap.erase(pIG);
	_DebugTrackGroupDestroyCount++;
}

// display the count of undestroyed interface groups
void CInterfaceManager::DebugTrackGroupsDump()
{
	// dump groups
	nldebug( "AJM DEBUG: %d Interface Groups remaining", _DebugTrackGroupCreateCount-_DebugTrackGroupDestroyCount );

	FOREACH( itIG, mapInterfaceGroupPtr2Int, _DebugTrackGroupMap )
	{
		nldebug( "  %d", itIG->second );
	}
}

// return the index for an interface group
int CInterfaceManager::DebugTrackGroupsGetId( CInterfaceGroup *pIG )
{
	mapInterfaceGroupPtr2Int::iterator it = _DebugTrackGroupMap.find( pIG );
	if( it != _DebugTrackGroupMap.end() )
		return it->second;
	return -1;
}

#endif // AJM_DEBUG_TRACK_INTERFACE_GROUPS

// ------------------------------------------------------------------------------------------------
CInterfaceManager::CInterfaceManager()
{
	_Instance = this;
	_DbRootNode = new CCDBNodeBranch("ROOT");
	_ScreenW = _ScreenH = 0;
	_LastInGameScreenW = _LastInGameScreenH = 0;
	_Pointer = NULL;
	_DescTextTarget = NULL;
	_GlobalColor = CRGBA(255,255,255,255);
	_GlobalColorForContent = _GlobalColor;
	_ContentAlpha = 255;
	_ContainerAlpha = 255;
	_GlobalContentAlpha = 255;
	_GlobalContainerAlpha = 255;
	_GlobalRolloverFactorContent = 255;
	_GlobalRolloverFactorContainer = 255;
	_MouseOverWindow= false;
	_MouseHandlingEnabled= true;
	_ConfigLoaded = false;
	_LogState = false;
	_KeysLoaded = false;
	_RProp = NULL;
	_GProp = NULL;
	_BProp = NULL;
	_AProp = NULL;
	_AlphaRolloverSpeedDB = NULL;
	_NeutralColor = NULL;
	_WarningColor = NULL;
	_ErrorColor = NULL;
	_EmotesInitialized = false;

	// context help
	_DeltaTimeStopingContextHelp= 0;
	_MaxTimeStopingContextHelp= 0.2f;
	_LastXContextHelp= -10000;
	_LastYContextHelp= -10000;

	// Global initialization
	// *********************


	//init the input handler through the parsing of a config file
	CInputHandlerManager *InputHandlerManager = CInputHandlerManager::getInstance();
	string filename = CPath::lookup(ClientCfg.XMLInputFile, false);
	if (!filename.empty())
		InputHandlerManager->readInputConfigFile(filename);

	// Interface Manager init
	_ViewRenderer.checkNewScreenSize();
	_ViewRenderer.init();

	_CurrentMode = 0;

	_InGame = false;

	_LocalSyncActionCounter= 0;
	// 4Bits counter.
	_LocalSyncActionCounterMask= 15;

	for(uint i=0;i<CHARACTERISTICS::NUM_CHARACTERISTICS;i++)
	{
		_CurrentPlayerCharac[i]= 0;
	}

	_CheckMailNode = NULL;
	_CheckForumNode = NULL;
	_UpdateWeatherTime = 0;

	_ContextHelpActive = true;

	_DBB_UI_DUMMY = NULL;
	_DB_UI_DUMMY_QUANTITY = NULL;
	_DB_UI_DUMMY_QUALITY = NULL;
	_DB_UI_DUMMY_SHEET = NULL;
	_DB_UI_DUMMY_NAMEID = NULL;
	_DB_UI_DUMMY_ENCHANT = NULL;
	_DB_UI_DUMMY_SLOT_TYPE = NULL;
	_DB_UI_DUMMY_PHRASE = NULL;
	_DB_UI_DUMMY_WORNED = NULL;
	_DB_UI_DUMMY_PREREQUISIT_VALID = NULL;
	_DB_UI_DUMMY_FACTION_TYPE = NULL;

#ifdef AJM_DEBUG_TRACK_INTERFACE_GROUPS
	_DebugTrackGroupSet.clear();
	_DebugTrackGroupMap.clear();
	_DebugTrackGroupCreateCount = 0;
	_DebugTrackGroupDestroyCount = 0;
#endif // AJM_DEBUG_TRACK_INTERFACE_GROUPS
}

// ------------------------------------------------------------------------------------------------
CInterfaceManager::~CInterfaceManager()
{
	reset(); // to flush IDStringWaiters

	//delete the interface group
	for (uint32 i = 0; i < _MasterGroups.size(); ++i)
	{
		delete _MasterGroups[i].Group;
	}
	_ParentPositionsMap.clear();
	_ParentSizesMap.clear();
	_ParentSizesMaxMap.clear();
	_LuaClassAssociation.clear();
	_Templates.clear();
	_Instance = NULL;

	if (_DbRootNode)
	{
		delete _DbRootNode;
		_DbRootNode = NULL;
	}

	// release the local string mapper
	delete _UIStringMapper;
	_UIStringMapper = NULL;

	// release the database observers
	releaseServerToLocalAutoCopyObservers();
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::reset()
{
	_ViewRenderer.reset();
	_CtrlsUnderPointer.clear();
	_CurCtrlContextHelp = "";
	_ViewsUnderPointer.clear();
	_GroupsUnderPointer.clear();
	_CaptureKeyboard = NULL;
	_OldCaptureKeyboard = NULL;
	setCapturePointerLeft(NULL);
	setCapturePointerRight(NULL);
	_ActiveAnims.clear();
	for (uint32 i = 0; i < _IDStringWaiters.size(); ++i)
		delete _IDStringWaiters[i];
	_IDStringWaiters.clear();
	CGroupFrame::resetDisplayTypes();

	_NeutralColor			= NULL;
	_WarningColor			= NULL;
	_ErrorColor				= NULL;
	_RProp					= NULL;
	_GProp					= NULL;
	_BProp					= NULL;
	_AProp					= NULL;
	_AlphaRolloverSpeedDB	= NULL;
}

// ------------------------------------------------------------------------------------------------
// unhook from observers we are tangled up in
void CInterfaceManager::releaseServerToLocalAutoCopyObservers()
{
	ServerToLocalAutoCopyInventory.release();
	ServerToLocalAutoCopyExchange.release();
	ServerToLocalAutoCopyContextMenu.release();
	ServerToLocalAutoCopySkillPoints.release();
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::resetShardSpecificData()
{
	_LocalSyncActionCounter= 0;
	CGroupSkills::InhibitSkillUpFX = true;
	CBarManager::getInstance()->resetShardSpecificData();
	CBotChatManager::getInstance()->setCurrPage(NULL);

	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	pPM->setEquipInvalidation(0, 0);

	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(getElementFromId(WIN_TEMPINV));
	if (pGC != NULL)
		pGC->setActive(false);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::destroy ()
{
	delete _Instance;
	_Instance = NULL;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::initLogin()
{
	// Init LUA Scripting
	initLUA();

	// Create String mapper
	if (_UIStringMapper == NULL)
		_UIStringMapper = CStringMapper::createLocalMapper();

	// Clear the action manager
	Actions.clear();
	EditActions.clear();

	// Register action in the action manager
	ActionsContext.addActionsManager(&Actions, "");
	ActionsContext.addActionsManager(&EditActions, RZ_CATEGORY_EDIT);


	if (ClientCfg.XMLLoginInterfaceFiles.size()==0)
	{
		nlinfo("no xml login config files in client.cfg");
		return;
	}

	nldebug("Textures Login Interface");

	for (vector<string>::iterator it = ClientCfg.TexturesLoginInterface.begin(), end = ClientCfg.TexturesLoginInterface.end(); it != end; ++it)
	{
		nldebug("Textures Login Interface: %s", (*it).c_str());
		loadTextures(*it + ".tga", *it + ".txt", false);
	}

	for (vector<string>::iterator it = ClientCfg.TexturesLoginInterfaceDXTC.begin(), end = ClientCfg.TexturesLoginInterfaceDXTC.end(); it != end; ++it)
	{
		nldebug("Textures Login Interface DXTC: %s", (*it).c_str());
		loadTextures(*it + ".tga", *it + ".txt", true);
	}

	parseInterface (ClientCfg.XMLLoginInterfaceFiles, false);

	updateAllLocalisedElements();

	activateMasterGroup ("ui:login", true);

	{
		initActions();
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::uninitLogin()
{

	activateMasterGroup ("ui:login", false);

	removeAll();
	reset();

	_Pointer = NULL;

	CInterfaceLink::removeAllLinks();

	ICDBNode::CTextId textId("UI");
	_DbRootNode->removeNode(textId);

	{
		uninitActions();
	}

	// Close LUA Scripting
	uninitLUA();
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::initOutGame()
{
	// create String mapper
	if (_UIStringMapper == NULL)
		_UIStringMapper = CStringMapper::createLocalMapper();

	// Clear the action manager
	Actions.clear();
	EditActions.clear();

	// Register action in the action manager
	ActionsContext.addActionsManager(&Actions, "");
	ActionsContext.addActionsManager(&EditActions, RZ_CATEGORY_EDIT);

	// Init LUA Scripting
	initLUA();

	if (ClientCfg.SelectCharacter != -1) 
		return;

	{
		if (SoundMngr != NULL)
		{
			NLSOUND::UAudioMixer *pMixer = SoundMngr->getMixer();
			pMixer->loadSampleBank(false, "ui_outgame");
			CVector initpos ( 0.0f, 0.0f, 0.0f );
			pMixer->setListenerPos(initpos);
		}
	}


	//NLMEMORY::CheckHeap (true);


	if (ClientCfg.XMLOutGameInterfaceFiles.size()==0)
	{
		nlinfo("no xml outgame config files in client.cfg");
		return;
	}

	nldebug("Textures OutGame Interface");

	for (vector<string>::iterator it = ClientCfg.TexturesOutGameInterface.begin(), end = ClientCfg.TexturesOutGameInterface.end(); it != end; ++it)
	{
		nldebug("Textures OutGame Interface: %s", (*it).c_str());
		loadTextures(*it + ".tga", *it + ".txt", false);
	}

	for (vector<string>::iterator it = ClientCfg.TexturesOutGameInterfaceDXTC.begin(), end = ClientCfg.TexturesOutGameInterfaceDXTC.end(); it != end; ++it)
	{
		nldebug("Textures OutGame Interface DXTC: %s", (*it).c_str());
		loadTextures(*it + ".tga", *it + ".txt", true);
	}

	parseInterface (ClientCfg.XMLOutGameInterfaceFiles, false);

	updateAllLocalisedElements();

	activateMasterGroup ("ui:outgame", true);

//	if (!ClientCfg.FSHost.empty())
//	{
//		// Hide the Launch Editor button, it works only with a Shard Unifier and web pages
//		CInterfaceElement *elt = getElementFromId("ui:outgame:edit_session_but");
//		elt->setActive(false);
//	}

	// Init the action manager
	{

		initActions();
	}
	//NLMEMORY::CheckHeap (true);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::uninitOutGame()
{

	if (ClientCfg.SelectCharacter != -1)
		return;

	disableModalWindow();

	//_DbRootNode->display("");
	CBotChatManager::getInstance()->setCurrPage(NULL);

	CInterfaceItemEdition::getInstance()->setCurrWindow(NULL);

	NLMISC::TTime initStart;
	initStart = ryzomGetLocalTime ();
	if (SoundMngr != NULL)
	{
		NLSOUND::UAudioMixer *pMixer = SoundMngr->getMixer();
		pMixer->unloadSampleBank("ui_outgame");
	}
	//nlinfo ("%d seconds for uninitOutGame", (uint32)(ryzomGetLocalTime ()-initStart)/1000);

	initStart = ryzomGetLocalTime ();
	activateMasterGroup ("ui:outgame", false);
	//nlinfo ("%d seconds for activateMasterGroup", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	initStart = ryzomGetLocalTime ();
	removeAll();
	//nlinfo ("%d seconds for removeAll", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	initStart = ryzomGetLocalTime ();
	reset();
	//nlinfo ("%d seconds for reset", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	// reset the mouse pointer to avoid invalid pointer access
	_Pointer = NULL;
	initStart = ryzomGetLocalTime ();
	CInterfaceLink::removeAllLinks();
	//nlinfo ("%d seconds for removeAllLinks", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	initStart = ryzomGetLocalTime ();
	ICDBNode::CTextId textId("UI");
	_DbRootNode->removeNode(textId);
	//nlinfo ("%d seconds for removeNode", (uint32)(ryzomGetLocalTime ()-initStart)/1000);

	// Init the action manager
	{

		initStart = ryzomGetLocalTime ();
		uninitActions();
	//	nlinfo ("%d seconds for uninitActions", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	}

	// Close LUA Scripting
	uninitLUA();

	//NLMEMORY::CheckHeap (true);
}


void badXMLParseMessageBox()
{
	NL3D::UDriver::TMessageBoxId	ret = Driver->systemMessageBox(	"Interface XML reading failed!\n"
																		"Some XML files are corrupted and may have been removed.\n"
																		"Ryzom may need to be restarted to run properly.\n"
																		"Would you like to quit now?",
																		"XML reading failed!",
																		NL3D::UDriver::yesNoType,
																		NL3D::UDriver::exclamationIcon);
	if (ret == NL3D::UDriver::yesId)
	{
		extern void quitCrashReport ();
		quitCrashReport ();
		exit (EXIT_FAILURE);
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::initInGame()
{
	loginFinished = true;
	_LogState = false;

	// Whole initInGame profile
	NLMISC::TTime initStart;
	initStart = ryzomGetLocalTime ();

	// Init LUA Scripting
	initLUA();

	// create the _UIStringMapper
	if (_UIStringMapper == NULL)
		_UIStringMapper = CStringMapper::createLocalMapper();

	// Clear the action manager
	Actions.clear();
	EditActions.clear();

	// Register action in the action manager
	ActionsContext.addActionsManager(&Actions, "");
	ActionsContext.addActionsManager(&EditActions, RZ_CATEGORY_EDIT);


	if (SoundMngr != NULL)
	{
		NLSOUND::UAudioMixer *pMixer = SoundMngr->getMixer();
		pMixer->loadSampleBank(false, "ui_ingame");
	}


//	NLMEMORY::CheckHeap (true);

	if (ClientCfg.XMLInterfaceFiles.size()==0)
	{
		nlinfo("no xml config files in client.cfg");
		return;
	}

	// Textures
	loadIngameInterfaceTextures();

//	NLMEMORY::CheckHeap (true);

	// Skill Manager Init
	CSkillManager *pSM = CSkillManager::getInstance();
	pSM->initInGame();

 	// SBrick Manager Init
	CSBrickManager *pSBM = CSBrickManager::getInstance();
 	pSBM->initInGame();
	pSM->initTitles();

 	// SPhrase Manager DB Init (BEFORE loading). Must be init AFTER skill and brick init
 	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	pPM->initInGame();

	// UI & keys
	loadUI();
	loadKeys();

	// Initialize armour color (config.xml)
	CDBCtrlSheet::initArmourColors();

	// Initialize inventory manager : link to DB and to interface element so must be here
	getInventory().init();
	// Same for temp inventory manager
	CTempInvManager::getInstance();
	// Initialize guild manager
	CGuildManager::getInstance();

//	NLMEMORY::CheckHeap (true);

	//init chat output
	ChatDisplayer = new CChatDisplayer;
	g_log.addDisplayer (ChatDisplayer);
	NLMISC::ErrorLog->addDisplayer (ChatDisplayer);
	NLMISC::WarningLog->addDisplayer (ChatDisplayer);
	NLMISC::InfoLog->addDisplayer (ChatDisplayer);
	NLMISC::DebugLog->addDisplayer (ChatDisplayer);
	NLMISC::AssertLog->addDisplayer (ChatDisplayer);

	// load bot chat datas
	//CBotChat::init();

	// init the bot chat
	nlassert (BotChatPageAll == NULL);
	BotChatPageAll = new CBotChatPageAll;
	BotChatPageAll->init();

//	NLMEMORY::CheckHeap (true);

	// init the list of people
	PeopleInterraction.init();

	// flush system msg buffer
	for( uint i=0; i<PeopleInterraction.SystemMessageBuffer.size(); ++i )
	{
		displaySystemInfo(PeopleInterraction.SystemMessageBuffer[i].Str, PeopleInterraction.SystemMessageBuffer[i].Cat);
	}
	PeopleInterraction.SystemMessageBuffer.clear();

	// Init macro manager
	CMacroCmdManager::getInstance()->initInGame();

	{
		H_AUTO( RZUpdAll )

		updateAllLocalisedElements(); // To init all things
	}

	// Interface config
	loadInterfaceConfig();

	// Must do extra init for people interaction after load
	PeopleInterraction.initAfterLoad();

	//CBotChatUI::refreshActiveWindows(); // bot chat windows are saved too..

	activateMasterGroup ("ui:interface", true);

	// Update the time in the ui database
	_CheckMailNode = getDbProp("UI:VARIABLES:MAIL_WAITING");
	_CheckForumNode = getDbProp("UI:VARIABLES:FORUM_UPDATED");

	// Init the action manager
	{

		initActions();
	}
	_InGame = true;

	// Init bubble manager
	InSceneBubbleManager.init();

	// Init Memory Bar for phraseManager. DB and ctrl gray state
	pPM->updateMemoryBar();

	// Init emotes
	_EmotesInitialized = false;
	initEmotes();

	// init chat manager
	ChatMngr.initInGame();

	// Init FlyingText manager
	FlyingTextManager.initInGame();

	// Init Bar Manager (HP, SAP etc... Bars)
	CBarManager::getInstance()->initInGame();

	// Init interface props linked to client time
	initClientTime();

	// Whole initInGame profile
	nlinfo ("%d seconds for initInGame", (uint32)(ryzomGetLocalTime ()-initStart)/1000);

	// reset the compass target
 	CGroupCompas *gc = dynamic_cast<CGroupCompas *>(getElementFromId("ui:interface:compass"));
	if (gc && gc->isSavedTargetValid())
	{
		gc->setTarget(gc->getSavedTarget());
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::loadIngameInterfaceTextures()
{
	nldebug("Textures Ingame Interface");

	for (vector<string>::iterator it = ClientCfg.TexturesInterface.begin(), end = ClientCfg.TexturesInterface.end(); it != end; ++it)
	{
		nldebug("Textures Ingame Interface: %s", (*it).c_str());
		loadTextures(*it + ".tga", *it + ".txt", false);
	}

	for (vector<string>::iterator it = ClientCfg.TexturesInterfaceDXTC.begin(), end = ClientCfg.TexturesInterfaceDXTC.end(); it != end; ++it)
	{
		nldebug("Textures Ingame Interface DXTC: %s", (*it).c_str());
		loadTextures(*it + ".tga", *it + ".txt", true);
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::loadUI()
{
	// Copy the array of file to load
	vector<string> xmlFilesToParse = getInGameXMLInterfaceFiles();

	if (!parseInterface (xmlFilesToParse, false))
	{
		badXMLParseMessageBox();
	}

	configureQuitDialogBox();
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::configureQuitDialogBox()
{
	// Configure the quit dialog box according to the server
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	string quitDialogMainStr = "ui:interface:quit_dialog";
	string quitDialogStr = quitDialogMainStr + ":indent_middle";
	CInterfaceGroup *quitDlg = dynamic_cast<CInterfaceGroup*>(pIM->getElementFromId(quitDialogStr));
	if (quitDlg)
	{
		CInterfaceElement *eltRet, *eltQuit, *eltQuitNow;
		eltRet = quitDlg->getElement(quitDialogStr+":return_mainland");
		eltQuit = quitDlg->getElement(quitDialogStr+":ryzom");
		eltQuitNow = quitDlg->getElement(quitDialogStr+":ryzom_now");
		sint buttonDeltaY;
		fromString(getDefine("quit_button_delta_y"), buttonDeltaY);
		extern R2::TUserRole UserRoleInSession;

		bool sessionOwner = (R2::getEditor().getMode() != R2::CEditor::NotInitialized && R2::getEditor().getDMC().getEditionModule().isSessionOwner());

		// Show Launch Editor if not in editor mode
		CInterfaceElement *eltCancel = quitDlg->getElement(quitDialogStr+":cancel");
		CInterfaceElement *eltEdit = quitDlg->getElement(quitDialogStr+":launch_editor");
		if (eltEdit)
		{
			if (UserRoleInSession != R2::TUserRole::ur_editor && !sessionOwner)
			{
				eltEdit->setY(buttonDeltaY);
				eltEdit->setActive(true);

				if (eltCancel)
					(safe_cast<CCtrlTextButton*>(eltCancel))->setText(CI18N::get("uittQuitCancel"));
			}
			else
			{
				eltEdit->setY(0); // prevent from displaying a gap between two shown buttons
				eltEdit->setActive(false);

				if (eltCancel)
					(safe_cast<CCtrlTextButton*>(eltCancel))->setText(sessionOwner ? CI18N::get("uittQuitCancel") : CI18N::get("uittQuitCancelEditor"));
			}
		}

		// Other buttons
		if (IsInRingSession || (ClientCfg.Local && ClientCfg.R2EDEnabled))
		{
			// display "return to mainland", unless we are the scenario owner (player or 'aventure master')
			if (eltRet)
			{
				if (!sessionOwner || R2::getEditor().getMode()==R2::CEditor::EditionMode)
				{
					//eltRet->setY(buttonDeltaY);
					const char *textLabel = (UserRoleInSession == R2::TUserRole::ur_editor) ? "uittLeaveEditor" : "uittReturnToMainland";
					(safe_cast<CCtrlTextButton*>(eltRet))->setText(CI18N::get(textLabel));
					eltRet->setY(buttonDeltaY);
					eltRet->setActive(true); // show Return to Mainland / PLAY
				}
				else
				{
					eltRet->setY(0);
					// when an owner of the session, there's an additionnal 'stop' button
					eltRet->setActive(false);
				}
			}
			if (eltQuit)
			{
				eltQuit->setY(0);
				eltQuit->setActive(false);
			}
			if (eltQuitNow)
			{
				eltQuitNow->setY(buttonDeltaY);
				eltQuitNow->setActive(true); // show Quit (Now)
			}
		}
		else
		{
			if (eltRet)
			{
				eltRet->setY(0); // prevent from displaying a gap between two shown buttons
				eltRet->setActive(false);
			}
			if (eltQuit)
			{
				eltQuit->setY(buttonDeltaY);
				eltQuit->setActive(true); // show Quit (with progress bar)
			}
			if (eltQuitNow)
			{
				eltQuitNow->setY(0);
				eltQuitNow->setActive(false);
			}
		}
	}

	// Make all controls have the same size
	CInterfaceGroup *quitDlgMain = dynamic_cast<CInterfaceGroup*>(pIM->getElementFromId(quitDialogMainStr));
	if ( quitDlgMain )
	{

		quitDlgMain->invalidateCoords();
		for (uint k = 0; k < 3; ++k)
		{
			quitDlgMain->updateCoords(); // calculate the width of the text buttons
		}
	}
	const std::vector<CCtrlBase*>& controls = quitDlg->getControls();
	sint32 biggestWidth = 0;
	for ( std::vector<CCtrlBase*>::const_iterator ic=controls.begin(); ic!=controls.end(); ++ic )
	{
		CCtrlTextButton *ctb = dynamic_cast<CCtrlTextButton*>(*ic);
		if ( ! ctb )
			continue;

		if ( ctb->getW() > biggestWidth )
			biggestWidth = ctb->getW();
	}
	for ( std::vector<CCtrlBase*>::const_iterator ic=controls.begin(); ic!=controls.end(); ++ic )
	{
		CCtrlTextButton *ctb = dynamic_cast<CCtrlTextButton*>(*ic);
		if ( ! ctb )
			continue;

		ctb->setWMin( biggestWidth );
	}

	if ( quitDlgMain )
	{

		quitDlgMain->invalidateCoords();
		for (uint k = 0; k < 3; ++k)
		{
			quitDlgMain->updateCoords(); // calculate the width of the text buttons
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::loadKeys()
{
	if (ClientCfg.R2EDEnabled) // in R2ED mode the CEditor class deals with it
		return;

	CMacroCmdManager::getInstance()->removeAllMacros();

	vector<string> xmlFilesToParse;

	// Does the keys file exist ?
	string userKeyFileName = "save/keys_"+PlayerSelectedFileName+".xml";
	if (CFile::fileExists(userKeyFileName) && CFile::getFileSize(userKeyFileName) > 0)
	{
		// Load the user key file
		xmlFilesToParse.push_back (userKeyFileName);
	}
	// Load the default key (but don't replace existings bounds, see keys.xml "key_def_no_replace")
	xmlFilesToParse.push_back ("keys.xml");

	if (!parseInterface (xmlFilesToParse, true))
	{
		badXMLParseMessageBox();
	}

	_KeysLoaded = true;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::loadInterfaceConfig()
{
	// Load interface.cfg
	if (ClientCfg.R2EDEnabled) // in R2ED mode the CEditor class deals with it
		return;

	loadConfig ("save/interface_" + PlayerSelectedFileName + ".icfg"); // Invalidate coords of changed groups

	_ConfigLoaded = true;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::uninitInGame0 ()
{

	// Autosave of the keys
	if (_KeysLoaded)
	{
		if (!ClientCfg.R2EDEnabled)
		{
			saveKeys ("save/keys_" + PlayerSelectedFileName + ".xml");
		}
		_KeysLoaded = false;
	}

	// Autosave of the interface in interface.cfg
	if (_ConfigLoaded)
	{
		if (!ClientCfg.R2EDEnabled)
		{
			saveConfig ("save/interface_" + PlayerSelectedFileName + ".icfg");
		}
		_ConfigLoaded = false;
	}
}


// ------------------------------------------------------------------------------------------------
void CInterfaceManager::uninitInGame1 ()
{

	// release Bar Manager (HP, SAP etc... Bars)
	CBarManager::getInstance()->releaseInGame();

	// release FlyingTextManager
	FlyingTextManager.releaseInGame();

	// release chat manager
	ChatMngr.releaseInGame();

	// Reset the chat text manager
	CChatTextManager::getInstance().reset();

	// release emotes
	uninitEmotes();

	// release bubble manager
	InSceneBubbleManager.release();

	// kill chat displayer
	if( ChatDisplayer )
	{
		DebugLog->removeDisplayer (ChatDisplayer);
		InfoLog->removeDisplayer (ChatDisplayer);
		WarningLog->removeDisplayer (ChatDisplayer);
		ErrorLog->removeDisplayer (ChatDisplayer);
		AssertLog->removeDisplayer (ChatDisplayer);
		g_log.removeDisplayer(ChatDisplayer);
		delete ChatDisplayer;
		ChatDisplayer = NULL;
	}

	// Release inventory manager
	CInventoryManager::releaseInstance();
	// Same for temp inventory manager
	CTempInvManager::releaseInstance();
	// Release guild manager
	CGuildManager::release();

	// release bot chat and manager
	CBotChatManager::getInstance()->setCurrPage(NULL);
	delete BotChatPageAll;
	BotChatPageAll = NULL;
	CBotChatManager::releaseInstance();

	//release CInterfaceItemEdition
	CInterfaceItemEdition::getInstance()->setCurrWindow(NULL);
	CInterfaceItemEdition::releaseInstance();

	// release task bar manager
	CTaskBarManager::releaseInstance();

	// People inetrraction release
	PeopleInterraction.release();

	if (SoundMngr != NULL)
	{
		NLSOUND::UAudioMixer *pMixer = SoundMngr->getMixer();
		pMixer->unloadSampleBank("ui_ingame");
	}

	// disable the game_quitting modal window
	disableModalWindow();

	// Remove all interface objects (containers, groups, variables, defines, ...)
	activateMasterGroup ("ui:interface", false);
	removeAll();
	reset();
	CInterfaceLink::removeAllLinks();

	// Release DDX manager, before DB remove
	CDDXManager::getInstance()->release();

	// Release client time, before DB remove
	releaseClientTime();

	// remove DB entry
	ICDBNode::CTextId textId("UI");
	_DbRootNode->removeNode(textId);

	// Uninit the action manager
	{

		uninitActions();
	}

	// uninit phrase mgr
	CSPhraseManager	*pPM = CSPhraseManager::getInstance();
	pPM->reset();
	CSPhraseManager::releaseInstance(); // must release before BrickManager, SkillManager

	// uninit brick manager
	// Don't release the instance because must not lost brick map and data
	CSBrickManager::getInstance()->uninitInGame();

	// Uninit skill manager (after phrase mgr)
	// AJM don't release SkillManager, else impulse msg update guild title will crash :(
	CSkillManager::getInstance()->uninitTitles();

	// Uninit macro manager
	CMacroCmdManager::getInstance()->uninitInGame();

	// Release interface help
	CInterfaceHelp::release();

	// Release guild manager
	CGuildManager::release();

	// Close LUA Scripting
	uninitLUA();

	_InGame = false;
	_NeutralColor = NULL;
	_WarningColor = NULL;
	_ErrorColor = NULL;
	_AlphaRolloverSpeedDB = NULL;
	_RProp = NULL;
	_GProp = NULL;
	_BProp = NULL;
	_AProp = NULL;

#ifdef AJM_DEBUG_TRACK_INTERFACE_GROUPS
	CInterfaceManager::getInstance()->DebugTrackGroupsDump();

//	NLMEMORY::ReportMemoryLeak();
#endif

}


// ------------------------------------------------------------------------------------------------
void CInterfaceManager::flushDebugWindow()
{
	if (ChatDisplayer != NULL)
		ChatDisplayer->update();
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::updateFrameEvents()
{

	H_AUTO_USE ( RZ_Client_Update_Frame_Events )

	flushDebugWindow();

	// Handle anims done in 2 times because some AH can add or remove anims
	// First ensure we are working on a safe vector and update all anims
	sint i;
	vector<CInterfaceAnim*> vTmp = _ActiveAnims;
	for (i = 0; i < (sint)vTmp.size(); ++i)
	{
		CInterfaceAnim *pIA = vTmp[i];
		pIA->update();
	}
	CCDBNodeBranch::flushObserversCalls();
	//
	// Next : Test if we have to remove anims
	for (i = 0; i < (sint)_ActiveAnims.size(); ++i)
	{
		CInterfaceAnim *pIA = _ActiveAnims[i];
		if (pIA->isFinished())
		{
			_ActiveAnims.erase (_ActiveAnims.begin()+i);
			--i;
		}
	}
	//
	CCDBNodeBranch::flushObserversCalls();

	// Handle waiting texts from server
	processServerIDString();

	if (_InGame)
	{
		// Execute current macro
		CMacroCmdManager::getInstance()->updateMacroExecution();

		// Update guild handling (check if we have to rebuild)
		CGuildManager::getInstance()->update();

		// Update contact list with incoming server string ids
		PeopleInterraction.updateWaitingContacts();

		// Connect and receive/send to the yubo chat
		checkYuboChat();

		// Update string if some waiting
		CEncyclopediaManager::getInstance()->updateAllFrame();

		// Setup the weather setup in the player's map
		if ((T0 - _UpdateWeatherTime) > (1 * 5 * 1000))
		{
			_UpdateWeatherTime = T0;
			ucstring str =	CI18N::get ("uiTheSeasonIs") +
							CI18N::get ("uiSeason"+toStringEnum(computeCurrSeason())) +
							CI18N::get ("uiAndTheWeatherIs") +
							CI18N::get (WeatherManager.getCurrWeatherState().LocalizedName);


			CViewText *pVT = dynamic_cast<CViewText*>(getElementFromId("ui:interface:map:content:map_content:weather"));
			if (pVT != NULL)
				pVT->setText(str);

			// The date feature is temporarily disabled
			str.clear();

			// numeric version
			//str = CI18N::get("uiDate");
			//str += toString("%04d", RT.getRyzomYear()) + "/";
			//str += toString("%01d", RT.getRyzomCycle()+1) + " : ";
			//str += toString("%02d", RT.getRyzomMonthInCurrentCycle()+1) + "/";
			//str += toString("%02d", RT.getRyzomDayOfMonth()+1) + " - "; // Start at 1 for January
			//str += toString("%02d", (sint)RT.getRyzomTime()) + " " + CI18N::get("uiMissionTimerHour");

			// literal version
			// str = CI18N::get("uiDate");
			str += toString("%02d", (sint)RT.getRyzomTime()) + CI18N::get("uiMissionTimerHour") + " - ";
			str += CI18N::get("ui"+WEEKDAY::toString( (WEEKDAY::EWeekDay)RT.getRyzomDayOfWeek() )) + ", ";
			str += CI18N::get("ui"+MONTH::toString( (MONTH::EMonth)RT.getRyzomMonthInCurrentCycle() )) + " ";
			str += toString("%02d", RT.getRyzomDayOfMonth()+1) + ", ";
			str += CI18N::get("uiAtysianCycle" + toString(RT.getRyzomCycle()+1) + "Ordinal") + " " + CI18N::get("uiAtysianCycle") + " ";
			str += toString("%04d", RT.getRyzomYear());

			pVT = dynamic_cast<CViewText*>(getElementFromId("ui:interface:map:content:map_content:time"));
			if (pVT != NULL)
				pVT->setText(str);

			str.clear();
			// Update the clock in the compass if enabled.
			pVT = dynamic_cast<CViewText*>(getElementFromId("ui:interface:compass:clock:time"));
			if (pVT != NULL)
			{
				if (pVT->getActive())
				{
					str = getTimestampHuman("%H:%M");
					pVT->setText(str);
				}
			}
		}
	}

	// send clock tick msg to ctrl that are captured
	CEventDescriptorSystem clockTick;
	clockTick.setEventTypeExtended(CEventDescriptorSystem::clocktick);
	if (_CapturePointerLeft)
	{
		_CapturePointerLeft->handleEvent(clockTick);
	}
	if (_CapturePointerRight)
	{
		_CapturePointerRight->handleEvent(clockTick);
	}

	// and send clock tick msg to ctrl that are registered
	std::vector<CCtrlBase*> clockMsgTarget = _ClockMsgTargets;
	for(std::vector<CCtrlBase*>::iterator it = clockMsgTarget.begin(); it != clockMsgTarget.end(); ++it)
	{
		(*it)->handleEvent(clockTick);
	}
	CCDBNodeBranch::flushObserversCalls();

 	// Update SPhrase manager
 	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	pPM->update();

	// if there's an external lua debugger, update it
	extern void luaDebuggerMainLoop();
	luaDebuggerMainLoop();

	// handle gc for lua
	if (_LuaState) _LuaState->handleGC();

	CBGDownloaderAccess::getInstance().update();

}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::updateFrameViews(NL3D::UCamera camera)
{

	H_AUTO ( RZ_Interface_updateFrameViews )

	if (!camera.empty())
		_ViewRenderer.setWorldSpaceFrustum (camera.getFrustum());

	checkCoords();
	drawViews(camera);

	// The interface manager may change usual Global setup. reset them.
	TextContext->setShadeColor(CRGBA::Black);

}

// ------------------------------------------------------------------------------------------------
CInterfaceOptions *CInterfaceManager::getOptions(const string &name)
{
	map<string, NLMISC::CSmartPtr<CInterfaceOptions> >::iterator it = _OptionsMap.find(name);
	if (it == _OptionsMap.end())
		return NULL;
	else
		return it->second;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::runActionHandler (const string &ahCmdLine, CCtrlBase *pCaller, const string &ahUserParams)
{
	if (ahCmdLine.empty()) return;

	// Special AH form ("ah:params") ?
	string::size_type i = ahCmdLine.find(':');
	string	ahName;
	string	ahParams;
	if(i!=string::npos)
	{
		ahName= ahCmdLine.substr(0, i);
		ahParams= ahCmdLine.substr(i+1);
	}
	else
	{
		ahName= ahCmdLine;
	}

	// Replace params if defined
	if(!ahUserParams.empty())
		ahParams= ahUserParams;

	// Execute the action handler
	CActionHandlerFactoryManager *pAHFM = CActionHandlerFactoryManager::getInstance();
	map<string, IActionHandler*>::iterator it = pAHFM->FactoryMap.find (ahName);
	if (it == pAHFM->FactoryMap.end())
	{
		nlwarning ("not found action handler : %s",ahName.c_str());
		return;
	}
	IActionHandler *pAH = it->second;
	pAH->execute (pCaller, ahParams);

	// Quick Help
	const string submitQuickHelp = "submit_quick_help";
	it = pAHFM->FactoryMap.find(submitQuickHelp);
	if(it == pAHFM->FactoryMap.end())
	{
		nlwarning ("not found action handler : %s", submitQuickHelp.c_str());
		return;
	}
	pAH = it->second;
	const std::string event = ahName + ":" + ahParams;
	pAH->execute(NULL, event);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::runActionHandler (IActionHandler *pAH, CCtrlBase *pCaller, const std::string &Params)
{
	if (pAH == NULL)
	{
		nlwarning ("no action handler");
		return;
	}
	pAH->execute (pCaller, Params);
	string AHName = getAHName(pAH);

	// Quick Help
	const string submitQuickHelp = "submit_quick_help";
	CActionHandlerFactoryManager *pAHFM = CActionHandlerFactoryManager::getInstance();
	map<string, IActionHandler*>::iterator it = pAHFM->FactoryMap.find (AHName);
	it = pAHFM->FactoryMap.find(submitQuickHelp);
	if(it == pAHFM->FactoryMap.end())
	{
		nlwarning ("not found action handler : %s", submitQuickHelp.c_str());
		return;
	}
	pAH = it->second;
	const std::string event = AHName + ":" + Params;
	pAH->execute(NULL, event);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::setupOptions()
{
	// After parsing options and templates node -> init system options.
	CInterfaceOptions	*opt= getOptions("system");
	if(opt)
	{
		// List here all Special options
		_SystemOptions[OptionCtrlSheetGrayColor]= opt->getValue("ctrl_sheet_gray_color");
		_SystemOptions[OptionCtrlTextGrayColor]= opt->getValue("ctrl_text_gray_color");
		_SystemOptions[OptionCtrlSheetRedifyColor]= opt->getValue("ctrl_sheet_redify_color");
		_SystemOptions[OptionCtrlTextRedifyColor]= opt->getValue("ctrl_text_redify_color");
		_SystemOptions[OptionCtrlSheetGreenifyColor]= opt->getValue("ctrl_sheet_greenify_color");
		_SystemOptions[OptionCtrlTextGreenifyColor]= opt->getValue("ctrl_text_greenify_color");
		_SystemOptions[OptionViewTextOverBackColor]= opt->getValue("text_over_back_color");
		_SystemOptions[OptionFont]= opt->getValue("font");
		_SystemOptions[OptionAddCoefFont]= opt->getValue("add_coef_font");
		_SystemOptions[OptionMulCoefAnim]= opt->getValue("mul_coef_anim");
		_SystemOptions[OptionTimeoutBubbles]= opt->getValue("bubbles_timeout");
		_SystemOptions[OptionTimeoutMessages]= opt->getValue("messages_timeout");
		_SystemOptions[OptionTimeoutContext]= opt->getValue("context_timeout");
		_SystemOptions[OptionTimeoutContextHtml]= opt->getValue("context_html_timeout");
	}

	// Try to change font if any
	string sFont = _SystemOptions[OptionFont].getValStr();
	if ((!sFont.empty()) && (Driver != NULL))
		resetTextContext(sFont.c_str(), true);

	// Continue to parse the rest of the interface
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::parseInterface (const std::vector<std::string> &xmlFileNames, bool reload, bool isFilename)
{
	// cache some commonly used db nodes
	_DBB_UI_DUMMY = getDbBranch( "UI:DUMMY" );
	_DB_UI_DUMMY_QUANTITY = getDbProp( "UI:DUMMY:QUANTITY", true );
	_DB_UI_DUMMY_QUALITY = getDbProp( "UI:DUMMY:QUALITY", true );
	_DB_UI_DUMMY_SHEET = getDbProp( "UI:DUMMY:SHEET", true );
	_DB_UI_DUMMY_NAMEID = getDbProp( "UI:DUMMY:NAMEID", true );
	_DB_UI_DUMMY_ENCHANT = getDbProp( "UI:DUMMY:ENCHANT", true );
	_DB_UI_DUMMY_SLOT_TYPE = getDbProp( "UI:DUMMY:SLOT_TYPE", true );
	_DB_UI_DUMMY_PHRASE = getDbProp( "UI:DUMMY:PHRASE", true );
	_DB_UI_DUMMY_WORNED = getDbProp( "UI:DUMMY:WORNED", true );
	_DB_UI_DUMMY_PREREQUISIT_VALID = getDbProp( "UI:DUMMY:PREREQUISIT_VALID", true );
	_DB_UI_DUMMY_FACTION_TYPE = getDbProp( "UI:DUMMY:FACTION_TYPE", true );

	_DB_UI_DUMMY_QUANTITY->setValue64(0);
	_DB_UI_DUMMY_QUALITY->setValue64(0);
	_DB_UI_DUMMY_SHEET->setValue64(0);
	_DB_UI_DUMMY_NAMEID->setValue64(0);
	_DB_UI_DUMMY_ENCHANT->setValue64(0);
	_DB_UI_DUMMY_SLOT_TYPE->setValue64(0);
	_DB_UI_DUMMY_PHRASE->setValue64(0);
	_DB_UI_DUMMY_WORNED->setValue64(0);
	_DB_UI_DUMMY_PREREQUISIT_VALID->setValueBool(true);
	_DB_UI_DUMMY_FACTION_TYPE->setValue64(0);

	return CInterfaceParser::parseInterface (xmlFileNames, reload, isFilename);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::loadTextures (const string &textFileName, const string &uvFileName, bool uploadDXTC)
{
	_ViewRenderer.loadTextures (textFileName, uvFileName, uploadDXTC);
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::loadConfig (const string &filename)
{
	// reset the interface
	vector<string> v;
	if (ClientCfg.R2EDEnabled)
	{
		runProcedure ("proc_reset_r2ed_interface", NULL, v);
	}
	else
	{
		runProcedure ("proc_reset_interface", NULL, v);
	}

	// By default, consider the reset interface has been set with the current resolution
	{
		uint32	w,h;
		// NB: even if minimzed, getScreenSize() no more return 0 values (return the last setuped screen size)
		_ViewRenderer.getScreenSize(w, h);
		// Windows are positioned according to resolution, and we must backup W/H for the system that move windows when the resolution change
		_LastInGameScreenW= w;
		_LastInGameScreenH= h;
	}

	// if the config file doesn't exist,just quit
	CIFile f;
	string sFileName;
	sFileName = NLMISC::CPath::lookup (filename, false);
	if (sFileName == "" || !f.open(sFileName))
		return false;


	// *** Load the config file
	uint32	nNbMode;
	CInterfaceConfig ic;
	bool	lastInGameScreenResLoaded= false;
	try
	{
		sint ver = f.serialVersion(ICFG_STREAM_VERSION);

		// serial user chats info (serial it before position of windows so that they can be updated properly)
		if (ver >= 1)
		{
			f.serialCheck(uint32('_ICU'));
			if (!PeopleInterraction.loadUserChatsInfos(f))
			{
				nlwarning("Bad user chat saving");
			}
		}

		// header
		f.serialCheck(uint32('GFCI'));
		f.serial(nNbMode);
		f.serial(_CurrentMode);
		if(ver>=10)
		{
			f.serial(_LastInGameScreenW);
			f.serial(_LastInGameScreenH);
			lastInGameScreenResLoaded= true;
		}

		// Load All Window configuration of all Modes
		for (uint32 i = 0; i < nNbMode; ++i)
		{
			NLMISC::contReset(_Modes[i]);
			// must create a tmp mem stream because desktop image expect its datas to occupy the whole stream
			// This is because of old system that manipulated desktop image direclty as a mem stream
			CMemStream ms;
			if (!ms.isReading()) ms.invert();
			uint32 length;
			f.serial(length);
			if (length > 0)
			{
				// HACK. if the version is <=2, then the CInterfaceConfig has no serialVersion. append here a 0
				if(ver<=2)
				{
					uint8 *pBuffer = ms.bufferToFill(length+1);
					pBuffer[0]= 0;
					f.serialBuffer(pBuffer+1, length);
				}
				else
				{
					uint8 *pBuffer = ms.bufferToFill(length);
					f.serialBuffer(pBuffer, length);
				}
			}
			ms.seek(0, NLMISC::IStream::begin);
			_Modes[i].serial(ms); // build desktop image from stream
		}

		// load UI_DB_SAVE_VERSION
		uint32	uiDbSaveVersion= 0;	// default to 0 for old version of .icfg
		if(ver>=9)
		{
			f.serial(uiDbSaveVersion);
		}

		// read database
		ic.streamToDataBase(f, uiDbSaveVersion);


		// special for in game: backup last mission because of delayed update
		{
			CCDBNodeLeaf *pNL = getDbProp("UI:SAVE:MISSION_SELECTED", false);
			if (pNL)
			{
				CCDBNodeLeaf *pSelectedMissionBackup = getDbProp("UI:VARIABLES:MISSION_SELECTED_PREV_SESSION", true);
				pSelectedMissionBackup->setValue64(pNL->getValue64());
			}
		}
		//

		// Deprecated. for Compatibility purpose: Load TaskBar.
		if(ver>=2)
		{
			CTaskBarManager	*pTBM= CTaskBarManager::getInstance();
			pTBM->serial(f);
		}

		// Load user landmarks
		ContinentMngr.serialUserLandMarks(f);

		CCDBNodeLeaf *pNL = getDbProp( "SERVER:INTERFACES:NB_BONUS_LANDMARKS" );
		if ( pNL )
		{
			ICDBNode::CTextId textId;
			pNL->addObserver( &_LandmarkObs, textId);
		}

		// Info Windows position.
		if(ver>=5)
			CInterfaceHelp::serialInfoWindows(f);
		else // Default pos
			CInterfaceHelp::resetWindowPos(-100);

		// Macro On Memory Position
		CSPhraseManager		*pPM = CSPhraseManager::getInstance();
		if(ver>=7)
			pPM->serialMacroMemory(f);

		if(ver>=8)
			CGroupInSceneBubbleManager::serialInSceneBubbleInfo(f);

		if (ver >= 11)
		{
			if ( ! PeopleInterraction.loadUserDynChatsInfos(f))
			{
				nlwarning("Bad user dyn chat saving");
			}
		}
	}
	catch(const NLMISC::EStream &)
	{
		f.close();
		string	sFileNameBackup = sFileName+"backup";
		if (CFile::fileExists(sFileNameBackup))
			CFile::deleteFile(sFileNameBackup);
		CFile::moveFile(sFileNameBackup.c_str(), sFileName.c_str());
		nlwarning("Config loading failed : restore default");
		vector<string> v;
		if (!ClientCfg.R2EDEnabled)
		{
			runProcedure ("proc_reset_interface", NULL, v);
		}
		return false;
	}
	f.close();

	// *** If saved resolution is different from the current one setuped, must fix positions in _Modes
	if(lastInGameScreenResLoaded)
	{
		// NB: we are typically InGame here (even though the _InGame flag is not yet set)
		// Use the screen size of the config file. Don't update current UI, just _Modes
		moveAllWindowsToNewScreenSize(ClientCfg.Width, ClientCfg.Height, false);
	}

	// *** apply the current mode
	_Modes[_CurrentMode].toCurrentDesktop();

	// *** Apply the NPC icon display mode
	CNPCIconCache::getInstance().init(!ClientCfg.R2EDEnabled && getDbProp("UI:SAVE:INSCENE:FRIEND:MISSION_ICON")->getValueBool());

	return true;
}


// ------------------------------------------------------------------------------------------------
void CInterfaceManager::CDBLandmarkObs::update(ICDBNode *node)
{
	uint nbBonusLandmarks = ((CCDBNodeLeaf*)node)->getValue32();
	ContinentMngr.checkNumberOfUserLandmarks( STANDARD_NUM_USER_LANDMARKS + nbBonusLandmarks );
}


// visitor to send onQuit msg on all element
class CQuitVisitor : public CInterfaceElementVisitor
{
public:
	bool IsR2ED;
	bool BadWindowFound; //
	uint Desktop;
	virtual void visit(CInterfaceElement *elem)	{ elem->onQuit(); }
	virtual void visitGroup(CInterfaceGroup *group)
	{
		if (!IsR2ED) return;
		if (Desktop != 0) return;
		if (group->getShortId() == "gestionsets")
		{
			if (group->getActive())
			{
				BadWindowFound = true;
			}
		}
	}
};


// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::saveConfig (const string &filename)
{

	CQuitVisitor quitVisitor;

	quitVisitor.IsR2ED = false;
	if (nlstricmp(NLMISC::CFile::getFilename(filename).substr(0, 5), "r2ed_") == 0)
	{
		quitVisitor.IsR2ED = true;
	}
	quitVisitor.BadWindowFound = false;

	nlinfo( "Saving interface config : %s", filename.c_str() );

	COFile f;

	if (!f.open(filename)) return false;

	CInterfaceConfig ic;



	// cleanup all desktops
	for(uint k = 0; k < MAX_NUM_MODES; ++k)
	{
		quitVisitor.Desktop = k;
		setMode(k);
		visit(&quitVisitor);
		checkCoords();
	}
	setMode(0);
	setMode(_CurrentMode);

	if (quitVisitor.BadWindowFound)
	{
		#ifdef NL_DEBUG
			nlassert(0);
		#endif
		// tmp patch : when trying to overwrite the r2ed_ config, if a bad window is found, just do nothing ...
		return true;
	}

	/*
	_Modes[_CurrentMode].clear();
	if (_Modes[_CurrentMode].isReading()) _Modes[_CurrentMode].invert();
	clearAllEditBox();
	restoreAllContainersBackupPosition();
	ic.interfaceManagerToStream(_Modes[_CurrentMode]);
	*/

	uint32 i;

	i = MAX_NUM_MODES;
	try
	{
		f.serialVersion(ICFG_STREAM_VERSION);

		// serial user chats info (serial it before position of windows so that they can be updated properly)
		f.serialCheck(uint32('_ICU'));
		if (!PeopleInterraction.saveUserChatsInfos(f))
		{
			nlwarning("Config saving failed");
			// couldn't save result so do not continue
			f.close();
			return false;
		}

		// header
		f.serialCheck(uint32('GFCI'));
		f.serial(i);
		f.serial(_CurrentMode);
		f.serial(_LastInGameScreenW);
		f.serial(_LastInGameScreenH);

		// Save All Window configuration of all Modes
		for (i = 0; i < MAX_NUM_MODES; ++i)
		{
			// must create a tmp mem stream because desktop image expect its datas to occupy the whole stream
			// This is because of old system that manipulated desktop image direclty as a mem stream
			CMemStream ms;
			if (ms.isReading()) ms.invert();
			_Modes[i].serial(ms);
			uint32 length = ms.length();
			f.serial(length);
			if (length > 0)
			{
				f.serialBuffer(const_cast<uint8 *>(ms.buffer()), length);
			}
		}

		// write UI_DB_SAVE_VERSION
		uint32	uiDbSaveVersion;
		fromString(getDefine("UI_DB_SAVE_VERSION"), uiDbSaveVersion);
		f.serial(uiDbSaveVersion);

		// write database
		ic.dataBaseToStream(f);

		// Deprecated. for Compatibility purpose: Save TaskBar.
		CTaskBarManager	*pTBM= CTaskBarManager::getInstance();
		pTBM->serial(f);

		// Save user landmarks
		ContinentMngr.serialUserLandMarks(f);

		// Info Windows position.
		CInterfaceHelp::serialInfoWindows(f);

		// Macro On Memory Position
		CSPhraseManager		*pPM = CSPhraseManager::getInstance();
		pPM->serialMacroMemory(f);

		CGroupInSceneBubbleManager::serialInSceneBubbleInfo(f);

		if ( ! PeopleInterraction.saveUserDynChatsInfos(f))
		{
			nlwarning("Bad user dyn chat saving");
			return false;
		}
	}
	catch(const NLMISC::EStream &)
	{
		f.close();
		nlwarning("Config saving failed.");
		return false;
	}
	f.close();

	ContinentMngr.serialFOWMaps();

	return true;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::checkCoords()
{
	H_AUTO ( RZ_Interface_validateCoords )

	uint32 nMasterGroup;

	{
		H_AUTO ( RZ_Interface_checkCoords )

		// checkCoords all the windows
		for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
				{
					list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
					list<CInterfaceGroup*>::const_iterator itw;
					for (itw = rList.begin(); itw!= rList.end();)
					{
						CInterfaceGroup *pIG = *itw;
						itw++;	// since checkCoords invalidate the iterator, be sure we move to the next one before
						if (pIG->getActive())
							pIG->checkCoords ();
					}
				}
			}
		}
	}

	bool bRecomputeCtrlUnderPtr = false;
	{
		H_AUTO ( RZ_Interface_updateCoords )

		// updateCoords all the needed windows
		for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
				{
					list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
					list<CInterfaceGroup*>::const_iterator itw;
					for (itw = rList.begin(); itw!= rList.end(); itw++)
					{
						CInterfaceGroup *pIG = *itw;
						bool		updateCoordCalled= false;
						// updateCoords the window only if the master group is his parent and if need it
						// do it until updateCoords() no more invalidate coordinates!!
						while (pIG->getParent()==rMG.Group && (pIG->getInvalidCoords()>0))
						{
							bRecomputeCtrlUnderPtr = true;
							// Update as many pass wanted (3 time for complex resizing, 1 for scroll for example)
							uint	numPass= pIG->getInvalidCoords();
							// reset before updateCoords
							pIG->resetInvalidCoords();
							for(uint i=0;i<numPass;i++)
							{
								pIG->updateCoords ();
							}
							updateCoordCalled= true;
						}
						// If the group need to update pos each frame (eg: CGroupInScene),
						// and updateCoords not called
						if(pIG->getParent()==rMG.Group && !updateCoordCalled && pIG->isNeedFrameUpdatePos())
						{
							// This Group will compute the delta to apply.
							pIG->onFrameUpdateWindowPos(0,0);
						}
					}
				}
			}
		}

		if (_Pointer != NULL)
		{
			_Pointer->updateCoords();
		}
	}



	if (bRecomputeCtrlUnderPtr)
	{
		H_AUTO ( RZ_Interface_RecomputeCtrlUnderPtr )
		if (_Pointer)
		{
			sint32 mx = _Pointer->getX();
			sint32 my = _Pointer->getY();
			getViewsUnder (mx, my, _ViewsUnderPointer);
			getCtrlsUnder (mx, my, _CtrlsUnderPointer);
			getGroupsUnder (mx, my, _GroupsUnderPointer);
			CInterfaceGroup *ptr = getWindowUnder (mx, my);
			_WindowUnder = ptr?ptr->getId():"";
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::drawViews(NL3D::UCamera camera)
{
	{
		H_AUTO ( RZ_Interface_DrawViews_Setup )

		_ViewRenderer.activateWorldSpaceMatrix (false);

		CCDBNodeBranch::flushObserversCalls();

		// If an element has captured the keyboard, make sure it is alway visible (all parent windows active)
		if (_CaptureKeyboard != NULL)
		{
			CCtrlBase *cb = _CaptureKeyboard;
			do
			{
				if (!cb->getActive())
				{
					setCaptureKeyboard(NULL);
					break;
				}
				cb = cb->getParent();
			}
			while (cb);
		}
		// Check if screen size changed
		uint32 w, h;
		_ViewRenderer.checkNewScreenSize ();
		_ViewRenderer.getScreenSize (w, h);
		if ((w != _ScreenW) || (h != _ScreenH))
		{
			// No Op if screen minimized
			if(w!=0 && h!=0 && !_ViewRenderer.isMinimized())
			{
				updateAllLocalisedElements ();
				_ScreenW = w;
				_ScreenH = h;
			}
		}

		// Update global color from database
		_GlobalColor = CRGBA (	(uint8)getDbProp("UI:SAVE:COLOR:R")->getValue32(),
								(uint8)getDbProp("UI:SAVE:COLOR:G")->getValue32(),
								(uint8)getDbProp("UI:SAVE:COLOR:B")->getValue32(),
								(uint8)getDbProp("UI:SAVE:COLOR:A")->getValue32()  );
		_GlobalColorForContent.R = _GlobalColor.R;
		_GlobalColorForContent.G = _GlobalColor.G;
		_GlobalColorForContent.B = _GlobalColor.B;
		_GlobalColorForContent.A = (uint8) (( (uint16) _GlobalColor.A * (uint16) _ContentAlpha) >> 8);

		// Update global alphaS from database
		_GlobalContentAlpha = (uint8)getDbProp("UI:SAVE:CONTENT_ALPHA")->getValue32();
		_GlobalContainerAlpha = (uint8)getDbProp("UI:SAVE:CONTAINER_ALPHA")->getValue32();
		_GlobalRolloverFactorContent = (uint8)getDbProp("UI:SAVE:CONTENT_ROLLOVER_FACTOR")->getValue32();
		_GlobalRolloverFactorContainer = (uint8)getDbProp("UI:SAVE:CONTAINER_ROLLOVER_FACTOR")->getValue32();

		// Update Player characteristics (for Item carac requirement Redifying)
		nlctassert(CHARACTERISTICS::NUM_CHARACTERISTICS==8);
		for (uint i=0; i<CHARACTERISTICS::NUM_CHARACTERISTICS; ++i)
			_CurrentPlayerCharac[i]= getDbValue32(toString("SERVER:CHARACTER_INFO:CHARACTERISTICS%d:VALUE", i));

//		_CurrentPlayerCharac[CHARACTERISTICS::constitution]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:Constitution");
//		_CurrentPlayerCharac[CHARACTERISTICS::constitution]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:Constitution");
//		_CurrentPlayerCharac[CHARACTERISTICS::metabolism]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:Metabolism");
//		_CurrentPlayerCharac[CHARACTERISTICS::intelligence]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:Intelligence");
//		_CurrentPlayerCharac[CHARACTERISTICS::wisdom]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:Wisdom");
//		_CurrentPlayerCharac[CHARACTERISTICS::strength]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:Strength");
//		_CurrentPlayerCharac[CHARACTERISTICS::well_balanced]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:WellBalanced");
//		_CurrentPlayerCharac[CHARACTERISTICS::dexterity]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:Dexterity");
//		_CurrentPlayerCharac[CHARACTERISTICS::will]= getDbValue32("SERVER:CHARACTER_INFO:CHARACTERISTICS:Will");
	}


	{
		H_AUTO ( RZ_Interface_DrawWindows )

		/*  Draw all the windows
			To minimize texture swapping, we first sort per Window, then we sort per layer, then we render per Global Texture.
			Computed String are rendered in on big drawQuads at last part of each layer
		*/
		CCDBNodeBranch::flushObserversCalls();
		//
		for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
		{
			SMasterGroup &rMG = _MasterGroups[nMasterGroup];
			if (rMG.Group->getActive())
			{
				// Sort world space windows
				rMG.sortWorldSpaceGroup ();

				for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; ++nPriority)
				{
					if ( (nPriority == WIN_PRIORITY_WORLD_SPACE) && !camera.empty())
					{
						Driver->setViewMatrix(CMatrix::Identity);
						Driver->setModelMatrix(CMatrix::Identity);
						Driver->setFrustum(camera.getFrustum());
						_ViewRenderer.activateWorldSpaceMatrix (true);
					}

					list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
					list<CInterfaceGroup*>::const_iterator itw;

					for (itw = rList.begin(); itw != rList.end(); itw++)
					{
						CInterfaceGroup *pIG = *itw;
						if( pIG ) // TODO: debug null pointer in PrioritizedWindows list
						{
							if (pIG->getActive())
							{
								// Draw all the elements of this window in the layers in ViewRendered
								pIG->draw ();
								// flush the layers
								_ViewRenderer.flush ();
							}
						}
					}

					if ( (nPriority == WIN_PRIORITY_WORLD_SPACE) && !camera.empty())
					{
						Driver->setMatrixMode2D11();
						_ViewRenderer.activateWorldSpaceMatrix (false);
					}
				}
			}
		}
	}

	{
		H_AUTO ( RZ_Interface_DrawViews_After )

		CCDBNodeBranch::flushObserversCalls();

		// draw the special over extend text
		drawOverExtendViewText();

		// draw the context help
		drawContextHelp ();

		// Draw the pointer and DND Item
		if (_Pointer != NULL)
		{
			//_Pointer->updateCoords();

			if (_Pointer->show())
			{
				CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>((CCtrlBase*)_CapturePointerLeft);
				if ((pCS != NULL) && (pCS->isDraging()))
				{
					sint	x= _Pointer->getX() - pCS->getDeltaDragX();
					sint	y= _Pointer->getY() - pCS->getDeltaDragY();
					pCS->drawSheet (x, y, false, false);

					// if the control support CopyDrag, and if copy key pressed, display a tiny "+"
					if(pCS->canDragCopy() && testDragCopyKey())
					{
						CViewRenderer &rVR = getViewRenderer();
						sint	w= rVR.getSystemTextureW(CViewRenderer::DragCopyTexture);
						sint	h= rVR.getSystemTextureW(CViewRenderer::DragCopyTexture);
						rVR.draw11RotFlipBitmap (pCS->getRenderLayer()+1, x-w/2, y-h/2, 0, false,
							rVR.getSystemTextureId(CViewRenderer::DragCopyTexture));
					}
				}
			}

			// Even if hardware, Force draw if the cursor is a string (ATTACK etc...)
			/*if (_Pointer->getActive() && (!IsMouseCursorHardware () || _Pointer->getStringMode()) )
 				_Pointer->draw ();*/
			if (_Pointer->getActive())
 				_Pointer->draw ();
		}

		// flush layers
		_ViewRenderer.flush();

		// todo hulud remove Return in 2d world
		Driver->setMatrixMode2D11();

		// flush obs
		CCDBNodeBranch::flushObserversCalls();
	}
}


// ----------------------------------------------------------------------------
CCtrlBase* CInterfaceManager::getNewContextHelpCtrl()
{
	// get the top most ctrl under us
	CCtrlBase *best = NULL;
	sint8 bestRenderLayer = -128;
	for (sint i = (sint32)_CtrlsUnderPointer.size()-1; i>=0; i--)
	{
		CCtrlBase	*pICL = _CtrlsUnderPointer[i];
		if (pICL->getRenderLayer() > bestRenderLayer)
		{
			if ((pICL->getActive()) && (!pICL->emptyContextHelp()))
			{
				if (!_Pointer) return pICL;
				sint32 mx, my;
				_Pointer->getPointerPos(mx, my);
				if (pICL->preciseHitTest(mx, my))
				{
					best = pICL;
					bestRenderLayer	= pICL->getRenderLayer();
				}
			}
		}
	}
	if (!best)
	{
		// if a control was not found, try with the groups
		sint8 bestRenderLayer = -128;
		for (sint i = (sint32)_GroupsUnderPointer.size()-1; i>=0; i--)
		{
			CCtrlBase	*pICL = _GroupsUnderPointer[i];
			if (pICL->getRenderLayer() > bestRenderLayer)
			{
				if ((pICL->getActive()) && (!pICL->emptyContextHelp()))
				{
					if (!_Pointer) return pICL;
					sint32 mx, my;
					_Pointer->getPointerPos(mx, my);
					if (pICL->preciseHitTest(mx, my))
					{
						best = pICL;
						bestRenderLayer	= pICL->getRenderLayer();
					}
				}
			}
		}
	}
	return best;
}

// ----------------------------------------------------------------------------
CInterfaceGroup	*CInterfaceManager::getWindowForActiveMasterGroup(const std::string &window)
{
	// Search for all elements
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			CInterfaceElement	*pEL= getElementFromId( rMG.Group->getId() + ":" + window);
			if(pEL && pEL->isGroup())
				return (CInterfaceGroup*)pEL;
		}
	}

	return NULL;
}


// ----------------------------------------------------------------------------
void CInterfaceManager::updateTooltipCoords()
{
	updateTooltipCoords(_CurCtrlContextHelp);
}

// ----------------------------------------------------------------------------
void CInterfaceManager::updateTooltipCoords(CCtrlBase *newCtrl)
{
	if (!newCtrl) return;
	if (!newCtrl->getInvalidCoords()) return;
	CInterfaceGroup *groupContextHelp = getWindowForActiveMasterGroup(newCtrl->getContextHelpWindowName());
	if(groupContextHelp)
	{
		CViewText *pTxt = (CViewText*)groupContextHelp->getView("text");
		if (pTxt != NULL)
		{
			pTxt->setTextFormatTaged(_ContextHelpText);
			// update only to get correct W/H
			groupContextHelp->updateCoords ();


			// **** Compute parent coordinates
			CCtrlBase::TToolTipParentType	parentType= newCtrl->getToolTipParent();
			CInterfaceGroup	*win= NULL;
			// adjust to the mouse by default
			sint32		xParent= _Pointer->getX();
			sint32		yParent= _Pointer->getY();
			sint32		wParent= 0;
			sint32		hParent= 0;
			// adjust to the window
			if(parentType==CCtrlBase::TTWindow || parentType==CCtrlBase::TTSpecialWindow)
			{
				if(parentType==CCtrlBase::TTWindow)
					win= newCtrl->getRootWindow();
				else
					win= dynamic_cast<CInterfaceGroup*>(getElementFromId(newCtrl->getToolTipSpecialParent()));
				if(win)
				{
					xParent = win->getXReal();
					yParent = win->getYReal();
					wParent = win->getWReal();
					hParent = win->getHReal();
				}
				// Bug...: leave default to pointer
			}
			// adjust to the ctrl
			else if (parentType==CCtrlBase::TTCtrl)
			{
				xParent = newCtrl->getXReal();
				yParent = newCtrl->getYReal();
				wParent = newCtrl->getWReal();
				hParent = newCtrl->getHReal();
				// Additionaly, must clip this ctrl with its parent
				// (else animals are buggy for instance)
				CInterfaceGroup	*parent= newCtrl->getParent();
				if(parent)
				{
					sint32	xClip,yClip,wClip,hClip;
					parent->getClip(xClip,yClip,wClip,hClip);
					// clip bottom left
					xParent= max(xParent, xClip);
					yParent= max(yParent, yClip);
					// clip top right
					sint32	xrParent= min(xParent+ wParent, xClip+wClip);
					sint32	ytParent= min(yParent+ hParent, yClip+hClip);
					wParent= max((sint32)0, xrParent-xParent);
					hParent= max((sint32)0, ytParent-yParent);
				}
			}


			// **** resolve auto posref
			uint clampCount = adjustTooltipPosition(newCtrl, win, newCtrl->getToolTipParentPosRef(), newCtrl->getToolTipPosRef(), xParent, yParent, wParent, hParent);
			if (clampCount != 0)
			{
				// try to fallback on alternate tooltip posref
				uint altClampCount = adjustTooltipPosition(newCtrl, win, newCtrl->getToolTipParentPosRefAlt(), newCtrl->getToolTipPosRefAlt(), xParent, yParent, wParent, hParent);
				if (altClampCount > clampCount)
				{
					// worst ? resume to first posref
					adjustTooltipPosition(newCtrl, win, newCtrl->getToolTipParentPosRef(), newCtrl->getToolTipPosRef(), xParent, yParent, wParent, hParent);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
void CInterfaceManager::drawContextHelp ()
{
	if (!_Pointer || !_ContextHelpActive)
		return;


	sint32 x = _Pointer->getX();
	sint32 y = _Pointer->getY();


	// ***************
	// **** try to disable
	// ***************
	// test disable first, so can recheck asap if another present. see below
	if(_CurCtrlContextHelp)
	{
		if(x!=_LastXContextHelp || y!=_LastYContextHelp)
		{
			// May change of ctrl!! => disable context help
			CCtrlBase	*newCtrl= getNewContextHelpCtrl();
			if(newCtrl!=_CurCtrlContextHelp)
			{
				// disable
				disableContextHelp();
			}
		}

		// Check if _CurCtrlContextHelp  is visible
		if (_CurCtrlContextHelp == NULL)
		{
			disableContextHelp();
		}
		else
		{
			bool bVisible = true;
			if (_CurCtrlContextHelp->getActive() == false)
				bVisible = false;
			CInterfaceGroup *pParent = _CurCtrlContextHelp->getParent();
			while (pParent != NULL)
			{
				if (pParent->getActive() == false)
					bVisible = false;
				pParent = pParent->getParent();
			}
			if (!bVisible)
				disableContextHelp();
		}
	}


	// ***************
	// **** try to acquire
	// ***************
	if(!_CurCtrlContextHelp)
	{
		// get the ctrl of interset
		CCtrlBase	*newCtrl= getNewContextHelpCtrl();

		if(x==_LastXContextHelp && y==_LastYContextHelp)
			_DeltaTimeStopingContextHelp+= DT;
		else
			_DeltaTimeStopingContextHelp= 0;

		// If reach the time limit
		if( _DeltaTimeStopingContextHelp > _MaxTimeStopingContextHelp || (newCtrl && newCtrl->wantInstantContextHelp()))
		{
			// if present, get the ctx help text.
			if(newCtrl)
			{
				// get the text
				newCtrl->getContextHelpToolTip(_ContextHelpText);
				// UserDefined context help
				if( !newCtrl->getContextHelpActionHandler().empty() )
				{
					runActionHandler(newCtrl->getContextHelpActionHandler(), newCtrl, newCtrl->getContextHelpAHParams() );
				}

				// If the text is finally empty (Special AH case), abort
				if(_ContextHelpText.empty())
					newCtrl= NULL;
			}

			// not present? wait furthermore to move the mouse.
			if(!newCtrl)
				_DeltaTimeStopingContextHelp= 0;
			else
			{
				// enable
				_CurCtrlContextHelp = newCtrl->getId();
				newCtrl->invalidateCoords();
			}
		}
	}

	updateTooltipCoords(_CurCtrlContextHelp);


	// ***************
	// **** display
	// ***************
	if(_CurCtrlContextHelp)
	{
		CInterfaceGroup *groupContextHelp = getWindowForActiveMasterGroup(_CurCtrlContextHelp->getContextHelpWindowName());
		if(groupContextHelp)
		{
			/** If there's a modal box around, should be sure that the context help doesn't intersect it.
			  * If this is the case, we just disable it, unless the tooltip was generated by the current modal window
			  */
			if (!_ModalStack.empty())
			{
				CInterfaceGroup *mw = _ModalStack.back().ModalWindow;
				if (mw && mw->isIn(*groupContextHelp))
				{
					if (_CurCtrlContextHelp->isSonOf(mw))
					{
						groupContextHelp->executeLuaScriptOnDraw();
						groupContextHelp->draw ();
						// flush layers
						_ViewRenderer.flush();
					}
				}
				else
				{
					groupContextHelp->executeLuaScriptOnDraw();
					groupContextHelp->draw ();
					// flush layers
					_ViewRenderer.flush();
				}
			}
			else
			{
				groupContextHelp->executeLuaScriptOnDraw();
				groupContextHelp->draw ();
				// flush layers
				_ViewRenderer.flush();
			}
		}
	}

	// Bkup movement
	_LastXContextHelp= x;
	_LastYContextHelp= y;
}

void CInterfaceManager::setContextHelpActive(bool active)
{
	if (!active)
	{
		disableContextHelp();
	}
	_ContextHelpActive = active;
}


uint CInterfaceManager::adjustTooltipPosition(CCtrlBase *newCtrl,
											  CInterfaceGroup *win,
											  THotSpot ttParentRef,
											  THotSpot ttPosRef,
											  sint32 xParent,
											  sint32 yParent,
											  sint32 wParent,
											  sint32 hParent
											 )
{
	CCtrlBase::TToolTipParentType	parentType= newCtrl->getToolTipParent();
	CInterfaceGroup *groupContextHelp = getWindowForActiveMasterGroup(newCtrl->getContextHelpWindowName());
	if(ttPosRef==Hotspot_TTAuto || ttParentRef==Hotspot_TTAuto)
	{
		// NB: keep the special window if type is specialwindow (defined above)
		if(!win)
			win= newCtrl->getRootWindow();
		sint32	xWin= 0;
		sint32	yWin= 0;
		sint32	wWin= 0;
		sint32	hWin= 0;
		if(win)
		{
			xWin = win->getXReal();
			yWin = win->getYReal();
			wWin = win->getWReal();
			hWin = win->getHReal();
		}
		// for Window, display top or bottom according to window pos/size
		if(parentType==CCtrlBase::TTWindow || parentType==CCtrlBase::TTSpecialWindow)
		{
			sint32	top= (sint32)_ScreenH - (yWin+hWin);
			sint32	bottom= yWin;
			if(top>bottom)
			{
				ttParentRef= Hotspot_TL;
				ttPosRef= Hotspot_BL;
			}
			else
			{
				ttParentRef= Hotspot_BL;
				ttPosRef= Hotspot_TL;
			}
		}
		// for Ctrl, display top, left or right according to window pos/size
		else if(parentType==CCtrlBase::TTCtrl)
		{
			sint32	right= (sint32)_ScreenW - (xWin+wWin);
			sint32	left= xWin;
			if(right>left)
			{
				ttParentRef= Hotspot_TR;
				ttPosRef= Hotspot_BL;
			}
			else
			{
				ttParentRef= Hotspot_TL;
				ttPosRef= Hotspot_BR;
			}
		}
		else
		{
			// default (mouse)
			ttParentRef= Hotspot_BL;
			ttPosRef= Hotspot_BL;
		}
	}

	// **** compute coordinates of the tooltip
	sint32 x= xParent;
	sint32 y= yParent;
	if (ttParentRef & Hotspot_Mx)
		y += hParent/2;
	if (ttParentRef & Hotspot_Tx)
		y += hParent;
	if (ttParentRef & Hotspot_xM)
		x += wParent/2;
	if (ttParentRef & Hotspot_xR)
		x += wParent;

	// adjust according to self posref
	if (ttPosRef & Hotspot_Mx)
		y -= groupContextHelp->getHReal()/2;
	if (ttPosRef & Hotspot_Tx)
		y -= groupContextHelp->getHReal();
	if (ttPosRef & Hotspot_xM)
		x -= groupContextHelp->getWReal()/2;
	if (ttPosRef & Hotspot_xR)
		x -= groupContextHelp->getWReal();


	// **** clamp to screen coords, and set
	uint clampCount = 0;

	if ((x+groupContextHelp->getW()) > groupContextHelp->getParent()->getWReal())
	{
		++ clampCount;
		x = groupContextHelp->getParent()->getWReal() - groupContextHelp->getW();
	}
	if (x < 0)
	{
		x = 0;
		++ clampCount;
	}
	if ((y+groupContextHelp->getH()) > groupContextHelp->getParent()->getHReal())
	{
		y = groupContextHelp->getParent()->getHReal() - groupContextHelp->getH();
		++ clampCount;
	}
	if (y < 0)
	{
		y = 0;
		++ clampCount;
	}

	// update coords 3 times is required
	groupContextHelp->setX (x);
	groupContextHelp->setY (y);
	groupContextHelp->updateCoords ();
	groupContextHelp->updateCoords ();
	groupContextHelp->updateCoords ();

	return clampCount;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::setGlobalColor (NLMISC::CRGBA col)
{
	if (!_RProp)
	{
		_RProp = getDbProp("UI:SAVE:COLOR:R");
		_GProp = getDbProp("UI:SAVE:COLOR:G");
		_BProp = getDbProp("UI:SAVE:COLOR:B");
		_AProp = getDbProp("UI:SAVE:COLOR:A");
	}
	_RProp ->setValue32 (col.R);
	_GProp ->setValue32 (col.G);
	_BProp ->setValue32 (col.B);
	_AProp ->setValue32 (col.A);

	_GlobalColor = col;

	// set the global color for content (the same with modulated alpha)
	_GlobalColorForContent = _GlobalColor;
	_GlobalColorForContent.A = (uint8) (( (uint16) _GlobalColorForContent.A * (uint16) _ContentAlpha) >> 8);
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::isControlInWindow (CCtrlBase *ctrl, CInterfaceGroup *pNewCurrentWnd)
{
	//  We don't want to capture control if they are not owned by the top most window selected.
	// Check all the parent of the control, one must be pNewCurrentWnd, else NULL
	CInterfaceGroup		*parent= ctrl->getParent();
	while(parent!=NULL)
	{
		if(parent==pNewCurrentWnd)
			return true;
		else
			parent= parent->getParent();
	}
	// not found => must not capture it
	return false;
}

// ------------------------------------------------------------------------------------------------
uint CInterfaceManager::getDepth (CCtrlBase *ctrl, CInterfaceGroup *pNewCurrentWnd)
{
	uint depth = 1;
	CInterfaceGroup		*parent= ctrl->getParent();
	while (parent != NULL)
	{
		if (parent == pNewCurrentWnd)
			break;
		else
			parent = parent->getParent();
		depth++;
	}
	// The Resizer Ctrls take the precedence over Sons controls.
	return depth + ctrl->getDeltaDepth();
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::handleEvent (const CEventDescriptor& event)
{
	bool	handled= false;

	// Check if we can receive events (no anims!)
	for (uint i = 0; i < _ActiveAnims.size(); ++i)
		if (_ActiveAnims[i]->isDisableButtons())
			return false;

	if (event.getType() == CEventDescriptor::key)
	{
		CEventDescriptorKey &eventDesc = (CEventDescriptorKey&)event;
		_LastEventKeyDesc = eventDesc;

		// Any Key event disable the ContextHelp
		disableContextHelp();

		// Hide menu if the key is pushed
//		if ((eventDesc.getKeyEventType() == CEventDescriptorKey::keydown) && !_ModalStack.empty() && !eventDesc.getKeyAlt() && !eventDesc.getKeyCtrl() && !eventDesc.getKeyShift())
		// Hide menu (or popup menu) is ESCAPE pressed
		if( eventDesc.getKeyEventType() == CEventDescriptorKey::keychar && eventDesc.getChar() == KeyESCAPE )
		{
			if(_ModalStack.size() > 0)
			{
				CModalWndInfo mwi = _ModalStack.back();
				if (mwi.ModalExitKeyPushed)
					disableModalWindow();
			}
		}

		// Manage "quit window" If the Key is ESCAPE, no captureKeyboard
		if( eventDesc.getKeyEventType() == CEventDescriptorKey::keychar && eventDesc.getChar() == KeyESCAPE )
		{
			// Get the last escapable active top window. NB: this is ergonomically better.
			CInterfaceGroup	*win= getLastEscapableTopWindow();
			if( win )
			{
				// If the window is a modal, must pop it.
				if( dynamic_cast<CGroupModal*>(win) )
				{
					if(!win->getAHOnEscape().empty())
						runActionHandler(win->getAHOnEscape(), win, win->getAHOnEscapeParams());
					popModalWindow();
					handled= true;
				}
				// else just disable it.
				// Special case: leave the escape Key to the CaptureKeyboard .
				else if(!_CaptureKeyboard )
				{
					if(!win->getAHOnEscape().empty())
						runActionHandler(win->getAHOnEscape(), win, win->getAHOnEscapeParams());
					win->setActive(false);
					handled= true;
				}
			}
		}

		// Manage complex "Enter"
		if (eventDesc.getKeyEventType() == CEventDescriptorKey::keychar && eventDesc.getChar() == KeyRETURN)
		{
			// If the  top window has Enter AH
			CInterfaceGroup	*tw= getTopWindow();
			if(tw && !tw->getAHOnEnter().empty())
			{
				// if the captured keyboard is in this Modal window, then must handle him in priority
				if(_CaptureKeyboard && _CaptureKeyboard->getRootWindow()==tw)
				{
					bool result = _CaptureKeyboard->handleEvent(event);
					CCDBNodeBranch::flushObserversCalls();
					return result;
				}
				else
				{
					// The window or modal control the OnEnter. Execute, and don't go to the chat.
					runActionHandler(tw->getAHOnEnter(), tw, tw->getAHOnEnterParams());
					handled= true;
				}
			}

			// else the 'return' key bring back to the last edit box (if possible)
			CCtrlBase *oldCapture = _OldCaptureKeyboard ? _OldCaptureKeyboard : _DefaultCaptureKeyboard;
			if (_CaptureKeyboard == NULL && oldCapture && !handled)
			{
				/* If the editbox does not want to recover focus, then abort. This possibility is normaly avoided
					through setCaptureKeyboard() which already test getRecoverFocusOnEnter(), but it is still possible
					for the default capture (main chat) or the old captured window to not want to recover
					(temporary Read Only chat for instance)
				*/
				if(!dynamic_cast<CGroupEditBox*>(oldCapture) ||
					dynamic_cast<CGroupEditBox*>(oldCapture)->getRecoverFocusOnEnter())
				{
					_CaptureKeyboard = oldCapture;
					notifyElementCaptured(_CaptureKeyboard);
					// make sure all parent windows are active
					CCtrlBase *cb = _CaptureKeyboard;
					CGroupContainer *lastContainer = NULL;
					for(;;)
					{
						CGroupContainer *gc = dynamic_cast<CGroupContainer *>(cb);
						if (gc) lastContainer = gc;
						cb->forceOpen();
						if (cb->getParent())
						{
							cb = cb->getParent();
						}
						else
						{
							cb->invalidateCoords();
							break;
						}
					}
					if (lastContainer)
					{
						setTopWindow(lastContainer);
						lastContainer->enableBlink(1);
					}
					handled= true;
				}
			}
		}

		// General case: handle it in the Captured keyboard
		if (_CaptureKeyboard != NULL && !handled)
		{
			bool result = _CaptureKeyboard->handleEvent(event);
			CCDBNodeBranch::flushObserversCalls();
			return result;
		}
	}
	else if (event.getType() == CEventDescriptor::mouse && _MouseHandlingEnabled )
	{
		CEventDescriptorMouse &eventDesc = (CEventDescriptorMouse&)event;

		// First thing to do : Capture handling
		if (_CapturePointerLeft != NULL)
			handled|= _CapturePointerLeft->handleEvent(event);

		if (_CapturePointerRight != NULL && _CapturePointerRight!=_CapturePointerLeft)
			handled|= _CapturePointerRight->handleEvent(event);

		CInterfaceGroup *ptr = getWindowUnder (eventDesc.getX(), eventDesc.getY());
		_WindowUnder = ptr?ptr->getId():"";

		// Any Mouse event but move disable the ContextHelp
		if(eventDesc.getEventTypeExtended() != CEventDescriptorMouse::mousemove)
		{
			disableContextHelp();
		}

		// get the group under the mouse
		CInterfaceGroup *pNewCurrentWnd = _WindowUnder;
		_MouseOverWindow= pNewCurrentWnd!=NULL;


		NLMISC::CRefPtr<CGroupModal>	clickedOutModalWindow;

		// modal special features
		if (!_ModalStack.empty())
		{
			CModalWndInfo mwi = _ModalStack.back();
			if(mwi.ModalWindow)
			{
				// If we are not in "click out" mode so we dont handle controls other than those of the modal
				if (pNewCurrentWnd != mwi.ModalWindow && !mwi.ModalExitClickOut)
				{
					pNewCurrentWnd = NULL;
				}
				else
				{
					// If there is a handler on click out launch it
					if (pNewCurrentWnd != mwi.ModalWindow)
						if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown ||
							(eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightdown))
							if (!mwi.ModalHandlerClickOut.empty())
								runActionHandler(mwi.ModalHandlerClickOut,NULL,mwi.ModalClickOutParams);

					// If the current window is not the modal and if must quit on click out
					if(pNewCurrentWnd != mwi.ModalWindow && mwi.ModalExitClickOut)
					{
						// NB: don't force handle==true because to quit a modal does not avoid other actions

						// quit if click outside
						if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown ||
							(eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightdown))
						{
							clickedOutModalWindow = dynamic_cast<CGroupModal *>((CInterfaceGroup*)mwi.ModalWindow);
							// disable the modal
							popModalWindow();
							if (!_ModalStack.empty())
							{
								// don't handle event unless it is a previous modal window
								uint k = 0;
								for(k = 0; k < _ModalStack.size(); ++k)
								{
									if (_ModalStack[k].ModalWindow == pNewCurrentWnd)
									{
										break;
									}
								}
								if (k == _ModalStack.size())
								{
									pNewCurrentWnd = NULL; // can't handle event before we have left all modal windows
								}
							}
							movePointer (0,0); // Reget controls under pointer
						}
					}
				}
			}
		}

		// Manage LeftClick.
		if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown)
		{
			if ((pNewCurrentWnd != NULL) && (_ModalStack.empty()) && (pNewCurrentWnd->getOverlappable()))
			{
				CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(pNewCurrentWnd);
				if (pGC != NULL)
				{
					if (!pGC->isGrayed()) setTopWindow(pNewCurrentWnd);
				}
				else
				{
					setTopWindow(pNewCurrentWnd);
				}
			}

			// must not capture a new element if a sheet is currentlty being dragged.
			// This may happen when alt-tab has been used => the sheet is dragged but the left button is up
			if (!CDBCtrlSheet::getDraggedSheet())
			{
				// Take the top most control.
				uint nMaxDepth = 0;
				for (sint32 i = (sint32)_CtrlsUnderPointer.size()-1; i >= 0; i--)
				{
					CCtrlBase	*ctrl= _CtrlsUnderPointer[i];
					if (ctrl && ctrl->isCapturable() && isControlInWindow(ctrl, pNewCurrentWnd))
					{
						uint d = getDepth(ctrl, pNewCurrentWnd);
						if (d > nMaxDepth)
						{
							nMaxDepth = d;
							_CapturePointerLeft = ctrl;
						}
					}
				}
				notifyElementCaptured(_CapturePointerLeft);
				if (clickedOutModalWindow && !clickedOutModalWindow->OnPostClickOut.empty())
				{
					runActionHandler(clickedOutModalWindow->OnPostClickOut, _CapturePointerLeft, clickedOutModalWindow->OnPostClickOutParams);
				}
			}
			//if found
			if (_CapturePointerLeft != NULL)
			{
				// consider clicking on a control implies handling of the event.
				handled= true;

				// handle the capture
				_CapturePointerLeft->handleEvent(event);
			}
		}

		// Manage RightClick
		if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightdown)
		{
			if ((pNewCurrentWnd != NULL) && (_ModalStack.empty()) && (pNewCurrentWnd->getOverlappable()))
			{
				CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(pNewCurrentWnd);
				if (pGC != NULL)
				{
					if (!pGC->isGrayed()) setTopWindow(pNewCurrentWnd);
				}
				else
				{
					setTopWindow(pNewCurrentWnd);
				}
			}

			// Take the top most control.
			{
				uint nMaxDepth = 0;
				for (sint32 i = (sint32)_CtrlsUnderPointer.size()-1; i >= 0; i--)
				{
					CCtrlBase	*ctrl= _CtrlsUnderPointer[i];
					if (ctrl && ctrl->isCapturable() && isControlInWindow(ctrl, pNewCurrentWnd))
					{
						uint d = getDepth(ctrl , pNewCurrentWnd);
						if (d > nMaxDepth)
						{
							nMaxDepth = d;
							_CapturePointerRight = ctrl;
						}
					}
				}
				notifyElementCaptured(_CapturePointerRight);
				if (clickedOutModalWindow && !clickedOutModalWindow->OnPostClickOut.empty())
				{
					runActionHandler(clickedOutModalWindow->OnPostClickOut, _CapturePointerRight, clickedOutModalWindow->OnPostClickOutParams);
				}
			}
			//if found
			if (_CapturePointerRight != NULL)
			{
				// handle the capture
				handled |= _CapturePointerRight->handleEvent(event);
			}
		}
		if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightup)
		{
			if (!handled)
				if (pNewCurrentWnd != NULL)
					pNewCurrentWnd->handleEvent(event);
			if (_CapturePointerRight != NULL)
			{
				EventsListener.addUIHandledButtonMask(rightButton); // prevent 'click in scene' as mouse was previously captured
																   // (more a patch that anything, but 'UserControls' test for 'mouse up'
																   // directly later in the main loop (not through message queue), so it has no way of knowing that the event was handled...
				setCapturePointerRight(NULL);
				handled= true;
			}
		}

		// window handling. if not handled by a control
		if (!handled)
		{
			if (((pNewCurrentWnd != NULL) && _ModalStack.empty()) || ((!_ModalStack.empty() && _ModalStack.back().ModalWindow == pNewCurrentWnd)))
			{
				CEventDescriptorMouse ev2 = eventDesc;
				sint32 x= eventDesc.getX(), y = eventDesc.getY();
				if (pNewCurrentWnd)
				{
					pNewCurrentWnd->absoluteToRelative (x, y);
					ev2.setX (x); ev2.setY (y);
					handled|= pNewCurrentWnd->handleEvent (ev2);
				}

				// After handle event of a left click, may set window Top if movable (infos etc...)
				//if( (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown) && pNewCurrentWnd->isMovable() )
				//	setTopWindow(pNewCurrentWnd);
			}
		}

		// Put here to let a chance to the window to handle if the capture dont
		if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftup)
		{
			if (_CapturePointerLeft != NULL)
			{
				EventsListener.addUIHandledButtonMask (leftButton); // prevent 'click in scene' as mouse was previously captured
																  // (more a patch that anything, but 'UserControls' test for 'mouse up'
																  // directly later in the main loop (not through message queue), so it has no way of knowing that the event was handled...
				setCapturePointerLeft(NULL);
				//handled= true;
			}
		}


		// If the current window is the modal, may Modal quit. Do it after standard event handle
		if(!_ModalStack.empty() && pNewCurrentWnd == _ModalStack.back().ModalWindow)
		{
			// NB: don't force handle==true because to quit a modal does not avoid other actions
			CModalWndInfo mwi = _ModalStack.back();
			//  and if must quit on click right
			if(mwi.ModalExitClickR)
			{
				// quit if click right
				if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouserightup)
					// disable the modal
					disableModalWindow();
			}

			//  and if must quit on click left
			if(mwi.ModalExitClickL)
			{
				// quit if click right
				if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftup)
					// disable the modal
					disableModalWindow();
			}
		}

		// If the mouse is over a window, always consider the event is taken (avoid click behind)
		handled|= _MouseOverWindow;
	}

	CCDBNodeBranch::flushObserversCalls();
	// event handled?
	return handled;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::movePointer (sint32 dx, sint32 dy)
{
	if (!_Pointer) return;
	uint32 nScrW, nScrH;
	sint32 oldpx, oldpy, newpx, newpy, disppx, disppy, olddisppx, olddisppy;

	_ViewRenderer.getScreenSize (nScrW, nScrH);
	_Pointer->getPointerPos (oldpx, oldpy);

	olddisppx = oldpx;
	olddisppy = oldpy;

	newpx = oldpx + dx;
	newpy = oldpy + dy;

	if (newpx < 0) newpx = 0;
	if (newpy < 0) newpy = 0;
	if (newpx > (sint32)nScrW) newpx = nScrW;
	if (newpy > (sint32)nScrH) newpy = nScrH;
	dx = newpx - oldpx;
	dy = newpy - oldpy;

	disppx = newpx;
	disppy = newpy;

	_Pointer->setPointerPos (newpx, newpy);
	_Pointer->setPointerDispPos (disppx, disppy);

	// must get back coordinates because of snapping
	sint32 mx = _Pointer->getX();
	sint32 my = _Pointer->getY();
	getViewsUnder (mx, my, _ViewsUnderPointer);
	getCtrlsUnder (mx, my, _CtrlsUnderPointer);
	getGroupsUnder (mx, my, _GroupsUnderPointer);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::movePointerAbs(sint32 px, sint32 py)
{
	if (!_Pointer) return;
	uint32 nScrW, nScrH;
	_ViewRenderer.getScreenSize (nScrW, nScrH);
	clamp(px, 0, (sint32) nScrW);
	clamp(py, 0, (sint32) nScrH);
	//
	_Pointer->setPointerPos (px, py);
	_Pointer->setPointerDispPos (px, py);
	//
	getViewsUnder (px, py, _ViewsUnderPointer);
	getCtrlsUnder (px, py, _CtrlsUnderPointer);
	getGroupsUnder (px, py, _GroupsUnderPointer);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::getNewWindowCoordToNewScreenSize(sint32 &x, sint32 &y, sint32 w, sint32 h, sint32 newScreenW, sint32 newScreenH) const
{
	// NB: x is relative to Left of the window (and Left of screen)
	// NB: y is relative to Top of the window  (but Bottom of screen)

	/*
		The goal here is to move the window so it fit the new resolution
		But we don't want to change its size (because somes windows just can't)
		We also cannot use specific code according to each window because user may completly modify his interface
		So the strategy is to dectect on which "side" (or center) the window is the best sticked,
		and then just move the window according to this position
	*/

	// *** First detect from which screen position the window is the more sticked (borders or center)
	// In X: best hotspot is left, middle or right?
	sint32	posXToLeft= x;
	sint32	posXToMiddle= x+w/2-_LastInGameScreenW/2;
	sint32	posXToRight= _LastInGameScreenW-(x+w);
	sint32	bestXHotSpot= Hotspot_xL;
	sint32	bestXPosVal= posXToLeft;
	if(abs(posXToMiddle) < bestXPosVal)
	{
		bestXHotSpot= Hotspot_xM;
		bestXPosVal= abs(posXToMiddle);
	}
	if(posXToRight < bestXPosVal)
	{
		bestXHotSpot= Hotspot_xR;
		bestXPosVal= posXToRight;
	}

	// Same In Y: best hotspot is bottom, middle or top?
	// remember here that y is the top of window (relative to bottom of screen)
	sint32	posYToBottom= y-h;
	sint32	posYToMiddle= y-h/2-_LastInGameScreenH/2;
	sint32	posYToTop= _LastInGameScreenH-y;
	sint32	bestYHotSpot= Hotspot_Bx;
	sint32	bestYPosVal= posYToBottom;
	const	sint32	middleYWeight= 6;		// Avoid default Mission/Team/Map/ContactList positions to be considered as "middle"
	if(abs(posYToMiddle)*middleYWeight < bestYPosVal)
	{
		bestYHotSpot= Hotspot_Mx;
		bestYPosVal= abs(posYToMiddle)*middleYWeight;
	}
	if(posYToTop < bestYPosVal)
	{
		bestYHotSpot= Hotspot_Tx;
		bestYPosVal= posYToTop;
	}

	// *** According to best matching hotspot, and new screen resolution, move the window
	// x
	if(bestXHotSpot==Hotspot_xL)
		x= x;
	else if(bestXHotSpot==Hotspot_xM)
		x= newScreenW/2 + posXToMiddle - w/2;
	else if(bestXHotSpot==Hotspot_xR)
		x= newScreenW - posXToRight - w;
	// y
	if(bestYHotSpot==Hotspot_Bx)
		y= y;
	else if(bestYHotSpot==Hotspot_Mx)
		y= newScreenH/2 + posYToMiddle + h/2;
	else if(bestYHotSpot==Hotspot_Tx)
		y= newScreenH - posYToTop;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::moveAllWindowsToNewScreenSize(sint32 newScreenW, sint32 newScreenH, bool fixCurrentUI)
{
	// If resolutions correctly setuped, and really different from new setup
	if( _LastInGameScreenW >0 && _LastInGameScreenH>0 &&
		newScreenW >0 && newScreenH>0 &&
		(_LastInGameScreenW != newScreenW || _LastInGameScreenH != newScreenH)
		)
	{
		// *** Do it for the Active Desktop (if wanted)
		if(fixCurrentUI)
		{
			// only for ui:interface (not login, nor outgame)
			for (uint nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
			{
				SMasterGroup &rMG = _MasterGroups[nMasterGroup];
				if(!rMG.Group || rMG.Group->getId()!="ui:interface")
					continue;

				// For all priorities, but the worldspace one
				for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
				{
					if(nPriority==WIN_PRIORITY_WORLD_SPACE)
						continue;

					// For All windows (only layer 0 group container)
					list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
					list<CInterfaceGroup*>::const_iterator itw;
					for (itw = rList.begin(); itw != rList.end(); itw++)
					{
						CInterfaceGroup *pIG = *itw;
						if(!pIG->isGroupContainer())
							continue;
						CGroupContainer	*gc= safe_cast<CGroupContainer*>(pIG);
						if(gc->getLayerSetup()!=0)
							continue;
						// should all be BL / TL
						if(gc->getParentPosRef()!=Hotspot_BL || gc->getPosRef()!=Hotspot_TL)
							continue;

						// Get current window coordinates
						sint32	x= pIG->getX();				// x is relative to Left of the window
						sint32	y= pIG->getY();				// y is relative to Top of the window
						sint32	w= pIG->getW(false);		// the window may be hid, still get the correct(or estimated) W
						sint32	h= pIG->getH(false);		// the window may be hid, still get the correct(or estimated) H

						// Compute the new coordinate
						getNewWindowCoordToNewScreenSize(x, y, w, h, newScreenW, newScreenH);

						// Change
						pIG->setX(x);
						pIG->setY(y);
					}
				}
			}
		}

		// *** Do it for All Backuped Desktops
		for(uint md=0;md<MAX_NUM_MODES;md++)
		{
			CInterfaceConfig::CDesktopImage		&mode= _Modes[md];
			// For all containers of this mode
			for(uint gc=0;gc<mode.GCImages.size();gc++)
			{
				CInterfaceConfig::SCont	&gcCont= mode.GCImages[gc];
				// Compute the new coordinate, directly in the X/Y fields of the structure
				getNewWindowCoordToNewScreenSize(gcCont.X, gcCont.Y, gcCont.W, gcCont.H ,newScreenW, newScreenH);
			}
		}
	}

	// Now those are the last screen coordinates used for window position correction
	if(newScreenW >0 && newScreenH>0)
	{
		_LastInGameScreenW= newScreenW;
		_LastInGameScreenH= newScreenH;
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::updateAllLocalisedElements()
{

	uint32 nMasterGroup;

	uint32 w, h;
	_ViewRenderer.checkNewScreenSize ();
	_ViewRenderer.getScreenSize (w, h);

	// Update ui:* (limit the master containers to the height of the screen)
	for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		rMG.Group->setW (w);
		rMG.Group->setH (h);
	}
	_ViewRenderer.setClipWindow(0, 0, w, h);

	// If all conditions are OK, move windows so they fit correctly with new screen size
	// Do this work only InGame when Config is loaded
	if(	_InGame && _ConfigLoaded )
		moveAllWindowsToNewScreenSize(w,h,true);

	// Invalidate coordinates of all Windows of each MasterGroup
	for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];

		rMG.Group->invalidateTexts (false);
		rMG.Group->invalidateCoords ();
		for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
		{
			list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
			list<CInterfaceGroup*>::const_iterator itw;
			for (itw = rList.begin(); itw != rList.end(); itw++)
			{
				CInterfaceGroup *pIG = *itw;
				pIG->invalidateTexts (false);
				pIG->invalidateCoords ();
			}
		}
	}

	// setup for all
	for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		bool bActive = rMG.Group->getActive ();
		rMG.Group->setActive (true);
		rMG.Group->updateCoords ();
		rMG.Group->setActive (bActive);
	}

	// update coords one
	checkCoords();

	// Action by default (container opening
	for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		rMG.Group->launch ();
	}

}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::addDBObserver (ICDBNode::IPropertyObserver* observer, ICDBNode::CTextId   id)
{
	return _DbRootNode->addObserver(observer, id);
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::removeDBObserver (ICDBNode::IPropertyObserver* observer, ICDBNode::CTextId  id)
{
	return _DbRootNode->removeObserver(observer, id);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::addServerString (const std::string &sTarget, uint32 id, IStringProcess *cb)
{
	if (id == 0)
	{
		CInterfaceExprValue val;
		val.setUCString (ucstring(""));
		CInterfaceLink::setTargetProperty (sTarget, val);
		return;
	}
	SIDStringWaiter *pISW = new SIDStringWaiter;
	pISW->Id = id;
	pISW->IdOrString = false;
	pISW->Target = sTarget;
	pISW->Cb = cb;
	_IDStringWaiters.push_back(pISW);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::addServerID (const std::string &sTarget, uint32 id, IStringProcess *cb)
{
	if (id == 0)
	{
		CInterfaceExprValue val;
		val.setUCString (ucstring(""));
		CInterfaceLink::setTargetProperty (sTarget, val);
		return;
	}
	SIDStringWaiter *pISW = new SIDStringWaiter;
	pISW->Id = id;
	pISW->IdOrString = true;
	pISW->Target = sTarget;
	pISW->Cb = cb;
	_IDStringWaiters.push_back(pISW);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::processServerIDString()
{
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();

	for (uint32 i = 0; i < _IDStringWaiters.size(); ++i)
	{
		bool bAffect = false;
		ucstring ucstrToAffect;
		SIDStringWaiter *pISW = _IDStringWaiters[i];
		if (pISW->IdOrString == true) // ID !
		{
			if (pSMC->getString (pISW->Id, ucstrToAffect))
				bAffect = true;
		}
		else // String !
		{
			if (pSMC->getDynString (pISW->Id, ucstrToAffect))
				bAffect = true;
		}

		if (bAffect)
		{
			CInterfaceExprValue val;
			bool bValid = true;

			if (pISW->Cb != NULL)
			{
				bValid = pISW->Cb->cbIDStringReceived(ucstrToAffect);
				delete pISW->Cb;
			}

			if (bValid)
			{
				val.setUCString (ucstrToAffect);
				CInterfaceLink::setTargetProperty (pISW->Target, val);
			}

			// Remove entry
			_IDStringWaiters.erase (_IDStringWaiters.begin()+i);
			i--;
			delete pISW;
		}
	}
}

// ------------------------------------------------------------------------------------------------
sint32		CInterfaceManager::getDbValue32 (const std::string & name)
{
	CCDBNodeLeaf	*node= getDbProp(name, false);
	if(node)
		return node->getValue32();
	else
		return 0;
}

// ------------------------------------------------------------------------------------------------
CCDBNodeLeaf* CInterfaceManager::getDbProp(const std::string & name, bool bCreate)
{
	if (name.empty()) return NULL;
	CCDBNodeLeaf *pDBNL = NULL;
	pDBNL = dynamic_cast<CCDBNodeLeaf*>(_DbRootNode->getNode( ICDBNode::CTextId(name), bCreate ));
	return pDBNL;
}

// ------------------------------------------------------------------------------------------------
CCDBNodeBranch *CInterfaceManager::getDbBranch(const std::string &name)
{
	if (name.empty()) return NULL;
	CCDBNodeBranch	*nodeBranch;
	nodeBranch = dynamic_cast<CCDBNodeBranch*>(_DbRootNode->getNode( ICDBNode::CTextId(name), false ));
	return nodeBranch;
}

// ------------------------------------------------------------------------------------------------
CInterfaceGroup* CInterfaceManager::getWindowUnder (sint32 x, sint32 y)
{
	H_AUTO (RZ_Interface_Window_Under )

	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0; nPriority--)
			{
				const list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
				list<CInterfaceGroup*>::const_reverse_iterator itw;
				for (itw = rList.rbegin(); itw != rList.rend(); itw++)
				{
					CInterfaceGroup *pIG = *itw;
					if (pIG->getActive() && pIG->getUseCursor())
					{
						if (pIG->isWindowUnder (x, y))
							return pIG;
					}
				}
			}
		}
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
CInterfaceGroup* CInterfaceManager::getGroupUnder (sint32 x, sint32 y)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0; nPriority--)
			{
				const list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
				list<CInterfaceGroup*>::const_reverse_iterator itw;
				for (itw = rList.rbegin(); itw != rList.rend(); itw++)
				{
					CInterfaceGroup *pIG = *itw;
					if (pIG->getActive() && pIG->getUseCursor())
					{
						CInterfaceGroup *pIGunder = pIG->getGroupUnder (x ,y);
						if (pIGunder != NULL)
							return pIGunder;
					}
				}
			}
		}
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::getViewsUnder (sint32 x, sint32 y, std::vector<CViewBase*> &vVB)
{
	vVB.clear ();

	// No Op if screen minimized
	if(_ViewRenderer.isMinimized())
		return;

	uint32 sw, sh;
	_ViewRenderer.getScreenSize(sw, sh);
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0; nPriority--)
			{
				const list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
				list<CInterfaceGroup*>::const_reverse_iterator itw;
				for (itw = rList.rbegin(); itw != rList.rend(); itw++)
				{
					CInterfaceGroup *pIG = *itw;

					// Accecpt if not modal clip
					if (pIG->getActive() && pIG->getUseCursor())
					{
						if (pIG->getViewsUnder (x, y, 0, 0, (sint32) sw, (sint32) sh, vVB))
							return ;
					}
				}
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::getCtrlsUnder (sint32 x, sint32 y, std::vector<CCtrlBase*> &vICL)
{
	vICL.clear ();

	// No Op if screen minimized
	if(_ViewRenderer.isMinimized())
		return;

	uint32 sw, sh;
	_ViewRenderer.getScreenSize(sw, sh);
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0 ; nPriority--)
			{
				const list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
				list<CInterfaceGroup*>::const_reverse_iterator itw;
				for (itw = rList.rbegin(); itw != rList.rend(); itw++)
				{
					CInterfaceGroup *pIG = *itw;

					// Accecpt if not modal clip
					if (_ModalStack.empty() || _ModalStack.back().ModalWindow == pIG || _ModalStack.back().ModalExitClickOut)
					if (pIG->getActive() && pIG->getUseCursor())
					{
						if (pIG->getCtrlsUnder (x, y, 0, 0, (sint32) sw, (sint32) sh, vICL))
							return;
					}
				}
			}
		}
	}
}


// ------------------------------------------------------------------------------------------------
void CInterfaceManager::getGroupsUnder (sint32 x, sint32 y, std::vector<CInterfaceGroup *> &vIGL)
{
	vIGL.clear ();

	// No Op if screen minimized
	if(_ViewRenderer.isMinimized())
		return;

	uint32 sw, sh;
	_ViewRenderer.getScreenSize(sw, sh);
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0 ; nPriority--)
			{
				const list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
				list<CInterfaceGroup*>::const_reverse_iterator itw;
				for (itw = rList.rbegin(); itw != rList.rend(); itw++)
				{
					CInterfaceGroup *pIG = *itw;

					// Accecpt if not modal clip
					if (_ModalStack.empty() || _ModalStack.back().ModalWindow == pIG || _ModalStack.back().ModalExitClickOut)
					if (pIG->getActive() && pIG->getUseCursor())
					{
						if (pIG->isIn(x, y))
						{
							vIGL.push_back(pIG);
							pIG->getGroupsUnder (x, y, 0, 0, (sint32) sw, (sint32) sh, vIGL);
							return;
						}
					}
				}
			}
		}
	}
}


// ------------------------------------------------------------------------------------------------
void CInterfaceManager::activateMasterGroup (const std::string &sMasterGroupName, bool bActive)
{
	CInterfaceGroup *pIG = getMasterGroupFromId (sMasterGroupName);
	if (pIG != NULL)
	{
		pIG->setActive(bActive);
		pIG->invalidateCoords();
	}
}

// ------------------------------------------------------------------------------------------------
CInterfaceGroup* CInterfaceManager::getWindow(CInterfaceElement *pIE)
{
	CInterfaceGroup *pIG = pIE->getParent();
	if (pIG == NULL) return NULL;
	if (pIG->getParent() == NULL) return NULL;
	while (pIG->getParent()->getParent() != NULL)
	{
		pIG = pIG->getParent();
	}
	return pIG;
}

// ------------------------------------------------------------------------------------------------
CInterfaceElement* CInterfaceManager::getElementFromId (const std::string &sEltId)
{
	// System special
	if(sEltId == _CtrlLaunchingModalId)
		return getCtrlLaunchingModal();

	// Search for all elements
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		CInterfaceElement *pIEL = rMG.Group->getElement (sEltId);
		if (pIEL != NULL)
			return pIEL;
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
CInterfaceElement* CInterfaceManager::getElementFromDefine (const std::string &defineId)
{
	return getElementFromId(getDefine(defineId));
}

// ------------------------------------------------------------------------------------------------
CInterfaceElement* CInterfaceManager::getElementFromId (const std::string &sStart, const std::string &sEltId)
{
	CInterfaceElement *pIEL = getElementFromId (sEltId);
	if (pIEL == NULL)
	{
		string sZeStart = sStart, sTmp;
		if (sZeStart[sZeStart.size()-1] == ':')
			sZeStart = sZeStart.substr(0, sZeStart.size()-1);

		while (sZeStart != "")
		{
			if (sEltId[0] == ':')
				sTmp = sZeStart	+ sEltId;
			else
				sTmp = sZeStart	+ ":" + sEltId;
			pIEL = getElementFromId (sTmp);
			if (pIEL != NULL)
				return pIEL;
			string::size_type nextPos = sZeStart.rfind(':');
			if (nextPos == string::npos) break;
			sZeStart = sZeStart.substr(0, nextPos);
		}
	}
	return pIEL;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::setTopWindow (CInterfaceGroup* win)
{
	//find the window in the window list
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			rMG.setTopWindow(win);
/*
			for (list<CInterfaceGroup*>::iterator it = rMG.Windows.begin(); it != rMG.Windows.end() ;it++)
			{
				if (win == *it)
				{
					CInterfaceGroup* buf = *it;

					CInterfaceGroup* pIGLast = rMG.Windows.back();
					if (pIGLast != NULL)
					{
						CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(pIGLast);
						if (pGC != NULL)
						{
							pGC->setHighLighted(false,0);
						}
					}

					rMG.Windows.erase(it);
					rMG.Windows.push_back(buf);
					break;
				}
			}
*/
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::setBackWindow(CInterfaceGroup* win)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			rMG.setBackWindow(win);
/*
			//find the window in the window list
			for (list<CInterfaceGroup*>::iterator it = rMG.Windows.begin(); it != rMG.Windows.end() ;it++)
			{
				if (win == *it)
				{
					CInterfaceGroup* buf = *it;
					rMG.Windows.erase(it);
					rMG.Windows.push_front(buf);
					break;
				}
			}
*/
		}
	}
}

// ------------------------------------------------------------------------------------------------
CInterfaceGroup		*CInterfaceManager::getTopWindow (uint8 nPriority) const
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			// return the first.
			if(rMG.PrioritizedWindows[nPriority].empty())
				return NULL;
			else
				return rMG.PrioritizedWindows[nPriority].back();
		}
	}
	return NULL;
}


// ------------------------------------------------------------------------------------------------
CInterfaceGroup		*CInterfaceManager::getBackWindow (uint8 nPriority) const
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			// return the first.
			if(rMG.PrioritizedWindows[nPriority].empty())
				return NULL;
			else
				return rMG.PrioritizedWindows[nPriority].front();
		}
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::enableModalWindow (CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG)
{
	// disable any modal before. release keyboard
	disableModalWindow();
	pushModalWindow(ctrlLaunchingModal, pIG);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::enableModalWindow (CCtrlBase *CtrlLaunchingModal, const std::string &groupName)
{
	CInterfaceGroup	*group= dynamic_cast<CGroupModal*>( getElementFromId(groupName) );
	if(group)
	{
		// enable the modal
		enableModalWindow(CtrlLaunchingModal, group);
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::disableModalWindow ()
{
	while (!_ModalStack.empty())
	{
		CModalWndInfo winInfo = _ModalStack.back();
		_ModalStack.pop_back(); // must pop back as early as possible because 'setActive' may trigger another 'popModalWindow', leading to a crash
		// disable old modal window
		if(winInfo.ModalWindow)
		{
			setBackWindow(winInfo.ModalWindow);
			winInfo.ModalWindow->setActive(false);
		}
	}

	// disable any context help
	_CurCtrlContextHelp = "";
	_DeltaTimeStopingContextHelp = 0;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::pushModalWindow(CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG)
{
	// enable the wanted modal
	if(pIG)
	{
		CModalWndInfo mwi;
		mwi.ModalWindow = pIG;
		mwi.CtrlLaunchingModal = ctrlLaunchingModal;
		// setup special group
		CGroupModal		*groupModal= dynamic_cast<CGroupModal*>(pIG);
		if(groupModal)
		{
			mwi.ModalExitClickOut = groupModal->ExitClickOut;
			mwi.ModalExitClickL = groupModal->ExitClickL;
			mwi.ModalExitClickR = groupModal->ExitClickR;
			mwi.ModalHandlerClickOut = groupModal->OnClickOut;
			mwi.ModalClickOutParams = groupModal->OnClickOutParams;
			mwi.ModalExitKeyPushed = groupModal->ExitKeyPushed;
			// update coords of the modal
			if(groupModal->SpawnOnMousePos)
			{
				groupModal->SpawnMouseX = _Pointer->getX();
				groupModal->SpawnMouseY = _Pointer->getY();

			}
		}
		else
		{
			// default for group not modal. Backward compatibility
			mwi.ModalExitClickOut = false;
			mwi.ModalExitClickL = false;
			mwi.ModalExitClickR = false;
			mwi.ModalExitKeyPushed = false;
		}

		_ModalStack.push_back(mwi);

		// update coords and activate the modal
		mwi.ModalWindow->invalidateCoords();
		mwi.ModalWindow->setActive(true);
		setTopWindow(mwi.ModalWindow);
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::pushModalWindow(CCtrlBase *ctrlLaunchingModal, const std::string &groupName)
{
	CInterfaceGroup	*group= dynamic_cast<CGroupModal*>( getElementFromId(groupName) );
	if(group)
	{
		// enable the modal
		enableModalWindow(ctrlLaunchingModal, group);
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::popModalWindow()
{
	if (!_ModalStack.empty())
	{
		CModalWndInfo winInfo = _ModalStack.back();
		_ModalStack.pop_back(); // must pop back as early as possible because 'setActive' may trigger another 'popModalWindow', leading to a crash
		if(winInfo.ModalWindow)
		{
			setBackWindow(winInfo.ModalWindow);
			winInfo.ModalWindow->setActive(false);
		}
		if (!_ModalStack.empty())
		{
			if(_ModalStack.back().ModalWindow)
			{
				_ModalStack.back().ModalWindow->setActive(true);
				setTopWindow(_ModalStack.back().ModalWindow);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::popModalWindowCategory(const std::string &category)
{
	for(;;)
	{
		if (_ModalStack.empty()) break;
		if (!_ModalStack.back().ModalWindow) break;
		CGroupModal *gm = dynamic_cast<CGroupModal *>((CInterfaceGroup*)(_ModalStack.back().ModalWindow));
		if (gm && gm->Category == category)
		{
			_ModalStack.back().ModalWindow->setActive(false);
			_ModalStack.pop_back();
		}
		else
		{
			break;
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::setCaptureKeyboard(CCtrlBase *c)
{
	CGroupEditBox	*oldEb= dynamic_cast<CGroupEditBox*>((CCtrlBase*)_CaptureKeyboard);
	CGroupEditBox	*newEb= dynamic_cast<CGroupEditBox*>(c);

	if (_CaptureKeyboard && _CaptureKeyboard != c)
	{
		_CaptureKeyboard->onKeyboardCaptureLost();
	}
	// If the old capturedKeyboard is an editBox and allow recoverFocusOnEnter
	if ( oldEb && oldEb->getRecoverFocusOnEnter() )
	{
		_OldCaptureKeyboard = _CaptureKeyboard;
	}
	if ( newEb )
	{
		CGroupEditBox::disableSelection();

		if (!newEb->getAHOnFocus().empty())
		{
			runActionHandler(newEb->getAHOnFocus(), newEb, newEb->getAHOnFocusParams());
		}

	}
	_CaptureKeyboard = c;
	notifyElementCaptured(c);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::resetCaptureKeyboard()
{
	CCtrlBase *captureKeyboard = _CaptureKeyboard;
	_OldCaptureKeyboard = NULL;
	_CaptureKeyboard = NULL;
	if (captureKeyboard)
	{
		captureKeyboard->onKeyboardCaptureLost();
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::runProcedure (const string &procName, CCtrlBase *pCaller,
									  const vector<string> &paramList)
{
	CstItProcedureMap	it= _ProcedureMap.find(procName);
	if(it!=_ProcedureMap.end())
	{
		// TMP TMP
		/*std::string params;
		for(uint k = 0; k < paramList.size(); ++k)
		{
			params += paramList[k] + " ";
		}

		nlinfo("runProcedure : %s : %s",procName.c_str(), params.c_str());*/
		const CProcedure	&proc= it->second;
		// Run all actions
		for(uint i=0;i<proc.Actions.size();i++)
		{
			const CAction		&action= proc.Actions[i];
			// test if the condition for the action is valid
			if (action.CondBlocks.size() > 0)
			{
				CInterfaceExprValue result;
				result.setBool(false);
				string cond;
				action.buildCond(paramList, cond);
				CInterfaceExpr::eval(cond, result, NULL);
				if (result.toBool())
				{
					if (!result.getBool()) continue;
				}
			}
			// build the params sting
			string params;
			action.buildParams(paramList, params);
			// run
			//nlwarning("step %d : %s, %s", (int) i, action.Action.c_str(), params.c_str());
			runActionHandler(action.Action, pCaller, params);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::setProcedureAction(const std::string &procName, uint actionIndex, const std::string &ah, const std::string &params)
{
	ItProcedureMap	it= _ProcedureMap.find(procName);
	if(it!=_ProcedureMap.end())
	{
		CProcedure	&proc= it->second;
		// set wanted action
		if(actionIndex<proc.Actions.size())
		{
			CAction		&action= proc.Actions[actionIndex];
			action.Action= ah;
			action.ParamBlocks.clear();
			action.ParamBlocks.resize(1);
			action.ParamBlocks[0].String= params;
		}
	}
}

// ------------------------------------------------------------------------------------------------
uint CInterfaceManager::getProcedureNumActions(const std::string &procName) const
{
	CstItProcedureMap	it= _ProcedureMap.find(procName);
	if(it!=_ProcedureMap.end())
	{
		const CProcedure	&proc= it->second;
		return (uint)proc.Actions.size();
	}
	else
		return 0;
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::getProcedureAction(const std::string &procName, uint actionIndex, std::string &ah, std::string &params) const
{
	CstItProcedureMap	it= _ProcedureMap.find(procName);
	if(it!=_ProcedureMap.end())
	{
		const CProcedure	&proc= it->second;
		if(actionIndex<proc.Actions.size())
		{
			const CAction		&action= proc.Actions[actionIndex];
			// if not a variable parametrized Params
			if(action.ParamBlocks.size()==1 && action.ParamBlocks[0].NumParam==-1)
			{
				ah= action.Action;
				params= action.ParamBlocks[0].String;
				return true;
			}
		}
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::startAnim (const string &animId)
{
	TAnimMap::iterator it = _AnimMap.find(animId);
	if (it == _AnimMap.end())
	{
		nlwarning ("anim %s not found", animId.c_str());
		return;
	}
	CInterfaceAnim *pIT = it->second;
	stopAnim (animId);
	pIT->start();
	_ActiveAnims.push_back (pIT);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::stopAnim (const string &animId)
{
	TAnimMap::iterator it = _AnimMap.find(animId);
	if (it == _AnimMap.end())
	{
		nlwarning ("anim %s not found", animId.c_str());
		return;
	}
	CInterfaceAnim *pIT = it->second;
	for (uint i = 0; i < _ActiveAnims.size(); ++i)
		if (_ActiveAnims[i] == pIT)
		{
			_ActiveAnims.erase (_ActiveAnims.begin()+i);
			if (!pIT->isFinished())
				pIT->stop();
			return;
		}
}


// ------------------------------------------------------------------------------------------------
void CInterfaceManager::messageBoxInternal(const string &msgBoxGroup, const ucstring &text, const string &masterGroup, TCaseMode caseMode)
{
	CInterfaceGroup *group= dynamic_cast<CInterfaceGroup*>(getElementFromId(masterGroup+":" + msgBoxGroup));
	CViewText		*viewText= dynamic_cast<CViewText*>(getElementFromId(masterGroup+":" + msgBoxGroup + ":text"));

	if (group && viewText)
	{
		viewText->setCaseMode(caseMode);
		viewText->setText(text);
		enableModalWindow(NULL, group);
		// don't understand why but need to update coords here
		group->updateCoords();
		group->updateCoords();
	}
}

// ------------------------------------------------------------------------------------------------
void	CInterfaceManager::messageBox(const ucstring &text, const string &masterGroup, TCaseMode caseMode)
{
	messageBoxInternal("message_box", text, masterGroup, caseMode);
}


// ------------------------------------------------------------------------------------------------
void CInterfaceManager::messageBoxWithHelp(const ucstring &text, const std::string &masterGroup,
										   const std::string &ahOnOk, const std::string &paramsOnOk,
										   TCaseMode caseMode)
{
	// replace the procedure "proc_valid_message_box_ok" action
	setProcedureAction("proc_message_box_with_help_ok", 1, ahOnOk, paramsOnOk);
	const char *mbName = "message_box_with_help";
	// if no action handler is wanted, then assume that
	// clicking 'ok' do not have any consequence, so allow exiting the message box by clicking
	// outside of it (this behavior is wanted on the login page, to allow to reclick on 'login' without
	// having to click 'ok' in the message box each time)
	CInterfaceGroup *group= dynamic_cast<CInterfaceGroup*>(getElementFromId(masterGroup+":" + mbName));
	CGroupModal *gm = dynamic_cast<CGroupModal *>(group);
	if (gm)
	{
		gm->ExitClickOut = ahOnOk.empty();
	}
	messageBoxInternal(mbName, text, masterGroup, caseMode);
}


// ------------------------------------------------------------------------------------------------
void	CInterfaceManager::validMessageBox(TValidMessageIcon icon, const ucstring &text, const std::string &ahOnOk,
	const std::string &paramsOnOk, const std::string &ahOnCancel, const std::string &paramsOnCancel, const string &masterGroup)
{
	CInterfaceGroup *group= dynamic_cast<CInterfaceGroup*>(getElementFromId(masterGroup+":valid_message_box"));
	CViewText		*viewText= dynamic_cast<CViewText*>(getElementFromId(masterGroup+":valid_message_box:text"));
	CViewBitmap		*viewBitmap= dynamic_cast<CViewBitmap*>(getElementFromId(masterGroup+":valid_message_box:icon_group:icon"));

	if (group && viewText)
	{
		// replace the procedure "proc_valid_message_box_ok" action
		setProcedureAction("proc_valid_message_box_ok", 1, ahOnOk, paramsOnOk);
		// replace the procedure "proc_valid_message_box_cancel" action
		setProcedureAction("proc_valid_message_box_cancel", 1, ahOnCancel, paramsOnCancel);

		// set text and icon
		viewText->setText(text);
		if(viewBitmap)
		{
			bool	active= true;
			if(icon==QuestionIconMsg)
				viewBitmap->setTexture("brick_default.tga");
			else if(icon==WarningIconMsg)
				viewBitmap->setTexture("W_warning.tga");
			else if(icon==ErrorIconMsg)
				viewBitmap->setTexture("No_Action.tga");
			else
				active= false;
			viewBitmap->setActive(active);
		}

		// Go
		enableModalWindow(NULL, group);
		// don't understand why but need to update coords here
		group->updateCoords();
		group->updateCoords();
	}
}


// ------------------------------------------------------------------------------------------------
bool	CInterfaceManager::getCurrentValidMessageBoxOnOk(string &ahOnOk, const std::string &masterGroup)
{
	// any modal window opened?
	CInterfaceGroup	*mw= getModalWindow();
	if(!mw)
		return false;

	// Is this modal window the valid_message_box window?
	CInterfaceGroup *group= dynamic_cast<CInterfaceGroup*>(getElementFromId(masterGroup+":valid_message_box"));
	if(mw==group)
	{
		// Ok, get the current procedure OnOk action
		string	dummyParams;
		if(getProcedureAction("proc_valid_message_box_ok", 1, ahOnOk, dummyParams))
			return true;
	}

	return false;
}


// ------------------------------------------------------------------------------------------------
void	CInterfaceManager::setContextHelpText(const ucstring &text)
{
	_ContextHelpText = text;
}


// ------------------------------------------------------------------------------------------------
void	CInterfaceManager::disableContextHelp()
{
	_CurCtrlContextHelp = "";
	_DeltaTimeStopingContextHelp = 0;
}

// ------------------------------------------------------------------------------------------------
void	CInterfaceManager::disableContextHelpForControl(CCtrlBase *pCtrl)
{
	if(!pCtrl)
		return;
	if(_CurCtrlContextHelp == pCtrl )
		disableContextHelp();
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::makeWindow(CInterfaceGroup *group)
{
	if (!group) return;
	uint32 i = 0;
	for (i = 0; i < _MasterGroups.size(); ++i)
	{
		if (_MasterGroups[i].Group == group->getParent())
				break;
	}

	if (i == _MasterGroups.size())
	{
		string stmp = string("not found master group for window: ")+group->getId();
		nlwarning (stmp.c_str());
		return;
	}
	else
	{
		// check if group hasn't been inserted twice.
		if (_MasterGroups[i].isWindowPresent(group))
		{
			nlwarning("Window inserted twice");
		}
		else
		{
			_MasterGroups[i].addWindow(group,group->getPriority());
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::unMakeWindow(CInterfaceGroup *group, bool noWarning)
{
	if (!group) return;
	uint32 i = 0;
	for (i = 0; i < _MasterGroups.size(); ++i)
	{
		if (_MasterGroups[i].Group == group->getParent())
				break;
	}

	if (i == _MasterGroups.size())
	{
		if (!noWarning)
		{
			string stmp = string("not found master group for window: ")+group->getId();
			nlwarning (stmp.c_str());
		}
		return;
	}
	else
	{
		// check if group hasn't been inserted twice.
		if (!_MasterGroups[i].isWindowPresent(group))
		{
			if (!noWarning)
				nlwarning("Window not inserted in master group");
		}
		else
		{
			_MasterGroups[i].delWindow(group);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::notifyElementCaptured(CCtrlBase *c)
{
	std::set<CCtrlBase *> seen;
	CCtrlBase *curr = c;
	while (curr)
	{
		seen.insert(curr);
		curr->elementCaptured(c);
		curr = curr->getParent();
	}
	// also warn the ctrl under the pointer
	for (uint i = 0; i < (uint) _CtrlsUnderPointer.size(); ++i)
	{
		if (!seen.count(_CtrlsUnderPointer[i]))
		{
			_CtrlsUnderPointer[i]->elementCaptured(c);
		}
	}
}


// ***************************************************************************
void	CInterfaceManager::enableMouseHandling(bool handle)
{
	_MouseHandlingEnabled= handle;
	if(!handle)
	{
		if(!_Pointer)
			return;

		// If Left captured, reset
		if( _CapturePointerLeft )
		{
			setCapturePointerLeft(NULL);
		}

		// Same for Right
		if( _CapturePointerRight )
		{
			setCapturePointerRight(NULL);
		}

		// Avoid any problem with modals
		disableModalWindow();
	}
}


// ***************************************************************************
void CInterfaceManager::displayDebugInfo(const ucstring &str, TSystemInfoMode mode /*=InfoMsg*/)
{
	if (PeopleInterraction.DebugInfo)
		PeopleInterraction.ChatInput.DebugInfo.displayMessage(str, getDebugInfoColor(mode), 2);
}

// ***************************************************************************
NLMISC::CRGBA CInterfaceManager::getDebugInfoColor(TSystemInfoMode mode)
{
	if (_NeutralColor == NULL) // not initialised ?
	{
		#define SYSTEM_INFO_COLOR_DB_PATH "UI:VARIABLES:SYSTEM_INFOS:COLORS"
		_NeutralColor = getDbProp(SYSTEM_INFO_COLOR_DB_PATH ":NEUTRAL");
		_WarningColor = getDbProp(SYSTEM_INFO_COLOR_DB_PATH ":WARNING");
		_ErrorColor = getDbProp(SYSTEM_INFO_COLOR_DB_PATH ":ERROR");
	}
	NLMISC::CRGBA color;
	switch(mode)
	{
		case InfoMsg: color.setPacked(_NeutralColor->getValue32()); break;
		case WarningMsg: color.setPacked(_WarningColor->getValue32()); break;
		case ErrorMsg: color.setPacked(_ErrorColor->getValue32()); break;
		default:
			color = CRGBA::White;
		break;
	}
	return color;
}

// ***************************************************************************
void CInterfaceManager::displaySystemInfo(const ucstring &str, const string &cat)
{
	CClientConfig::SSysInfoParam::TMode mode = CClientConfig::SSysInfoParam::Normal;
	CRGBA color = CRGBA::White;


	map<string, CClientConfig::SSysInfoParam>::const_iterator it = ClientCfg.SystemInfoParams.find(strlwr(cat));
	if (it != ClientCfg.SystemInfoParams.end())
	{
		mode = it->second.Mode;
		color = it->second.Color;
	}

	if (mode != CClientConfig::SSysInfoParam::OverOnly && mode != CClientConfig::SSysInfoParam::Around)
	{
		if (PeopleInterraction.SystemInfo)
			PeopleInterraction.ChatInput.SystemInfo.displayMessage(str, color, 2);
		else
		{
			CPeopleInterraction::CSysMsg sysMsg;
			sysMsg.Str = str;
			sysMsg.Cat = cat;
			PeopleInterraction.SystemMessageBuffer.push_back( sysMsg );
		}
	}

	// If over popup a string at the bottom of the screen
	if ((mode == CClientConfig::SSysInfoParam::Over) || (mode == CClientConfig::SSysInfoParam::OverOnly))
		InSceneBubbleManager.addMessagePopup(str, color);
	else if (mode == CClientConfig::SSysInfoParam::Center)
		InSceneBubbleManager.addMessagePopupCenter(str, color);
	else if (mode == CClientConfig::SSysInfoParam::Around && PeopleInterraction.AroundMe.Window)
		PeopleInterraction.ChatInput.AroundMe.displayMessage(str, color, 2);
}

// ***************************************************************************
CRGBA CInterfaceManager::getSystemInfoColor(const std::string &cat)
{
	CRGBA col = CRGBA::White;
	map<string, CClientConfig::SSysInfoParam>::const_iterator it = ClientCfg.SystemInfoParams.find(strlwr(cat));
	if (it != ClientCfg.SystemInfoParams.end())
		col = it->second.Color;
	return col;
}

// ***************************************************************************
void	CInterfaceManager::launchContextMenuInGame (const std::string &nameOfCM)
{
	// Launch the context menu in-game: can't appear while dragging an item
	if (CDBCtrlSheet::getDraggedSheet() == NULL)
	{
		if (_ModalStack.empty())
		{
			// We must be in-game !
			CInterfaceGroup *pMG = getMasterGroupFromId("ui:interface");
			// TMP nico : try with login screen:
			if (!pMG)
			{
				pMG = getMasterGroupFromId("ui:login");
			}
			if (!pMG)
			{
				pMG = getMasterGroupFromId("ui:outgame");
			}
			if ((pMG != NULL) && (pMG->getActive()))
			{
				CInterfaceElement *pIE = getElementFromId(nameOfCM);
				CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(pIE);
				if (pIG != NULL)
				{
					enableModalWindow (NULL, pIG);
				}
			}
		}
	}
}

// ***************************************************************************
void CInterfaceManager::registerClockMsgTarget(CCtrlBase *vb)
{
	if (!vb) return;
	if (isClockMsgTarget(vb))
	{
		nlwarning("<CInterfaceManager::registerClockMsgTarget> Element %s is already registered", vb->getId().c_str());
		return;
	}
	_ClockMsgTargets.push_back(vb);
}

// ***************************************************************************
void CInterfaceManager::unregisterClockMsgTarget(CCtrlBase *vb)
{
	if (!vb) return;
	std::vector<CCtrlBase*>::iterator it = std::find(_ClockMsgTargets.begin(), _ClockMsgTargets.end(), vb);
	if (it != _ClockMsgTargets.end())
	{
		_ClockMsgTargets.erase(it);
	}
}

// ***************************************************************************
bool CInterfaceManager::isClockMsgTarget(CCtrlBase *vb) const
{
	std::vector<CCtrlBase*>::const_iterator it = std::find(_ClockMsgTargets.begin(), _ClockMsgTargets.end(), vb);
	return it != _ClockMsgTargets.end();
}

// ***************************************************************************
void CInterfaceManager::setContentAlpha(uint8 alpha)
{
	_ContentAlpha = alpha;
	// update alpha of global color
	_GlobalColorForContent.A = alpha;/*(uint8) (( (uint16) _GlobalColor.A * (uint16) _ContentAlpha) >> 8);*/
}

// ***************************************************************************
void CInterfaceManager::setContainerAlpha(uint8 alpha)
{
	_ContainerAlpha = alpha;
	// update alpha of global color
	_GlobalColor.A = alpha;/*(uint8) (( (uint16) _GlobalColor.A * (uint16) _ContainerAlpha) >> 8);	*/
}


// ***************************************************************************
void CInterfaceManager::updateGroupContainerImage(CGroupContainer &gc, uint8 mode)
{
	if (mode >= MAX_NUM_MODES)
	{
		nlwarning("wrong desktop");
		return;
	}
	_Modes[mode].updateGroupContainerImage(gc);
}

// ***************************************************************************
void CInterfaceManager::removeGroupContainerImage(const std::string &groupName, uint8 mode)
{
	if (mode >= MAX_NUM_MODES)
	{
		nlwarning("wrong desktop");
		return;
	}
	_Modes[mode].removeGroupContainerImage(groupName);

}

// ***************************************************************************
void	CInterfaceManager::setMode(uint8 newMode)
{
	if (newMode >= MAX_NUM_MODES)
		return;

	if (newMode == _CurrentMode)
		return;

	// Check if we can change vdesk !
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority=0; nPriority < WIN_PRIORITY_MAX; ++nPriority)
			{
				list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
				list<CInterfaceGroup*>::const_iterator itw;
				for (itw = rList.begin(); itw!= rList.end(); itw++)
				{
					CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(*itw);
					if ((pGC != NULL)&&(pGC->getActive()))
					{
						// if this GC is a Full modal window, or if it is a modal son of another GC,
						if (pGC->isModal() || pGC->isModalSon())
						{
							setTopWindow(pGC);
							pGC->enableBlink(2);
							return;
						}
						else
						if (pGC->isGrayed())
						{
							// Make the corresponding child blink
							pGC->blinkAllSons();
							return;
						}
					}
				}
			}
		}
	}

	// check if there's a special behaviour with current captured ctrl that prevent from changing desktop
	if (_CapturePointerLeft != NULL)
	{
		if (!_CapturePointerLeft->canChangeVirtualDesktop()) return;
	}
	if (_CapturePointerRight != NULL)
	{
		if (!_CapturePointerRight->canChangeVirtualDesktop()) return;
	}


	_Modes[_CurrentMode].fromCurrentDesktop();
	_Modes[newMode].toCurrentDesktop();
	//CBotChatUI::refreshActiveWindows();

	_CurrentMode = newMode;
	checkCoords();
}

// ***************************************************************************
void	CInterfaceManager::resetMode(uint8 newMode)
{
	if (newMode >= MAX_NUM_MODES)
		return;
	NLMISC::contReset(_Modes[newMode]);
}


// for dump of interface content
struct CDumpedGroup
{
	CInterfaceGroup *Group;
	uint			 Depth; // depth in the tree
};

// ***************************************************************************
void CInterfaceManager::dumpUI(bool /* indent */)
{
	std::vector<CDumpedGroup> left;
	left.resize(_MasterGroups.size());
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		left[nMasterGroup].Group = _MasterGroups[nMasterGroup].Group;
		left[nMasterGroup].Depth = 0;
	}

	while (!left.empty())
	{
		CInterfaceGroup *ig = left.back().Group;
		if (ig)
		{
			uint currDepth = left.back().Depth;
			std::string id = ig->getId();
			std::string::size_type pos = id.find_last_of(':');
			if (pos != std::string::npos)
			{
				id = id.substr(pos + 1);
			}
			std::string info(currDepth * 4, ' ');
			info += id;
			info += toString(", address=0x%p", ig);
			nlinfo(info.c_str());
			// dump view & controls for this group
			for(uint k = 0; k < ig->getViews().size(); ++k)
			{
				std::string info(currDepth * 4, ' ');
				info += toString("View %d / %d : ", (int) k + 1, (int) ig->getViews().size());
				if (ig->getViews()[k])
				{
					info += id;
					info += toString(", type = %s, address=0x%p", typeid(*ig->getViews()[k]).name(), ig->getViews()[k]);
				}
				else
				{
					info += "<NULL>";
				}
				nlinfo(info.c_str());
			}
			//
			for(uint k = 0; k < ig->getControls().size(); ++k)
			{
				std::string info(currDepth * 4, ' ');
				info += toString("Ctrl %d / %d : ", (int) k + 1, (int) ig->getControls().size());
				if (ig->getControls()[k])
				{
					info += id;
					info += toString(", type = %s, address=0x%p", typeid(*ig->getControls()[k]).name(), ig->getControls()[k]);
				}
				else
				{
					info += "<NULL>";
				}
				nlinfo(info.c_str());
			}

			//
			left.pop_back();

			for(uint k = 0; k < ig->getNumGroup(); ++k)
			{
				CDumpedGroup dg;
				dg.Group = ig->getGroup(k);
				dg.Depth = currDepth + 1;
				left.push_back(dg);
			}
		}
	}
}

// ***************************************************************************
void CInterfaceManager::displayUIViewBBoxs(const std::string &uiFilter)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		for (uint8 nPriority=0; nPriority < WIN_PRIORITY_MAX; ++nPriority)
		{
			list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
			list<CInterfaceGroup*>::iterator it;
			for(it = rList.begin(); it != rList.end(); ++it)
			{
				if (*it) (*it)->renderWiredQuads(CInterfaceElement::RenderView, uiFilter);
			}
		}
	}
}

// ***************************************************************************
void CInterfaceManager::displayUICtrlBBoxs(const std::string &uiFilter)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		for (uint8 nPriority=0; nPriority < WIN_PRIORITY_MAX; ++nPriority)
		{
			list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
			list<CInterfaceGroup*>::iterator it;
			for(it = rList.begin(); it != rList.end(); ++it)
			{
				if (*it) (*it)->renderWiredQuads(CInterfaceElement::RenderCtrl, uiFilter);
			}
		}
	}
}

// ***************************************************************************
void CInterfaceManager::displayUIGroupBBoxs(const std::string &uiFilter)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		for (uint8 nPriority=0; nPriority < WIN_PRIORITY_MAX; ++nPriority)
		{
			list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
			list<CInterfaceGroup*>::iterator it;
			for(it = rList.begin(); it != rList.end(); ++it)
			{
				if (*it) (*it)->renderWiredQuads(CInterfaceElement::RenderGroup, uiFilter);
			}
		}
	}
}

// ***************************************************************************
void writeComboActionMap (const CActionsManager &actions, xmlNodePtr node, const string &context)
{
	// Get the combo defined
	const CActionsManager::TComboActionMap	&combos = actions.getComboActionMap ();
	CActionsManager::TComboActionMap::const_iterator ite = combos.begin ();
	while (ite != combos.end ())
	{
		// Key node
		xmlNodePtr keyNode = xmlNewChild ( node, NULL, (const xmlChar*)"key", NULL );

		// Props
		xmlSetProp (keyNode, (const xmlChar*)"name", (const xmlChar*)CEventKey::getStringFromKey(ite->first.Key).c_str());

		if (ite->first.KeyButtons&shiftKeyButton)
			xmlSetProp (keyNode, (const xmlChar*)"shift", (const xmlChar*)"1");
		if (ite->first.KeyButtons&ctrlKeyButton)
			xmlSetProp (keyNode, (const xmlChar*)"ctrl", (const xmlChar*)"1");
		if (ite->first.KeyButtons&altKeyButton)
			xmlSetProp (keyNode, (const xmlChar*)"menu", (const xmlChar*)"1");

		xmlSetProp (keyNode, (const xmlChar*)"action", (const xmlChar*)ite->second.Name.c_str());
		if (!(const xmlChar*)ite->second.Argu.empty())
			xmlSetProp (keyNode, (const xmlChar*)"params", (const xmlChar*)ite->second.Argu.c_str());

		// Context
		if (!context.empty ())
			xmlSetProp (keyNode, (const xmlChar*)"context", (const xmlChar*)context.c_str());

		ite++;
	}
}

// ***************************************************************************

void writeMacros (xmlNodePtr node)
{
	const std::vector<CMacroCmd> &macros = CMacroCmdManager::getInstance()->getMacros();
	for (uint i = 0; i < macros.size(); ++i)
	{
		macros[i].writeTo(node);
	}
}

// ***************************************************************************

bool	CInterfaceManager::saveKeys(const std::string &filename)
{
	bool ret = false;

	try
	{
		COFile file;
		if (file.open (filename))
		{
			COXml xmlStream;
			xmlStream.init (&file);

			xmlDocPtr doc = xmlStream.getDocument ();
			xmlNodePtr node = xmlNewDocNode(doc, NULL, (const xmlChar*)"interface_config", NULL);
			xmlDocSetRootElement (doc, node);

			writeComboActionMap (Actions, node, "");
			writeComboActionMap (EditActions, node, RZ_CATEGORY_EDIT);

			writeMacros (node);

			// Flush the stream
			xmlStream.flush();

			// Close the stream
			file.close ();

			// Done
			ret = true;
		}
		else
		{
			nlwarning ("Can't open the file %s", filename.c_str());
		}
	}
	catch (const Exception &e)
	{
		nlwarning ("Error while writing the file %s : %s. Remove it.", filename.c_str(), e.what ());
		CFile::deleteFile(filename);
	}
	return ret;
}

// ***************************************************************************
CInterfaceGroup		*CInterfaceManager::getLastEscapableTopWindow() const
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = WIN_PRIORITY_MAX; nPriority > 0; nPriority--)
			{
				const list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority-1];
				list<CInterfaceGroup*>::const_reverse_iterator	it;
				it= rList.rbegin();
				for(;it!=rList.rend();it++)
				{
					if((*it)->getActive() && (*it)->getEscapable())
						return *it;
				}
			}
		}
	}
	return NULL;
}

// ***************************************************************************
void CInterfaceManager::setWindowPriority (CInterfaceGroup *pWin, uint8 nNewPriority)
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			if (rMG.isWindowPresent(pWin))
			{
				rMG.delWindow(pWin);
				rMG.addWindow(pWin, nNewPriority);
			}
		}
	}
}

// ***************************************************************************
bool CInterfaceManager::deletePlayerConfig (const std::string &playerFileIdent)
{
	string fileName= "save/interface_" + playerFileIdent + ".icfg";
	return CFile::deleteFile(fileName);
}


// ***************************************************************************
bool CInterfaceManager::deletePlayerKeys (const std::string &playerFileIdent)
{
	string fileName = "save/keys_"+playerFileIdent+".xml";
	string fileNameEditor = "save/keys_r2ed_"+playerFileIdent+".xml";
	return CFile::deleteFile(fileName) && CFile::deleteFile(fileNameEditor);
}

// ***************************************************************************
void CInterfaceManager::log(const ucstring &str)
{
	if (_LogState)
	{
		// Open file with the name of the player
		const string fileName= "save/log_" + PlayerSelectedFileName + ".txt";
		FILE *f = fopen(fileName.c_str(), "at");
		if (f != NULL)
		{
			const string finalString = string(NLMISC::IDisplayer::dateToHumanString()) + " * " + str.toUtf8();
			fprintf(f, "%s\n", finalString.c_str());
		}
		fclose(f);
	}
}

// ***************************************************************************
void CInterfaceManager::clearAllEditBox()
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		for (uint8 nPriority=0; nPriority < WIN_PRIORITY_MAX; ++nPriority)
		{
			list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
			list<CInterfaceGroup*>::iterator it;
			for(it = rList.begin(); it != rList.end(); ++it)
			{
				CInterfaceGroup *pIG = *it;
				if (pIG != NULL)
					pIG->clearAllEditBox();
			}
		}
	}
}

// ***************************************************************************
void CInterfaceManager::restoreAllContainersBackupPosition()
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		for (uint8 nPriority=0; nPriority < WIN_PRIORITY_MAX; ++nPriority)
		{
			list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
			list<CInterfaceGroup*>::iterator it;
			for(it = rList.begin(); it != rList.end(); ++it)
			{
				if (*it) (*it)->restoreAllContainersBackupPosition();
			}
		}
	}
}


// ***************************************************************************
uint8	CInterfaceManager::getLastTopWindowPriority() const
{
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		const SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			return rMG.LastTopWindowPriority;
		}
	}
	return 0;
}


// ***************************************************************************
void	CInterfaceManager::removeRefOnCtrl (CCtrlBase *ctrlBase)
{
	if (_CurCtrlContextHelp == ctrlBase)
		_CurCtrlContextHelp = "";
	if (getCapturePointerLeft() == ctrlBase)
		setCapturePointerLeft(NULL);
	if (getCapturePointerRight() == ctrlBase)
		setCapturePointerRight (NULL);
	if (getCaptureKeyboard() == ctrlBase)
		setCaptureKeyboard(NULL);
	if (getOldCaptureKeyboard() == ctrlBase)
		setOldCaptureKeyboard(NULL);
	if (getDefaultCaptureKeyboard() == ctrlBase)
		setDefaultCaptureKeyboard(NULL);
	uint i;
	for (i=0; i<_CtrlsUnderPointer.size(); i++)
	{
		if (_CtrlsUnderPointer[i] == ctrlBase)
		{
			_CtrlsUnderPointer.erase (_CtrlsUnderPointer.begin()+i);
			i--;
		}
	}

	// Unregister from ClockMsgTargets
	unregisterClockMsgTarget (ctrlBase);
}


// ***************************************************************************
void CInterfaceManager::removeRefOnView (CViewBase *viewBase)
{
	uint i;
	for (i=0; i<_ViewsUnderPointer.size(); i++)
	{
		if (_ViewsUnderPointer[i] == viewBase)
		{
			_ViewsUnderPointer.erase (_ViewsUnderPointer.begin()+i);
			i--;
		}
	}
}

// ***************************************************************************
void CInterfaceManager::removeRefOnGroup (CInterfaceGroup *group)
{
	uint i;
	for (i=0; i<_GroupsUnderPointer.size(); i++)
	{
		if (_GroupsUnderPointer[i] == group)
		{
			_GroupsUnderPointer.erase (_GroupsUnderPointer.begin()+i);
			i--;
		}
	}
}



// ***************************************************************************

uint CInterfaceManager::getUserDblClickDelay()
{
	uint nVal = 50;
	CCDBNodeLeaf *pNL = getDbProp("UI:SAVE:DOUBLE_CLICK_SPEED");
	if (pNL != NULL)
		nVal = pNL->getValue32();
	uint dbclickDelay = (uint)(DOUBLE_CLICK_MIN + (DOUBLE_CLICK_MAX-DOUBLE_CLICK_MIN) * (float)nVal / 100.0f);
	return dbclickDelay;
}

// ***************************************************************************
void CInterfaceManager::submitEvent (const std::string &event)
{
	// Submit the event to the quick help system
	runActionHandler("submit_quick_help", NULL, event);
}

// ***************************************************************************
void CInterfaceManager::visit(CInterfaceElementVisitor *visitor)
{
	nlassert(visitor);
	for (uint nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		if (_MasterGroups[nMasterGroup].Group)
		{
			_MasterGroups[nMasterGroup].Group->visit(visitor);
		}
	}
}

// ***************************************************************************
void CInterfaceManager::incLocalSyncActionCounter()
{
	_LocalSyncActionCounter++;
}


// ***************************************************************************
void	CInterfaceManager::setOverExtendViewText(CViewText *vt, CRGBA backGround)
{
	_OverExtendViewText= vt;
	_OverExtendViewTextBackColor= backGround;
}

// ***************************************************************************
void	CInterfaceManager::drawOverExtendViewText()
{
//	CViewRenderer	&rVR= getViewRenderer();

	if(_OverExtendViewText)
	{
		CViewText	*vtSrc= safe_cast<CViewText*>((CInterfaceElement*)_OverExtendViewText);

		CInterfaceGroup *groupOver = getWindowForActiveMasterGroup("over_extend_view_text");
		if(groupOver)
		{
			CViewText *vtDst = dynamic_cast<CViewText*>(groupOver->getView("text"));
			if (vtDst != NULL)
			{
				// Copy all aspects to the view
				vtDst->setText (vtSrc->getText());
				vtDst->setFontSize (vtSrc->getFontSize());
				vtDst->setColor (vtSrc->getColor());
				vtDst->setModulateGlobalColor(vtSrc->getModulateGlobalColor());
				vtDst->setShadow(vtSrc->getShadow());
				vtDst->setShadowColor(vtSrc->getShadowColor());
				vtDst->setCaseMode(vtSrc->getCaseMode());
				vtDst->setUnderlined(vtSrc->getUnderlined());

				// setup background
				CViewBitmap	*pBack= dynamic_cast<CViewBitmap*>(groupOver->getView("midback"));
				CViewBitmap	*pOutline= dynamic_cast<CViewBitmap*>(groupOver->getView("midoutline"));
				if(pBack)
					pBack->setColor(_OverExtendViewTextBackColor);
				if(pOutline)
				{
					pOutline->setColor(vtSrc->getColor());
					pOutline->setModulateGlobalColor(vtSrc->getModulateGlobalColor());
				}

				// the group is the position of the overed text, but apply the delta of borders (vtDst X/Y)
				sint32 x = vtSrc->getXReal() - vtDst->getX();
				sint32 y = vtSrc->getYReal() - vtDst->getY();

				// update one time only to get correct W/H
				groupOver->updateCoords ();

				if(!vtSrc->isClampRight())
				{
					// clamped from the left part
					x += vtSrc->getWReal() - vtDst->getWReal();
				}

				// clamp to screen coords, and set
				if ((x+groupOver->getW()) > groupOver->getParent()->getWReal())
					x = groupOver->getParent()->getWReal() - groupOver->getW();
				if (x < 0)
					x = 0;
				if ((y+groupOver->getH()) > groupOver->getParent()->getHReal())
					y = groupOver->getParent()->getHReal() - groupOver->getH();
				if (y < 0)
					y = 0;

				// set pos
				groupOver->setX (x);
				groupOver->setY (y);

				// update coords 3 times is required
				groupOver->updateCoords ();
				groupOver->updateCoords ();
				groupOver->updateCoords ();

				// draw
				groupOver->draw ();
				// flush layers
				_ViewRenderer.flush();
			}
		}

		// Reset the ptr so at next frame, won't be rendered (but if reset)
		_OverExtendViewText= NULL;
	}
}


#if !FINAL_VERSION

// ***************************************************************************
NLMISC_COMMAND( localCounter, "Get value of local counter", "" )
{
	if (args.size() != 0) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->displaySystemInfo(ucstring(toString(im->getLocalSyncActionCounter())));
	return true;
}

#endif

// ***************************************************************************

NLMISC_COMMAND(loadui, "Load an interface file", "<loadui [all]/interface.xml>")
{
	if (args.size() != 1)
		return false;

	CInterfaceManager *im = CInterfaceManager::getInstance();

	std::vector<std::string> xmlFileNames;

	if (args[0] == "all")
		xmlFileNames = CInterfaceManager::getInGameXMLInterfaceFiles();
	else
		xmlFileNames.push_back (args[0]);

	bool result = im->parseInterface (xmlFileNames, true);
	#if !FINAL_VERSION
	if (result)
		CInterfaceManager::getInstance()->displaySystemInfo("File "+xmlFileNames.back()+" loaded successfully.");
	else
		CInterfaceManager::getInstance()->displaySystemInfo("File "+xmlFileNames.back()+" NOT loaded successully.");
	#endif

	// Invalidate the texts
	im->updateAllLocalisedElements();

	// reset captures
	im->setCapturePointerLeft(NULL);
	im->setCapturePointerRight(NULL);
	im->setOldCaptureKeyboard(NULL);
	im->setCaptureKeyboard(NULL);

	return result;
}


// ***************************************************************************
void CInterfaceManager::displayWebWindow(const string & name, const string & url)
{
	CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(getElementFromId(name));
	if (pIG != NULL)
	{
		pIG->setActive(true);
		pIG->updateCoords();
		pIG->center();
	}

	runActionHandler("browse", NULL, "name="+name+":content:html|url="+url);
}
/*
// ***************************************************************************
class CHandlerDispWebOnQuit : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		if (ClientCfg.Local)
			CInterfaceManager::getInstance()->runActionHandler("enter_modal", pCaller, "group=ui:interface:quit_dialog");
		else
			CInterfaceManager::getInstance()->displayWebWindow("ui:interface:web_on_quit", "http://213.208.119.190/igpoll/poll_form.php");
	}
};
REGISTER_ACTION_HANDLER (CHandlerDispWebOnQuit, "disp_web_on_quit");

// ***************************************************************************
class CHandlerExitWebOnQuit : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager::getInstance()->runActionHandler("quit_ryzom", pCaller);
	}
};
REGISTER_ACTION_HANDLER (CHandlerExitWebOnQuit, "exit_web_on_quit");
*/

// ***************************************************************************
// EMOTES
// ***************************************************************************

struct CEmoteEntry
{
	uint32 EmoteId;
	string Path;
	string Anim;
	bool   UsableFromClientUI;

	bool operator< (const CEmoteEntry & entry) const
	{
		string path1 = Path;
		string path2 = entry.Path;

		for(;;)
		{
			string::size_type pos1 = path1.find('|');
			string::size_type pos2 = path2.find('|');

			ucstring s1 = toUpper(CI18N::get(path1.substr(0, pos1)));
			ucstring s2 = toUpper(CI18N::get(path2.substr(0, pos2)));

			sint result = s1.compare(s2);
			if (result != 0)
				return (result < 0);

			if (pos1 == string::npos)
				return (pos2 != string::npos);
			if (pos2 == string::npos)
				return false;

			path1 = path1.substr(pos1 + 1);
			path2 = path2.substr(pos2 + 1);
		}
		return false;
	}
};

// ***************************************************************************
void CInterfaceManager::initEmotes()
{
	_EmotesInitialized = true;
	CTextEmotListSheet *pTELS = dynamic_cast<CTextEmotListSheet*>(SheetMngr.get(CSheetId("list.text_emotes")));
	if (pTELS == NULL)
		return;

	static list<CEmoteEntry> entries;
	if (entries.empty())
	{
		for (uint i = 0; i < pTELS->TextEmotList.size(); i++)
		{
			CEmoteEntry entry;
			entry.EmoteId = i;
			entry.Path = pTELS->TextEmotList[i].Path;
			entry.Anim = pTELS->TextEmotList[i].Anim;
			entry.UsableFromClientUI = pTELS->TextEmotList[i].UsableFromClientUI;
			entries.push_back(entry);
		}
		entries.sort();
	}

	// The list of behaviour missnames emotList
	CEmotListSheet *pEmotList = dynamic_cast<CEmotListSheet*>(SheetMngr.get(CSheetId("list.emot")));
	nlassert (pEmotList != NULL);
	nlassert (pEmotList->Emots.size() <= 255);

	// Get the focus beta tester flag
	bool betaTester = false;

	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	CSkillManager		*pSM = CSkillManager::getInstance();

	betaTester = pSM->isTitleUnblocked(CHARACTER_TITLE::FBT);
	string	previousMind = "";
	CGroupSubMenu *pFirstMenu = 0;

	for (list<CEmoteEntry>::const_iterator it = entries.begin(); it != entries.end(); it++)
	{
		uint32 nEmoteNb = (*it).EmoteId;
		string sState = (*it).Anim;
		string sName = (*it).Path;

		// Check that the emote can be added to UI
		// ---------------------------------------
		if( (*it).UsableFromClientUI == false )
		{
			continue;
		}

		// Check the emote reserved for FBT (hardcoded)
		// --------------------------------------------
		if (sState == "FBT" && !betaTester)
			continue;

		// Get the behaviour from the list of emotes
		// -----------------------------------------
		uint8 nBehav = 255;
		uint32 i, j;
		for (i = 0; i < pEmotList->Emots.size(); ++i)
			if (CAnimationStateSheet::getAnimationStateName(pEmotList->Emots[i]) == sState)
			{
				nBehav = (uint8)i;
				break;
			}

		// Add to the game context menu
		// ----------------------------
		uint32 nbToken = 1;
		for (i = 0; i < sName.size(); ++i)
			if (sName[i] == '|')
				nbToken++;

		CGroupMenu *pRootMenu = dynamic_cast<CGroupMenu*>(pIM->getElementFromId("ui:interface:user_chat_emote_menu"));
		nlassert(pRootMenu);

		CGroupSubMenu *pMenu = pRootMenu->getRootMenu();
		nlassert(pMenu);

		// Add to the game context menu
		// ----------------------------
		for (i = 0; i < nbToken; ++i)
		{
			string sTmp;
			if (i != (nbToken-1))
				sTmp = sName.substr(0,sName.find('|'));
			else
				sTmp = sName;

			// Look if this part of the path is already present
			bool bFound = false;
			for (j = 0; j < pMenu->getNumLine(); ++j)
			{
				if (sTmp == pMenu->getLineId(j))
				{
					bFound = true;
					break;
				}
			}


			if (!bFound) // Create it
			{
				if (i != (nbToken-1))
				{
					pMenu->addLine (CI18N::get(sTmp), "", "", sTmp);

					// Create a sub menu
					CGroupSubMenu *pNewSubMenu = new CGroupSubMenu(CViewBase::TCtorParam());
					pMenu->setSubMenu(j, pNewSubMenu);

					if (pFirstMenu == 0)
						pFirstMenu = pNewSubMenu;
				}
				else
				{
					// Create a line
					pMenu->addLine ("/" + CI18N::get(sTmp), "emote",
						"nb="+toString(nEmoteNb)+"|behav="+toString(nBehav), sTmp);
				}
			}

			// Jump to sub menu
			if (i != (nbToken-1))
			{
				pMenu = pMenu->getSubMenu(j);
				sName = sName.substr(sName.find('|')+1,sName.size());
			}
		}

		// Create new command
		// ------------------
		if (CI18N::hasTranslation(sName))
		{
			CGroupSubMenu *pMenu = pRootMenu->getRootMenu();

			// convert command to utf8 since emote translation can have strange chars
			string cmdName = (toLower(CI18N::get(sName))).toUtf8();
			if(ICommand::exists(cmdName))
			{
				nlwarning("Translation for emote %s already exist: '%s' exist twice", sName.c_str(), cmdName.c_str());
			}
			else
			{
				CEmoteCmd *pNewCmd = new CEmoteCmd(cmdName.c_str(), "", "");
				pNewCmd->EmoteNb = nEmoteNb;
				pNewCmd->Behaviour = nBehav;
				_EmoteCmds.push_back(pNewCmd);

				// Quick-Emote too ?
				for (i = 0; i< pMenu->getNumLine (); i++)
				{
					if (sName == pMenu->getLineId (i))
					{
						// Yeah that's a quick emote too; set command
						pMenu->addLineAtIndex (i,
								"@{FFFF}/" + toLower(CI18N::get(sName)),
								"emote", "nb="+toString(nEmoteNb)+"|behav="+toString(nBehav),
								"", "", "", false, false, true);

						pMenu->removeLine (i+1);
						break;
					}
				}
			}
		}
		else
		{
			nlwarning("No translation for emote %s", sName.c_str());
		}
	}

	// Insert separators
	if (pFirstMenu)
	{
		pFirstMenu->addSeparatorAtIndex (0, "Positive");
		pFirstMenu->addSeparatorAtIndex (4, "Neutral");
		pFirstMenu->addSeparatorAtIndex (8, "Negative");
	}

}

// ***************************************************************************
void CInterfaceManager::uninitEmotes()
{
	if( !_EmotesInitialized )
		return;
	_EmotesInitialized = false;

	// reset the emotes menu
	CTextEmotListSheet *pTELS = dynamic_cast<CTextEmotListSheet*>(SheetMngr.get(CSheetId("list.text_emotes")));
	if (pTELS != NULL && pTELS->TextEmotList.size() > 0)
	{
		// get the emotes menu id
		string sPath = pTELS->TextEmotList[0].Path;
		string sId = sPath.substr(0, sPath.find('|'));

		// get the emotes menu
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupMenu *pRootMenu = dynamic_cast<CGroupMenu*>(pIM->getElementFromId("ui:interface:game_context_menu"));
		if( pRootMenu )
		{
			CGroupSubMenu *pMenu = pRootMenu->getRootMenu();
			for (uint i = 0; i < pMenu->getNumLine(); ++i)
			{
				if (pMenu->getLineId(i) == sId)
				{
					pMenu = pMenu->getSubMenu(i);
					pMenu->reset();
					break;
				}
			}
		}
	}

	// clear commands
	for (uint32 i = 0; i < _EmoteCmds.size(); ++i)
		delete _EmoteCmds[i];
	_EmoteCmds.clear();
}

// ***************************************************************************
void CInterfaceManager::updateEmotes()
{
	uninitEmotes();
	initEmotes();
}

// ***************************************************************************
// Just call the action handler with good params
bool CInterfaceManager::CEmoteCmd::execute(const std::string &/* rawCommandString */, const vector<string> &args, CLog &/* log */, bool /* quiet */, bool /* human */)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	string customPhrase;
	if( args.size() > 0 )
	{
		customPhrase = args[0];
	}
	for(uint i = 1; i < args.size(); ++i )
	{
		customPhrase += " ";
		customPhrase += args[i];
	}
	pIM->runActionHandler("emote", NULL, "nb="+toString(EmoteNb)+"|behav="+toString(Behaviour)+"|custom_phrase="+customPhrase);
	return true;
}

// ----------------------------------------------------------------------------
static bool isSwimming()
{
	if (UserEntity != NULL)
		return (UserEntity->mode() == MBEHAV::SWIM || UserEntity->mode() == MBEHAV::MOUNT_SWIM);
	else
		return false;
}

static bool isStunned()
{
	if (UserEntity != NULL)
		return (UserEntity->behaviour() == MBEHAV::STUNNED);
	else
		return false;
}

static bool isDead()
{
	if (UserEntity != NULL)
		return (UserEntity->mode() == MBEHAV::DEATH);
	else
		return false;
}

// ***************************************************************************
class CHandlerEmote : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// An emote is 2 things : a phrase and an animation
		// Phrase is the phrase that server returns in chat system
		// Behav is the animation played
		// CustomPhrase is an user phrase which can replace default phrase
		string sPhraseNb = getParam(sParams, "nb");
		string sBehav = getParam(sParams, "behav");
		string sCustomPhrase = getParam(sParams, "custom_phrase");

		uint32 phraseNb;
		fromString(sPhraseNb, phraseNb);
		uint8 behaviour;
		fromString(sBehav, behaviour);

		MBEHAV::EBehaviour behavToSend = (MBEHAV::EBehaviour)(MBEHAV::EMOTE_BEGIN + behaviour);
		uint16 phraseNbToSend = (uint16)phraseNb;

		if (EAM)
		{
			const uint nbBehav = EAM->getNbEmots(); // Miscalled: this is the number of behaviour for all emotes
			if ((behaviour >= nbBehav) || (behaviour == 255))
				behavToSend = MBEHAV::IDLE;
		}
		else
		{
			if (behaviour == 255)
				behavToSend = MBEHAV::IDLE;
		}

		/* Emotes forbidden when dead, emotes with behav forbidden when
		 * stunned or swimming */
		if ( ( behavToSend != MBEHAV::IDLE && (isSwimming() || isStunned() || isDead() ) ) )
		{
			return;
		}

		if( sCustomPhrase.empty() )
		{
			// Create the message and send.
			const string msgName = "COMMAND:EMOTE";
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
			{
				out.serialEnum(behavToSend);
				out.serial(phraseNbToSend);
				NetMngr.push(out);
				//nlinfo("impulseCallBack : %s %d %d sent", msgName.c_str(), (uint32)behavToSend, phraseNbToSend);
			}
			else
				nlwarning("command 'emote': unknown message named '%s'.", msgName.c_str());
		}
		else
		{
			// Create the message and send.
			const string msgName = "COMMAND:CUSTOM_EMOTE";
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
			{
				ucstring ucstr;
				ucstr.fromUtf8(sCustomPhrase);

				if( sCustomPhrase == "none" )
				{
					if( behavToSend == MBEHAV::IDLE )
					{
						// display "no animation for emote"
						CInterfaceManager	*pIM= CInterfaceManager::getInstance();
						ucstring msg = CI18N::get("msgCustomizedEmoteNoAnim");
						string cat = getStringCategory(msg, msg);
						pIM->displaySystemInfo(msg, cat);
						return;
					}
				}
				else
				{
					ucstr = ucstring("&EMT&") + UserEntity->getDisplayName() + ucstring(" ") + ucstr;
				}

				out.serialEnum(behavToSend);
				out.serial(ucstr);
				NetMngr.push(out);
				//nlinfo("impulseCallBack : %s %d %s sent", msgName.c_str(), (uint32)behavToSend, sCustomPhrase.c_str());
			}
			else
				nlwarning("command 'emote': unknown message named '%s'.", msgName.c_str());
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerEmote, "emote");

// ***************************************************************************
bool	CInterfaceManager::testDragCopyKey()
{
	// hardcoded for now
	return Driver->AsyncListener.isKeyDown(KeyCONTROL) ||
		Driver->AsyncListener.isKeyDown(KeyLCONTROL) ||
		Driver->AsyncListener.isKeyDown(KeyRCONTROL);
}

// ***************************************************************************
void CInterfaceManager::setCapturePointerLeft(CCtrlBase *c)
{
	// additionally, abort any dragging
	if(CDBCtrlSheet::getDraggedSheet())
	{
		CDBCtrlSheet::getDraggedSheet()->abortDraging();
	}
	_CapturePointerLeft = c;
	notifyElementCaptured(c);
}

// ***************************************************************************
void CInterfaceManager::setCapturePointerRight(CCtrlBase *c)
{
	_CapturePointerRight = c;
	notifyElementCaptured(c);
}

// ***************************************************************************
void	CInterfaceManager::notifyMailAvailable()
{
	if (_CheckMailNode != NULL)
		_CheckMailNode->setValue32(1);
}

void	CInterfaceManager::notifyForumUpdated()
{
	if (_CheckForumNode != NULL)
		_CheckForumNode->setValue32(1);
}


// ***************************************************************************
void CInterfaceManager::resetTextIndex()
{
	uint32 nMasterGroup;
	for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];

		rMG.Group->invalidateTexts (true);
		for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
		{
			list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
			list<CInterfaceGroup*>::const_iterator itw;
			for (itw = rList.begin(); itw != rList.end(); itw++)
			{
				CInterfaceGroup *pIG = *itw;
				pIG->invalidateTexts (true);
			}
		}
	}
}

// ***************************************************************************
CInterfaceElement *getInterfaceResource(const std::string &key)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	return pIM->getElementFromId (key);
}


// ***************************************************************************
void	CInterfaceManager::sendStringToYuboChat(const ucstring &str)
{
	// If the yubo chat really connected
	if(_YuboChat.connected())
	{
		_YuboChat.send(str);
	}
}

// ***************************************************************************
void	CInterfaceManager::checkYuboChat()
{
	// flush the receive queue
	if(_YuboChat.connected())
	{
		std::list<ucstring>		toReceive;
		_YuboChat.receive(toReceive);
		while(!toReceive.empty())
		{
			PeopleInterraction.ChatInput.YuboChat.displayMessage(toReceive.front(), CRGBA::White, 2, NULL);
			toReceive.pop_front();
		}
	}
}

// ***************************************************************************
void	CInterfaceManager::connectYuboChat()
{
	// force disconnection if was connected
	_YuboChat.disconnect();

	uint32 KlientChatPort = 0;

	extern TSessionId HighestMainlandSessionId;
	switch(HighestMainlandSessionId.asInt())
	{
	case 101: KlientChatPort = 6002; break;	// fr
	case 102: KlientChatPort = 6003; break;	// de
	case 103: KlientChatPort = 6001; break;	// en
	case 301: KlientChatPort = 4000; break;	// yubo
	default:
		if(!ClientCfg.KlientChatPort.empty())
			fromString(ClientCfg.KlientChatPort, KlientChatPort);
		break;
	}

	// check if must reconnect
	if(KlientChatPort != 0 && !_YuboChat.connected())
	{
		// NB: hard code url, to avoid "client.cfg trojan"
		// (a client.cfg with an url pointing to a hacker site, to grab login/password)
		extern std::string LoginLogin, LoginPassword;
		_YuboChat.connect(string("chat.ryzom.com:")+toString(KlientChatPort), LoginLogin, LoginPassword);

		// Inform the interface that the chat is present
		getDbProp("UI:VARIABLES:YUBO_CHAT_PRESENT")->setValue32(1);
	}
}

// ***************************************************************************
bool	CInterfaceManager::executeLuaScript(const std::string &luaScript, bool smallScript)
{
	H_AUTO ( RZ_Interface_executeLuaScript )

	nlassert(_LuaState);
	try
	{
		if(smallScript)
			_LuaState->executeSmallScript(luaScript);
		else
			_LuaState->executeScript(luaScript);
	}
	catch(const ELuaError &e)
	{
		std::string msg = e.luaWhat();
		char filename[MAX_PATH];
		char exceptionName[MAX_PATH];
		int line;
		// Hamster: quick fix on AJM code but sscanf is still awfull
		if (sscanf(msg.c_str(), "%s: %s.lua:%d:",exceptionName, filename, &line) == 3) // NB: test not exact here, but should work in 99,9 % of cases
		{
			msg = CLuaIHM::createGotoFileButtonTag(filename, line) + msg;
			nlwarning(formatLuaErrorNlWarn(msg).c_str());
			displaySystemInfo(formatLuaErrorSysInfo(msg));
		}
		else	// AJM: handle the other 0.1% of cases
		{
			// Yoyo: seems that previous test doesn't work.... btw, must still print the message please...
			nlwarning(formatLuaErrorNlWarn(msg).c_str());
			displaySystemInfo(formatLuaErrorSysInfo(msg));
		}
		return false;
	}
	return true;
}

// ***************************************************************************
void	CInterfaceManager::reloadAllLuaFileScripts()
{
	std::set<std::string>::const_iterator	it= _LuaFileScripts.begin();
	for(;it!=_LuaFileScripts.end();it++)
	{
		string	error;
		// if fail to reload a script, display the error code
		if(!loadLUA(*it, error))
		{
			displaySystemInfo(formatLuaErrorSysInfo(error));
		}
	}
}

// ***************************************************************************
std::vector<std::string>		CInterfaceManager::getInGameXMLInterfaceFiles()
{
	// Original Files
	vector<string>	ret;
	ret= ClientCfg.XMLInterfaceFiles;

	// Resolve any conflict (with CPath scheme, AddOn Should take the precedence)
	// But still preserve order given in XMLInterfaceFiles (important for config.xml for instance)
	set<string>		fileSet;
	for(uint i=0;i<ret.size();i++)
	{
		fileSet.insert(ret[i]);
	}

	// Add R2 Editor .xml. This is removed as it will not be done when initializing CEditor
//	if (ClientCfg.R2EDEnabled)
//	{
//		// Add them to 'ret', only if not already inserted
//		//	since parser will crash on any duplicates
//		for(uint i=0;i<ClientCfg.XMLR2EDInterfaceFiles.size();i++)
//		{
//			if(fileSet.find(ClientCfg.XMLR2EDInterfaceFiles[i])==fileSet.end())
//			{
//				fileSet.insert(ClientCfg.XMLR2EDInterfaceFiles[i]);
//				ret.push_back(ClientCfg.XMLR2EDInterfaceFiles[i]);
//			}
//		}
//	}

	// Get Addons .xml
	vector<string>	adds;
	InterfaceAddOnManager.getFiles("*.xml", adds);

	// Add them to 'ret', only if not already inserted
	for(uint i=0;i<adds.size();i++)
	{
		if(fileSet.find(adds[i])==fileSet.end())
		{
			fileSet.insert(adds[i]);
			ret.push_back(adds[i]);
		}
	}

	return  ret;
}

// ***************************************************************************
void		CInterfaceManager::formatLuaStackContext(std::string &stackContext)
{
	stackContext= string("@{FC8A}") + stackContext + "@{FC8F} ";
}
std::string CInterfaceManager::formatLuaErrorNlWarn(const std::string &error)
{
	// Remove color tags (see formatLuaErrorSC())
	std::string		ret= error;
	strFindReplace(ret, "@{FC8A}", "");
	strFindReplace(ret, "@{FC8F}", "");
	return ret;
}

// ***************************************************************************
void		CInterfaceManager::dumpLuaString(const std::string &str)
{
	nlinfo(str.c_str());
	displaySystemInfo(formatLuaErrorSysInfo(str));
}

// ***************************************************************************
void		CInterfaceManager::getLuaValueInfo(std::string &str, sint index)
{
	nlassert(_LuaState);
	CLuaState	&ls= *_LuaState;

	sint	type= ls.type(index);
	if(type==LUA_TNIL)
		str= "nil";
	else if(type==LUA_TNUMBER)
		str= NLMISC::toString(ls.toNumber(index));
	else if(type==LUA_TBOOLEAN)
		str= ls.toBoolean(index)?"true":"false";
	else if(type==LUA_TSTRING)
	{
		ls.toString(index, str);
		str= toString("'") + str + toString("'");
	}
	else
	{
		str= ls.getTypename(type);
		str+= ":";
		str+= NLMISC::toString("%p", ls.toPointer(index));
		// If its a table, append the size.
		if(type==LUA_TTABLE)
		{
			ls.pushNil();		// first key
			uint	count= 0;
			while (ls.next(index-1))
			{
				ls.pop();		// remove 'value'; keeps `key' for next iteration
				count++;
			}
			str+= NLMISC::toString(" (size=%d)", count);
		}
		// If its a Userdata, try to display UI info
		else if(type==LUA_TUSERDATA)
		{
			if(CLuaIHM::isUIOnStack(ls, index))
			{
				CInterfaceElement	*ui= CLuaIHM::getUIOnStack(ls, index);
				str+= NLMISC::toString(" (ui=%p)", ui);
			}
		}
	}
}

// ***************************************************************************
void		CInterfaceManager::dumpLuaKeyValueInfo(uint recursTableLevel, uint tabLevel)
{
	nlassert(_LuaState);
	CLuaState	&ls= *_LuaState;
	CLuaStackChecker lsc(&ls);

	// Dump Key Str
	string	key;
	getLuaValueInfo(key, -2);
	// Dump Value Str
	string	value;
	getLuaValueInfo(value, -1);

	// display.
	string	res;
	// append tab for table hierarchy
	for(uint i=0;i<tabLevel;i++)
		res+= "    ";
	// display key and value
	res+= key + " == " + value;
	dumpLuaString(res);

	// If the value is a table, and can recurs dumping
	if(recursTableLevel>0 && ls.type(-1)==LUA_TTABLE)
	{
		ls.pushNil();		// first key
		while (ls.next(-2))
		{
			// display the key value pair of this table (recurs)
			dumpLuaKeyValueInfo(recursTableLevel-1, tabLevel+1);
			ls.pop();		// remove 'value'; keeps `key' for next iteration
		}
	}

}

// ***************************************************************************
void		CInterfaceManager::dumpLuaState(uint detail)
{
	if(!_LuaState)
	{
		dumpLuaString("LUA State not created");
		return;
	}

	// clamp detailed info to 2 (display at max content of eaxh Env of each group)
	clamp(detail, 0U, 2U);

	// Dump the Memory State
	dumpLuaString(NLMISC::toString("Memory Used : %d Kb", _LuaState->getGCCount()));
	dumpLuaString(NLMISC::toString("GC Threshold: %d Kb", _LuaState->getGCThreshold()));

	// If want to display some detailed info
	if(detail>0)
	{
		CLuaState	&ls= *_LuaState;
		CLuaStackChecker lsc(&ls);

		// *** Dump all Lua Env Tables
		ls.push(IHM_LUA_ENVTABLE);
		ls.getTable(LUA_REGISTRYINDEX);		// __ui_envtable
		ls.pushNil();		// first key
		uint	count= 0;
		while (ls.next(-2))
		{
			// `key' is at index -2 and `value' at index -1
			dumpLuaKeyValueInfo(detail-1, 1);
			ls.pop();		// remove 'value'; keeps `key' for next iteration
			count++;
		}
		// pop table
		ls.pop();

		dumpLuaString(NLMISC::toString("Number of EnvTable for ui groups: %d", count));
	}
}

// ***************************************************************************
void		CInterfaceManager::luaGarbageCollect()
{
	if(!_LuaState)
		return;
	dumpLuaString("Collecting Garbaged LUA variables");
	_LuaState->setGCThreshold(0);
	dumpLuaString(NLMISC::toString("Memory Used : %d Kb", _LuaState->getGCCount()));
}

// ***************************************************************************
void CInterfaceManager::hideAllWindows()
{
	for (uint nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
			{
				list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
				list<CInterfaceGroup*>::const_iterator itw;
				for (itw = rList.begin(); itw!= rList.end();)
				{
					CInterfaceGroup *pIG = *itw;
					itw++;	// since setActive invalidate the iterator, be sure we move to the next one before
					pIG->setActive(false);
				}
			}
		}
	}
}

// ***************************************************************************
void CInterfaceManager::hideAllNonSavableWindows()
{
	for (uint nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		SMasterGroup &rMG = _MasterGroups[nMasterGroup];
		if (rMG.Group->getActive())
		{
			for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
			{
				list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
				list<CInterfaceGroup*>::const_iterator itw;
				for (itw = rList.begin(); itw!= rList.end();)
				{
					CInterfaceGroup *pIG = *itw;
					CGroupContainer *cont = dynamic_cast<CGroupContainer *>(pIG);
					itw++;	// since setActive invalidate the iterator, be sure we move to the next one before
					if (!cont || !cont->isSavable())
					{
						pIG->setActive(false);
					}
				}
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::createLocalBranch(const std::string &fileName, NLMISC::IProgressCallback &progressCallBack)
{
	try
	{
		CIFile file;
		if (file.open (fileName))
		{
			// Init an xml stream
			CIXml read;
			read.init (file);

			//Parse the parser output!!!
			CCDBNodeBranch *localNode = new CCDBNodeBranch("LOCAL");
			localNode->init( read.getRootNode (), progressCallBack );
			_DbRootNode->attachChild(localNode,"LOCAL");

			// Create the observers for auto-copy SERVER->LOCAL of inventory
			ServerToLocalAutoCopyInventory.init("INVENTORY");

			// Create the observers for auto-copy SERVER->LOCAL of exchange
			ServerToLocalAutoCopyExchange.init("EXCHANGE");

			// Create the observers for auto-copy SERVER->LOCAL of dm (animator) gift
			ServerToLocalAutoCopyDMGift.init("DM_GIFT");

			// Create the observers for auto-copy SERVER->LOCAL of context menu
			ServerToLocalAutoCopyContextMenu.init("TARGET:CONTEXT_MENU");

			// Create the observers for auto-copy SERVER->LOCAL of Skill Points
			ServerToLocalAutoCopySkillPoints.init("USER");
		}
	}
	catch (const Exception &e)
	{
		// Output error
		nlwarning ("CFormLoader: Error while loading the form %s: %s", fileName.c_str(), e.what());
	}
}

// ------------------------------------------------------------------------------------------------
#ifdef NL_OS_WINDOWS
#	pragma warning (push)
#	pragma warning (disable : 4355)			// 'this' used in base member initializer list
#endif
CInterfaceManager::CServerToLocalAutoCopy::CServerToLocalAutoCopy() : _LocalObserver(*this), _ServerObserver(*this)
{
	_ServerCounter= NULL;
	_UpdateList.reserve(300);
	_LocalUpdating= false;
}
#ifdef NL_OS_WINDOWS
#	pragma warning (pop)
#endif

// ------------------------------------------------------------------------------------------------
// unhook from everything we are tangled up in
void CInterfaceManager::CServerToLocalAutoCopy::release()
{
	_Nodes.clear();
	_ServerCounter = NULL;
	_ServerNodeMap.clear();
	_LocalNodeMap.clear();
	_UpdateList.clear();
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::CServerToLocalAutoCopy::buildRecursLocalLeaves(CCDBNodeBranch *branch, std::vector<CCDBNodeLeaf*> &leaves)
{
	for(uint i=0;i<branch->getNbNodes();i++)
	{
		ICDBNode	*node= branch->getNode(i);
		if(node)
		{
			CCDBNodeLeaf	*leaf= dynamic_cast<CCDBNodeLeaf*>(node);
			if(leaf)
			{
				// just append to list
				leaves.push_back(leaf);
			}
			else
			{
				// recurs if a branch (should be...)
				CCDBNodeBranch	*sonBranch= dynamic_cast<CCDBNodeBranch*>(node);
				if(sonBranch)
					buildRecursLocalLeaves(sonBranch, leaves);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::CServerToLocalAutoCopy::init(const std::string &dbPath)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Get the synchronisation Counter in Server DB
	_ServerCounter= pIM->getDbProp(string("SERVER:") + dbPath + ":COUNTER", false);

	// if found
	if(_ServerCounter)
	{
		// **** Add Observers on all nodes
		// add the observers when server node change
		pIM->addDBObserver(&_ServerObserver, string("SERVER:") + dbPath);

		// add the observers when local node change
		pIM->addDBObserver(&_LocalObserver, string("LOCAL:") + dbPath);

		// **** Init the Nodes shortcut
		// Parse all Local Nodes
		CCDBNodeBranch	*localBranch= pIM->getDbBranch(string("LOCAL:") + dbPath);
		if(localBranch)
		{
			uint	i;
			std::vector<CCDBNodeLeaf*>		leaves;
			buildRecursLocalLeaves(localBranch, leaves);

			// --- build _Nodes
			_Nodes.reserve(leaves.size());
			for(i=0;i<leaves.size();i++)
			{
				CCDBNodeLeaf	*localLeaf= leaves[i];

				// get the SERVER associated node name
				string	serverLeafStr= *localLeaf->getName();
				CCDBNodeBranch* parent= localLeaf->getParent();
				while( *parent->getName()!="LOCAL" )
				{
					serverLeafStr= *parent->getName()+":"+serverLeafStr;
					parent= parent->getParent();
				}
				serverLeafStr= "SERVER:" + serverLeafStr;

				// try then to get this server node
				CCDBNodeLeaf	*serverLeaf= pIM->getDbProp(serverLeafStr, false);
				if(serverLeaf)
				{
					// Both server and local leaves exist, ok, append to _Nodes
					CNode	node;
					node.ServerNode= serverLeaf;
					node.LocalNode= localLeaf;
					_Nodes.push_back(node);
				}
			}

			// --- Init the maps
			_ServerNodeMap.reserve(leaves.size());
			_LocalNodeMap.reserve(leaves.size());
			// For all valid _Nodes, insert in "map"
			for(i=0;i<_Nodes.size();i++)
			{
				CNodeLocalComp	lc;
				CNodeServerComp	sc;
				lc.Node= &_Nodes[i];
				sc.Node= &_Nodes[i];
				_LocalNodeMap.push_back(lc);
				_ServerNodeMap.push_back(sc);
			}
			// then sort
			sort(_LocalNodeMap.begin(), _LocalNodeMap.end());
			sort(_ServerNodeMap.begin(), _ServerNodeMap.end());

		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::CServerToLocalAutoCopy::onServerChange(ICDBNode *serverNode)
{
	if(_Nodes.empty())
		return;
	CCDBNodeLeaf *serverLeaf = safe_cast<CCDBNodeLeaf *>(serverNode);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Add the leaf to the update list. only if not the counter
	if(serverLeaf != _ServerCounter)
	{
		// build the map key
		CNode				nodeComp;
		CNodeServerComp		sc;
		nodeComp.ServerNode= serverLeaf;
		sc.Node= &nodeComp;
		// try to find the node associated to this server leaf
		uint	index= searchLowerBound(_ServerNodeMap, sc);
		// if found
		if( index>0 || _ServerNodeMap[0].Node->ServerNode==serverLeaf )
		{
			CNode	*node= _ServerNodeMap[index].Node;
			// if this node is not already inserted
			if(!node->InsertedInUpdateList)
			{
				// insert
				node->InsertedInUpdateList= true;
				_UpdateList.push_back(node);
			}
		}
	}

	// if the client and server are synchonized.
	if( ClientCfg.Local || pIM->localActionCounterSynchronizedWith(_ServerCounter) )
	{
		// update all leaves
		for(uint i=0;i<_UpdateList.size();i++)
		{
			CNode	*node= _UpdateList[i];

			_LocalUpdating= true;
			node->LocalNode->setValue64(node->ServerNode->getValue64());
			_LocalUpdating= false;

			// reset inserted flag
			node->InsertedInUpdateList= false;
		}

		// clear update list
		_UpdateList.clear();
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::CServerToLocalAutoCopy::onLocalChange(ICDBNode *localNode)
{
	if(_Nodes.empty())
		return;

	// if the local changes because of localLeaf->setValue64() in onServerChange(), no-op !!!
	if(_LocalUpdating)
		return;

	CCDBNodeLeaf *localLeaf = safe_cast<CCDBNodeLeaf *>(localNode);

	// Add the leaf to the update list
	// build the map key
	CNode				nodeComp;
	CNodeLocalComp		lc;
	nodeComp.LocalNode= localLeaf;
	lc.Node= &nodeComp;
	// try to find the node associated to this local leaf
	uint	index= searchLowerBound(_LocalNodeMap, lc);
	// if found
	if( index>0 || _LocalNodeMap[0].Node->LocalNode==localLeaf )
	{
		CNode	*node= _LocalNodeMap[index].Node;
		// if this node is not already inserted
		if(!node->InsertedInUpdateList)
		{
			// insert
			node->InsertedInUpdateList= true;
			_UpdateList.push_back(node);
		}
	}
}

// ------------------------------------------------------------------------------------------------
char* CInterfaceManager::getTimestampHuman(const char* format /* "[%H:%M:%S] " */)
{
	static char cstime[25];
	time_t date;
	time (&date);
	struct tm *tms = localtime(&date);
	if (tms)
	{
		strftime(cstime, 25, format, tms);
	}
	else
	{
		strcpy(cstime, "");
	}

	return cstime;
}


/*
 * Parse tokens in a chatmessage or emote
 * 
 * Valid subjects:
 * $me$
 * $t$
 * $tt$
 * $tm1$..$tm8$
 * 
 * Valid parameters:
 * $<subject>.name$
 * $<subject>.title$
 * $<subject>.race$ 
 * $<subject>.guild$ 
 * $<subject>.gs(m/f/n)$
 *
 * Default parameter if parameter result is empty:
 * $<subject>.<parameter>/<default>$
 * 
 * All \d's in default parameter remove a following character.
 */
bool CInterfaceManager::parseTokens(ucstring& ucstr)
{
	ucstring str = ucstr;
	ucstring start_token("$");
	ucstring end_token("$");
	size_t start_pos = 0;
	size_t end_pos = 1;

	sint endless_loop_protector = 0;
	while ((start_pos < str.length() - 1) &&
		((start_pos = str.find(start_token, start_pos)) != string::npos))
	{
		endless_loop_protector++;
		if (endless_loop_protector > 100)
		{
			break; 
		}

		// Get the whole token substring first
		end_pos = str.find(end_token, start_pos + 1);

		if ((start_pos == ucstring::npos) || 
			(end_pos   == ucstring::npos) || 
			(end_pos   <= start_pos + 1))
		{
			// Wrong formatting; give up on this one.
			start_pos = max(start_pos, end_pos);
			continue;
		}

		// Get everything between the two "$"
		size_t token_start_pos = start_pos + start_token.length();
		size_t token_end_pos   = end_pos - end_token.length();
		if (token_start_pos < token_end_pos)
		{
			// Wrong formatting; give up on this one.
			start_pos = end_pos;
			continue;
		}

		ucstring token_whole = str.luabind_substr(start_pos, end_pos - start_pos + 1);
		ucstring token_string = token_whole.luabind_substr(1, token_whole.length() - 2);
		ucstring token_replacement = token_whole;
		ucstring token_default = token_whole;

		ucstring token_subject;
		ucstring token_param;

		// Does the token have a parameter?
		// If not it is 'name' by default
		vector<ucstring> token_vector;
		vector<ucstring> param_vector;
		splitUCString(token_string, ucstring("."), token_vector);
		if (token_vector.size() == 0)
		{
			// Wrong formatting; give up on this one.
			start_pos = end_pos;
			continue;
		}
		token_subject = token_vector[0];
		if (token_vector.size() == 1)
		{
			splitUCString(token_subject, ucstring("/"), param_vector);
			token_subject = (param_vector.size() > 0) ? param_vector[0] : ucstring("");
			token_param = ucstring("name");
		}
		else if (token_vector.size() > 1)
		{
			token_param = token_vector[1];
			if (token_param.luabind_substr(0, 3) != ucstring("gs("))
			{
				splitUCString(token_vector[1], ucstring("/"), param_vector);
				token_param = (param_vector.size() > 0) ? param_vector[0] : ucstring("");
			}
		}

		// Get any default value, if not gs
		sint extra_replacement = 0;
		if (token_param.luabind_substr(0, 3) != ucstring("gs("))
		{
			if (param_vector.size() == 2)
			{
				// Set default value
				token_replacement = param_vector[1];
				// Delete following chars for every '\d' in default
				string::size_type token_replacement_pos;
				while ((token_replacement_pos = token_replacement.find(ucstring("\\d"))) != string::npos)
				{
					token_replacement.replace(token_replacement_pos, 2, ucstring(""));
					extra_replacement++;
				}
				token_default = token_replacement;
			}
		}

		CEntityCL *pTokenSubjectEntity = NULL;

		if (token_subject == ucstring("me"))
		{
			pTokenSubjectEntity = static_cast<CEntityCL*>(UserEntity);
		}
		else if (token_subject == ucstring("t"))
		{
			// Target
			uint targetSlot = UserEntity->targetSlot();
			pTokenSubjectEntity = EntitiesMngr.entity(targetSlot);
		}
		else if (token_subject == ucstring("tt"))
		{
			// Target's target
			uint targetSlot = UserEntity->targetSlot();
			CEntityCL *target = EntitiesMngr.entity(targetSlot);

			if (target)
			{
				// Check the new slot.
				CLFECOMMON::TCLEntityId newSlot = target->targetSlot();
				CEntityCL* pE = EntitiesMngr.entity(newSlot);
				if (pE)
				{
					pTokenSubjectEntity = pE;
				}
			}
		}
		else if ((token_subject.length() == 3) &&
			     (token_subject.luabind_substr(0, 2) == ucstring("tm")))
		{
			// Teammate
			uint indexInTeam = 0;
			fromString(token_subject.luabind_substr(2, 1).toString(), indexInTeam);

			// Make 0-based
			--indexInTeam;
			if (indexInTeam < PeopleInterraction.TeamList.getNumPeople() )
			{
				// Index is the database index (serverIndex() not used for team list)
				CCDBNodeLeaf *pNL = CInterfaceManager::getInstance()->getDbProp( NLMISC::toString(TEAM_DB_PATH ":%hu:NAME", indexInTeam ), false);
				if (pNL && pNL->getValueBool() )
				{	
					// There is a character corresponding to this index
					pNL = CInterfaceManager::getInstance()->getDbProp( NLMISC::toString( TEAM_DB_PATH ":%hu:UID", indexInTeam ), false );
					if (pNL)
					{
						CLFECOMMON::TClientDataSetIndex compressedIndex = pNL->getValue32();

						// Search entity in vision
						CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex( compressedIndex );
						if (entity)
						{
							pTokenSubjectEntity = entity;
						}
					}
				}
			}
		}
		else
		{
			// Unknown token subject, skip it
			start_pos = end_pos;
			continue;
		}

		if (pTokenSubjectEntity != NULL)
		{
			// Parse the parameter
			if (token_param == ucstring("name"))
			{
				ucstring name = pTokenSubjectEntity->getDisplayName();
				// special case where there is only a title, very rare case for some NPC
				if (name.empty())
				{
					name = pTokenSubjectEntity->getTitle();
				}
				token_replacement = name.empty() ? token_replacement : name;
			}
			else if (token_param == ucstring("title"))
			{
				ucstring title = pTokenSubjectEntity->getTitle();
				token_replacement = title.empty() ? token_replacement : title;
			}
			else if (token_param == ucstring("race"))
			{
				CCharacterCL *pC = dynamic_cast<CCharacterCL*>(pTokenSubjectEntity);
				if (pC)
				{
					EGSPD::CPeople::TPeople race = pC->people();
					if (race >= EGSPD::CPeople::Playable && race <= EGSPD::CPeople::EndPlayable)
					{
						ucstring srace = NLMISC::CI18N::get("io" + EGSPD::CPeople::toString(race));
						token_replacement = srace.empty() ? token_replacement : srace;
					}
				}
			}
			else if (token_param == ucstring("guild"))
			{
				STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
				ucstring ucGuildName;
				if (pSMC->getString(pTokenSubjectEntity->getGuildNameID(), ucGuildName))
				{
					token_replacement = ucGuildName.empty() ? token_replacement : ucGuildName;
				}
			}
			else if (token_param.luabind_substr(0, 3) == ucstring("gs(") &&
				token_param.luabind_substr(token_param.length() - 1 , 1) == ucstring(")"))
			{
				// Gender string
				vector<ucstring> strList;
				ucstring gender_string = token_param.luabind_substr(3, token_param.length() - 4);
				splitUCString(gender_string, ucstring("/"), strList);

				if (strList.size() <= 1)
				{
					start_pos = end_pos;
					continue;
				}

				// We only care about the gender if the subject is humanoid.
				GSGENDER::EGender gender = GSGENDER::neutral;
				if (pTokenSubjectEntity->isUser() || pTokenSubjectEntity->isPlayer() || pTokenSubjectEntity->isNPC())
				{
					CCharacterCL *pC = dynamic_cast<CCharacterCL*>(pTokenSubjectEntity);
					if (pC)
					{
						gender = pC->getGender();
					}
				}

				// The neuter part is optional. Fallback to male if something is wrong.
				GSGENDER::EGender g = ((uint)gender >= strList.size()) ? GSGENDER::male : gender;
				token_replacement = strList[g];
			}
		}

		if (token_whole == token_replacement)
		{
			// Nothing to replace; show message and exit
			CInterfaceManager *im = CInterfaceManager::getInstance();
			ucstring message = ucstring(CI18N::get("uiUntranslatedToken"));
			message.replace(message.find(ucstring("%s")), 2, token_whole);
			im->displaySystemInfo(message);
			return false;
		}

		// Replace token
		size_t token_whole_pos = str.find(token_whole);

		// Only do extra replacement spaces if using default
		extra_replacement = (token_replacement == token_default) ? extra_replacement : 0;
		if (str.find(token_whole, start_pos) != string::npos)
		{
			str = str.replace(token_whole_pos, token_whole.length() + extra_replacement, token_replacement);
			start_pos = token_whole_pos + token_replacement.length();
		}
	}

	ucstr = str;
	return true;;
}