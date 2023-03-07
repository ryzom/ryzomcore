// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2022  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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




//////////////
// Includes //
//////////////
#include "stdpch.h"

// Misc.
#include "nel/misc/i18n.h"
#include "nel/misc/path.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/algo.h"
#include "nel/misc/system_utils.h"
// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/stereo_display.h"
// Game Share
//#include "game_share/gd_time.h"		// \todo GUIGUI : TO DELETE/CHANGE
#include "game_share/gender.h"
#include "game_share/character_summary.h"
#include "game_share/roles.h"
#include "game_share/character_title.h"
#include "game_share/shard_names.h"
#include "game_share/utils.h"
#include "game_share/bg_downloader_msg.h"

// Std.
#include <vector>
// Client
#include "connection.h"
#include "nel/gui/action_handler.h"
#include "sound_manager.h"
#include "input.h"
#include "login.h"
#include "login_progress_post_thread.h"

#include "client_cfg.h"
#include "actions_client.h"
#include "user_entity.h"
#include "time_client.h"
#include "net_manager.h"
#include "string_manager_client.h"
#include "far_tp.h"
#include "movie_shooter.h"

// Interface part
#include "interface_v3/interface_manager.h"
#include "interface_v3/character_3d.h"
#include "nel/gui/ctrl_button.h"
#include "interface_v3/input_handler_manager.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/interface_expr.h"
#include "init_main_loop.h"
#include "continent_manager.h"
#include "interface_v3/group_quick_help.h"
#include "nel/gui/dbgroup_combo_box.h"

#include "r2/dmc/client_edition_module.h"
#include "r2/editor.h"
#include "game_share/scenario.h"
#include "session_browser_impl.h"

#include "bg_downloader_access.h"
#include "main_loop.h"

#include "misc.h"


////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLNET;
using namespace std;
using namespace RSMGR;
using namespace R2;



/////////////
// Externs //
/////////////
extern uint32			Version;	// Client Version.
extern UDriver			*Driver;
extern UTextContext		*TextContext;
extern bool				game_exit;

extern CSoundManager	*SoundMngr;

extern bool serverReceivedReady;
extern CContinentManager	ContinentMngr;

extern bool MovieShooterSaving;
extern void endMovieShooting();
extern void replayMovieShooting();
extern void saveMovieShooting();
extern void	displaySpecialTextProgress(const char *text);
extern bool InitMouseWithCursor(bool hardware);

extern bool SetMousePosFirstTime;

/////////////
// Globals // initialization occurs in the function : connection
/////////////
bool userChar;
bool noUserChar;
bool ConnectInterf;
bool CreateInterf;
bool CharacterInterf;
TTime UniversalTime;
std::vector<CCharacterSummary>	CharacterSummaries;
std::string						UsedFSAddr;
std::string						UserPrivileges;
uint8 ServerPeopleActive = 255;
uint8 ServerCareerActive = 255;

bool WaitServerAnswer;
bool CharNameValidArrived;
bool CharNameValid;
string CharNameValidDBLink;
uint8		PlayerSelectedSlot = 0;
string		PlayerSelectedFileName;
TSessionId	PlayerSelectedMainland= (TSessionId)0;	// This is the mainland selected at the SELECT perso!!
std::string	PlayerSelectedHomeShardName;
std::string	PlayerSelectedHomeShardNameWithParenthesis;
extern std::string CurrentCookie;

std::string NewKeysCharNameWanted; // name of the character for which a new keyset must be created
std::string NewKeysCharNameValidated;
std::string GameKeySet = "keys.xml";
std::string RingEditorKeySet = "keys_r2ed.xml";

string		ScenarioFileName;
sint 		LoginCharsel = -1;

std::string ImportCharacter;

static const char *KeySetVarName = "BuiltInKeySets";

#define GROUP_LIST_CHARACTER            "ui:outgame:charsel_import:import_list"
#define	GROUP_LIST_MAINLAND				"ui:outgame:appear_mainland:mainland_list"
#define	GROUP_LIST_KEYSET				"ui:outgame:appear_keyset:keyset_list"
vector<CMainlandSummary>	Mainlands;
TSessionId					MainlandSelected = (TSessionId)0;	// This is the mainland selected at the CREATE perso!!

// This is the home session (mainland) of the selected character
TSessionId			CharacterHomeSessionId = (TSessionId)0;


bool PatchBegun = false;

// \todo GUIGUI : USE TRANSPORT CLASS.
//	SVersionAnswer versionAnswer;

#if !FINAL_VERSION
static bool DebugOutgameUI = false;
static bool DebugOutgameReloadUI = false;
#endif

// Finite State Machine : all the states before entering the game
// ------------------------------------------------------------------------------------------------
TInterfaceState globalMenu();


bool hasPrivilegeDEV() { return (UserPrivileges.find(":DEV:") != std::string::npos); }
bool hasPrivilegeSGM() { return (UserPrivileges.find(":SGM:") != std::string::npos); }
bool hasPrivilegeGM() { return (UserPrivileges.find(":GM:") != std::string::npos); }
bool hasPrivilegeVG() { return (UserPrivileges.find(":VG:") != std::string::npos); }
bool hasPrivilegeSG() { return (UserPrivileges.find(":SG:") != std::string::npos); }
bool hasPrivilegeG() { return (UserPrivileges.find(":G:") != std::string::npos); }
bool hasPrivilegeEM() { return (UserPrivileges.find(":EM:") != std::string::npos); }
bool hasPrivilegeEG() { return (UserPrivileges.find(":EG:") != std::string::npos); }


// Restore the video mode (fullscreen for example) after the connection (done in a window)
void connectionRestoreVideoMode ()
{
	if (StereoDisplay)
		StereoDisplayAttached = StereoDisplay->attachToDisplay();

	// And setup hardware mouse if we have to
	InitMouseWithCursor (ClientCfg.HardwareCursor && !StereoDisplayAttached);
	SetMouseFreeLook ();
	SetMouseCursor ();
	SetMouseSpeed (ClientCfg.CursorSpeed);
	SetMouseAcceleration (ClientCfg.CursorAcceleration);

	// Restore user UI scaling
	CInterfaceManager::getInstance()->setInterfaceScale(ClientCfg.InterfaceScale, ClientCfg.InterfaceScaleAuto);
}


#define UI_VARIABLES_SCREEN_WEBSTART		8


// ***************************************************************************
// Called to reload the start test page in test browser mode
class CAHOnReloadTestPage: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		// need to reset password and current screen
		CGroupHTML *pGH = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(GROUP_BROWSER));

		pGH->browse(ClientCfg.TestBrowserUrl.c_str());

	}
};
REGISTER_ACTION_HANDLER (CAHOnReloadTestPage, "on_reload_test_page");


// ------------------------------------------------------------------------------------------------
void	setOutGameFullScreen()
{
	if (!ClientCfg.Local && ClientCfg.SelectCharacter == -1)
	{
		if (StereoDisplayAttached)
			StereoDisplay->detachFromDisplay();
		StereoDisplayAttached = false;

		InitMouseWithCursor(ClientCfg.HardwareCursor && !StereoDisplayAttached);
	}

	// Enable auto scaling in login window
	CInterfaceManager::getInstance()->setInterfaceScale(1.0f, true);
}

// ------------------------------------------------------------------------------------------------
class CSoundGlobalMenu
{
public:
	CSoundGlobalMenu()
	{
		_MusicWantedAsync= false;
		_NbFrameBeforeChange= NbFrameBeforeChangeMax;
	}
	void    reset();
	void	setMusic(const string &music, bool async);
	void	updateSound();
private:
	string		_MusicPlayed;
	string		_MusicWanted;
	bool		_MusicWantedAsync;
	sint		_NbFrameBeforeChange;
	enum	{NbFrameBeforeChangeMax= 10};
};

void	CSoundGlobalMenu::reset()
{
	_MusicPlayed.clear();
	_MusicWanted.clear();
}

void	CSoundGlobalMenu::updateSound()
{
	// **** update the music played
	// The first music played is the music played at loading, before select char
	if (_MusicPlayed.empty())
		_MusicPlayed = toLowerAscii(LoadingMusic.empty() ? ClientCfg.StartMusic : LoadingMusic);
	if (_MusicWanted.empty())
		_MusicWanted = toLowerAscii(LoadingMusic.empty() ? ClientCfg.StartMusic : LoadingMusic);

	// because music is changed when the player select other race for instance,
	// wait the 3D to load (stall some secs)

	// if the wanted music is the same as the one currently playing, just continue playing
	if(_MusicPlayed!=_MusicWanted)
	{
		// wait nbFrameBeforeChangeMax before actually changing the music
		_NbFrameBeforeChange--;
		if(_NbFrameBeforeChange<=0)
		{
			_MusicPlayed= _MusicWanted;
			// play the music
			if (SoundMngr != NULL)
				SoundMngr->playMusic(_MusicPlayed, 500, _MusicWantedAsync, true, true);
		}
	}


	// **** update mngr
	if (SoundMngr != NULL)
		SoundMngr->update();
}

void	CSoundGlobalMenu::setMusic(const string &music, bool async)
{
	_MusicWanted= toLowerAscii(music);
	_MusicWantedAsync= async;
	// reset the counter
	_NbFrameBeforeChange= NbFrameBeforeChangeMax;
}
static	CSoundGlobalMenu	SoundGlobalMenu;


// New version of the menu after the server connection
//
// If you add something in this function, check CFarTP,
// some kind of reinitialization might be useful over there.
// ------------------------------------------------------------------------------------------------
bool connection (const string &cookie, const string &fsaddr)
{

	NLMISC::TTime connStart = ryzomGetLocalTime();
	NLMISC::TTime connLast = connStart;
	NLMISC::TTime connCurrent = connLast;

	game_exit = false;

	// set resolution from cfg after login
	connectionRestoreVideoMode ();

	// Preload continents
	{
		const string nmsg("Loading continents...");
		ProgressBar.newMessage (ClientCfg.buildLoadingString(nmsg) );
		ContinentMngr.preloadSheets();

		connLast = connCurrent;
		connCurrent = ryzomGetLocalTime();
		nlinfo ("PROFILE: %d seconds (%d total) for Loading continents", (uint32)(connCurrent-connLast)/1000, (uint32)(connCurrent-connStart)/1000);
	}

	if (!fsaddr.empty () && !cookie.empty ())
	{
		// it means that we have a nel_launcher values, so we are online
		ClientCfg.Local = 0;
		nlinfo ("Using the nel launcher parameters '%s' '%s'", cookie.c_str (), fsaddr.c_str ());
	}

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// If the Client is in in Local Mode -> init the Time and return.
	if (ClientCfg.Local)
	{
#ifdef ENABLE_INCOMING_MSG_RECORDER
		NetMngr.init("", "");
		// Set the impulse callback.
		NetMngr.setImpulseCallback (impulseCallBack);
		// Set the database.
		NetMngr.setDataBase (IngameDbMngr.getNodePtr());
		// init the string manager cache.
		STRING_MANAGER::CStringManagerClient::instance()->initCache("", ClientCfg.LanguageCode);	// VOIR BORIS
#endif
		return true;
	}

	ProgressBar.setFontFactor(1.0f);

	// Init out game
	setOutGameFullScreen();

	string nmsg("Initializing outgame...");
	ProgressBar.newMessage (ClientCfg.buildLoadingString(nmsg) );
	pIM->initOutGame();

	connLast = connCurrent;
	connCurrent = ryzomGetLocalTime();
	nlinfo ("PROFILE: %d seconds (%d total) for Initializing outgame", (uint32)(connCurrent-connLast)/1000, (uint32)(connCurrent-connStart)/1000);

	// Init user interface
	nmsg = "Initializing user interface...";
	ProgressBar.newMessage (ClientCfg.buildLoadingString(nmsg) );

	// Hide cursor for interface
	//Driver->showCursor (false);

	// Init global variables
	userChar		= false;
	noUserChar		= false;
	ConnectInterf	= true;
	CreateInterf	= true;
	CharacterInterf	= true;
	WaitServerAnswer= false;

	FarTP.setOutgame();

	// Start the finite state machine
	static bool firstConnection = true;
	TInterfaceState InterfaceState = AUTO_LOGIN;
	// TInterfaceState InterfaceState = firstConnection ? AUTO_LOGIN : GLOBAL_MENU;
	/*if (!firstConnection)
	{
		noUserChar = userChar = false;
		WaitServerAnswer = true;
	}*/

	NLGUI::CDBManager::getInstance()->getDbProp ("UI:CURRENT_SCREEN")->setValue32(ClientCfg.Local ? 6 : -1); // TMP TMP
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	// Active inputs
	Actions.enable(true);
	EditActions.enable(true);

	if (ClientCfg.SelectCharacter == -1)
	{
		// not initialized at login and remain hardware until here ...

		// Re-initialise the mouse (will be now in hardware mode, if required)
		//InitMouseWithCursor (ClientCfg.HardwareCursor && !StereoDisplayAttached); // the return value of enableLowLevelMouse() has already been tested at startup

		// no ui init if character selection is automatic
		//SetMouseFreeLook ();
		//SetMouseCursor ();
		SetMouseSpeed (ClientCfg.CursorSpeed);
		SetMouseAcceleration (ClientCfg.CursorAcceleration);

		NLGUI::CDBManager::getInstance()->getDbProp("UI:SELECTED_SLOT")->setValue32(ClientCfg.SelectedSlot);
		PlayerSelectedSlot = ClientCfg.SelectedSlot;
	}

	connLast = connCurrent;
	connCurrent = ryzomGetLocalTime();
	nlinfo ("PROFILE: %d seconds (%d total) for Initializing user interface", (uint32)(connCurrent-connLast)/1000, (uint32)(connCurrent-connStart)/1000);

	nlinfo ("PROFILE: %d seconds for connection", (uint32)(ryzomGetLocalTime ()-connStart)/1000);

	// Init web box

	// TMP TMP
	if (ClientCfg.Local)
	{
		InterfaceState = GLOBAL_MENU;
	}

	// No loading music here, this is right before character selection, using the existing music

	// Create the loading texture. We can't do that before because we need to add search path first.
	beginLoading (LoadBackground);
	UseEscapeDuringLoading = USE_ESCAPE_DURING_LOADING;

	while ((InterfaceState != GOGOGO_IN_THE_GAME) && (InterfaceState != QUIT_THE_GAME))
	{
		switch (InterfaceState)
		{
			case AUTO_LOGIN:
				InterfaceState = autoLogin (cookie, fsaddr, firstConnection);
				break;

			case GLOBAL_MENU:
				if (!ClientCfg.Local)
				{
					if (ClientCfg.SelectCharacter == -1)
					{
						NLGUI::CDBManager::getInstance()->getDbProp ("UI:CURRENT_SCREEN")->setValue32(0); // 0 == select
					}
				}
				InterfaceState = globalMenu();
 				break;
			case GOGOGO_IN_THE_GAME:
				break;
			case QUIT_THE_GAME:
				break;
		}
	}

	firstConnection = false;

	// Restore user UI scaling
	CInterfaceManager::getInstance()->setInterfaceScale(ClientCfg.InterfaceScale, ClientCfg.InterfaceScaleAuto);

	// Disable inputs
	Actions.enable(false);
	EditActions.enable(false);

	resetTextContext("uiFontSans", true);

	if (InterfaceState == GOGOGO_IN_THE_GAME)
	{
		// set background downloader to 'paused' to ease loading of client
#ifdef RYZOM_BG_DOWNLOADER
		pauseBGDownloader();
#endif
		return true;
	}

	if (InterfaceState == QUIT_THE_GAME)
		return false;
	nlassert ((InterfaceState == GOGOGO_IN_THE_GAME) || (InterfaceState == QUIT_THE_GAME));
	return true;
}


