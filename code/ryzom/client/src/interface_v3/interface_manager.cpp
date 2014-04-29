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
#include "interface_config.h"
#include "task_bar_manager.h"
#include "guild_manager.h"
#include "../client_cfg.h"
#include "encyclopedia_manager.h"
// Expr
#include "nel/gui/interface_expr.h"
#include "register_interface_elements.h"
// Action / Observers
#include "nel/gui/action_handler.h"
#include "action_handler_misc.h"
#include "interface_observer.h"
#include "nel/gui/interface_anim.h"
#include "interface_ddx.h"
#include "action_handler_help.h"
#include "action_handler_item.h"
// View
#include "nel/gui/view_bitmap.h"
//#include "view_bitmap_progress.h"
#include "view_bitmap_faber_mp.h"
#include "nel/gui/view_bitmap_combo.h"
#include "nel/gui/view_text.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/view_text_formated.h"
// Ctrl
#include "nel/gui/ctrl_scroll.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/ctrl_text_button.h"
// DBCtrl
#include "dbctrl_sheet.h"
// Group
#include "nel/gui/group_list.h"
#include "nel/gui/group_menu.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_modal.h"
#include "nel/gui/group_editbox.h"
#include "group_in_scene_bubble.h"
#include "group_skills.h"
#include "group_compas.h"
#include "nel/gui/group_html.h"

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
#include "../entity_animation_manager.h"	// for emotes
#include "../net_manager.h"				// for emotes
#include "../client_chat_manager.h"		// for emotes
#include "../entities.h"

#include "../../common/src/game_share/ryzom_database_banks.h"

#include "chat_text_manager.h"
#include "../npc_icon.h"

#include "nel/gui/lua_helper.h"
using namespace NLGUI;
#include "nel/gui/lua_ihm.h"
#include "lua_ihm_ryzom.h"

#include "add_on_manager.h"

#include "game_share/r2_share_itf.h"

#include "../time_client.h"

#include "../r2/editor.h"
#include "../r2/dmc/client_edition_module.h"

#include "../bg_downloader_access.h"

#include "parser_modules.h"

#include "../global.h"
#include "user_agent.h"

using namespace NLMISC;

namespace NLGUI
{
	extern void luaDebuggerMainLoop();
}

extern CClientChatManager   ChatMngr;
extern CContinentManager ContinentMngr;
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

extern bool loginFinished;
void setLoginFinished( bool f );
// Edit actions
CActionsManager EditActions;

CInterfaceManager * CInterfaceManager::_Instance = NULL;

CChatDisplayer * ChatDisplayer = NULL;


void initActions();
void uninitActions();

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

class CDesktopUpdater : public CWidgetManager::INewScreenSizeHandler
{
public:
	void process( uint32 w, uint32 h )
	{
		CInterfaceManager::getInstance()->updateDesktops( w, h );
	}
};

class CDrawDraggedSheet : public CWidgetManager::IOnWidgetsDrawnHandler
{
public:
	void process()
	{
		if ( CWidgetManager::getInstance()->getPointer()->show())
		{
			CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>( CWidgetManager::getInstance()->getCapturePointerLeft() );
			if ((pCS != NULL) && (pCS->isDragged()))
			{
				sint	x= CWidgetManager::getInstance()->getPointer()->getX() - pCS->getDeltaDragX();
				sint	y= CWidgetManager::getInstance()->getPointer()->getY() - pCS->getDeltaDragY();
				pCS->drawSheet (x, y, false, false);

				// if the control support CopyDrag, and if copy key pressed, display a tiny "+"
				if(pCS->canDragCopy() && CInterfaceManager::getInstance()->testDragCopyKey())
				{
					CViewRenderer &rVR = *CViewRenderer::getInstance();
					sint	w= rVR.getSystemTextureW(CViewRenderer::DragCopyTexture);
					sint	h= rVR.getSystemTextureW(CViewRenderer::DragCopyTexture);
					rVR.draw11RotFlipBitmap (pCS->getRenderLayer()+1, x-w/2, y-h/2, 0, false,
						rVR.getSystemTextureId(CViewRenderer::DragCopyTexture));
				}
			}
		}
	}
};


class CStringManagerTextProvider : public CViewTextID::IViewTextProvider
{
	bool getString( uint32 stringId, ucstring &result )
	{
		return STRING_MANAGER::CStringManagerClient::instance()->getString( stringId, result );
	}

	bool getDynString( uint32 dynStringId, ucstring &result )
	{
		return STRING_MANAGER::CStringManagerClient::instance()->getDynString( dynStringId, result );
	}
};

