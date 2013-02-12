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


//////////////
// INCLUDES //
//////////////
// Misc.
#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#include <shellapi.h>
#else
#include <csignal>
#endif

#ifdef NL_OS_MAC
#include <stdio.h>
#include <sys/resource.h>
#include "nel/misc/dynloadlib.h"
#include "app_bundle_utils.h"
#endif

#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/net/tcp_sock.h"

//#define TEST_CRASH_COUNTER
#ifdef TEST_CRASH_COUNTER
#include "nel/net/email.h"
 #undef FINAL_VERSION
 #define FINAL_VERSION 1
#endif // TEST_CRASH_COUNTER

// game share
#include "game_share/ryzom_version.h"

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

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;

//
// Macros
//

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

#ifndef NL_OS_WINDOWS
static void sigHandler(int Sig)
{
	// redirect the signal for the next time
	signal(Sig, sigHandler);
	nlwarning("Ignoring signal SIGPIPE");
}
#endif

//---------------------------------------------------
// MAIN :
// Entry for the Application.
//---------------------------------------------------
#ifdef NL_OS_WINDOWS

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
/*
static bool connect()
{
	string server = "crashcounter.nevrax.com";

	if(CrashCounterSock.connected())
		return true;

	try
	{
		// add the default port if no port in the cfg
		if(server.find(':') == string::npos)
			server+=":80";
		CrashCounterSock.connect(CInetAddress(server));
		if(!CrashCounterSock.connected())
		{
			nlwarning("Can't connect to web server '%s'", server.c_str());
			goto end;
		}
	}
	catch(const Exception &e)
	{
		nlwarning("Can't connect to web server '%s': %s", server.c_str(), e.what());
		goto end;
	}

	return true;

end:

	if(CrashCounterSock.connected())
		CrashCounterSock.close ();

	return false;
}

// ***************************************************************************
static bool send(const string &url)
{
	if (CrashCounterSock.connected())
	{
		string buffer = "GET " + url +
			" HTTP/1.0\n"
			"Host: crashcounter.nevrax.com\n"
			"User-agent: Ryzom\n"
			"\n";
		uint32 size = (uint32)buffer.size();
		if(!url.empty())
		{
			if(CrashCounterSock.send((uint8 *)buffer.c_str(), size, false) != CSock::Ok)
			{
				nlwarning ("Can't send data to the server");
				return false;
			}
		}
		return true;
	}
	return false;
}

// ***************************************************************************
static bool receive(string &res)
{
	if (CrashCounterSock.connected())
	{
		uint32 size;
		res = "";

		uint8 buf[1024];

		for(;;)
		{
			size = 1023;

			if (CrashCounterSock.receive((uint8*)buf, size, false) == CSock::Ok)
			{
				buf[1023] = '\0';
				res += (char*)buf;
				//nlinfo("block received '%s'", buf);
			}
			else
			{
				buf[size] = '\0';
				res += (char*)buf;
				//nlwarning ("server connection closed");
				break;
			}
		}
		//nlinfo("all received '%s'", res.c_str());
		return true;
	}
	else
		return false;
}

string CrashFeedback = "CRASHED";

INT_PTR CALLBACK ReportDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT rect;
			RECT rectDesktop;
			GetWindowRect (hwndDlg, &rect);
			GetWindowRect (GetDesktopWindow (), &rectDesktop);
			SetWindowPos (hwndDlg, HWND_TOPMOST, (rectDesktop.right-rectDesktop.left-rect.right+rect.left)/2, (rectDesktop.bottom-rectDesktop.top-rect.bottom+rect.top)/2, 0, 0, SWP_NOSIZE);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case FROZEN:
			CrashFeedback = "USER_FROZEN";
			EndDialog (hwndDlg, IDOK);
			break;
		case REBOOTED:
			CrashFeedback = "USER_REBOOTED";
			EndDialog (hwndDlg, IDOK);
			break;
		case WINDOWED:
			CrashFeedback = "USER_WINDOWED";
			EndDialog (hwndDlg, IDOK);
			break;
		case NO_WINDOW:
			CrashFeedback = "USER_NO_WINDOW";
			EndDialog (hwndDlg, IDOK);
			break;
		case KILLED:
			CrashFeedback = "USER_KILLED";
			EndDialog (hwndDlg, IDOK);
			break;
		case NOT_CRASHED:
			CrashFeedback = "USER_NOT_CRASHED";
			EndDialog (hwndDlg, IDOK);
			break;
		case CRASHED:
			CrashFeedback = "CRASHED";
			EndDialog (hwndDlg, IDOK);
			break;
		}
		break;
	}
	return FALSE;
}

void initCrashReport ()
{
	//
	bool crashed = CFile::isExists (getLogDirectory() + "ryzom_started");
	bool during_release = false;
	bool exception_catched = false;
	bool breakpointed = false;
	bool dumped = false;
	bool report_failed = false;
	bool report_refused = false;
	bool report_sent = false;
	if (crashed && CFile::isExists (getLogDirectory() + "during_release"))
		during_release = CFile::getFileModificationDate (getLogDirectory() + "ryzom_started") <= CFile::getFileModificationDate (getLogDirectory() + "during_release");
	if (crashed && CFile::isExists (getLogDirectory() + "exception_catched"))
		exception_catched = CFile::getFileModificationDate (getLogDirectory() + "ryzom_started") <= CFile::getFileModificationDate (getLogDirectory() + "exception_catched");
	if (crashed && CFile::isExists (getLogDirectory() + "breakpointed"))
		breakpointed = CFile::getFileModificationDate ("ryzom_started") <= CFile::getFileModificationDate (getLogDirectory() + "breakpointed");
	if (crashed && CFile::isExists (getLogDirectory() + "nel_debug.dmp"))
		dumped = CFile::getFileModificationDate (getLogDirectory() + "ryzom_started") <= CFile::getFileModificationDate (getLogDirectory() + "nel_debug.dmp");
	if (crashed && CFile::isExists (getLogDirectory() + "report_failed"))
		report_failed = CFile::getFileModificationDate (getLogDirectory() + "ryzom_started") <= CFile::getFileModificationDate (getLogDirectory() + "report_failed");
	if (crashed && CFile::isExists (getLogDirectory() + "report_refused"))
		report_refused = CFile::getFileModificationDate (getLogDirectory() + "ryzom_started") <= CFile::getFileModificationDate (getLogDirectory() + "report_refused");
	if (crashed && CFile::isExists (getLogDirectory() + "report_sent"))
		report_sent = CFile::getFileModificationDate (getLogDirectory() + "ryzom_started") <= CFile::getFileModificationDate (getLogDirectory() + "report_sent");
	CFile::createEmptyFile(getLogDirectory() + "ryzom_started");
	connect();
	if (report_sent)
		send("/?crashtype=REPORT_SENT");
	else if (report_refused)
		send("/?crashtype=REPORT_REFUSED");
	else if (report_failed)
		send("/?crashtype=REPORT_FAILED");
	else if (dumped)
		send("/?crashtype=DUMPED");
	else if (breakpointed)
		send("/?crashtype=BREAKPOINTED");
	else if (exception_catched)
		send("/?crashtype=EXCEPTION_CATCHED");
	else if (during_release)
		send("/?crashtype=DURING_RELEASE");
	else if (crashed)
	{
		//DialogBox (HInstance, MAKEINTRESOURCE(IDD_CRASH_INFORMATION), NULL, ReportDialogProc);
		//send("/?crashtype="+CrashFeedback);
		send("/?crashtype=CRASHED");
	}
	else
		send("/?crashtype=NOT_CRASHED");
	string res;
	receive(res);
#ifdef TEST_CRASH_COUNTER
	MessageBox (NULL, res.c_str(), res.c_str(), MB_OK);
#endif // TEST_CRASH_COUNTER
}*/

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE /* hPrevInstance */, LPSTR cmdline, int /* nCmdShow */)
#else
int main(int argc, char **argv)
#endif
{
	// init the Nel context
	CApplicationContext *appContext = new CApplicationContext;

	createDebug();

#ifndef NL_DEBUG
	INelContext::getInstance().getDebugLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getInfoLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getWarningLog()->removeDisplayer("DEFAULT_SD");
#endif // NL_DEBUG

	// if client_default.cfg is not in current directory, use application default directory
	if (!CFile::isExists("client_default.cfg"))
	{
		std::string currentPath = CPath::getApplicationDirectory("Ryzom");

		if (!CFile::isExists(currentPath)) CFile::createDirectory(currentPath);

		CPath::setCurrentPath(currentPath);
	}

	// temporary buffer to store Ryzom full path
	char filename[1024];

#ifdef NL_OS_MAC
	struct rlimit rlp, rlp2, rlp3;

	getrlimit(RLIMIT_NOFILE, &rlp);

	rlp2.rlim_cur = 1024;
	rlp2.rlim_max = rlp.rlim_max;
	setrlimit(RLIMIT_NOFILE, &rlp2);

	getrlimit(RLIMIT_NOFILE, &rlp3);
	nlinfo("rlimit before %d %d\n", rlp.rlim_cur, rlp.rlim_max);
	nlinfo("rlimit after %d %d\n", rlp3.rlim_cur, rlp3.rlim_max);

	// add the bundle's plugins path as library search path (for nel drivers)
	CLibrary::addLibPath(getAppBundlePath() + "/Contents/PlugIns/nel/");
#endif

#if defined(NL_OS_WINDOWS)

	/* Windows bug: When the Window IconeMode is in "ThumbNails" mode, the current path is set to
		"document settings"..... Force the path to be the path of the exe
	*/
	{
#ifdef FINAL_VERSION
		char	str[4096];
		uint	len= GetModuleFileName(NULL, str, 4096);
		if(len && len<4096)
		{
			str[len]= 0;
			string	path= CFile::getPath(str);
//			if(!path.empty())
//				CPath::setCurrentPath(path.c_str());
		}
#endif // FINAL_VERSION
	}

	string sCmdLine = cmdline;
#if FINAL_VERSION
	//if (sCmdLine.find("/multi") == string::npos) // If '/multi' not found
	//{
	//	HANDLE mutex = CreateMutex (NULL, false, "RyzomClient");
	//	if (mutex && GetLastError() == ERROR_ALREADY_EXISTS)
	//		exit (0);
	//}

	//initCrashReport ();
#endif // FINAL_VERSION

	// Set default email value for reporting error
#ifdef TEST_CRASH_COUNTER
	//initCrashReport ();
	setReportEmailFunction ((void*)sendEmail);
	setDefaultEmailParams ("smtp.nevrax.com", "", "hulud@nevrax.com");

	if (string(cmdline) == "/crash")
		volatile int toto = *(int*)0;
	if (string(cmdline) == "/break")
		__asm
		{
			int 3
		};
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

	// extract the 2 or 3 first param (argv[1], argv[2] and argv[3]) it must be <login> <password> [shardId]
	vector<string> res;
	explode(sCmdLine, std::string(" "), res, true);

	// no shard id in ring mode
	if (res.size() >= 3)
	{
		LoginLogin = res[0];
		LoginPassword = res[1];
		if (!fromString(res[2], LoginShardId)) LoginShardId = -1;
	}
	else if (res.size() >= 2)
	{
		LoginLogin = res[0];
		LoginPassword = res[1];
		LoginShardId = -1;
	}

	GetModuleFileName(GetModuleHandle(NULL), filename, 1024);

	// Delete the .bat file because it s not useful anymore
	if (NLMISC::CFile::fileExists("updt_nl.bat"))
		NLMISC::CFile::deleteFile("updt_nl.bat");
	if (NLMISC::CFile::fileExists("bug_report.exe"))
		NLMISC::CFile::deleteFile("bug_report.exe");
	if (NLMISC::CFile::fileExists("bug_report_r.exe"))
		NLMISC::CFile::deleteFile("bug_report_r.exe");
	if (NLMISC::CFile::fileExists("bug_report_rd.exe"))
		NLMISC::CFile::deleteFile("bug_report_rd.exe");
	if (NLMISC::CFile::fileExists("bug_report_df.exe"))
		NLMISC::CFile::deleteFile("bug_report_df.exe");
	if (NLMISC::CFile::fileExists("bug_report_d.exe"))
		NLMISC::CFile::deleteFile("bug_report_d.exe");

	// Delete all the .ttf file in the /data directory
	{
		vector<string> files;
		NLMISC::CPath::getPathContent ("data", false, false, true, files, NULL, true);
		uint i;
		for (i=0; i<files.size(); i++)
		{
			if (strlwr (CFile::getExtension (files[i])) == "ttf")
				CFile::deleteFile (files[i]);
		}
	}

#else

	// TODO for Linux : splashscreen

	if (argc >= 4)
	{
		LoginLogin = argv[1];
		LoginPassword = argv[2];
		if (!fromString(argv[3], LoginShardId)) LoginShardId = -1;
	}
	else if (argc >= 3)
	{
		LoginLogin = argv[1];
		LoginPassword = argv[2];
		LoginShardId = -1;
	}

	strcpy(filename, argv[0]);


	// ignore signal SIGPIPE generated by libwww
	signal(SIGPIPE, sigHandler);

#endif

	// initialize patch manager and set the ryzom full path, before it's used
	CPatchManager *pPM = CPatchManager::getInstance();
	pPM->setRyzomFilename(NLMISC::CFile::getFilename(filename));

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
	if (string(cmdline) == "/release")
		volatile int toto = *(int*)0;
#endif // TEST_CRASH_COUNTER

	// Final release
	RYZOM_TRY("Releasing")
		release();
	RYZOM_CATCH("Releasing")

#if FINAL_VERSION || defined (TEST_CRASH_COUNTER)
	quitCrashReport ();
#endif // FINAL_VERSION

	// delete the Nel context
	delete appContext;

#ifdef NL_OS_WINDOWS
	/// always do a sanity check for early (in the sense of production days) memory coruption detection
	// _CrtCheckMemory();
#endif

	// EXIT of the Application.
	return EXIT_SUCCESS;

} // main/WinMain