//

// Allow user to reselect character after the server reconnection
// ------------------------------------------------------------------------------------------------
bool reconnection()
{

	game_exit = false;

	setOutGameFullScreen();

	// Preload continents
	{
		const string nmsg ("Loading continents...");
		ProgressBar.newMessage (ClientCfg.buildLoadingString(nmsg) );
		ContinentMngr.preloadSheets();
	}
/*
	if (!fsaddr.empty () && !cookie.empty ())
	{
		// it means that we have a nel_launcher values, so we are online
		ClientCfg.Local = 0;
		nlinfo ("Using the nel launcher parameters '%s' '%s'", cookie.c_str (), fsaddr.c_str ());
	}

	// If the Client is in in Local Mode -> init the Time and return.
	if (ClientCfg.Local)
	{
#ifdef ENABLE_INCOMING_MSG_RECORDER
		NetMngr.init("", "");
		// Set the impulse callback.
		NetMngr.setImpulseCallback (impulseCallBack);
		// Set the database.
		NetMngr.setDataBase (IngameDbMngr.getNodePtr());
		// init the string manager cache.
		STRING_MANAGER::CStringManagerClient::instance()->initCache("", ClientCfg.LanguageCode);	// VOIR BORIS
#endif
		connectionRestoreVideoMode ();
		return true;
	}
*/

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	ProgressBar.setFontFactor(1.0f);

	// Init out game
	SoundGlobalMenu.reset();
	pIM->initOutGame();

	// Hide cursor for interface
	Driver->showCursor (false);

	// Init global variables
	userChar		= false;
	noUserChar		= false;
	ConnectInterf	= true;
	CreateInterf	= true;
	CharacterInterf	= true;
	WaitServerAnswer= false;

	FarTP.setOutgame();

	if (SoundMngr)
		SoundMngr->setupFadeSound(1.0f, 1.0f);

	// these two globals sequence GlobalMenu to display the character select dialog
	WaitServerAnswer = true;
	userChar = true;

	// Start the finite state machine
	TInterfaceState InterfaceState = GLOBAL_MENU;

	NLGUI::CDBManager::getInstance()->getDbProp ("UI:CURRENT_SCREEN")->setValue32(-1);
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	// Active inputs
	Actions.enable(true);
	EditActions.enable(true);

	if (ClientCfg.SelectCharacter == -1)
	{
		// Re-initialise the mouse (will be now in hardware mode, if required)
		SetMousePosFirstTime = true;
		InitMouseWithCursor (ClientCfg.HardwareCursor && !StereoDisplayAttached); // the return value of enableLowLevelMouse() has already been tested at startup

		// no ui init if character selection is automatic
		SetMouseFreeLook ();
		SetMouseCursor ();
		SetMouseSpeed (ClientCfg.CursorSpeed);
		SetMouseAcceleration (ClientCfg.CursorAcceleration);
		NLGUI::CDBManager::getInstance()->getDbProp("UI:SELECTED_SLOT")->setValue32(ClientCfg.SelectedSlot);
		PlayerSelectedSlot = ClientCfg.SelectedSlot;
	}

	// we want the teleport graphics to display (not like in Server Hop mode)
	//	this also kicks the state machine to sendReady() so we stop spinning in farTPmainLoop
	FarTP.setIngame();

	// Not loading music here, this is before character selection, keep existing music

	// Create the loading texture. We can't do that before because we need to add search path first.
	beginLoading (LoadBackground);
	UseEscapeDuringLoading = USE_ESCAPE_DURING_LOADING;

	// character selection menu
	while( InterfaceState == GLOBAL_MENU )	// != GOGOGO_IN_THE_GAME) && (InterfaceState != QUIT_THE_GAME))
	{
		if (ClientCfg.SelectCharacter == -1)
		{
			NLGUI::CDBManager::getInstance()->getDbProp ("UI:CURRENT_SCREEN")->setValue32(0); // 0 == select
		}
		InterfaceState = globalMenu();
	}

	// Restore user UI scaling
	CInterfaceManager::getInstance()->setInterfaceScale(ClientCfg.InterfaceScale, ClientCfg.InterfaceScaleAuto);

	// Disable inputs
	Actions.enable(false);
	EditActions.enable(false);

	resetTextContext("uiFontSans", true);

	if (InterfaceState == GOGOGO_IN_THE_GAME)
	{
#ifdef RYZOM_BG_DOWNLOADER
		pauseBGDownloader();
#endif
		return true;
	}
	if (InterfaceState == QUIT_THE_GAME)
		return false;
	nlassert ((InterfaceState == GOGOGO_IN_THE_GAME) || (InterfaceState == QUIT_THE_GAME));
	return true;
}

// Automatic connection to the server, the user can't do anything
// ------------------------------------------------------------------------------------------------
TInterfaceState autoLogin (const string &cookie, const string &fsaddr, bool firstConnection)
{
	noUserChar = userChar = false;
	string defaultPort = string(":47851");
	if(!fsaddr.empty())
	{
		// If we have a front end address from command line, use this one
		UsedFSAddr = fsaddr;
		if (UsedFSAddr.find(":") == string::npos)
		{
			UsedFSAddr += defaultPort;
		}
	}
	else
	{
		// Otherwise, use the front end address from configfile
		UsedFSAddr = ClientCfg.FSHost;
		FSAddr = UsedFSAddr; // to be able to do /reconnect
		LoginSM.pushEvent( CLoginStateMachine::ev_skip_all_login );
		if (UsedFSAddr.find(":") == string::npos)
		{
			UsedFSAddr += defaultPort;
			FSAddr += defaultPort; // to be able to do /reconnect
		}
	}

	if (firstConnection)
		NetMngr.init (cookie, UsedFSAddr);

	// Connection
	if (!ClientCfg.Local/*ace!ClientCfg.Light*/)
	{
		string result;

		if (firstConnection)
		{
			NetMngr.connect (result);

			if (!result.empty())
			{
				nlerror ("connection : %s.", result.c_str());
				return QUIT_THE_GAME;
			}

			// Ok the client is connected

			// Set the impulse callback.
			NetMngr.setImpulseCallback (impulseCallBack);
			// Set the database.
			NetMngr.setDataBase (IngameDbMngr.getNodePtr());

			// init the string manager cache.
			STRING_MANAGER::CStringManagerClient::instance()->initCache(ClientCfg.LanguageCode);
		}
	}
	else
	{
		CCharacterSummary cs;
		cs.Name = "babar";
		//cs.Surname = "l'elephant";
		cs.People = EGSPD::CPeople::Zorai;
		cs.VisualPropA.PropertySubData.Sex = 0; // Male
//Deprecated
//		cs.Role = ROLES::range_warrior;
//		cs.Job = JOBS::CasterBuffer;
//		cs.JobLevel = 16;
		CharacterSummaries.push_back(cs);

		cs.Name = "yeah";
		//cs.Surname = "zeelot";
		cs.People = EGSPD::CPeople::Matis;
		cs.VisualPropA.PropertySubData.Sex = 1; // Female
//Deprecated
//		cs.Role = ROLES::buffer_magician;
//		cs.Job = JOBS::CasterHealer;
//		cs.JobLevel = 8;
		CharacterSummaries.push_back(cs);

		userChar = true;
	}

	WaitServerAnswer = true;

	return GLOBAL_MENU;
}

// ------------------------------------------------------------------------------------------------
void globalMenuMovieShooter()
{

	if(MovieShooterSaving)
	{
		// Add the buffer frame to the movie.
		if(!MovieShooter.addFrame(TimeInSec, Driver))
		{
			// Fail to add the frame => abort.
			endMovieShooting();
		}
		else
		{
			// Ok, just add a display.
			displaySpecialTextProgress("MovieShooting");
		}
	}

}

// ------------------------------------------------------------------------------------------------
// Build a valid PlayerName for file Save selection.
std::string	buildPlayerNameForSaveFile(const std::string &playerNameIn)
{
	// remove any shard name appended
	string playerName = playerNameIn;
	string::size_type pos = playerNameIn.find('(');
	if(pos!=string::npos && pos>0)
	{
		playerName.resize(pos);
	}

	// replace any special ucchar with '_'
	string	ret;
	ret.resize(playerName.size());
	for(uint i=0;i<playerName.size();i++)
	{
		ucchar	c= playerName[i];
		if( (c>='A' && c<='Z') ||
			(c>='a' && c<='z') ||
			(c>='0' && c<='9') ||
			(c=='_') )
		{
			ret[i]= tolower(c); // TODO: toLowerAscii
		}
		else
			ret[i]= '_';
	}
	return ret;
}


#ifdef RYZOM_BG_DOWNLOADER
static bool LuaBGDSuccessFlag = true; // tmp, for debug

void updateBGDownloaderUI()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CBGDownloaderAccess &bgDownloader = CBGDownloaderAccess::getInstance();
	bool bgWindowVisible = true;
	if (im->isInGame())
	{
		static NLMISC::CRefPtr<CInterfaceElement> bgDownloaderWindow;
		if (!bgDownloaderWindow)
		{
			bgDownloaderWindow = CWidgetManager::getInstance()->getElementFromId("ui:interface:bg_downloader");
		}
		bgWindowVisible = bgDownloaderWindow && bgDownloaderWindow->getActive();
	}
	bool prevSuccess = LuaBGDSuccessFlag;
	if (isBGDownloadEnabled() && PatchBegun)
	{
		if (AvailablePatchs == 0)
		{
			if (LuaBGDSuccessFlag)
			{
				LuaBGDSuccessFlag = CLuaManager::getInstance().executeLuaScript("bgdownloader:setPatchSuccess()");
			}
		}
		else
		{
			switch(bgDownloader.getLastTaskResult())
			{
				case BGDownloader::TaskResult_Unknown:
				{
					float progress = 0.f;
					/*if (bgDownloader.getTotalSize() != 0)
					{
						progress = (float) bgDownloader.getPatchingSize() / bgDownloader.getTotalSize();
					}*/
					if (bgDownloader.getTotalFilesToGet() != 0)
					{
						progress = (bgDownloader.getCurrentFilesToGet() + bgDownloader.getCurrentFileProgress()) / bgDownloader.getTotalFilesToGet();
					}
					if (LuaBGDSuccessFlag && bgWindowVisible)
					{
						LuaBGDSuccessFlag = CLuaManager::getInstance().executeLuaScript(toString("bgdownloader:setPatchProgress(%f)", progress));
					}
					// display current priority of the downloader
					if (LuaBGDSuccessFlag && bgWindowVisible)
					{
						LuaBGDSuccessFlag = CLuaManager::getInstance().executeLuaScript("bgdownloader:displayPriority()");
					}
				}
				break;
				case BGDownloader::TaskResult_Success:
					if (LuaBGDSuccessFlag && bgWindowVisible)
					{
						LuaBGDSuccessFlag = CLuaManager::getInstance().executeLuaScript("bgdownloader:setPatchSuccess()");
					}
					// task finished
					AvailablePatchs = 0;
					if (bgDownloader.getPatchCompletionFlag(true /* clear flag */))
					{
						// when in-game, display a message to signal the end of the patch
						if (im->isInGame())
						{
							im->displaySystemInfo(CI18N::get("uiBGD_InGamePatchCompletion"), "BC");
						}
					}
				break;
				default:
					// error case
					if (LuaBGDSuccessFlag && bgWindowVisible)
					{
						LuaBGDSuccessFlag = CLuaManager::getInstance().executeLuaScript("bgdownloader:setPatchError()");
					}
				break;
			}
		}
	}
	else
	{
		if (LuaBGDSuccessFlag && bgWindowVisible)
		{
			if (isBGDownloadEnabled())
			{
				// no necessary patch for now
				LuaBGDSuccessFlag = CLuaManager::getInstance().executeLuaScript("bgdownloader:setNoNecessaryPatch()");
			}
			else
			{
				// no download ui
				LuaBGDSuccessFlag = CLuaManager::getInstance().executeLuaScript("bgdownloader:setNoDownloader()");
			}
		}
	}
	if (prevSuccess != LuaBGDSuccessFlag)
	{
		nlwarning("Some scipt error occurred");
	}
}
#endif

// compute patcher priority, depending on the presence of one or more mainland characters : in this case, give the patch a boost
void updatePatcherPriorityBasedOnCharacters()
{
#ifdef RYZOM_BG_DOWNLOADER
	if (isBGDownloadEnabled())
	{
		if (CBGDownloaderAccess::getInstance().getDownloadThreadPriority() != BGDownloader::ThreadPriority_Paused)
		{
			// choose priority based on available characters :
			bool hasMainlandChar = false;
			for(std::vector<CCharacterSummary>::iterator it = CharacterSummaries.begin(); it != CharacterSummaries.end(); ++it)
			{
				if (it->Name.empty()) continue;
				if (!it->InNewbieland)
				{
					hasMainlandChar = true;
					break;
				}
			}
			CBGDownloaderAccess::getInstance().requestDownloadThreadPriority(hasMainlandChar ? BGDownloader::ThreadPriority_Normal : BGDownloader::ThreadPriority_Low, false);
		}
	}
#endif
}