class CRyzomTextFormatter : public CViewTextFormated::IViewTextFormatter
{
public:
	ucstring formatString( const ucstring &inputString, const ucstring &paramString )
	{
		ucstring formatedResult;

		// Apply the format
		for(ucstring::const_iterator it = inputString.begin(); it != inputString.end();)
		{
			if (*it == '$')
			{
				++it;
				if (it == inputString.end())
					break;

				switch(*it)
				{
				case 't': // add text ID
					formatedResult += paramString;
					break;

				case 'P':
				case 'p':  // add player name
					if (ClientCfg.Local)
					{
						formatedResult += ucstring("player");
					}
					else
					{
						if(UserEntity)
						{
							ucstring name = UserEntity->getEntityName();
							if (*it == 'P') setCase(name, CaseUpper);
							formatedResult += name;
						}
					}
					break;
					//
				case 's':
				case 'b': // add bot name
					{
						ucstring botName;
						bool womanTitle = false;
						if (ClientCfg.Local)
						{
							botName = ucstring("NPC");
						}
						else
						{
							CLFECOMMON::TCLEntityId trader = CLFECOMMON::INVALID_SLOT;
							if(UserEntity)
								trader = UserEntity->trader();
							if (trader != CLFECOMMON::INVALID_SLOT)
							{
								CEntityCL *entity = EntitiesMngr.entity(trader);
								if (entity != NULL)
								{
									uint32 nDBid = entity->getNameId();
									if (nDBid != 0)
									{
										STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
										pSMC->getString(nDBid, botName);
									}
									else
									{
										botName = entity->getDisplayName();
									}
									CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(entity);
									if (pChar != NULL)
										womanTitle = pChar->getGender() == GSGENDER::female;
								}
							}
						}
						// get the title translated
						ucstring sTitleTranslated = botName;
						CStringPostProcessRemoveName spprn;
						spprn.Woman = womanTitle;
						spprn.cbIDStringReceived(sTitleTranslated);

						botName = CEntityCL::removeTitleAndShardFromName(botName);

						// short name (with no title such as 'guard', 'merchant' ...)
						if (*it == 's')
						{
							// But if there is no name, display only the title
							if (botName.empty())
								botName = sTitleTranslated;
						}
						else
						{
							// Else we want the title !
							if (!botName.empty())
								botName += " ";
							botName += sTitleTranslated;
						}

						formatedResult += botName;
					}
					break;
					default:
						formatedResult += (ucchar) '$';
					break;
				}
				++it;
			}
			else
			{
				formatedResult += (ucchar) *it;
				++it;
			}
		}

		return formatedResult;
	}
};

namespace
{
	CStringManagerTextProvider SMTextProvider;
	CRyzomTextFormatter RyzomTextFormatter;
}

CInterfaceManager* CInterfaceManager::getInstance()
{
	if( _Instance == NULL )
		_Instance = new CInterfaceManager();
	return _Instance;
}

// ------------------------------------------------------------------------------------------------
CInterfaceManager::CInterfaceManager()
{
	CWidgetManager::getInstance()->registerNewScreenSizeHandler( new CDesktopUpdater() );
	CWidgetManager::getInstance()->registerOnWidgetsDrawnHandler( new CDrawDraggedSheet() );

	CInterfaceParser *parser = dynamic_cast< CInterfaceParser* >( CWidgetManager::getInstance()->getParser() );

	parser->setSetupOptionsCallback( this );
	parser->addModule( "scene3d", new CIF3DSceneParser() );
	parser->addModule( "ddx", new CIFDDXParser() );
	parser->addModule( "action_category", new CActionCategoryParser() );
	parser->addModule( "command", new CCommandParser() );
	parser->addModule( "key", new CKeyParser() );
	parser->addModule( "macro", new CMacroParser() );
	parser->setCacheUIParsing( ClientCfg.CacheUIParsing );

	CViewRenderer::setDriver( Driver );
	CViewRenderer::setTextContext( TextContext );
	CViewRenderer::hwCursorScale = ClientCfg.HardwareCursorScale;
	CViewRenderer::hwCursors     = &ClientCfg.HardwareCursors;
	CViewRenderer::getInstance();
	CViewTextID::setTextProvider( &SMTextProvider );
	CViewTextFormated::setFormatter( &RyzomTextFormatter );

	CGroupHTML::options.trustedDomains = ClientCfg.WebIgTrustedDomains;
	CGroupHTML::options.languageCode = ClientCfg.getHtmlLanguageCode();
	CGroupHTML::options.appName = "Ryzom";
	CGroupHTML::options.appVersion = getUserAgent();

	NLGUI::CDBManager::getInstance()->resizeBanks( NB_CDB_BANKS );
	interfaceLinkUpdater = new CInterfaceLink::CInterfaceLinkUpdater();
	_ScreenW = _ScreenH = 0;
	_LastInGameScreenW = _LastInGameScreenH = 0;
	_DescTextTarget = NULL;
	_ConfigLoaded = false;
	_LogState = false;
	_KeysLoaded = false;
	CWidgetManager::getInstance()->resetColorProps();
	CWidgetManager::getInstance()->resetAlphaRolloverSpeedProps();
	CWidgetManager::getInstance()->resetGlobalAlphasProps();
	_NeutralColor = NULL;
	_WarningColor = NULL;
	_ErrorColor = NULL;
	_EmotesInitialized = false;


	// Global initialization
	// *********************

	// Interface Manager init
	CViewRenderer::getInstance()->checkNewScreenSize();
	{
		uint32 w,h;
		CViewRenderer::getInstance()->getScreenSize( w, h );
		CWidgetManager::getInstance()->setScreenWH( w, h );
	}
	CViewRenderer::getInstance()->init();

	_CurrentMode = 0;

	setInGame( false );

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
	CViewTextID::setTextProvider( NULL );
	CViewTextFormated::setFormatter( NULL );
	reset(); // to flush IDStringWaiters

	// release the database observers
	releaseServerToLocalAutoCopyObservers();

	/*
	removeFlushObserver( interfaceLinkUpdater );
	delete interfaceLinkUpdater;
	interfaceLinkUpdater = NULL;
	*/
	_Instance = NULL;
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::reset()
{
	CViewRenderer::getInstance()->reset();
	CWidgetManager::getInstance()->reset();

	for (uint32 i = 0; i < _IDStringWaiters.size(); ++i)
		delete _IDStringWaiters[i];
	_IDStringWaiters.clear();
	CGroupFrame::resetDisplayTypes();

	_NeutralColor			= NULL;
	_WarningColor			= NULL;
	_ErrorColor				= NULL;

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

void CInterfaceManager::setInGame( bool i )
{
	_InGame = i;
	CWidgetManager::getInstance()->setIngame( i );
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

	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_TEMPINV));
	if (pGC != NULL)
		pGC->setActive(false);
}

// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::destroy ()
{
	delete _Instance;
	_Instance = NULL;
}

void CInterfaceManager::initLUA()
{
	CInterfaceParser *parser = dynamic_cast< CInterfaceParser* >( CWidgetManager::getInstance()->getParser() );
	if( parser->isLuaInitialized() )
		return;

	parser->initLUA();

	if( !parser->isLuaInitialized() )
		return;

	CLuaIHMRyzom::RegisterRyzomFunctions( *( CLuaManager::getInstance().getLuaState() ) );
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::initLogin()
{
	// Init LUA Scripting
	initLUA();

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

	CWidgetManager::getInstance()->updateAllLocalisedElements();

	CWidgetManager::getInstance()->activateMasterGroup ("ui:login", true);

	{
		initActions();
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::uninitLogin()
{
	CInterfaceParser *parser = dynamic_cast< CInterfaceParser* >( CWidgetManager::getInstance()->getParser() );

	CWidgetManager::getInstance()->activateMasterGroup ("ui:login", false);

	parser->removeAll();
	reset();

	CWidgetManager::getInstance()->setPointer( NULL );

	CInterfaceLink::removeAllLinks();

	ICDBNode::CTextId textId("UI");
	NLGUI::CDBManager::getInstance()->getDB()->removeNode(textId);

	{
		uninitActions();
	}

	// Close LUA Scripting
	parser->uninitLUA();
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::initOutGame()
{
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

	CWidgetManager::getInstance()->updateAllLocalisedElements();

	CWidgetManager::getInstance()->activateMasterGroup ("ui:outgame", true);

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

	// Initialize the web browser
	{
		CGroupHTML *pGH = dynamic_cast<CGroupHTML*>( CWidgetManager::getInstance()->getElementFromId(GROUP_BROWSER));

		if (pGH)
		{
			pGH->setActive(true);
			pGH->browse(ClientCfg.PatchletUrl.c_str());
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::uninitOutGame()
{

	if (ClientCfg.SelectCharacter != -1)
		return;

	CWidgetManager::getInstance()->disableModalWindow();

	//_Database->display("");
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
	CWidgetManager::getInstance()->activateMasterGroup ("ui:outgame", false);

	CInterfaceParser *parser = dynamic_cast< CInterfaceParser* >( CWidgetManager::getInstance()->getParser() );
	//nlinfo ("%d seconds for activateMasterGroup", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	initStart = ryzomGetLocalTime ();
	parser->removeAll();
	//nlinfo ("%d seconds for removeAll", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	initStart = ryzomGetLocalTime ();
	reset();
	//nlinfo ("%d seconds for reset", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	// reset the mouse pointer to avoid invalid pointer access
	CWidgetManager::getInstance()->setPointer( NULL );
	initStart = ryzomGetLocalTime ();
	CInterfaceLink::removeAllLinks();
	//nlinfo ("%d seconds for removeAllLinks", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	initStart = ryzomGetLocalTime ();
	ICDBNode::CTextId textId("UI");
	NLGUI::CDBManager::getInstance()->getDB()->removeNode(textId);
	//nlinfo ("%d seconds for removeNode", (uint32)(ryzomGetLocalTime ()-initStart)/1000);

	// Init the action manager
	{

		initStart = ryzomGetLocalTime ();
		uninitActions();
	//	nlinfo ("%d seconds for uninitActions", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	}

	// Close LUA Scripting
	parser->uninitLUA();

	//NLMEMORY::CheckHeap (true);
}


void badXMLParseMessageBox()
{
	NL3D::UDriver *driver = CViewRenderer::getInstance()->getDriver();
	NL3D::UDriver::TMessageBoxId	ret = driver->systemMessageBox(	"Interface XML reading failed!\n"
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
	setLoginFinished( true );
	_LogState = false;

	// Whole initInGame profile
	NLMISC::TTime initStart;
	initStart = ryzomGetLocalTime ();

	// Init LUA Scripting
	initLUA();

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

		CWidgetManager::getInstance()->updateAllLocalisedElements(); // To init all things
	}

	// Interface config
	loadInterfaceConfig();

	// Must do extra init for people interaction after load
	PeopleInterraction.initAfterLoad();

	//CBotChatUI::refreshActiveWindows(); // bot chat windows are saved too..

	CWidgetManager::getInstance()->activateMasterGroup ("ui:interface", true);

	// Update the time in the ui database
	_CheckMailNode = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MAIL_WAITING");
	_CheckForumNode = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:FORUM_UPDATED");

	// Init the action manager
	{

		initActions();
	}
	setInGame( true );

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
 	CGroupCompas *gc = dynamic_cast<CGroupCompas *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:compass"));
	if (gc && gc->isSavedTargetValid())
	{
		gc->setTarget(gc->getSavedTarget());
	}

	CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHATLOG_STATE", false);
	if (node)
	{
		_LogState = (node->getValue32() != 0);
	}

	if (_LogState)
	{
		displaySystemInfo(CI18N::get("uiLogTurnedOn"));
	}
	else
	{
		displaySystemInfo(CI18N::get("uiLogTurnedOff"));
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
	CInterfaceGroup *quitDlg = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(quitDialogStr));
	if (quitDlg)
	{
		CInterfaceElement *eltRet, *eltQuit, *eltQuitNow;
		eltRet = quitDlg->getElement(quitDialogStr+":return_mainland");
		eltQuit = quitDlg->getElement(quitDialogStr+":ryzom");
		eltQuitNow = quitDlg->getElement(quitDialogStr+":ryzom_now");
		sint buttonDeltaY;
		fromString( CWidgetManager::getInstance()->getParser()->getDefine("quit_button_delta_y"), buttonDeltaY);
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

		if (NoLogout)
		{
			eltEdit->setY(0);
			eltEdit->setActive(false);
			eltQuit->setY(0);
			eltQuit->setActive(false);
			eltQuitNow->setY(0);
			eltQuitNow->setActive(false);
			eltRet->setY(0);
			eltRet->setActive(false);
		}
	}

	// Make all controls have the same size
	CInterfaceGroup *quitDlgMain = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(quitDialogMainStr));
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
	CWidgetManager::getInstance()->disableModalWindow();

	// Remove all interface objects (containers, groups, variables, defines, ...)
	CWidgetManager::getInstance()->activateMasterGroup ("ui:interface", false);

	CInterfaceParser *parser = dynamic_cast< CInterfaceParser* >( CWidgetManager::getInstance()->getParser() );

	parser->removeAll();
	reset();
	CInterfaceLink::removeAllLinks();

	// Release DDX manager, before DB remove
	CDDXManager::getInstance()->release();

	// Release client time, before DB remove
	releaseClientTime();

	// remove DB entry
	ICDBNode::CTextId textId("UI");
	NLGUI::CDBManager::getInstance()->getDB()->removeNode(textId);

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
	parser->uninitLUA();

	setInGame( false );
	_NeutralColor = NULL;
	_WarningColor = NULL;
	_ErrorColor = NULL;
	CWidgetManager::getInstance()->resetColorProps();
	CWidgetManager::getInstance()->resetAlphaRolloverSpeedProps();
	CWidgetManager::getInstance()->resetGlobalAlphasProps();

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
	CWidgetManager::getInstance()->updateAnims();

	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	CWidgetManager::getInstance()->removeFinishedAnims();

	//
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

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


			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:weather"));
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

			pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:time"));
			if (pVT != NULL)
				pVT->setText(str);

			str.clear();
			// Update the clock in the compass if enabled.
			pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:compass:clock:time"));
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

	CWidgetManager::getInstance()->sendClockTickEvent();

	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

 	// Update SPhrase manager
 	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	pPM->update();

	// if there's an external lua debugger, update it
	luaDebuggerMainLoop();

	// handle gc for lua
	CLuaManager::getInstance().getLuaState()->handleGC();

	CBGDownloaderAccess::getInstance().update();

}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::updateFrameViews(NL3D::UCamera camera)
{

	H_AUTO ( RZ_Interface_updateFrameViews )

	if (!camera.empty())
		CViewRenderer::getInstance()->setWorldSpaceFrustum (camera.getFrustum());

	CWidgetManager::getInstance()->checkCoords();
	drawViews(camera);

	// The interface manager may change usual Global setup. reset them.
	CViewRenderer::getTextContext()->setShadeColor(CRGBA::Black);

}

void CInterfaceManager::setupOptions()
{
	CWidgetManager *wm = CWidgetManager::getInstance();
	wm->setupOptions();

	// Try to change font if any
	string sFont = wm->getSystemOption( CWidgetManager::OptionFont ).getValStr();

	if ((!sFont.empty()) && (Driver != NULL))
		resetTextContext(sFont.c_str(), true);
	// Continue to parse the rest of the interface
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::parseInterface (const std::vector<std::string> &xmlFileNames, bool reload, bool isFilename)
{
	// cache some commonly used db nodes
	_DBB_UI_DUMMY = NLGUI::CDBManager::getInstance()->getDbBranch( "UI:DUMMY" );
	_DB_UI_DUMMY_QUANTITY = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:QUANTITY", true );
	_DB_UI_DUMMY_QUALITY = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:QUALITY", true );
	_DB_UI_DUMMY_SHEET = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:SHEET", true );
	_DB_UI_DUMMY_NAMEID = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:NAMEID", true );
	_DB_UI_DUMMY_ENCHANT = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:ENCHANT", true );
	_DB_UI_DUMMY_SLOT_TYPE = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:SLOT_TYPE", true );
	_DB_UI_DUMMY_PHRASE = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:PHRASE", true );
	_DB_UI_DUMMY_WORNED = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:WORNED", true );
	_DB_UI_DUMMY_PREREQUISIT_VALID = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:PREREQUISIT_VALID", true );
	_DB_UI_DUMMY_FACTION_TYPE = NLGUI::CDBManager::getInstance()->getDbProp( "UI:DUMMY:FACTION_TYPE", true );

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

	return CWidgetManager::getInstance()->getParser()->parseInterface (xmlFileNames, reload, isFilename);
}

// ------------------------------------------------------------------------------------------------
void CInterfaceManager::loadTextures (const string &textFileName, const string &uvFileName, bool uploadDXTC)
{
	CViewRenderer::getInstance()->loadTextures (textFileName, uvFileName, uploadDXTC);
}

// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::loadConfig (const string &filename)
{
	// reset the interface
	vector<string> v;
	if (ClientCfg.R2EDEnabled)
	{
		CWidgetManager::getInstance()->runProcedure ("proc_reset_r2ed_interface", NULL, v);
	}
	else
	{
		CWidgetManager::getInstance()->runProcedure ("proc_reset_interface", NULL, v);
	}

	// By default, consider the reset interface has been set with the current resolution
	{
		uint32	w,h;
		// NB: even if minimzed, getScreenSize() no more return 0 values (return the last setuped screen size)
		CViewRenderer::getInstance()->getScreenSize(w, h);
		// Windows are positioned according to resolution, and we must backup W/H for the system that move windows when the resolution change
		_LastInGameScreenW= w;
		_LastInGameScreenH= h;
	}

	// if the config file doesn't exist,just quit
	CIFile f;
	string sFileName;
	sFileName = NLMISC::CPath::lookup (filename, false);
	if (sFileName.empty() || !f.open(sFileName))
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
			f.serialCheck(NELID("_ICU"));
			if (!PeopleInterraction.loadUserChatsInfos(f))
			{
				nlwarning("Bad user chat saving");
			}
		}

		// header
		f.serialCheck(NELID("GFCI"));
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
			CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:MISSION_SELECTED", false);
			if (pNL)
			{
				CCDBNodeLeaf *pSelectedMissionBackup = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MISSION_SELECTED_PREV_SESSION", true);
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

		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp( "SERVER:INTERFACES:NB_BONUS_LANDMARKS" );
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
			CWidgetManager::getInstance()->runProcedure ("proc_reset_interface", NULL, v);
		}
		return false;
	}
	f.close();

	// *** If saved resolution is different from the current one setuped, must fix positions in _Modes
	if(lastInGameScreenResLoaded)
	{
		// NB: we are typically InGame here (even though the _InGame flag is not yet set)
		// Use the screen size of the config file. Don't update current UI, just _Modes
		CWidgetManager::getInstance()->moveAllWindowsToNewScreenSize(ClientCfg.Width, ClientCfg.Height, false);
		updateDesktops( ClientCfg.Width, ClientCfg.Height );
	}

	// *** apply the current mode
	_Modes[_CurrentMode].toCurrentDesktop();

	// *** Apply the NPC icon display mode
	CNPCIconCache::getInstance().init(!ClientCfg.R2EDEnabled && NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:INSCENE:FRIEND:MISSION_ICON")->getValueBool());

	return true;
}


// ------------------------------------------------------------------------------------------------
void CInterfaceManager::CDBLandmarkObs::update(ICDBNode *node)
{
	ContinentMngr.updateUserLandMarks();
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
		CWidgetManager::getInstance()->checkCoords();
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
		f.serialCheck(NELID("_ICU"));
		if (!PeopleInterraction.saveUserChatsInfos(f))
		{
			nlwarning("Config saving failed");
			// couldn't save result so do not continue
			f.close();
			return false;
		}

		// header
		f.serialCheck(NELID("GFCI"));
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
		fromString( CWidgetManager::getInstance()->getParser()->getDefine("UI_DB_SAVE_VERSION"), uiDbSaveVersion);
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
void CInterfaceManager::drawViews(NL3D::UCamera camera)
{
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	// Update Player characteristics (for Item carac requirement Redifying)
	nlctassert(CHARACTERISTICS::NUM_CHARACTERISTICS==8);
	for (uint i=0; i<CHARACTERISTICS::NUM_CHARACTERISTICS; ++i)
	{
		NLMISC::CCDBNodeLeaf *node = _CurrentPlayerCharacLeaf[i] ? &*_CurrentPlayerCharacLeaf[i]
			: &*(_CurrentPlayerCharacLeaf[i] = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:CHARACTERISTICS%d:VALUE", i), false));
		_CurrentPlayerCharac[i] = node ? node->getValue32() : 0;
	}

	CWidgetManager::getInstance()->drawViews( camera );

	// flush obs
	IngameDbMngr.flushObserverCalls();
}


// ------------------------------------------------------------------------------------------------
bool CInterfaceManager::handleEvent (const NLGUI::CEventDescriptor& event)
{
	bool handled = false;

	handled = CWidgetManager::getInstance()->handleEvent( event );

	if( event.getType() == NLGUI::CEventDescriptor::mouse )
	{
		NLGUI::CEventDescriptorMouse &eventDesc = (NLGUI::CEventDescriptorMouse&)event;

		if( ( eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup ) && handled )
		{
			// prevent 'click in scene' as mouse was previously captured
			// (more a patch that anything, but 'UserControls' test for 'mouse up'
			// directly later in the main loop (not through message queue), so it has no way of knowing that the event was handled...
			if( CWidgetManager::getInstance()->getCapturePointerRight() == NULL )
				EventsListener.addUIHandledButtonMask(rightButton);
		}else
		if( ( eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup ) && handled )
		{
			// prevent 'click in scene' as mouse was previously captured
			// (more a patch that anything, but 'UserControls' test for 'mouse up'
			// directly later in the main loop (not through message queue), so it has no way of knowing that the event was handled...
			if( CWidgetManager::getInstance()->getCapturePointerLeft() == NULL )
				EventsListener.addUIHandledButtonMask(leftButton);
		}

	}

	IngameDbMngr.flushObserverCalls();
	// event handled?
	return handled;
}

void CInterfaceManager::updateDesktops( uint32 newScreenW, uint32 newScreenH )
{
	// *** Do it for All Backuped Desktops
	for(uint md=0;md<MAX_NUM_MODES;md++)
	{
		CInterfaceConfig::CDesktopImage		&mode= _Modes[md];
		// For all containers of this mode
		for(uint gc=0;gc<mode.GCImages.size();gc++)
		{
			CInterfaceConfig::SCont	&gcCont= mode.GCImages[gc];
			// Compute the new coordinate, directly in the X/Y fields of the structure
			CWidgetManager::getInstance()->getNewWindowCoordToNewScreenSize(gcCont.X, gcCont.Y, gcCont.W, gcCont.H ,newScreenW, newScreenH);
		}
	}
}

class InvalidateTextVisitor : public CInterfaceElementVisitor
{
public:
	InvalidateTextVisitor( bool reset )
	{
		this->reset = reset;
	}

	void visitGroup( CInterfaceGroup *group )
	{
		const std::vector< CViewBase* > &vs = group->getViews();
		for( std::vector< CViewBase* >::const_iterator itr = vs.begin(); itr != vs.end(); ++itr )
		{
			CViewText *vt = dynamic_cast< CViewText* >( *itr );
			if( vt != NULL )
			{
				if( reset )
					vt->resetTextIndex();
				vt->updateTextContext();
			}
		}
	}

private:
	bool reset;
};


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
void CInterfaceManager::messageBoxInternal(const string &msgBoxGroup, const ucstring &text, const string &masterGroup, TCaseMode caseMode)
{
	CInterfaceGroup *group= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(masterGroup+":" + msgBoxGroup));
	CViewText		*viewText= dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(masterGroup+":" + msgBoxGroup + ":text"));

	if (group && viewText)
	{
		viewText->setCaseMode(caseMode);
		viewText->setText(text);
		CWidgetManager::getInstance()->enableModalWindow(NULL, group);
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
	CWidgetManager::getInstance()->setProcedureAction("proc_message_box_with_help_ok", 1, ahOnOk, paramsOnOk);
	const char *mbName = "message_box_with_help";
	// if no action handler is wanted, then assume that
	// clicking 'ok' do not have any consequence, so allow exiting the message box by clicking
	// outside of it (this behavior is wanted on the login page, to allow to reclick on 'login' without
	// having to click 'ok' in the message box each time)
	CInterfaceGroup *group= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(masterGroup+":" + mbName));
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
	CInterfaceGroup *group= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(masterGroup+":valid_message_box"));
	CViewText		*viewText= dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(masterGroup+":valid_message_box:text"));
	CViewBitmap		*viewBitmap= dynamic_cast<CViewBitmap*>(CWidgetManager::getInstance()->getElementFromId(masterGroup+":valid_message_box:icon_group:icon"));

	if (group && viewText)
	{
		// replace the procedure "proc_valid_message_box_ok" action
		CWidgetManager::getInstance()->setProcedureAction("proc_valid_message_box_ok", 1, ahOnOk, paramsOnOk);
		// replace the procedure "proc_valid_message_box_cancel" action
		CWidgetManager::getInstance()->setProcedureAction("proc_valid_message_box_cancel", 1, ahOnCancel, paramsOnCancel);

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
		CWidgetManager::getInstance()->enableModalWindow(NULL, group);
		// don't understand why but need to update coords here
		group->updateCoords();
		group->updateCoords();
	}
}


// ------------------------------------------------------------------------------------------------
bool	CInterfaceManager::getCurrentValidMessageBoxOnOk(string &ahOnOk, const std::string &masterGroup)
{
	// any modal window opened?
	CInterfaceGroup	*mw= CWidgetManager::getInstance()->getModalWindow();
	if(!mw)
		return false;

	// Is this modal window the valid_message_box window?
	CInterfaceGroup *group= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(masterGroup+":valid_message_box"));
	if(mw==group)
	{
		// Ok, get the current procedure OnOk action
		string	dummyParams;
		if( CWidgetManager::getInstance()->getParser()->getProcedureAction("proc_valid_message_box_ok", 1, ahOnOk, dummyParams))
			return true;
	}

	return false;
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
		_NeutralColor = NLGUI::CDBManager::getInstance()->getDbProp(SYSTEM_INFO_COLOR_DB_PATH ":NEUTRAL");
		_WarningColor = NLGUI::CDBManager::getInstance()->getDbProp(SYSTEM_INFO_COLOR_DB_PATH ":WARNING");
		_ErrorColor = NLGUI::CDBManager::getInstance()->getDbProp(SYSTEM_INFO_COLOR_DB_PATH ":ERROR");
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

	if (mode == CClientConfig::SSysInfoParam::Center || mode == CClientConfig::SSysInfoParam::CenterAround)
		InSceneBubbleManager.addMessagePopupCenter(str, color);

	// If over popup a string at the bottom of the screen
	if ((mode == CClientConfig::SSysInfoParam::Over) || (mode == CClientConfig::SSysInfoParam::OverOnly))
		InSceneBubbleManager.addMessagePopup(str, color);
	else if ( (mode == CClientConfig::SSysInfoParam::Around || mode == CClientConfig::SSysInfoParam::CenterAround)
		&& PeopleInterraction.AroundMe.Window)
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
	if (CCtrlDraggable::getDraggedSheet() == NULL)
	{
		if ( !CWidgetManager::getInstance()->hasModal() )
		{
			// We must be in-game !
			CInterfaceGroup *pMG = CWidgetManager::getInstance()->getMasterGroupFromId("ui:interface");
			// TMP nico : try with login screen:
			if (!pMG)
			{
				pMG = CWidgetManager::getInstance()->getMasterGroupFromId("ui:login");
			}
			if (!pMG)
			{
				pMG = CWidgetManager::getInstance()->getMasterGroupFromId("ui:outgame");
			}
			if ((pMG != NULL) && (pMG->getActive()))
			{
				CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(nameOfCM);
				CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(pIE);
				if (pIG != NULL)
				{
					CWidgetManager::getInstance()->enableModalWindow (NULL, pIG);
				}
			}
		}
	}
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

	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();

	// Check if we can change vdesk !
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
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
							CWidgetManager::getInstance()->setTopWindow(pGC);
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
	if ( CWidgetManager::getInstance()->getCapturePointerLeft() != NULL)
	{
		if (!CWidgetManager::getInstance()->getCapturePointerLeft()->canChangeVirtualDesktop()) return;
	}
	if ( CWidgetManager::getInstance()->getCapturePointerRight() != NULL)
	{
		if (!CWidgetManager::getInstance()->getCapturePointerRight()->canChangeVirtualDesktop()) return;
	}


	_Modes[_CurrentMode].fromCurrentDesktop();
	_Modes[newMode].toCurrentDesktop();
	//CBotChatUI::refreshActiveWindows();

	_CurrentMode = newMode;
	CWidgetManager::getInstance()->checkCoords();
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
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
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
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
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
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
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
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
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
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
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
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	for (uint32 nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];
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
void CInterfaceManager::visit(CInterfaceElementVisitor *visitor)
{
	nlassert(visitor);
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
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
	CWidgetManager::getInstance()->updateAllLocalisedElements();

	// reset captures
	CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
	CWidgetManager::getInstance()->setCapturePointerRight(NULL);
	CWidgetManager::getInstance()->setOldCaptureKeyboard(NULL);
	CWidgetManager::getInstance()->setCaptureKeyboard(NULL);

	return result;
}


// ***************************************************************************
void CInterfaceManager::displayWebWindow(const string & name, const string & url)
{
	CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(name));
	if (pIG != NULL)
	{
		pIG->setActive(true);
		pIG->updateCoords();
		pIG->center();
	}

	CAHManager::getInstance()->runActionHandler("browse", NULL, "name="+name+":content:html|url="+url);
}
/*
// ***************************************************************************
class CHandlerDispWebOnQuit : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		if (ClientCfg.Local)
			CAHManager::getInstance()->runActionHandler("enter_modal", pCaller, "group=ui:interface:quit_dialog");
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
		CAHManager::getInstance()->runActionHandler("quit_ryzom", pCaller);
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

		CGroupMenu *pRootMenu = dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:user_chat_emote_menu"));
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
		CGroupMenu *pRootMenu = dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:game_context_menu"));
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
	CAHManager::getInstance()->runActionHandler("emote", NULL, "nb="+toString(EmoteNb)+"|behav="+toString(Behaviour)+"|custom_phrase="+customPhrase);
	return true;
}

// ***************************************************************************
bool	CInterfaceManager::testDragCopyKey()
{
	// hardcoded for now
	return Driver->AsyncListener.isKeyDown(KeyCONTROL) ||
		Driver->AsyncListener.isKeyDown(KeyLCONTROL) ||
		Driver->AsyncListener.isKeyDown(KeyRCONTROL);
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
	std::vector< CWidgetManager::SMasterGroup > &_MasterGroups = CWidgetManager::getInstance()->getAllMasterGroup();
	for (nMasterGroup = 0; nMasterGroup < _MasterGroups.size(); nMasterGroup++)
	{
		CWidgetManager::SMasterGroup &rMG = _MasterGroups[nMasterGroup];

		InvalidateTextVisitor inv( true );
		rMG.Group->visitGroupAndChildren( &inv );
		for (uint8 nPriority = 0; nPriority < WIN_PRIORITY_MAX; nPriority++)
		{
			list<CInterfaceGroup*> &rList = rMG.PrioritizedWindows[nPriority];
			list<CInterfaceGroup*>::const_iterator itw;
			for (itw = rList.begin(); itw != rList.end(); itw++)
			{
				CInterfaceGroup *pIG = *itw;
				pIG->visitGroupAndChildren( &inv );
			}
		}
	}
}

// ***************************************************************************
CInterfaceElement *getInterfaceResource(const std::string &key)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	return CWidgetManager::getInstance()->getElementFromId (key);
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
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:YUBO_CHAT_PRESENT")->setValue32(1);
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
void		CInterfaceManager::dumpLuaString(const std::string &str)
{
	nlinfo(str.c_str());
	displaySystemInfo(LuaHelperStuff::formatLuaErrorSysInfo(str));
}

// ***************************************************************************
void		CInterfaceManager::getLuaValueInfo(std::string &str, sint index)
{
	CLuaState	&ls= *( CLuaManager::getInstance().getLuaState() );

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
	CLuaState	&ls= *( CLuaManager::getInstance().getLuaState() );
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
	CLuaState *_LuaState = CLuaManager::getInstance().getLuaState();

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
			NLGUI::CDBManager::getInstance()->getDB()->attachChild(localNode,"LOCAL");

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
	_ServerCounter= NLGUI::CDBManager::getInstance()->getDbProp(string("SERVER:") + dbPath + ":COUNTER", false);

	// if found
	if(_ServerCounter)
	{
		ICDBNode::CTextId textId;

		// **** Add Observers on all nodes
		// add the observers when server node change
		textId = ICDBNode::CTextId( string("SERVER:") + dbPath );
		NLGUI::CDBManager::getInstance()->getDB()->addObserver(&_ServerObserver, textId );

		// add the observers when local node change
		textId = ICDBNode::CTextId( string("LOCAL:") + dbPath );
		NLGUI::CDBManager::getInstance()->getDB()->addObserver(&_LocalObserver, textId );

		// **** Init the Nodes shortcut
		// Parse all Local Nodes
		CCDBNodeBranch	*localBranch= NLGUI::CDBManager::getInstance()->getDbBranch(string("LOCAL:") + dbPath);
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
				CCDBNodeLeaf	*serverLeaf= NLGUI::CDBManager::getInstance()->getDbProp(serverLeafStr, false);
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
		if (token_start_pos > token_end_pos)
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
				CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp( NLMISC::toString(TEAM_DB_PATH ":%hu:NAME", indexInTeam ), false);
				if (pNL && pNL->getValueBool() )
				{
					// There is a character corresponding to this index
					pNL = NLGUI::CDBManager::getInstance()->getDbProp( NLMISC::toString( TEAM_DB_PATH ":%hu:UID", indexInTeam ), false );
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


