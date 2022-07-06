// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2015-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//////////////
// INCLUDES //
//////////////
// Misc.
#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#include <shellapi.h>
#else
#include <csignal>
#endif

#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/net/tcp_sock.h"
#include "nel/misc/cmd_args.h"

//#define TEST_CRASH_COUNTER
#ifdef TEST_CRASH_COUNTER
 #undef FINAL_VERSION
 #define FINAL_VERSION 1
#endif // TEST_CRASH_COUNTER

// Client
#include "resource.h"
#include "init.h"
#include "login.h"
#include "login_patch.h"
#include "connection.h"
#include "init_main_loop.h"
#include "main_loop.h"
#include "release.h"
#include "client_cfg.h"
#include "far_tp.h"
#include "user_agent.h"

#ifdef RZ_USE_STEAM
#include "steam_client.h"
#endif

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;

//
// Macros
//

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

//
// RYZOM_TRY and RYZOM_CATCH aim is to catch differently in dev and final version
//    In final version, we catch everything and nlerror the problem to display a NeL message box
//    In dev version, we just catch EFatalError() and we leave the OS to catch the exception to enable us to cancel/debug it
//

// We don't catch(...) because these exception are already trapped with the se_translation that generate the NeL message box

#define RYZOM_TRY(_block) try { nlinfo(_block" of Ryzom...");
#define RYZOM_CATCH(_block) nlinfo(_block" of Ryzom success"); } catch(const EFatalError &) { return EXIT_FAILURE; }

/////////////
// GLOBALS //
/////////////
static uint32 Version = 1;	// Client Version.

string	Cookie;
string	FSAddr;


///////////////
// FUNCTIONS //
///////////////

static CTcpSock CrashCounterSock;

void quitCrashReport ()
{
	//if (NLMISC::CFile::fileExists(getLogDirectory() + "ryzom_started"))
		//CFile::deleteFile (getLogDirectory() + "ryzom_started");
	// must disconnect now, else could crash at dtor time because nldebug -> access a new INelContext()
	contReset(CrashCounterSock);
}

// make it global if other classes/functions want to access to it
NLMISC::CCmdArgs Args;

//---------------------------------------------------
// MAIN :
// Entry for the Application.
//---------------------------------------------------
#ifdef NL_OS_WINDOWS

// enable optimus for NVIDIA cards
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

void pump ()
{
	// Display the window
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

INT_PTR CALLBACK MyDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	return FALSE;
}