// Launch the interface to choose a character
// ------------------------------------------------------------------------------------------------
TInterfaceState globalMenu()
{
	CLoginProgressPostThread::getInstance().step(CLoginStep(LoginStep_CharacterSelection, "login_step_character_selection"));

#ifdef RYZOM_BG_DOWNLOADER
	CBGDownloaderAccess &bgDownloader = CBGDownloaderAccess::getInstance();

	if (isBGDownloadEnabled())
	{
		// If there's a need for mainland download, then proceed
		if (AvailablePatchs	 & (1 << BGDownloader::DownloadID_MainLand))
		{
			// if a task is already started, then this was a situation where player went back from game to the character selection,
			// so just unpause
			BGDownloader::TTaskResult dummyResult;
			ucstring				  dummyMessage; // OLD
			if (!bgDownloader.isTaskEnded(dummyResult, dummyMessage))
			{
				unpauseBGDownloader();
			}
		}
	}
#endif

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	sint32 nScreenConnecting, nScreenIntro, nScreenServerCrashed;
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("screen_connecting"), nScreenConnecting);
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("screen_intro"), nScreenIntro);
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("screen_crashing"), nScreenServerCrashed);

	// SKIP INTRO : Write to the database if we have to skip the intro and write we want to skip further intro to client cfg
	if (ClientCfg.SkipIntro)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:SKIP_INTRO", false);
		if (pNL != NULL)
			pNL->setValue64(1);
	}

	TGameCycle serverTick = NetMngr.getCurrentServerTick();
	bool PlayerWantToGoInGame = false;
	bool firewallTimeout = false;

	ProgressBar.finish(); // no progress while selecting character

	while (PlayerWantToGoInGame == false)
	{

#if !FINAL_VERSION
		if (DebugOutgameReloadUI)
		{
			DebugOutgameReloadUI = false;
			pIM->uninitOutGame();
			pIM->initOutGame();
			CWidgetManager::getInstance()->activateMasterGroup ("ui:outgame", true);
			NLGUI::CDBManager::getInstance()->getDbProp ("UI:CURRENT_SCREEN")->setValue32(2); // TMP TMP
			IngameDbMngr.flushObserverCalls();
			NLGUI::CDBManager::getInstance()->flushObserverCalls();
			CWidgetManager::getInstance()->getElementFromId("ui:outgame:charsel")->setActive(false);
			CWidgetManager::getInstance()->getElementFromId("ui:outgame:charsel")->setActive(true);
			// Active inputs
			Actions.enable(true);
			EditActions.enable(true);
#ifdef RYZOM_BG_DOWNLOADER
			LuaBGDSuccessFlag = true;
#endif
			CWidgetManager::getInstance()->getParser()->reloadAllLuaFileScripts();
		}
#endif

#ifdef RYZOM_BG_DOWNLOADER
		updateBGDownloaderUI();
#endif

		// Update network.
		try
		{
			if ( ! firewallTimeout )
				NetMngr.update();
		}
		catch (const EBlockedByFirewall&)
		{
			if ( NetMngr.getConnectionState() == CNetManager::Disconnect )
			{
				firewallTimeout = true;
			}
			else
			{
				// Display the firewall alert string
				CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:outgame:connecting:title"));
				if (pVT != NULL)
					pVT->setTextLocalized("uiFirewallAlert", true);

				// The mouse and fullscreen mode should be unlocked for the user to set the firewall permission
				nlSleep( 30 ); // 'nice' the client, and prevent to make too many send attempts
			}

		}
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();

		// check if we can send another dated block
		if (NetMngr.getCurrentServerTick() != serverTick)
		{
			//
			serverTick = NetMngr.getCurrentServerTick();
			NetMngr.send(serverTick);
		}
		else
		{
			// Send dummy info
			NetMngr.send();
		}
		// Update the DT T0 and T1 global variables
		updateClientTime();
		CInputHandlerManager::getInstance()->pumpEvents();
		Driver->clearBuffers(CRGBA::Black);
		Driver->setMatrixMode2D11();

		// Update sound
		SoundGlobalMenu.updateSound();

		// Interface handling & displaying (processes clicks...)
		pIM->updateFrameEvents();
		pIM->updateFrameViews(NULL);
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();

		// Movie shooter
		globalMenuMovieShooter();

		// Force the client to sleep a bit.
		if(ClientCfg.Sleep >= 0)
		{
			nlSleep(ClientCfg.Sleep);
		}

#if !FINAL_VERSION
		if (DebugOutgameUI)
		{
			pIM->displayUIViewBBoxs("");
			pIM->displayUICtrlBBoxs("");
			pIM->displayUIGroupBBoxs("");
			displayDebugUIUnderMouse();
		}
#endif

		// Display
		Driver->swapBuffers();

		// SERVER INTERACTIONS WITH INTERFACE
		if (WaitServerAnswer)
		{
			if (noUserChar || userChar)
			{
#ifdef RYZOM_BG_DOWNLOADER
				if (isBGDownloadEnabled())
				{
					// If there's a need for mainland download, then proceed
					if (AvailablePatchs	 & (1 << BGDownloader::DownloadID_MainLand))
					{
						// if a task is already started, then this was a situation where player went back from game to the character selection,
						// so just unpause
						BGDownloader::TTaskResult dummyResult;
						ucstring				  dummyMessage; // OLD
						if (bgDownloader.isTaskEnded(dummyResult, dummyMessage))
						{
							// launch mainland patch as a background task
							BGDownloader::CTaskDesc task(BGDownloader::DLState_GetAndApplyPatch,
														 (1 << BGDownloader::DownloadID_MainLand));
							bgDownloader.startTask(task, getBGDownloaderCommandLine(), false /* showDownloader */);

							// choose priority based on available characters :
							updatePatcherPriorityBasedOnCharacters();

							PatchBegun = true;
						}
					}
				}
#endif

				//nlinfo("impulseCallBack : received userChars list");
				noUserChar = userChar = false;
				if( FarTP.isReselectingChar() || !FarTP.isServerHopInProgress() ) // if doing a Server Hop, expect serverReceivedReady without action from the user
				{
					sint charSelect = -1;
					if (ClientCfg.SelectCharacter != -1)
						charSelect = ClientCfg.SelectCharacter;

					if (LoginCharsel != -1)
						charSelect = LoginCharsel;

					WaitServerAnswer = false;
					if (charSelect == -1)
					{
						CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:SERVER_RECEIVED_CHARS", false);
						if (pNL != NULL)
						{
							pNL->setValue64 (1); // Send impulse to interface observers
							IngameDbMngr.flushObserverCalls();
							NLGUI::CDBManager::getInstance()->flushObserverCalls();
							pNL->setValue64 (0);
							IngameDbMngr.flushObserverCalls();
							NLGUI::CDBManager::getInstance()->flushObserverCalls();
						}
					}
					else
					{
						// check that the pre selected character is available
						if (CharacterSummaries[charSelect].People == EGSPD::CPeople::Unknown || charSelect > 4)
						{
							// BAD ! preselected char does not exist, use the first available or fail
							uint i;
							for (i=0; i<CharacterSummaries.size(); ++i)
							{
								if (CharacterSummaries[i].People != EGSPD::CPeople::Unknown)
									break;
							}
							if (i == CharacterSummaries.size())
							{
								Driver->systemMessageBox("You have no character for the current user.\nClient will exit.", "Char loading error", UDriver::okType, UDriver::exclamationIcon);
								exit(-1);
							}
							else
							{
								UDriver::TMessageBoxId ret = Driver->systemMessageBox("The pre-selected character doesn't exist.\nDo you want to use the first available character instead ?", "Char loading error", UDriver::yesNoType, UDriver::warningIcon);
								if (ret == UDriver::noId)
									exit(-1);
								else
									charSelect = i;
							}
						}
						// Auto-selection for fast launching (dev only)
						CAHManager::getInstance()->runActionHandler("launch_game", NULL, toString("slot=%d|edit_mode=0", charSelect));

						if (LoginCharsel == -1)
							ClientCfg.SelectCharacter = charSelect;
					}

				}

				// Clear sending buffer that may contain prevous QUIT_GAME when getting back to the char selection screen
				NetMngr.flushSendBuffer();
			}

			if (CharNameValidArrived)
			{
				//nlinfo("impulseCallBack : received CharNameValidArrived");
				CharNameValidArrived = false;
				WaitServerAnswer = false;
				if (ClientCfg.SelectCharacter == -1)
				{
					CCDBNodeLeaf *pNL;
					pNL = NLGUI::CDBManager::getInstance()->getDbProp(CharNameValidDBLink,false);
					if (pNL != NULL)
					{
						if (CharNameValid)
							pNL->setValue64(1);
						else
							pNL->setValue64(0);
					}

					pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:SERVER_RECEIVED_VALID", false);
					if (pNL != NULL)
					{
						pNL->setValue64 (1); // Send impulse to interface observers
						IngameDbMngr.flushObserverCalls();
						NLGUI::CDBManager::getInstance()->flushObserverCalls();
						pNL->setValue64 (0);
						IngameDbMngr.flushObserverCalls();
						NLGUI::CDBManager::getInstance()->flushObserverCalls();
					}
				}
			}

			if (serverReceivedReady)
			{
				//nlinfo("impulseCallBack : received serverReceivedReady");
				serverReceivedReady = false;
				WaitServerAnswer = false;
				PlayerWantToGoInGame = true;
			}
		}
		else
		{
			noUserChar = false;
			userChar = false;
			CharNameValidArrived = false;
			serverReceivedReady = false;
		}

		// Check if server disconnect the client
		if (!ClientCfg.Local)
		{
			if (NetMngr.getConnectionState() == CNetManager::Disconnect)
			{
				// Display the connection failure screen
				CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:CURRENT_SCREEN", false);
				if (pNL != NULL)
					pNL->setValue64 (nScreenServerCrashed);

				if ( firewallTimeout )
				{
					// Display the firewall error string instead of the normal failure string
					CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:outgame:crashing:title"));
					if (pVT != NULL)
					{
						pVT->setMultiLine( true );
						pVT->setTextLocalized(CI18N::get("uiFirewallFail")+".\n"+
									  CI18N::get("uiFirewallAlert")+".", false);
					}
				}
			}
		}



		// We want to quit the game without playing
		if (game_exit)
			return QUIT_THE_GAME;
	}

	if (ClientCfg.SelectCharacter != -1)
		PlayerSelectedSlot = ClientCfg.SelectCharacter;

	// Notify the state machine that we're exiting from global menu
	LoginSM.pushEvent(CLoginStateMachine::ev_global_menu_exited);

	//  Init the current Player Name (for interface.cfg and sentence.name save). Make a good File Name.
	string playerName = CharacterSummaries[PlayerSelectedSlot].Name.toUtf8();
	PlayerSelectedFileName = buildPlayerNameForSaveFile(playerName);

	// Init the current Player Home shard Id and name
	CharacterHomeSessionId = CharacterSummaries[PlayerSelectedSlot].Mainland;
	PlayerSelectedMainland= CharacterSummaries[PlayerSelectedSlot].Mainland;
	PlayerSelectedHomeShardName.clear();
	PlayerSelectedHomeShardNameWithParenthesis.clear();
	for(uint i=0;i<CShardNames::getInstance().getSessionNames().size();i++)
	{
		const CShardNames::TSessionName	&sessionName= CShardNames::getInstance().getSessionNames()[i];
		if(PlayerSelectedMainland == sessionName.SessionId)
		{
			PlayerSelectedHomeShardName= sessionName.DisplayName;
			PlayerSelectedHomeShardNameWithParenthesis= '(' + PlayerSelectedHomeShardName + ')';
		}
	}


	// Restore video mode
	if (ClientCfg.SelectCharacter == -1)
		connectionRestoreVideoMode ();

	// Skip intro next time
	ClientCfg.writeBool("SkipIntro", true);

	//	return SELECT_CHARACTER;
	return GOGOGO_IN_THE_GAME;
}

#if !FINAL_VERSION
// ***************************************************************************
class CAHDebugOutgameDebugUI : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		DebugOutgameUI = !DebugOutgameUI;
	}
};
REGISTER_ACTION_HANDLER (CAHDebugOutgameDebugUI, "debug_outgame_ui");
// ***************************************************************************
class CAHDebugOutgameReloadUI : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		DebugOutgameReloadUI = !DebugOutgameReloadUI;
	}
};
REGISTER_ACTION_HANDLER (CAHDebugOutgameReloadUI, "debug_outgame_reload_ui");
#endif

// Init the character selection slot texts from the character summaries
// ------------------------------------------------------------------------------------------------
class CAHNetInitCharSel : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sPath = getParam(Params, "slottexts");
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		uint i;
		for (i = 0; i < CharacterSummaries.size(); ++i)
		{
			CCharacterSummary &rCS = CharacterSummaries[i];
			CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(sPath+":text"+NLMISC::toString(i));
			CViewText *pVT = dynamic_cast<CViewText*>(pIE);
			if (pVT == NULL) return;

			if (rCS.Name.empty())
				pVT->setTextLocalized("uiEmptySlot", true);
			else
				pVT->setTextLocalized(rCS.Name.toUtf8(), false);
		}
		// 5 slots
		for (; i < 5; ++i)
		{
			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(sPath+":text"+NLMISC::toString(i)));
			if (pVT == NULL) return;
			pVT->setTextLocalized("uiEmptySlot", true);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHNetInitCharSel, "net_init_char_sel");

// ------------------------------------------------------------------------------------------------
void setTarget(CCtrlBase *ctrl, const string &targetName, std::string &value)
{
	std::vector<CInterfaceLink::CTargetInfo> targets;
	// find first enclosing group
	CCtrlBase *currCtrl = ctrl;
	CInterfaceGroup *ig = NULL;
	while (currCtrl)
	{
		ig = dynamic_cast<CInterfaceGroup *>(currCtrl);
		if (ig != NULL) break;
		currCtrl = currCtrl->getParent();
	}
	if (ig)
	{
		CInterfaceExprValue exprValue;
		exprValue.setString(value);

		CInterfaceLink::splitLinkTargets(targetName, ig, targets);
		for(uint k = 0; k < targets.size(); ++k)
		{
			if (targets[k].Elem) targets[k].affect(exprValue);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void setTarget(CCtrlBase *ctrl, const string &targetName, uint32 value)
{
	std::vector<CInterfaceLink::CTargetInfo> targets;
	// find first enclosing group
	CCtrlBase *currCtrl = ctrl;
	CInterfaceGroup *ig = NULL;
	while (currCtrl)
	{
		ig = dynamic_cast<CInterfaceGroup *>(currCtrl);
		if (ig != NULL) break;
		currCtrl = currCtrl->getParent();
	}
	if (ig)
	{
		CInterfaceExprValue exprValue;
		exprValue.setInteger(value);

		CInterfaceLink::splitLinkTargets(targetName, ig, targets);
		for(uint k = 0; k < targets.size(); ++k)
		{
			if (targets[k].Elem) targets[k].affect(exprValue);
		}
	}
}

// ------------------------------------------------------------------------------------------------
class CAHGetSlot: public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sProp = getParam(Params, "prop");
		string sTarget = getParam(Params, "target");
		string sSlot = getParam(Params, "slot");

		CInterfaceExprValue result;
		if (!CInterfaceExpr::eval(sSlot, result))
			return;
		uint8 selectedSlot = (uint8)result.getInteger();
		if (selectedSlot >= CharacterSummaries.size())
			return;

		PlayerSelectedSlot = selectedSlot;

		if (CharacterSummaries[PlayerSelectedSlot].Name.empty())
			return;

		string sValue;
		uint32 nValue = 0;

		if (sProp == "name")
		{
			sValue = CharacterSummaries[PlayerSelectedSlot].Name.toUtf8();
			setTarget (pCaller, sTarget, sValue);
		}
/*			else if (sProp == "surname")
Deprecated	{
			sValue = CharacterSummaries[PlayerSelectedSlot].Surname;
			setTarget (pCaller, sTarget, sValue);
		}
*/			else if (sProp == "title")
		{
			bool womanTitle;
			if( CharacterSummaries[PlayerSelectedSlot].VisualPropA.PropertySubData.Sex == 1 )
			{
				UserEntity->setGender( GSGENDER::female );
				womanTitle = true;
			}
			else
			{
				UserEntity->setGender( GSGENDER::male );
				womanTitle = false;
			}
			string titleStr = CHARACTER_TITLE::toString(CharacterSummaries[PlayerSelectedSlot].Title);
			sValue = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(titleStr, womanTitle);
			{
				// Sometimes translation contains another title
				string::size_type pos = sValue.find('$');
				if (pos != string::npos)
				{
					sValue = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(sValue), womanTitle);
				}
			}
			setTarget (pCaller, sTarget, sValue);
		}
/*			else if (sProp == "orient")
Deprecated	{
			sValue = ROLES::roleToUCString(CharacterSummaries[PlayerSelectedSlot].Role);
			setTarget (pCaller, sTarget, sValue);
		}
		else if (sProp == "job")
		{
//Deprecated
//				sValue = JOBS::jobToUCString(CharacterSummaries[PlayerSelectedSlot].Job);
			sValue = JOBS::jobToUCString(JOBS::BladeBearer);
			setTarget (pCaller, sTarget, sValue);
		}
*/			else if (sProp == "level")
		{
//Deprecated
//				sValue = toString(CharacterSummaries[PlayerSelectedSlot].JobLevel);
			sValue = toString(1);
			setTarget (pCaller, sTarget, sValue);
		}
		else if (sProp == "pos")
		{
			nValue = CharacterSummaries[PlayerSelectedSlot].Location;
			setTarget (pCaller, sTarget, nValue);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHGetSlot, "get_slot");


// Setup the database from a database entry which represents a slot
// ------------------------------------------------------------------------------------------------
class CAHSetDBFromSlot : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sDBLink = getParam(Params, "dblink");
		string sSlot = getParam(Params, "slot");

		CInterfaceExprValue result;
		if (!CInterfaceExpr::eval(sSlot, result))
			return;

		PlayerSelectedSlot = (uint8)result.getInteger();

		if (PlayerSelectedSlot >= CharacterSummaries.size())
			return;

		// Setup the database from the character summary
		CCharacterSummary &rCS = CharacterSummaries[PlayerSelectedSlot];
		if (rCS.Name.empty())
			return;

		SCharacter3DSetup::setupDBFromCharacterSummary(sDBLink, rCS);
	}
};
REGISTER_ACTION_HANDLER (CAHSetDBFromSlot, "set_db_from_slot");


// Reset all the pushed radio button of a group
// ------------------------------------------------------------------------------------------------
class CAHResetPushed: public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sDBLink = getParam(Params, "dblink");
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(pCaller->getId(), sDBLink);
		CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(pIE);
		if (pIG == NULL) return;

		const vector<CCtrlBase*> vCB = pIG->getControls();
		for (uint i = 0; i < vCB.size(); ++i)
		{
			CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(vCB[i]);
			if (pBut && pBut->getType() == CCtrlBaseButton::RadioButton)
			{
				pBut->setPushed (false);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHResetPushed, "reset_pushed");





// Launch the game given a slot (slot is reference to the character summaries
// ------------------------------------------------------------------------------------------------
class CAHLaunchGame : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		// Get the edit/play mode
		string sEditMode = getParam(Params, "edit_mode");
		bool wantsEditMode = false;
		CInterfaceExprValue result;
		bool wantsNewScenario = false;

		if (CInterfaceExpr::eval(sEditMode, result))
		{
			wantsEditMode = (result.getInteger() == 1) || (result.getInteger() == 2);
			wantsNewScenario = (result.getInteger() == 2);
		}

		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (wantsEditMode)
		{
			// full patch needed for edition, warn the client
			if (AvailablePatchs != 0)
			{
				if (im->isInGame())
				{
					inGamePatchUncompleteWarning();
				}
				else
				{
					im->messageBoxWithHelp(CI18N::get("uiBGD_FullPatchNeeded"), "ui:outgame");
				}
				return;
			}
		}

		// Get the player selected slot
		string sSlot = getParam(Params, "slot");
		if (sSlot != "ingame_auto")
		{
			CInterfaceExprValue result;
			if (!CInterfaceExpr::eval(sSlot, result))
				return;
			PlayerSelectedSlot = (uint8)result.getInteger();
			if (PlayerSelectedSlot >= CharacterSummaries.size())
				return;

			ClientCfg.writeInt("SelectedSlot",PlayerSelectedSlot);
			if (ClientCfg.SaveConfig)
				ClientCfg.ConfigFile.save();
		}


		/*
		static volatile bool isMainlandCharacter = false; // TMP until we can get this info
		if (isMainlandCharacter)
		{
			nlassert(0); // use id="message_box" !!!
			if (AvailablePatchs != 0)
			{
				im->messageBoxWithHelp(CI18N::get("uiBGD_MainlandCharFullPatchNeeded"), "ui:outgame");
			}
			return;
		}
		*/


		// Select the right sheet to create the user character.
		ClientCfg.UserSheet = CharacterSummaries[PlayerSelectedSlot].SheetId.toString();

		// If the user wants to enter its editing session, get the ring server to Far TP to.
		if (wantsEditMode)
		{

			if (wantsNewScenario)
			{
				CSessionBrowserImpl	&sb = CSessionBrowserImpl::getInstance();
				sb.init(NULL);
				sb.closeEditSession(sb.getCharId());
				sb.waitOneMessage(CSessionBrowserImpl::getMessageName("on_invokeResult"));
			}
			if (FarTP.requestFarTPToSession( (TSessionId)0, PlayerSelectedSlot, CFarTP::LaunchEditor, false ))
			{
				WaitServerAnswer = true; // prepare to receive the character messages
			}

//			// If the player clicked 'Launch Editor', there was no CONNECTION:SELECT_CHAR sent yet,
//			// so don't wait for the EGS to acknowledge our quit message as he does not know our character
//			LoginSM.pushEvent(CLoginStateMachine::ev_ingame_return);

			return;
		}

		// Send CONNECTION:SELECT_CHAR
		CBitMemStream out;
		nlverify( GenericMsgHeaderMngr.pushNameToStream ("CONNECTION:SELECT_CHAR", out) );
		//nlinfo("impulseCallBack : CONNECTION:SELECT_CHAR '%d' sent.", PlayerSelectedSlot);


		CSelectCharMsg	SelectCharMsg;
		SelectCharMsg.c = (uint8)PlayerSelectedSlot;
		out.serial (SelectCharMsg);
		if (!ClientCfg.Local/*ace!ClientCfg.Light*/)
		{
			NetMngr.push(out);
			NetMngr.send(NetMngr.getCurrentServerTick());
		}

		//PlayerWantToGoInGame = true;

//		CBitMemStream out2;
//		if(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:ENTER", out2))
//		{
//			NetMngr.push(out2);
//			nlinfo("impulseCallBack : CONNECTION:ENTER sent");
//		}
//		else
//			nlwarning("unknown message name : 'CONNECTION:ENTER'.");

		WaitServerAnswer = true;
		if (ClientCfg.Local)
			serverReceivedReady = true;
	}
};
REGISTER_ACTION_HANDLER (CAHLaunchGame, "launch_game");


// Ask the server to create a character
// ------------------------------------------------------------------------------------------------
class CAHAskCreateChar : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Create the message for the server to create the character.
		CBitMemStream out;
		if (!GenericMsgHeaderMngr.pushNameToStream("CONNECTION:CREATE_CHAR", out))
		{
			nlwarning ("don't know message name CONNECTION:CREATE_CHAR");
			return;
		}

		// Setup the name
		string sEditBoxPath = getParam (Params, "name");
		string sFirstName = "NotSet";
		string sSurName = "NotSet";
		CGroupEditBox *pGEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(sEditBoxPath));
		if (pGEB != NULL)
			sFirstName = pGEB->getInputString();
		else
			nlwarning ("can't get edit box name : %s",sEditBoxPath.c_str());

		// Build the character summary from the database branch ui:temp:char3d
		CCharacterSummary CS;
		string sCharSumPath = getParam(Params, "charsum");
		SCharacter3DSetup::setupCharacterSummaryFromDB(CS, sCharSumPath);
		CS.Mainland = MainlandSelected;
		CS.Name = ucstring::makeFromUtf8(sFirstName); // FIXME: UTF-8 (serial)
		//CS.Surname = sSurName;

		// Create the message to send to the server from the character summary
		CCreateCharMsg CreateCharMsg;

		CreateCharMsg.setupFromCharacterSummary(CS);

		// Slot
		{
			string sSlot = getParam(Params, "slot");

			CInterfaceExprValue result;
			if (!CInterfaceExpr::eval(sSlot, result))
				return;

			CreateCharMsg.Slot = (uint8)result.getInteger();

			NLGUI::CDBManager::getInstance()->getDbProp("UI:SELECTED_SLOT")->setValue32(PlayerSelectedSlot);
		}

		// Setup the new career
		string sCaracBasePath = getParam (Params, "caracs");
		CreateCharMsg.NbPointFighter	= (uint8)NLGUI::CDBManager::getInstance()->getDbProp(sCaracBasePath+"FIGHT")->getValue32();
		CreateCharMsg.NbPointCaster		= (uint8)NLGUI::CDBManager::getInstance()->getDbProp(sCaracBasePath+"MAGIC")->getValue32();
		CreateCharMsg.NbPointCrafter	= (uint8)NLGUI::CDBManager::getInstance()->getDbProp(sCaracBasePath+"CRAFT")->getValue32();
		CreateCharMsg.NbPointHarvester	= (uint8)NLGUI::CDBManager::getInstance()->getDbProp(sCaracBasePath+"FORAGE")->getValue32();

		// Setup starting point
		string sLocationPath = getParam(Params, "loc");
		{
			CreateCharMsg.StartPoint = RYZOM_STARTING_POINT::borea;

			CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp (sLocationPath, false);
			if (pNL != NULL)
				CreateCharMsg.StartPoint = (RYZOM_STARTING_POINT::TStartPoint)(pNL->getValue64());
			else
				nlwarning(("Can't read starting point from the database : " + sLocationPath).c_str());

			if (CS.People == EGSPD::CPeople::Fyros)
				CreateCharMsg.StartPoint= (RYZOM_STARTING_POINT::TStartPoint)(((uint8)CreateCharMsg.StartPoint) + ((uint8)RYZOM_STARTING_POINT::fyros_start));
			else if (CS.People == EGSPD::CPeople::Matis)
				CreateCharMsg.StartPoint= (RYZOM_STARTING_POINT::TStartPoint)(((uint8)CreateCharMsg.StartPoint) + ((uint8)RYZOM_STARTING_POINT::matis_start));
			else if (CS.People == EGSPD::CPeople::Tryker)
				CreateCharMsg.StartPoint= (RYZOM_STARTING_POINT::TStartPoint)(((uint8)CreateCharMsg.StartPoint) + ((uint8)RYZOM_STARTING_POINT::tryker_start));
			else // if (CS.People == EGSPD::CPeople::Zorai)
				CreateCharMsg.StartPoint= (RYZOM_STARTING_POINT::TStartPoint)(((uint8)CreateCharMsg.StartPoint) + ((uint8)RYZOM_STARTING_POINT::zorai_start));

		}

		// Send the message to the server
		CreateCharMsg.serialBitMemStream (out);
		if (!ClientCfg.Local/*!ClientCfg.Light*/)
		{
			noUserChar = userChar = false;

			NetMngr.push(out);
			NetMngr.send(NetMngr.getCurrentServerTick());

			//nlinfo("impulseCallBack : CONNECTION:CREATE_CHAR sent");
			CreateCharMsg.dump();
		}
		else
		{
			userChar = true;
			if (CharacterSummaries.size() < 5)
				CharacterSummaries.push_back(CS);
		}
		WaitServerAnswer = true;
	}
};
REGISTER_ACTION_HANDLER (CAHAskCreateChar, "ask_create_char");



// Ask the server to delete a character
// ------------------------------------------------------------------------------------------------
class CAHAskDeleteChar : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		// Create the message for the server to create the character.
		CBitMemStream out;
		if (!GenericMsgHeaderMngr.pushNameToStream("CONNECTION:DELETE_CHAR", out))
		{
			nlwarning ("don't know message name CONNECTION:DELETE_CHAR");
			return;
		}

		// Get the selected slot
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		string sSlot = getParam(Params, "slot");

		CInterfaceExprValue result;
		if (!CInterfaceExpr::eval(sSlot, result))
			return;

		uint8 nSelectedSlot = (uint8)result.getInteger();
		if (nSelectedSlot >= CharacterSummaries.size())
			return;

		out.serial (nSelectedSlot);

		// Yoyo: delete the Local files. To avoid problem if recreate a character with same name.
		string playerName = CharacterSummaries[nSelectedSlot].Name.toUtf8();
		string playerDeletedFileName = buildPlayerNameForSaveFile(playerName);
		// Delete the 2 Local files
		pIM->deletePlayerConfig(playerDeletedFileName);
		pIM->deletePlayerKeys(playerDeletedFileName);

		// Send the message to the server
		if (!ClientCfg.Local/*ace!ClientCfg.Light*/)
		{
			noUserChar = userChar = false;
			NetMngr.push(out);
			NetMngr.send(NetMngr.getCurrentServerTick());

			//nlinfo("impulseCallBack : CONNECTION:DELETE_CHAR %d sent", nSelectedSlot);
		}
		else
		{
			if (nSelectedSlot < CharacterSummaries.size())
				CharacterSummaries.erase (CharacterSummaries.begin()+nSelectedSlot);
			if (CharacterSummaries.size() != 0)
				userChar = true;
			else
				noUserChar = true;
		}
		WaitServerAnswer = true;
	}
};
REGISTER_ACTION_HANDLER (CAHAskDeleteChar, "ask_delete_char");