HWND SlashScreen = NULL;
HINSTANCE HInstance;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE /* hPrevInstance */, LPSTR cmdline, int /* nCmdShow */)
#else
int main(int argc, char **argv)
#endif
{
#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// init the Nel context
	CApplicationContext *appContext = new CApplicationContext;

	// disable nldebug messages in logs in FV
#if FINAL_VERSION && defined(NL_RELEASE)
	DisableNLDebug = true;
#endif

	// don't create log.log anymore because client.log is used
	createDebug(NULL, false);

	INelContext::getInstance().setWindowedApplication(true);

#ifndef NL_DEBUG
	INelContext::getInstance().getDebugLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getInfoLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getWarningLog()->removeDisplayer("DEFAULT_SD");
#endif // NL_DEBUG

	Args.setVersion(getDisplayVersion());
	Args.setDescription("Ryzom client");
	Args.addArg("n", "nopatch", "value", "Use this to not use patch system");
	Args.addArg("p", "profile", "id", "Use this profile to determine what directory to use by default");
	Args.addAdditionalArg("login", "Login to use", true, false);
	Args.addAdditionalArg("password", "Password to use", true, false);
	Args.addAdditionalArg("shard_id", "Shard ID to use", true, false);
	Args.addAdditionalArg("slot", "Char slot to use", true, false);

#ifdef TEST_CRASH_COUNTER
	Args.addArg("", "crash", "", "Crash client before init");
	Args.addArg("", "break", "", "Create a break point");
	Args.addArg("", "release", "", "Crash client after init");
#endif // TEST_CRASH_COUNTER

	// extract the 2 or 3 first param (argv[1], argv[2] and argv[3]) it must be <login> <password> [shardId]
#ifdef NL_OS_WINDOWS
	if (!Args.parse(cmdline)) return 1;
#else
	if (!Args.parse(argc, argv)) return 1;
#endif

	// no shard id in ring mode
	std::string sLoginShardId;

	if (Args.haveAdditionalArg("login"))
	{
		LoginLogin = Args.getAdditionalArg("login").front();

		if (Args.haveAdditionalArg("password"))
		{
			LoginPassword = Args.getAdditionalArg("password").front();

			// password in hexadecimal
			if (LoginPassword.compare(0, 2, "0x") == 0)
			{
				std::string decodedPassword;

				// decode password
				if (fromHexa(LoginPassword.substr(2), decodedPassword))
				{
					// only use it if real hexadecimal
					LoginPassword = decodedPassword;
				}
			}

			if (Args.haveAdditionalArg("shard_id"))
				sLoginShardId = Args.getAdditionalArg("shard_id").front();

			if (Args.haveAdditionalArg("slot"))
			{
				if (!fromString(Args.getAdditionalArg("slot").front(), LoginCharsel))
					LoginCharsel = -1;
			}
		}
	}

	if (sLoginShardId.empty() || !fromString(sLoginShardId, LoginShardId))
		LoginShardId = std::numeric_limits<uint32>::max();

	// if client_default.cfg is not in current directory, use application default directory
	if (Args.haveArg("p") || !CFile::isExists("client_default.cfg"))
	{
		std::string currentPath = CPath::getApplicationDirectory("Ryzom");

		// create parent directory
		if (!CFile::isExists(currentPath)) CFile::createDirectory(currentPath);

		// append profile ID to directory
		if (Args.haveArg("p"))
		{
			currentPath = NLMISC::CPath::standardizePath(currentPath) + Args.getArg("p").front();

			if (!CFile::isExists(currentPath)) CFile::createDirectory(currentPath);
		}

		if (!CPath::setCurrentPath(currentPath)) return 1;
	}

#ifdef TEST_CRASH_COUNTER
	if (Args.haveLongArg("crash"))
	{
		volatile int toto = *(int*)0;
	}
#endif // TEST_CRASH_COUNTER

#if defined(NL_OS_WINDOWS)

#ifdef TEST_CRASH_COUNTER
	if (Args.haveLongArg("break"))
	{
		__debugbreak();
	}
#endif // TEST_CRASH_COUNTER

	HInstance = hInstance;

	// Get the bitmap size
	HRSRC hrsrc = FindResource(HInstance, MAKEINTRESOURCE(IDB_SLASH_SCREEN), RT_BITMAP);
	nlassert (hrsrc);
	HGLOBAL hBitmap = LoadResource (HInstance, hrsrc);
	nlassert (hBitmap);
	BITMAP *bitmap = (BITMAP*)LockResource(hBitmap);
	nlassert (bitmap);
	int width = bitmap->bmWidth;
	int height = bitmap->bmHeight;

	// Look the command line to see if we have a cookie and a addr
	SlashScreen = CreateDialog (hInstance, MAKEINTRESOURCE(IDD_SLASH_SCREEN), NULL, MyDialogProc);
	RECT rect;
	RECT rectDesktop;
	GetWindowRect (SlashScreen, &rect);
	GetWindowRect (GetDesktopWindow (), &rectDesktop);
	SetWindowPos (SlashScreen, HWND_TOP, (rectDesktop.right-rectDesktop.left-width)/2, (rectDesktop.bottom-rectDesktop.top-height)/2, width, height, 0);
	ShowWindow (SlashScreen, SW_SHOW);

	pump ();

	// Delete all the .ttf file in the /data directory
	{
		vector<string> files;
		NLMISC::CPath::getPathContent ("data", false, false, true, files, NULL, true);
		uint i;
		for (i=0; i<files.size(); i++)
		{
			if (toLowerAscii(CFile::getExtension (files[i])) == "ttf")
				CFile::deleteFile (files[i]);
		}
	}

#else
	// TODO for Linux : splashscreen
#endif

	// initialize log
	initLog();

#ifdef RZ_USE_STEAM
	CSteamClient steamClient;

	if (steamClient.init()){
		LoginCustomParameters = "&steam_auth_session_ticket=" + steamClient.getAuthSessionTicket();
		const char *steamLanguage = steamClient.GameLanguageWebApiFormat();
		std::string steamLanguageCompare(steamLanguage);
		//change language full to right format

		if (steamLanguageCompare != ClientCfg.LanguageCode)
		{
			nlinfo("Force Apply Steam Laguage: %s, current CFG Language %s", steamLanguageCompare.c_str(), ClientCfg.LanguageCode.c_str());
			ClientCfg.ForceLanguage= true;
			ClientCfg.LanguageCode = steamLanguageCompare;
		}
	}
#endif

#if !FINAL_VERSION
	LoginCustomParameters += "&dbg=1";
#endif

	// initialize patch manager and set the ryzom full path, before it's used
	CPatchManager *pPM = CPatchManager::getInstance();
	pPM->setRyzomFilename(Args.getProgramPath() + Args.getProgramName());

	/////////////////////////////////
	// Initialize the application. //
#ifndef TEST_CRASH_COUNTER
	RYZOM_TRY("Pre-Login Init")
		prelogInit();
	RYZOM_CATCH("Pre-Login Init")

	// Log the client and choose from shard
	RYZOM_TRY("Login")
	if (!ClientCfg.Local && (ClientCfg.TestBrowser || ClientCfg.FSHost.empty()))
	{
		if (login() == false)
		{
			quitCrashReport ();

#if !FINAL_VERSION
			// display the file access logger stats and clear out memory allocated for the file access logger
			//ICommand::execute("iFileAccessLogDisplay",*NLMISC::InfoLog);
			//ICommand::execute("iFileAccessLogStop",*NLMISC::InfoLog);
			//ICommand::execute("iFileAccessLogClear",*NLMISC::InfoLog);
#endif

			release();

			// delete all logs and displayers when we're not using logs macros anymore
			destroyDebug();
			CLog::releaseProcessName();

			// delete the Nel context
			delete appContext;

			return EXIT_SUCCESS;
		}
	}
	RYZOM_CATCH("Login")

	// Finish inits
	RYZOM_TRY("Post-Login Init")
		postlogInit();
	RYZOM_CATCH("Post-Login Init")
#endif // TEST_CRASH_COUNTER

	//////////////////////////////////////////
	// The real main loop
	//////////////////////////////////////////
#ifdef TEST_CRASH_COUNTER
	bool ok = false;
#else // TEST_CRASH_COUNTER
	bool ok = true;
#endif // TEST_CRASH_COUNTER
	while(ok)
	{
		// hulud : memory snapshot to track reconnection memory leak
		/* static int pass = 0;
		string filename = "z_mem_connection_" + toString (pass++) + ".csv";
		nlverify (NLMEMORY::StatisticsReport (filename.c_str(), true)); */

		//////////////////////////////////////////
		// Manage the connection to the server. //
		RYZOM_TRY("Connection")
			// If the connection return false we just want to quit the game
			if(!connection(Cookie, FSAddr))
			{
				releaseOutGame();
				break;
			}
		RYZOM_CATCH("Connection")

		///////////////////////////////
		// Initialize the main loop. //
		RYZOM_TRY("Main loop initialisation")
			initMainLoop();
		RYZOM_CATCH("Main loop initialisation")

		//////////////////////////////////////////////////
		// Main loop (biggest part of the application). //
		RYZOM_TRY("Main loop")
			ok = !mainLoop();
		RYZOM_CATCH("Main loop")

		/////////////////////////////
		// Release all the memory. //
		if (!FarTP.isReselectingChar())
		{
			RYZOM_TRY("Main loop releasing")
				releaseMainLoop(!ok);
			RYZOM_CATCH("Main loop releasing")
		}

		// Offline client quit now
		if(ClientCfg.Local)
			break;
	}

#if !FINAL_VERSION
	// display the file access logger stats and clear out memory allocated for the file access logger
	//ICommand::execute("iFileAccessLogDisplay",*NLMISC::InfoLog);
	//ICommand::execute("iFileAccessLogStop",*NLMISC::InfoLog);
	//ICommand::execute("iFileAccessLogClear",*NLMISC::InfoLog);
#endif

	//CFile::createEmptyFile(getLogDirectory() + "during_release");

#ifdef TEST_CRASH_COUNTER
	if (Args.haveLongArg("release"))
	{
		volatile int toto = *(int*)0;
	}
#endif // TEST_CRASH_COUNTER

	// Final release
	RYZOM_TRY("Releasing")
		release();
	RYZOM_CATCH("Releasing")

#if FINAL_VERSION || defined (TEST_CRASH_COUNTER)
	quitCrashReport ();
#endif // FINAL_VERSION

	// delete all logs and displayers when we're not using logs macros anymore
	destroyDebug();
	CLog::releaseProcessName();
	// CCoTask::releaseInstance();

	// delete the Nel context
	delete appContext;

#ifdef NL_OS_WINDOWS
	/// always do a sanity check for early (in the sense of production days) memory coruption detection
	// _CrtCheckMemory();
#endif

	// EXIT of the Application.
	return EXIT_SUCCESS;

} // main/WinMain