// ------------------------------------------------------------------------------------------------
string getTarget(CCtrlBase * /* ctrl */, const string &targetName)
{
	string sTmp = targetName;
	std::vector<CInterfaceLink::CTargetInfo> targetsVector;
	CInterfaceLink::splitLinkTargets(sTmp, NULL, targetsVector);

	CInterfaceLink::CTargetInfo &rTI = targetsVector[0];

	CInterfaceElement *elem = rTI.Elem;
	if (!elem)
	{
		nlwarning("<CInterfaceExpr::getprop> : Element is NULL");
		return "";
	}
	const CReflectedProperty *pRP = CReflectSystem ::getProperty(elem->getReflectedClassName(), rTI.PropertyName);

	if (pRP->Type == CReflectedProperty::String)
		return ((elem->*(pRP->GetMethod.GetString))());
	return "";
}

#ifdef RYZOM_LUA_UCSTRING
// ------------------------------------------------------------------------------------------------
ucstring getUCTarget(CCtrlBase * /* ctrl */, const string &targetName)
{
	string sTmp = targetName;
	std::vector<CInterfaceLink::CTargetInfo> targetsVector;
	CInterfaceLink::splitLinkTargets(sTmp, NULL, targetsVector);

	CInterfaceLink::CTargetInfo &rTI = targetsVector[0];

	CInterfaceElement *elem = rTI.Elem;
	if (!elem)
	{
		nlwarning("<CInterfaceExpr::getprop> : Element is NULL");
		return ucstring(""); // TODO: UTF-8 Lua
	}
	const CReflectedProperty *pRP = elem->getReflectedProperty(rTI.PropertyName);

	if (pRP->Type == CReflectedProperty::UCString)
		return ((elem->*(pRP->GetMethod.GetUCString))());
	return ucstring(""); // TODO: UTF-8 Lua
}
#endif

/*// Ask the server to rename a character
// ------------------------------------------------------------------------------------------------
class CAHAskRenameChar : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sName = getTarget(NULL,getParam(Params, "name"));
		string sSurname = getTarget(NULL,getParam(Params, "surname"));

		string sDBSlot = getParam(Params, "dbslot");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		uint8 nSelectedSlot = (uint8)NLGUI::CDBManager::getInstance()->getDbProp(sDBSlot,false)->getValue32();

		if (nSelectedSlot > CharacterSummaries.size())
			return;

		// Create the message for the server to create the character.
		CBitMemStream out;
		if (!GenericMsgHeaderMngr.pushNameToStream("CONNECTION:RENAME_CHAR", out))
		{
			nlwarning ("don't know message name CONNECTION:RENAME_CHAR");
			return;
		}

		// Get the selected slot
		out.serial (nSelectedSlot);
		out.serial (sName);
		out.serial (sSurname);

		// Send the message to the server
		if (!ClientCfg.Light)
		{
			noUserChar = userChar = false;
			NetMngr.push (out);
			NetMngr.send (NetMngr.getCurrentServerTick());

			nldebug("impulseCallBack : CONNECTION:RENAME_CHAR sent");

			// Wait for the character message which describe all the characters on a server
			while (!noUserChar && !userChar)
			{
				//NetMngr.waitForServer();
				NetMngr.update();
				NetMngr.send();
				nlSleep(100);
			}
		}
		else
		{
			CharacterSummaries[nSelectedSlot].FirstName = sName;
			CharacterSummaries[nSelectedSlot].Surname = sSurname;
		}
	}
};
REGISTER_ACTION_HANDLER (CAHAskRenameChar, "ask_rename_char");
*/

// Ask the server if the name is not already used
// ------------------------------------------------------------------------------------------------
class CAHAskValidName : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sTarget = getParam(Params, "target");
		string sDBLink = getParam(Params, "dblink");
		CharNameValidDBLink = sDBLink;

#ifdef RYZOM_LUA_UCSTRING
		string sName = getUCTarget(NULL,sTarget).toUtf8(); // TODO: UTF-8 Lua
#else
		string sName = getTarget(NULL, sTarget);
#endif

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if (sName.empty())
		{
			NLGUI::CDBManager::getInstance()->getDbProp(sDBLink,false)->setValue32(0);
			return;
		}

		// Ask the server
		CharNameValid = true;

		// PATCH DU BUG DE L'ESPACE !!!
		if (sName.find(' ') != ucstring::npos)
			CharNameValid = false;
		// PATCH DU BUG DE L'ESPACE !!!


		if (CharNameValid)
		{
			if (!ClientCfg.Local/*ace!ClientCfg.Light*/)
			{

				CBitMemStream out;
				if (!GenericMsgHeaderMngr.pushNameToStream("CONNECTION:ASK_NAME", out))
				{
					nlwarning ("don't know message name CONNECTION:ASK_NAME");
					return;
				}

				CCheckNameMsg checkNameMsg;
				checkNameMsg.Name = sName;
				checkNameMsg.HomeSessionId = MainlandSelected;
				checkNameMsg.serialBitMemStream(out);

				NewKeysCharNameWanted = sName;
				// append shard name
				for(uint k = 0; k < Mainlands.size(); ++k)
				{
					if (Mainlands[k].Id == MainlandSelected)
					{
						// extract name from mainland
						/*ucstring::size_type first = Mainlands[k].Name.find('('); // OLD
						ucstring::size_type last = Mainlands[k].Name.find(')');// OLD
						if (first != ucstring::npos && last != ucstring::npos && first < last)// OLD
						{
							NewKeysCharNameWanted += Mainlands[k].Name.substr(first, last - first + 1);
						}*/
						NewKeysCharNameWanted += ('(' + Mainlands[k].Name.toUtf8() + ')');
						break;
					}
				}

				NewKeysCharNameValidated.clear();

				NetMngr.push(out);
				NetMngr.send(NetMngr.getCurrentServerTick());

				//nlinfo("impulseCallBack : CONNECTION:ASK_NAME sent");

				// Wait for the valid character name message
				CharNameValidArrived = false;
			}
			else
			{

				CharNameValid = true;
				CharNameValidArrived = true;

				for (uint i = 0; i < CharacterSummaries.size(); ++i)
				{
					string ls = CharacterSummaries[i].Name.toString();
					if (ls == sName)
						CharNameValid = false;
				}
			}
		}
		else
		{
			CharNameValidArrived = true;
		}
		WaitServerAnswer = true;
	}
};
REGISTER_ACTION_HANDLER (CAHAskValidName, "ask_valid_name");

// ------------------------------------------------------------------------------------------------
class CAHPlaySound : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sName = getParam(Params, "name");
		TStringId id = CStringMapper::map(sName);
		if (SoundMngr != NULL)
			SoundMngr->spawnSource(id,CVector(0,0,0));
	}
};
REGISTER_ACTION_HANDLER (CAHPlaySound, "play_sound");

// ------------------------------------------------------------------------------------------------
class CAHPlayMusicOutgame : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		// get the name of the wanted music
		string	sName = getParam(Params, "name");
		bool	async;
		fromString(getParam(Params, "async"), async);

		// if empty name, return to default mode
		if (sName.empty())
			sName = ClientCfg.EmptySlotMusic;

		// change the music
		SoundGlobalMenu.setMusic(sName, async);
	}
};
REGISTER_ACTION_HANDLER (CAHPlayMusicOutgame, "play_music_outgame");

// ------------------------------------------------------------------------------------------------
class CAHRepeatUntil : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sProc = getParam(Params, "proc");
		string sCond = getParam(Params, "cond");
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		for(;;)
		{
			vector<string> p;
			p.push_back(sProc);
			CWidgetManager::getInstance()->runProcedure(sProc, pCaller, p);

			CInterfaceExprValue result;
			if (CInterfaceExpr::eval(sCond, result))
			{
				if (result.getBool())
					break;
			}
			else
			{
				break;
			}
		}

	}
};
REGISTER_ACTION_HANDLER (CAHRepeatUntil, "repeatuntil");


// ------------------------------------------------------------------------------------------------
class CAHDispInfo : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sStr = getParam(Params, "str");
		string sVal = getParam(Params, "val");

		string res;

		CInterfaceExprValue result;
		if (CInterfaceExpr::eval(sStr, result))
		{
			if (result.toString())
			{
				res += result.getString();
			}
		}
		if (CInterfaceExpr::eval(sVal, result))
		{
			if (result.toString())
			{
				res += result.getString();
			}
		}

		nlinfo(res.c_str());
	}
};
REGISTER_ACTION_HANDLER (CAHDispInfo, "disp_info");


// ***************************************************************************
class CAHInitMainlandList : public IActionHandler
{
public:

	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		//CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_MAINLAND));
		if (pList == NULL)
		{
			nlwarning("element " GROUP_LIST_MAINLAND " not found probably bad outgame.xml");
			return;
		}

		CInterfaceGroup *pPrevLine = NULL;
		for(uint i = 0; i < Mainlands.size(); i++)
		{
			vector< pair < string, string > > params;
			params.clear();
			params.push_back(pair<string,string>("id", toString(Mainlands[i].Id)));
			if (i>0)
				params.push_back(pair<string,string>("posref", "BL TL"));

			CInterfaceGroup *pNewLine = CWidgetManager::getInstance()->getParser()->createGroupInstance("t_mainland", GROUP_LIST_MAINLAND, params);
			if (pNewLine != NULL)
			{
				CViewBase *pVBon = pNewLine->getView("online");
				CViewBase *pVBoff = pNewLine->getView("offline");
				if ((pVBon != NULL) && (pVBoff != NULL))
				{
					pVBon->setActive(Mainlands[i].Online);
					pVBoff->setActive(!Mainlands[i].Online);
				}

				CViewText *pVT = dynamic_cast<CViewText*>(pNewLine->getView("name"));
				if (pVT != NULL)
				{
					std::string str = Mainlands[i].Name.toUtf8() + " " + Mainlands[i].Description.toUtf8();
					pVT->setTextLocalized(str, false);
				}

				// Add to the list
				pNewLine->setParent(pList);
				pNewLine->setParentSize(pList);
				pNewLine->setParentPos(pPrevLine);
				pList->addGroup(pNewLine);

				pPrevLine = pNewLine;
			}
		}
		// UI Patch
		if (!Mainlands.empty())
		{
			//choose default mainland from language code
			uint32 defaultMainland = 0;
			for(uint i = 0; i < Mainlands.size(); i++)
			{
				if( Mainlands[i].LanguageCode == ClientCfg.LanguageCode )
				{
					defaultMainland = i;
					break;
				}
			}

			CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_MAINLAND ":"+toString(Mainlands[defaultMainland].Id)+":but"));
			if (pCB != NULL)
			{
				pCB->setPushed(true);
				CAHManager::getInstance()->runActionHandler (pCB->getActionOnLeftClick(), pCB, pCB->getParamsOnLeftClick());
			}
		}
		pList->invalidateCoords();
	}
};
REGISTER_ACTION_HANDLER (CAHInitMainlandList, "init_mainland_list");


// ***************************************************************************
class CAHResetMainlandList : public IActionHandler
{
public:

	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		//CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_MAINLAND));
		pList->clearGroups();
	}
};
REGISTER_ACTION_HANDLER (CAHResetMainlandList, "reset_mainland_list");


// ***************************************************************************
class CAHMainlandSelect : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		nlinfo("CAHMainlandSelect called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CCtrlButton *pCB = NULL;
		// Unselect
		if (MainlandSelected.asInt() != 0)
		{
			pCB = dynamic_cast<CCtrlButton*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_MAINLAND ":"+toString(MainlandSelected)+":but"));
			if (pCB != NULL)
				pCB->setPushed(false);
		}

		pCB = dynamic_cast<CCtrlButton*>(pCaller);
		if (pCB != NULL)
		{
			string name = pCB->getId();
			name = name.substr(0,name.rfind(':'));
			uint32 mainland;
			fromString(name.substr(name.rfind(':')+1,name.size()), mainland);
			MainlandSelected = (TSessionId)mainland;

			pCB->setPushed(true);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHMainlandSelect, "mainland_select");


// ***************************************************************************
class CAHInitKeysetList : public IActionHandler
{
public:



	CInterfaceGroup *PrevLine;
	CInterfaceGroup *List;
	bool		First;

	CInterfaceGroup *buildTemplate(const std::string &templateName, const std::string &id)
	{
		vector< pair < string, string > > params;
		params.clear();
		params.push_back(pair<string,string>("id", id));
		if (!First)
		{
			params.push_back(pair<string,string>("posref", "BL TL"));
		}
		First = false;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		return CWidgetManager::getInstance()->getParser()->createGroupInstance(templateName, GROUP_LIST_KEYSET, params);
	}

	void addGroupInList(CInterfaceGroup *pNewLine)
	{
		if (!pNewLine) return;
		// Add to the list
		pNewLine->setParent(List);
		pNewLine->setParentSize(List);
		pNewLine->setParentPos(PrevLine);
		List->addGroup(pNewLine);

		PrevLine = pNewLine;
	}

	void addSeparator()
	{
		addGroupInList(buildTemplate("t_keyseparator", ""));
	}

	// add a new keyset in the list
	void addKeySet(const std::string &filename, const std::string &name, const std::string tooltip)
	{
		nlassert(List);
		CInterfaceGroup *pNewLine = buildTemplate("t_keyset", toString(filename));
		if (pNewLine != NULL)
		{
			CViewText *pVT = dynamic_cast<CViewText*>(pNewLine->getView("name"));
			if (pVT != NULL)
			{
				pVT->setTextLocalized(name, false);
			}

			CCtrlBase *pBut = pNewLine->getCtrl("but");
			if (pBut != NULL)
			{
				pBut->setDefaultContextHelp(tooltip);
			}
			addGroupInList(pNewLine);
		}
	}

	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		NewKeysCharNameWanted.clear();
		NewKeysCharNameValidated.clear();
		GameKeySet = "keys.xml";
		RingEditorKeySet = "keys_r2ed.xml";
		First = true;
		PrevLine = NULL;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		List = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_KEYSET));
		if (List == NULL)
		{
			nlwarning("element " GROUP_LIST_KEYSET " not found probably bad outgame.xml");
			return;
		}

		// built-in keysets
		CConfigFile::CVar *keySetVar = ClientCfg.ConfigFile.getVarPtr(KeySetVarName);
		sint wasdIndex = -1;
		sint zqsdIndex = -1;
		if (keySetVar && keySetVar->size() != 0)
		{
			for (uint k = 0; k < keySetVar->size(); ++k)
			{
				if (keySetVar->asString(k) == "zqsd") zqsdIndex = (sint) k;
				if (keySetVar->asString(k) == "wasd") wasdIndex = (sint) k;

				std::string strId = "uiCP_KeysetName_" + keySetVar->asString(k);
				strFindReplace(strId, ".", "_");
				const string &keySetName = CI18N::get(strId);
				strId = "uiCP_KeysetTooltip_" + keySetVar->asString(k);
				strFindReplace(strId, ".", "_");
				if (CI18N::hasTranslation(strId))
				{
					const string &keySetTooltip = CI18N::get(strId);
					addKeySet(keySetVar->asString(k), keySetName, keySetTooltip);
				}
			}
		}
		else
		{
			nlwarning("'%s' var not found in config file, or list is empty, proposing default keyset only", KeySetVarName);
			std::string defaultKeySet = "keys";
			const string &keySetName = CI18N::get("uiCP_KeysetName_" + defaultKeySet);
			const string &keySetTooltip = CI18N::get("uiCP_KeysetTooltip_" + defaultKeySet);
			addKeySet(defaultKeySet, keySetName, keySetTooltip);
		}

		// keyset from previous chars
		std::vector<std::string> savedFiles;
		CPath::getPathContent("save/", false, false, true, savedFiles);
		enum { GameKeys = 0x1, EditorKeys = 0x2 };
		typedef std::map<std::string, uint> TKeySetFileMap;
		TKeySetFileMap keySetFiles; // combination of 'GameKeys' & 'EditorKeys' flag for each character
		for (uint k = 0; k < savedFiles.size(); ++k)
		{
			if (testWildCard(CFile::getFilename(savedFiles[k]), "keys_*.xml"))
			{
				bool editorKeys = testWildCard(CFile::getFilename(savedFiles[k]), "keys_r2ed_*.xml");
				std::string baseName = CFile::getFilenameWithoutExtension(savedFiles[k]).substr(strlen(editorKeys ? "keys_r2ed_" : "keys_"));
				if(!keySetFiles.count(baseName)) keySetFiles[baseName] = 0;
				keySetFiles[baseName] |=  editorKeys ? EditorKeys : GameKeys;
			}
		}
		//
		bool separatorAdded = false;
		if (!keySetFiles.empty())
		{
			for(TKeySetFileMap::iterator it = keySetFiles.begin(); it != keySetFiles.end(); ++it)
			{
				string name;
				if (ClientCfg.Local)
				{
					name = it->first;
				}
				else
				{
					// search matching utf-8 string name from character summaries
					for (uint k = 0; k < CharacterSummaries.size(); ++k)
					{
						if (it->first == buildPlayerNameForSaveFile(CharacterSummaries[k].Name.toUtf8()))
						{
							name = CharacterSummaries[k].Name.toUtf8();
						}
					}
				}
				if (!name.empty())
				{
					if (!separatorAdded)
					{
						addSeparator();
						separatorAdded = true;
					}
					addKeySet(it->first, it->first, CI18N::get(std::string("uiCP_KeysetImport") + (it->second & GameKeys ? "_Game" : "")
																						  + (it->second & EditorKeys ? "_Editor" : "")));
				}
			}
		}

		// default to 'ZQSD' for French and Belgian keyboard, 'WASD' else
		bool wasd = !CSystemUtils::isAzertyKeyboard();

		/*sint startIndex = wasd ? wasdIndex : zqsdIndex;
		if (startIndex == -1) startIndex = 0;
		*/
		// TMP TMP : no way to have 2 keys for the same action for now -> default to 'arrows' setting.
		sint startIndex = 0;
		nlassert(startIndex >= 0);
		if (startIndex < (sint) List->getNumGroup())
		{
			CInterfaceGroup *gr = dynamic_cast<CInterfaceGroup *>(List->getGroup(startIndex));
			if (gr)
			{
				CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(gr->getCtrl("but"));
				if (pCB != NULL)
				{
					pCB->setPushed(true);
					CAHManager::getInstance()->runActionHandler (pCB->getActionOnLeftClick(), pCB, pCB->getParamsOnLeftClick());
				}
			}
		}
		List->invalidateCoords();
	}
};
REGISTER_ACTION_HANDLER (CAHInitKeysetList, "init_keyset_list");


// ***************************************************************************
class CAHResetKeysetList : public IActionHandler
{
public:

	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		//CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup *pList = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_KEYSET));
		pList->clearGroups();
	}
};
REGISTER_ACTION_HANDLER (CAHResetKeysetList, "reset_keyset_list");


// ***************************************************************************
class CAHResetKeysetSelect : public IActionHandler
{
public:
	std::string getIdPostFix(const std::string fullId)
	{
		std::string::size_type pos = fullId.find_last_of(":");
		if (pos != std::string::npos)
		{
			return fullId.substr(pos + 1);
		}
		return "";
	}
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		if (!pCaller) return;
		// 'unpush' all groups but the caller
		//
		struct CUnpush : public CInterfaceElementVisitor
		{
			CCtrlBase *Ref;
			virtual void visitCtrl(CCtrlBase *ctrl)
			{
				if (ctrl == Ref) return;
				CCtrlBaseButton *but = dynamic_cast<CCtrlBaseButton *>(ctrl);
				if (but)
				{
					but->setPushed(false);
				}
			}
		};
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup * list = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_KEYSET));
		if (list)
		{
			CUnpush unpusher;
			unpusher.Ref = pCaller;
			list->visit(&unpusher);
		}
		CCtrlBaseButton *but = dynamic_cast<CCtrlBaseButton *>(pCaller);
		if (but)
		{
			but->setPushed(true);
		}
		//
		GameKeySet = "keys.xml";
		RingEditorKeySet = "keys_r2ed.xml";
		if (!pCaller->getParent()) return;
		// compute the 2 filenames from the id
		// if id is in the built-in keysets :
		CConfigFile::CVar *keySetVar = ClientCfg.ConfigFile.getVarPtr(KeySetVarName);
		if (keySetVar && keySetVar->size() != 0)
		{
			for (uint k = 0; k < keySetVar->size(); ++k)
			{
				std::string id = getIdPostFix(pCaller->getParent()->getId());
				if (keySetVar->asString(k) == id)
				{
					GameKeySet = "keys" + string(id.empty() ? "" : "_") + id + ".xml";
					RingEditorKeySet = "keys_r2ed" + string(id.empty() ? "" : "_") + id + ".xml";
					return;
				}
			}
		}
		// ... else maybe from a previous character	?
		if (CFile::isExists("save/keys_" + getIdPostFix(pCaller->getParent()->getId()) + ".xml") )
		{
			GameKeySet = "keys_" + getIdPostFix(pCaller->getParent()->getId()) + ".xml";
		}
		if (CFile::isExists("save/keys_r2ed_" + getIdPostFix(pCaller->getParent()->getId()) + ".xml") )
		{
			RingEditorKeySet = "keys_r2ed_" + getIdPostFix(pCaller->getParent()->getId()) + ".xml";
		}
		// NB : key file will be copied for real when the new 'character summary' is

	}
};
REGISTER_ACTION_HANDLER (CAHResetKeysetSelect, "keyset_select");





// *************************** SCENARIO CONTROL WINDOW ***********************
// ***************************************************************************
// helper function for "setScenarioInformation"
static void setTextField(CInterfaceGroup* scenarioWnd, const std::string &uiName, const std::string &text)
{
	CInterfaceElement *result = scenarioWnd->findFromShortId(uiName);
	if(result)
	{
		CViewText* viewText = dynamic_cast<CViewText*>(result);
		if(viewText)
			viewText->setTextLocalized(text, false);
		CGroupEditBox* editBox = dynamic_cast<CGroupEditBox*>(result);
		if(editBox)
			editBox->setInputString(text);

	}
}
// helper function for "setScenarioInformation"
static void setTextField(CInterfaceGroup* scenarioWnd, const std::string &uiName, const ucstring &text) // TODO: UTF-8 Lua
{
	setTextField(scenarioWnd, uiName, text.toUtf8());
}
// helper function for "setScenarioInformation"
static std::string fieldLookup(const vector< pair< string, string > > &values, const std::string &id)
{
	for(uint i=0; i<values.size(); i++)
	{
		if (values[i].first == id) return values[i].second;
	}
	return "--";
}
//
static void setScenarioInformation(CInterfaceGroup* scenarioWnd, const string scenarioName)
{
	vector< pair< string, string > > values;
	if(R2::getEditor().isInitialized())
	{
		values = R2::getEditor().getDMC().getEditionModule().getScenarioHeader();
	}
	else
	{
		R2::CScenarioValidator sv;
		std::string md5, signature;
		sv.setScenarioToLoad(scenarioName, values, md5, signature, false);
	}
	//
	setTextField(scenarioWnd, "rules_value_text",			fieldLookup(values, "Rules"));
	uint levelRange = 0;
	uint32 nLevel;
	fromString(fieldLookup(values, "Level"), nLevel);
	switch(nLevel)
	{
		case 20:  levelRange = 0; break;
		case 50:  levelRange = 1; break;
		case 100: levelRange = 2; break;
		case 150: levelRange = 3; break;
		case 200: levelRange = 4; break;
		case 250: levelRange = 5; break;
	}
	setTextField(scenarioWnd, "level_value_text",			CI18N::get("uiRAP_Level" + toString(levelRange)));
	setTextField(scenarioWnd, "language_value_text",		CI18N::get("uiR2ED" + fieldLookup(values, "Language")));
	setTextField(scenarioWnd, "type_value_text",			CI18N::get("uiR2ED" + fieldLookup(values, "Type")));
	setTextField(scenarioWnd, "edit_small_description",		fieldLookup(values, "ShortDescription"));
	if(R2::getEditor().isInitialized())
	{
		setTextField(scenarioWnd, "scenario_value_text",		"'" + fieldLookup(values, "Title") + "'");
	}
}


void getChildrenControls(CInterfaceGroup* group, std::vector<CCtrlBase*> & controls)
{
	for(uint i=0; i<group->getGroups().size(); i++)
		getChildrenControls(group->getGroups()[i], controls);

	for(uint i=0; i<group->getControls().size(); i++)
		controls.push_back(group->getControls()[i]);
}

inline void setToggleButton(CInterfaceGroup* scenarioWnd, const string & buttonName, bool pushed)
{
	CInterfaceElement * result = scenarioWnd->findFromShortId(buttonName);
	if(result)
	{
		CInterfaceGroup * group = dynamic_cast<CInterfaceGroup*>(result);
		if(group)
		{
			result = group->findFromShortId(string("toggle_butt"));
			if(result)
			{
				CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
				if(baseButton)
					baseButton->setPushed(!pushed);
			}
		}
	}
}


class CAHScenarioControl : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHScenarioControl called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup* scenarioWnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_scenario_control"));
		if(!scenarioWnd) return;

		// -------- active some groups in function of Ryzom mode or Edition/Animation mode ----
		// active team toggle button?
		CInterfaceElement *result = scenarioWnd->findFromShortId(string("invite_team"));
		if(result)
		{
			CInterfaceGroup* groupTeam = dynamic_cast<CInterfaceGroup*>(result);
			if(groupTeam)
			{
				bool team = !(R2::getEditor().isInitialized());
				if(team)
					team = (NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:TEAM_MEMBER")->getValue8())!=0;
				groupTeam->setActive(team);
			}
		}

		// set scenario name label
		result = scenarioWnd->findFromShortId(string("current_scenario_label_text"));
		if(result)
		{
			CViewText* viewText = dynamic_cast<CViewText*>(result);
			if(viewText)
			{
				viewText->setTextLocalized(R2::getEditor().isInitialized() ? "uiR2EDScenarioName" : "uiR2EDScenarioFileName", true);
			}
		}

		// ok button tranlation
		result = scenarioWnd->findFromShortId(string("ok_button"));
		if(result)
		{
			CCtrlTextButton* okButton = dynamic_cast<CCtrlTextButton*>(result);
			if(okButton)
			{
				if(R2::getEditor().getAccessMode()!=R2::CEditor::AccessDM)
					okButton->setHardText(CI18N::get("uiR2EDLaunchScenario"));
				else
					okButton->setHardText(CI18N::get("uiR2EDApplyScenarioFilters"));
			}
		}

		// init current scenario name and parameters
		if(!R2::getEditor().isInitialized())
		{
			ScenarioFileName.clear();

			// empty scenario
			CInterfaceElement *result = scenarioWnd->findFromShortId(string("scenario_value_text"));
			if(result)
			{
				CViewText* viewText= dynamic_cast<CViewText*>(result);

				if(viewText)
					viewText->setText(std::string());
			}
		}
		setScenarioInformation(scenarioWnd, "");

		// hide description and information?
		result = scenarioWnd->findFromShortId(string("scenario_info_prop"));
		if(result)
			result->setActive(R2::getEditor().isInitialized());

		result = scenarioWnd->findFromShortId(string("description_gr"));
		if(result)
			result->setActive(R2::getEditor().isInitialized());

		// mainlands list
		result = scenarioWnd->findFromShortId(string("shards"));
		if(result)
		{
			CGroupList * shardList = dynamic_cast<CGroupList*>(result);
			if(shardList)
			{
				shardList->deleteAllChildren();

				for(uint i = 0; i < Mainlands.size(); i++)
				{
					vector< pair < string, string > > params;
					params.clear();
					params.push_back(pair<string,string>("id", toString(Mainlands[i].Id)));
					params.push_back(pair<string,string>("w", "1024"));
					params.push_back(pair<string,string>("tooltip", "uiRingFilterShard"));
					CInterfaceGroup *toggleGr = CWidgetManager::getInstance()->getParser()->createGroupInstance("label_toggle_button", shardList->getId(), params);
					shardList->addChild(toggleGr);
					// set unicode name
					CViewText *shardName = dynamic_cast<CViewText *>(toggleGr->getView("button_text"));
					if (shardName)
					{
						shardName->setTextLocalized(Mainlands[i].Name.toUtf8(), false);
					}
				}
			}
		}

		// show/display "back" button
		result = scenarioWnd->findFromShortId(string("load_button"));
		if(result)
		{
			CCtrlBaseButton * loadB = dynamic_cast<CCtrlBaseButton *>(result);
			if(loadB)
			{
				loadB->setActive(!R2::getEditor().isInitialized());
			}
		}

		// fill toggle buttons
		if(R2::getEditor().getAccessMode()==R2::CEditor::AccessDM)
		{
			CSessionBrowserImpl & sessionBrowser = CSessionBrowserImpl::getInstance();
			sessionBrowser.getSessionInfo(sessionBrowser.getCharId(), R2::getEditor().getDMC().getEditionModule().getCurrentAdventureId());

			if(sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_sessionInfoResult")))
			{
				TRaceFilter & raceFilter = sessionBrowser._LastRaceFilter;
				setToggleButton(scenarioWnd, "fyros", raceFilter.checkEnumValue(TRaceFilterEnum::rf_fyros));
				setToggleButton(scenarioWnd, "matis", raceFilter.checkEnumValue(TRaceFilterEnum::rf_matis));
				setToggleButton(scenarioWnd, "tryker", raceFilter.checkEnumValue(TRaceFilterEnum::rf_tryker));
				setToggleButton(scenarioWnd, "zorai", raceFilter.checkEnumValue(TRaceFilterEnum::rf_zorai));

				TReligionFilter & religionFilter = sessionBrowser._LastReligionFilter;
				setToggleButton(scenarioWnd, "kami", religionFilter.checkEnumValue(TReligionFilterEnum::rf_kami));
				setToggleButton(scenarioWnd, "karavan", religionFilter.checkEnumValue(TReligionFilterEnum::rf_karavan));
				setToggleButton(scenarioWnd, "neutral", religionFilter.checkEnumValue(TReligionFilterEnum::rf_neutral));

				TGuildFilter & guildFilter = sessionBrowser._LastGuildFilter;
				setToggleButton(scenarioWnd, "guild_gr", (guildFilter==TGuildFilter::gf_any_player));

				TShardFilter & shardFilter = sessionBrowser._LastShardFilter;
				for(uint i=0; i<Mainlands.size(); i++)
					setToggleButton(scenarioWnd, toString(Mainlands[i].Id), shardFilter.checkEnumValue((RSMGR::TShardFilterEnum::TValues) (1<<i)));

				TLevelFilter & levelFilter = sessionBrowser._LastLevelFilter;
				setToggleButton(scenarioWnd, "20", levelFilter.checkEnumValue(TLevelFilterEnum::lf_a));
				setToggleButton(scenarioWnd, "50", levelFilter.checkEnumValue(TLevelFilterEnum::lf_b));
				setToggleButton(scenarioWnd, "100", levelFilter.checkEnumValue(TLevelFilterEnum::lf_c));
				setToggleButton(scenarioWnd, "150", levelFilter.checkEnumValue(TLevelFilterEnum::lf_d));
				setToggleButton(scenarioWnd, "200", levelFilter.checkEnumValue(TLevelFilterEnum::lf_e));
				setToggleButton(scenarioWnd, "250", levelFilter.checkEnumValue(TLevelFilterEnum::lf_f));

				bool subscriptionClosed = sessionBrowser._LastSubscriptionClosed;
				result = scenarioWnd->findFromShortId(string("global_access_toggle_butt"));
				if(result)
				{
					CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
					if(baseButton)
						baseButton->setPushed(subscriptionClosed);
				}

				bool autoInvite = sessionBrowser._LastAutoInvite;
				result = scenarioWnd->findFromShortId(string("auto_invite_toggle_butt"));
				if(result)
				{
					CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
					if(baseButton)
						baseButton->setPushed(!autoInvite);
				}

				// description
				string description = sessionBrowser._LastDescription;
				if(!description.empty())
				{
					result = scenarioWnd->findFromShortId(string("edit_small_description"));
					if(result)
					{
						CGroupEditBox* editBox = dynamic_cast<CGroupEditBox*>(result);
						if(editBox)
							editBox->setInputString(description);
					}
				}
			}
			else
			{
				nlwarning("getSessionInfo callback return false");
			}
		}
		else
		{
			result = scenarioWnd->findFromShortId(string("access_players_filter"));
			if(result)
			{
				CInterfaceGroup* filtersGroup = dynamic_cast<CInterfaceGroup*>(result);
				if(filtersGroup)
				{
					std::vector<CCtrlBase*> controls;
					getChildrenControls(filtersGroup, controls);
					for(uint i=0; i<controls.size(); i++)
					{
						CCtrlBase* control = controls[i];
						CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(control);
						if(baseButton && (baseButton->getType()==CCtrlBaseButton::ToggleButton))
							baseButton->setPushed(false);
					}
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHScenarioControl, "init_scenario_control");


// ***************************************************************************
class CAHScenarioInformation : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		nlinfo("CAHScenarioDescription called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup* scenarioWnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_scenario_control"));
		if(!scenarioWnd) return;

		CInterfaceElement *result = scenarioWnd->findFromShortId(string("scenario_value_text"));
		if(result)
		{
			CViewText* viewText= dynamic_cast<CViewText*>(result);

			if(viewText)
			{
				ScenarioFileName = getParam(Params, "ScenarioName");
				setScenarioInformation(scenarioWnd, ScenarioFileName);

				string scenarioName = ScenarioFileName;
				string::size_type posScenarioName = 0;
				while(posScenarioName!=string::npos)
				{
					scenarioName = scenarioName.substr(posScenarioName==0?posScenarioName:posScenarioName+1);
					posScenarioName = scenarioName.find('/');
				}
				viewText->setTextLocalized(scenarioName, false);
			}
		}

		// active description and information
		result = scenarioWnd->findFromShortId(string("scenario_info_prop"));
		if(result)
			result->setActive(true);

		result = scenarioWnd->findFromShortId(string("description_gr"));
		if(result)
			result->setActive(true);
	}
};
REGISTER_ACTION_HANDLER (CAHScenarioInformation, "scenario_information");

// ***************************************************************************
class CAHHideCharsFilters : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		nlinfo("CAHHideCharsFilters called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup* scenarioWnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_scenario_control"));
		if(!scenarioWnd) return;

		bool lookingForPlayers = true;
		CInterfaceElement *result = scenarioWnd->findFromShortId(string("global_access_toggle_butt"));
		if(result)
		{
			CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
			if(baseButton)
				lookingForPlayers = !baseButton->getPushed(); // warning : on / off textures are inverted !!!
		}

		result = scenarioWnd->findFromShortId(string("access_body_gr"));
		if(result)
			result->setActive(lookingForPlayers);

		result = scenarioWnd->findFromShortId(string("sep_global_access"));
		if(result)
			result->setActive(lookingForPlayers);

		result = scenarioWnd->findFromShortId(string("auto_invite_label"));
		if(result)
			result->setActive(lookingForPlayers);

		result = scenarioWnd->findFromShortId(string("auto_invite_toggle_butt"));
		if(result)
			result->setActive(lookingForPlayers);

		result = scenarioWnd->findFromShortId(string("invite_team"));
		if(result)
		{
			bool team = (NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:TEAM_MEMBER")->getValue8())!=0;
			team = (team && !(R2::getEditor().isInitialized()) && lookingForPlayers);
			result->setActive(team);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHHideCharsFilters, "hide_chars_filters");

// ***************************************************************************
class CAHLoadScenario : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		nlinfo("CAHLoadScenario called");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup* scenarioWnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_scenario_control"));
		if(!scenarioWnd) return;

		CInterfaceElement *result = NULL;

		// load scenario
		if(!R2::getEditor().isInitialized())
		{
			R2::CEditor::setStartingAnimationFilename(ScenarioFileName);
		}

		// description
		string description;
		result = scenarioWnd->findFromShortId(string("edit_small_description"));
		if(result)
		{
			CGroupEditBox* editBox = dynamic_cast<CGroupEditBox*>(result);
			if(editBox)
				description = editBox->getInputString();
		}

		// races
		map<string, bool> races;
		races["fyros"] =  false;
		races["matis"] =  false;
		races["tryker"] =  false;
		races["zorai"] =  false;
		for(map<string, bool>::iterator itRace=races.begin(); itRace!=races.end(); itRace++)
		{
			result = scenarioWnd->findFromShortId(itRace->first);
			if(result)
			{
				CInterfaceGroup * group = dynamic_cast<CInterfaceGroup*>(result);
				if(group)
				{
					result = group->findFromShortId(string("toggle_butt"));
					if(result)
					{
						CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
						if(baseButton)
							itRace->second = !baseButton->getPushed();
					}
				}
			}
		}

		// religion
		map<string, bool> religions;
		religions["kami"] =  false;
		religions["karavan"] =  false;
		religions["neutral"] =  false;
		for(map<string, bool>::iterator itReligion=religions.begin(); itReligion!=religions.end(); itReligion++)
		{
			result = scenarioWnd->findFromShortId(itReligion->first);
			if(result)
			{
				CInterfaceGroup * group = dynamic_cast<CInterfaceGroup*>(result);
				if(group)
				{
					result = group->findFromShortId(string("toggle_butt"));
					if(result)
					{
						CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
						if(baseButton)
							itReligion->second = !baseButton->getPushed();
					}
				}
			}
		}

		// guild
		bool anyPlayer = false;
		result = scenarioWnd->findFromShortId(string("guild_gr"));
		if(result)
		{
			CInterfaceGroup * group = dynamic_cast<CInterfaceGroup*>(result);
			if(group)
			{
				result = group->findFromShortId(string("toggle_butt"));
				if(result)
				{
					CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
					if(baseButton)
						anyPlayer = !baseButton->getPushed();
				}
			}
		}

		// shards
		std::vector<bool>  shards(Mainlands.size(), false);
		for(uint i=0; i<Mainlands.size(); i++)
		{
			string firstKey = Mainlands[i].Description.toString();

			result = scenarioWnd->findFromShortId(toString(Mainlands[i].Id));
			if(result)
			{
				CInterfaceGroup * group = dynamic_cast<CInterfaceGroup*>(result);
				if(group)
				{
					result = group->findFromShortId(string("toggle_butt"));
					if(result)
					{
						CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
						if(baseButton)
							shards[i] = !baseButton->getPushed();
					}
				}
			}
		}

		// levels
		map<string, bool>  levels;
		levels["20"] = false;
		levels["50"] = false;
		levels["100"] = false;
		levels["150"] = false;
		levels["200"] = false;
		levels["250"] = false;
		for(map<string, bool>::iterator itLevel=levels.begin(); itLevel!=levels.end(); itLevel++)
		{
			result = scenarioWnd->findFromShortId(itLevel->first);
			if(result)
			{
				CInterfaceGroup * group = dynamic_cast<CInterfaceGroup*>(result);
				if(group)
				{
					result = group->findFromShortId(string("toggle_butt"));
					if(result)
					{
						CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
						if(baseButton)
							itLevel->second = !baseButton->getPushed();
					}
				}
			}
		}

		// global access
		bool globalAccess = false;
		result = scenarioWnd->findFromShortId(string("global_access_toggle_butt"));
		if(result)
		{
			CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
			if(baseButton)
				globalAccess = !baseButton->getPushed();
		}

		// auto invite
		bool autoInvite = false;
		result = scenarioWnd->findFromShortId(string("auto_invite_toggle_butt"));
		if(result)
		{
			CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
			if(baseButton)
				autoInvite = !baseButton->getPushed();
		}

		// invite your team
		bool inviteTeam = false;
		result = scenarioWnd->findFromShortId(string("team_toggle_butt"));
		if(result)
		{
			CCtrlBaseButton * baseButton = dynamic_cast<CCtrlBaseButton*>(result);
			if(baseButton)
				inviteTeam = !baseButton->getPushed();
		}

		bool launchScenarioFromRingAccessPoint = false;

		vector< pair< string, string > > values;
		if(R2::getEditor().isInitialized())
		{
			values = R2::getEditor().getDMC().getEditionModule().getScenarioHeader();
		}
		else
		{
			R2::CScenarioValidator sv;
			std::string md5, signature;
			sv.setScenarioToLoad(ScenarioFileName, values, md5, signature, false);
			launchScenarioFromRingAccessPoint = true;
		}

		string rules, level, title;
		string initialIsland, initialEntryPoint, initialSeason;
		std::string lang, scenarioType;
		std::string otherCharAccess;
		std::string nevraxScenario = "0";
		std::string trialAllowed = "0";
		for(uint i=0; i<values.size(); i++)
		{
			std::pair<std::string, std::string> pair = values[i];

			if(pair.first == "Rules") rules = pair.second;
			else if(pair.first == "Level") level = pair.second;
			else if(pair.first == "Title") title = pair.second;
			else if(pair.first == "InitialIsland")	initialIsland = pair.second;
			else if(pair.first == "InitialEntryPoint")	initialEntryPoint = pair.second;
			else if(pair.first == "InitialSeason")	initialSeason = pair.second;
			else if(pair.first == "Language")	lang = pair.second;
			else if(pair.first == "Type")	scenarioType = pair.second;
			else if(pair.first == "OtherCharAccess")	otherCharAccess = pair.second;
			else if(pair.first == "NevraxScenario")	nevraxScenario = pair.second;
			else if(pair.first == "TrialAllowed")	trialAllowed = pair.second;
		}

		uint nLevel;
		fromString(level, nLevel);
		R2::TSessionLevel sessionLevel = R2::TSessionLevel::TValues(nLevel/50 + 1);

		// ---- fix for old scenarii
		if (lang == "French")
			lang = "fr";
		else if (lang == "German" || lang == "Deutsch")
			lang = "de";
		else //if (lang == "English")
			lang = "en";

		if (nlstricmp(scenarioType, "Roleplay") == 0 || nlstricmp(scenarioType, "Role play") == 0)
			scenarioType = "so_story_telling";
		else if (nlstricmp(scenarioType, "Combat") == 0)
			scenarioType = "so_hack_slash";
		// --------------------------

		TRuleType ruleType(TRuleType::rt_strict);
		if(rules==CI18N::get("uiR2EDliberal"))
			ruleType = TRuleType(TRuleType::rt_liberal);
		else if(rules == CI18N::get("uiR2EDstrict"))
			ruleType = TRuleType(TRuleType::rt_strict);
		volatile static bool override = false;
		if (override)
		{
			if(rules== "Masterless")
				ruleType = TRuleType(TRuleType::rt_liberal);
			else if(rules == "Mastered")
				ruleType = TRuleType(TRuleType::rt_strict);
		}

		TRaceFilter raceFilter;
		if(races["fyros"])
			raceFilter.setEnumValue(TRaceFilterEnum::rf_fyros);
		if(races["matis"])
			raceFilter.setEnumValue(TRaceFilterEnum::rf_matis);
		if(races["tryker"])
			raceFilter.setEnumValue(TRaceFilterEnum::rf_tryker);
		if(races["zorai"])
			raceFilter.setEnumValue(TRaceFilterEnum::rf_zorai);

		TReligionFilter religionFilter;
		if(religions["kami"])
			religionFilter.setEnumValue(TReligionFilterEnum::rf_kami);
		if(religions["karavan"])
			religionFilter.setEnumValue(TReligionFilterEnum::rf_karavan);
		if(religions["neutral"])
			religionFilter.setEnumValue(TReligionFilterEnum::rf_neutral);

		TGuildFilter guildFilter(anyPlayer?TGuildFilter::gf_any_player:TGuildFilter::gf_only_my_guild);

		TShardFilter shardFilter;
		for (uint i = 0; i < shards.size(); ++i)
		{
			if (shards[i]) shardFilter.setEnumValue((RSMGR::TShardFilterEnum::TValues) (1<<i));
		}

		TLevelFilter levelFilter;
		if(levels["20"])
			levelFilter.setEnumValue(TLevelFilterEnum::lf_a);
		if(levels["50"])
			levelFilter.setEnumValue(TLevelFilterEnum::lf_b);
		if(levels["100"])
			levelFilter.setEnumValue(TLevelFilterEnum::lf_c);
		if(levels["150"])
			levelFilter.setEnumValue(TLevelFilterEnum::lf_d);
		if(levels["200"])
			levelFilter.setEnumValue(TLevelFilterEnum::lf_e);
		if(levels["250"])
			levelFilter.setEnumValue(TLevelFilterEnum::lf_f);

		uint32 charId = 0;
		if (!ClientCfg.Local)
			charId = (NetMngr.getLoginCookie().getUserId()<< 4) + (uint32) PlayerSelectedSlot;

		CSessionBrowserImpl & sessionBrowser = CSessionBrowserImpl::getInstance();

		if(R2::getEditor().getAccessMode() != R2::CEditor::AccessDM)
		{
			bool noob = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:IS_NEWBIE")->getValueBool();
			if (FreeTrial && noob && (nevraxScenario != "1" || trialAllowed != "1"))
			{
				CViewText* pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:warning_free_trial:text"));
				if (pVT != NULL)
					pVT->setTextLocalized("uiRingWarningFreeTrial", true);
				CAHManager::getInstance()->runActionHandler("enter_modal", pCaller, "group=ui:interface:warning_free_trial");

				return;
			}
		}


		if(R2::getEditor().getAccessMode()!=R2::CEditor::AccessDM)
		{
			if (launchScenarioFromRingAccessPoint)
			{
				// hibernate Edit Session if active
				sessionBrowser.hibernateEditSession(charId);
				if(!sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_invokeResult")))
				{
					nlwarning("hibernateEditSession callback return false");
				}

			}

			// schedule session
			bool launchSuccess = true;
			sessionBrowser.scheduleSession(charId, TSessionType::st_anim,
				title, description, sessionLevel,
				/*TAccessType::at_public,*/ ruleType, TEstimatedDuration::et_medium, 0, TAnimMode::am_dm,
				raceFilter, religionFilter, guildFilter, shardFilter, levelFilter, lang, RSMGR::TSessionOrientation(scenarioType),
				!globalAccess, autoInvite);

			if(sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_scheduleSessionResult")))
			{
				if(sessionBrowser._LastScheduleSessionResult==0)
				{
					// start session
					sessionBrowser.startSession(sessionBrowser._LastScheduleSessionCharId,
						sessionBrowser._LastScheduleSessionId);

					if(sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_invokeResult")))
					{

						if (launchScenarioFromRingAccessPoint)
						{
							if (!initialIsland.empty() && !initialEntryPoint.empty() && !initialSeason.empty())
							{
								sessionBrowser.setSessionStartParams(charId, sessionBrowser._LastScheduleSessionId, initialIsland, initialEntryPoint, initialSeason);
							}
						}

						TSessionPartStatus sessionStatus;
						if(ruleType==TRuleType::rt_liberal)
							sessionStatus = TSessionPartStatus(TSessionPartStatus::sps_play_invited);
						else
							sessionStatus = TSessionPartStatus(TSessionPartStatus::sps_anim_invited);

						// invite player
						sessionBrowser.inviteCharacter(
							sessionBrowser._LastScheduleSessionCharId,
							sessionBrowser._LastScheduleSessionId,
							sessionBrowser._LastScheduleSessionCharId,
							sessionStatus.toString());

						if(sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_invokeResult")))
						{
							// request session
							FarTP.requestFarTPToSession(sessionBrowser._LastScheduleSessionId, PlayerSelectedSlot, CFarTP::JoinSession,
								!R2::getEditor().isInitialized());
						}
						else
						{
							nlwarning("inviteCharacter callback return false");
						}

						if (sessionBrowser._LastInvokeResult != 0)
						{
							nlwarning("inviteCharacter callback use error values %d", sessionBrowser._LastInvokeResult);
						}

						if(sessionBrowser._LastInvokeResult == 14)
						{
							CViewText* pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:warning_free_trial:text"));
							if (pVT != NULL)
								pVT->setTextLocalized("uiRingWarningFreeTrial", true);
							CAHManager::getInstance()->runActionHandler("enter_modal", pCaller, "group=ui:interface:warning_free_trial");
						}


						// invite team
						if(inviteTeam)
						{
							for (uint i = 0 ; i < 8 ; ++i)
							{
								uint32 val = NLGUI::CDBManager::getInstance()->getDbProp(NLMISC::toString("SERVER:GROUP:%d:NAME",i))->getValue32();
								if(val!=0)
								{
									STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
									string res;
									if (pSMC->getString(val,res))
									{
										string charName = CEntityCL::removeTitleAndShardFromName(res);
										sessionBrowser.inviteCharacterByName(sessionBrowser._LastScheduleSessionCharId, charName);

										if(!sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_invokeResult")))
										{
											nlwarning("inviteCharacterByName callback return false");
										}

										if(sessionBrowser._LastInvokeResult == 14)
										{
											CViewText* pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:warning_free_trial:text"));
											if (pVT != NULL)
												pVT->setTextLocalized("uiRingWarningInviteFreeTrial", true);
											CAHManager::getInstance()->runActionHandler("enter_modal", pCaller, "group=ui:interface:warning_free_trial");
										}
									}
								}
							}
						}
					}
					else
					{
						nlwarning("startSession callback return false");
						launchSuccess = false;
					}
				}
				else if(sessionBrowser._LastScheduleSessionResult==10)
				{
					pIM->messageBoxWithHelp(CI18N::get("uiRingWarningBanishedPlayer"));
				}
				else
				{
					launchSuccess=false;
				}
			}
			else
			{
				nlwarning("scheduleSession callback return false");
				launchSuccess = false;
			}

			if(!launchSuccess)
			{
				pIM->messageBoxWithHelp(CI18N::get("uiRingLaunchScenarioError"));
			}
			else
			{
				scenarioWnd->setActive(false);
			}
		}
		else
		{
			// update session
			sessionBrowser.updateSessionInfo(charId, sessionBrowser._LastScheduleSessionId, title, 0, description, sessionLevel,
				/*TAccessType::at_public, */TEstimatedDuration::et_medium, 0, raceFilter, religionFilter,
				guildFilter, shardFilter, levelFilter, !globalAccess, autoInvite, lang, RSMGR::TSessionOrientation(scenarioType));

			if(!sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_invokeResult")))
			{
				nlwarning("updateSessionInfo callback return false");
				pIM->messageBoxWithHelp(CI18N::get("uiRingUpdateScenarioFiltersError"));
			}
			else
			{
				scenarioWnd->setActive(false);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHLoadScenario, "load_scenario");


// ***************************************************************************
class CAHOpenRingSessions : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if(!R2::getEditor().isInitialized())
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			CInterfaceGroup* ringSessionsWnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:ring_sessions"));
			if(!ringSessionsWnd) return;
			ringSessionsWnd->setActive(true);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHOpenRingSessions, "open_ring_sessions");

// ***************************************************************************
class CAHInitImportCharacter : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceGroup *list = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_CHARACTER));
		if (!list)
		{
			nlwarning("element " GROUP_LIST_CHARACTER " not found probably bad outgame.xml");
			return;
		}

		// retrieve saved files
		std::vector<string> savedCharacters;
		CPath::getPathContent("save/", false, false, true, savedCharacters);

		CInterfaceGroup *newLine;
		CInterfaceGroup *prevLine = NULL;

		for (uint i = 0; i < savedCharacters.size(); ++i)
		{
			// search saved characters only
			if (testWildCard(CFile::getFilename(savedCharacters[i]), "character_*.save"))
			{
				const std::string id = CFile::getFilenameWithoutExtension(savedCharacters[i]).substr(strlen("character_"));
				if (id.empty())
					continue;

				std::vector<pair<string, string>> params;
				params.clear();
				params.push_back(std::pair<string, string>("id", id));
				// adjust ref
				if (list->getNumGroup() > 0)
					params.push_back(std::pair<string, string>("posref", "BL TL"));

				newLine = CWidgetManager::getInstance()->getParser()->createGroupInstance("t_import", GROUP_LIST_CHARACTER, params);
				if (newLine)
				{
					CViewText *text = dynamic_cast<CViewText*>(newLine->getView("name"));
					if (text)
						text->setText(string(savedCharacters[i]));

					// first button is pushed
					CCtrlButton *button = dynamic_cast<CCtrlButton*>(newLine->getCtrl("but"));
					if (button && list->getNumGroup() == 0)
						button->setPushed(true);

					// add to the list now
					newLine->setParent(list);
					newLine->setParentSize(list);
					newLine->setParentPos(prevLine);

					list->addGroup(newLine);

					prevLine = newLine;
				}
			}
		}
		// none case
		if (list->getNumGroup() == 0)
			CLuaManager::getInstance().executeLuaScript("outgame:procCharselNotifaction(3)");

		list->invalidateCoords();
	}
};
REGISTER_ACTION_HANDLER( CAHInitImportCharacter, "import_char_init" );

// ***************************************************************************
class CAHResetImportCharacter : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceGroup *list = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_CHARACTER));
		if (list)
			list->clearGroups();

		if (!ImportCharacter.empty())
			ImportCharacter = "";
	}
};
REGISTER_ACTION_HANDLER( CAHResetImportCharacter, "import_char_reset" );

// ***************************************************************************
class CAHSelectImportCharacter : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const std::string &Params)
	{
		struct CUnpush : public CInterfaceElementVisitor
		{
			CCtrlBase *Ref;
			virtual void visitCtrl(CCtrlBase *ctrl)
			{
				if (ctrl == Ref) return;
				CCtrlBaseButton *but = dynamic_cast<CCtrlBaseButton*>(ctrl);
				if (but)
				{
					but->setPushed(false);
				}
			}
		};
		CInterfaceGroup *list = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(GROUP_LIST_CHARACTER));
		if (!list)
			return;

		// unselect
		if (Params.empty())
		{
			CUnpush unpusher;
			unpusher.Ref = pCaller;
			list->visit(&unpusher);
		}

		// now select
		std::string name;
		if (Params.empty())
		{
			CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(pCaller);
			if (!pCB)
				return;

			std::string id = pCB->getId();
			id = id.substr(0, id.rfind(':'));

			if (!fromString(id.substr(id.rfind(':')+1, id.size()), name))
				return;

			pCB->setPushed(true);
		}
		else
			if (!fromString(Params, name))
				return;

		ImportCharacter = "";
		// check filename and store
		if (CFile::fileExists(toString("save/character_%s.save", name.c_str())))
			ImportCharacter = name;
	}
};
REGISTER_ACTION_HANDLER( CAHSelectImportCharacter, "import_char_select" );

// ***************************************************************************
class CAHImportCharacter : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (ImportCharacter.empty())
			return;

		if (!CFile::fileExists(toString("save/character_%s.save", ImportCharacter.c_str())))
			return;

		bool success = false;

		CIFile fd;
		CCharacterSummary CS;
		// use temporary file until close()
		if (fd.open(toString("save/character_%s.save", ImportCharacter.c_str())))
		{
			try
			{
				CS.serial(fd);
				SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", CS);

				// validate import
				CDBManager::getInstance()->getDbProp("UI:TEMP:IMPORT")->setValue32(1);
				success = true;
			}
			catch (const EStream &e)
			{
				nlwarning(e.what());
			}
			fd.close();
		}
		else
			nlwarning("Failed to open file: save/character_%s.save", ImportCharacter.c_str());

		// user notification
		if (!success)
			CLuaManager::getInstance().executeLuaScript("outgame:procCharselNotifaction(2)");
		else
			CAHManager::getInstance()->runActionHandler("proc", NULL, "proc_charsel_create_new");
	}
};
REGISTER_ACTION_HANDLER( CAHImportCharacter, "import_char" );

// ***************************************************************************
class CAHExportCharacter : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &Params)
	{
		if (Params.empty())
			return;

		sint32 slot = -1;
		if (!fromString(getParam(Params, "slot"), slot))
			return;

		if (slot >= CharacterSummaries.size() || slot < 0)
			return;

		// retrieve infos
		CCharacterSummary &CS = CharacterSummaries[slot];
		if (CS.Name.empty())
			return;

		// extract name
		const std::string name = buildPlayerNameForSaveFile(CS.Name.toUtf8());

		COFile fd;
		bool success = false;
		// use temporary file until close()
		if (fd.open(toString("save/character_%s.save", name.c_str()), false, false, true))
		{
			try
			{
				fd.serial(CS);
				fd.flush();
				// validate
				success = true;
			}
			catch (const EStream &e)
			{
				nlwarning(e.what());
			}
			fd.close();
		}
		else
			nlwarning("Failed to open file: save/character_%s.save", name.c_str());

		const uint8 val = (success == true) ? 0 : 1;
		// user notification
		CLuaManager::getInstance().executeLuaScript(toString("outgame:procCharselNotifaction(%i)", val));
	}
};
REGISTER_ACTION_HANDLER( CAHExportCharacter, "export_char" );
