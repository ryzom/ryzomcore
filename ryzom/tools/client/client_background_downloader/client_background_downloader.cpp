/*
 * $Id:
 */

// client_background_downloader.cpp : Defines the class behaviors for the application.
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "client_background_downloaderDlg.h"
#include "intro_screen_dlg.h"
#include "install_task_dlg.h"
#include "choose_package_dlg.h"
#include "install_success_dlg.h"
#include "nel/misc/algo.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "../client/login_patch.h"
#include "../client/login_progress_post_thread.h"


#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/displayer.h"
#include "nel/misc/common.h"

#include "web_page_embedder.h"

#include "install_logic/game_downloader.h"


const char *CClient_background_downloaderApp::AppRegEntry = "Software\\Nevrax\\ryzom\\background_downloader";
const char *CClient_background_downloaderApp::RegKeyDisplayCloseWarning = "DisplayCloseWarning";

using NLMISC::CI18N;
using NLMISC::CPath;

const char *ConfigFileName = "client.cfg";

/////////////////////////////////////////////////////////////////////////////
// CClient_background_downloaderApp

BEGIN_MESSAGE_MAP(CClient_background_downloaderApp, CWinApp)
	//{{AFX_MSG_MAP(CClient_background_downloaderApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClient_background_downloaderApp construction

CClient_background_downloaderApp::CClient_background_downloaderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	if (OleInitialize(NULL) != S_OK)
	{
		exit(0);
	}
}


CClient_background_downloaderApp::~CClient_background_downloaderApp()
{	
	OleUninitialize();
	CLoginProgressPostThread::getInstance().release();
}


/////////////////////////////////////////////////////////////////////////////
// The one and only CClient_background_downloaderApp object

CClient_background_downloaderApp theApp;



/////////////////////////////////////////////////////////////////////////////
bool CClient_background_downloaderApp::parseCommandLine(const std::string &commandLine)
{
	// parse command line
	std::vector<std::string> args;
	NLMISC::splitString(commandLine, " ", args);

	//if (args.size() < 4)
	if (args.size() < 2)
	{		
		return false;
	}
	for (uint k = 0; k < args.size(); ++k)
	{
		while (NLMISC::strFindReplace(args[k], "\"", "")) {}
	}
	extern std::string R2ServerVersion;
	extern std::string VersionName;
	//R2ServerVersion = args[0];
	//VersionName		= args[1];
	//ServerPath      = args[2];
	//ServerVersion   = args[3];
	ServerPath      = args[0];
	ServerVersion   = args[1];
	VersionName = ServerVersion;
	PatchURIs.clear();
	for (uint k = 4; k < args.size(); ++k)
	{
		PatchURIs.push_back(args[k]);
	}
	return true;
}


void CClient_background_downloaderApp::initLog(const char *appName)
{
	NLMISC::createDebug();
	NLMISC::CFileDisplayer *RBGLogDisplayer = new NLMISC::CFileDisplayer(appName, true, "CLIENT.LOG");
	NLMISC::DebugLog->addDisplayer (RBGLogDisplayer);
	NLMISC::InfoLog->addDisplayer (RBGLogDisplayer);
	NLMISC::WarningLog->addDisplayer (RBGLogDisplayer);
	NLMISC::ErrorLog->addDisplayer (RBGLogDisplayer);
	NLMISC::AssertLog->addDisplayer (RBGLogDisplayer);
}



struct CResourceTranslationFileLoader : public CI18N::ILoadProxy
{
	virtual void loadStringFile(const std::string &filename, ucstring &text)
	{
		bool ok = false;
		int resID = 0;
		if (NLMISC::nlstricmp(filename, "en.uxt") == 0)
		{
			resID = IDR_EN_UXT;
		}
		else
		if (NLMISC::nlstricmp(filename, "fr.uxt") == 0)
		{
			resID = IDR_FR_UXT;
		}
		else 
		if (NLMISC::nlstricmp(filename, "de.uxt") == 0)
		{
			resID = IDR_DE_UXT;
		}
		else
		if (NLMISC::nlstricmp(filename, "wk.uxt") == 0)
		{
			resID = IDR_WK_UXT;
		}
		else		
		{
			nlwarning("No resource file found for language %s ! Using wk.uxt as a default",  filename.c_str());
			resID = IDR_WK_UXT;
		}
		HRSRC hrsrc = FindResource(theApp.m_hInstance, MAKEINTRESOURCE(resID), "UXT");
		if (hrsrc)
		{
			int resSize = SizeofResource(theApp.m_hInstance, hrsrc);
			if (resSize > 3)
			{
				HGLOBAL hglob = LoadResource(theApp.m_hInstance, hrsrc);
				if (hglob)
				{
					const char *datas = (const char *) LockResource(hglob);
					if (datas)
					{
						std::string rscStr;
						// skip first 3 characters (header of uxt file)
						rscStr.resize(resSize - 3);
						memcpy(&rscStr[0], datas + 3, resSize - 3);
						text.fromUtf8(rscStr);
						ok = true;
					}

				}
			}
		}
		if (!ok)
		{
			nlwarning("Failed to acquire resource for file %s", filename.c_str());
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CClient_background_downloaderApp initialization
BOOL CClient_background_downloaderApp::InitInstance()
{
	HANDLE mutex = CreateMutex (NULL, TRUE, BGDownloader::DownloaderMutexName);
	if (mutex && GetLastError() == ERROR_ALREADY_EXISTS) 
		exit (0);
	
	AfxEnableControlContainer();
	AfxInitRichEdit();

	CWebPageEmbedder::registerWebPageWindowClass(this->m_hInstance, "WebPage");	


	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
		// Background patch is launch with argument but Installer/updater is launch without parameter
	bool installMode = parseCommandLine((LPCTSTR) m_lpCmdLine) == false;

	std::string appName = NLMISC::strlwr(std::string(AfxGetAppName()));
	
	if (installMode)
	{
		initLog("ryzom_installer.log");
		// if no command line is found, assume that the app is launched as an installer
		nldebug("Loading config file");

		loadConfigFile(NULL, "Ryzom Installer");


		nldebug("Loading translation file for lang '%s'", getLanguage().c_str());
		CResourceTranslationFileLoader loadProxy;
		CI18N::setLoadProxy(&loadProxy);
		CI18N::load(getLanguage());
		
		nldebug("Set translation file");

		IGameDownloader* gd = IGameDownloader::getInstance();		
		// if not first time then do not display first windows


		
		int result;
		if (!gd->getFirstInstall())
		{
			
			nldebug("Launching Intro Dlg");
			CIntroScreenDlg introScreen;
			result = introScreen.DoModal();
			if (result != IDOK)
			{
				exit(0);
			}
			
		}
		nldebug("Mode '%s'", gd->getFirstInstall()?"Install":"Repair");
		if (!gd->getFirstInstall())
		{
			//reset cfg and string cache 
			gd->resetCfgAndStringCache();
			loadConfigFile(NULL, "Ryzom Installer");	
		}
		gd->doAction("welcome_continue", true);
		

		CInstallTaskDlg patchFiles;
		result = patchFiles.DoModal();


	
	}
	else
	{
		initLog("client_background_downloader.log");
		// If there's a command line, assume that the app is launched as a background downloader from the client
		CClient_background_downloaderDlg dlg;
		m_pMainWnd = &dlg;
		int nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


// ***************************************************************************
void CClient_background_downloaderApp::loadConfigFile(HWND parent, const char *appTitle)
{
	
	// load language from config file		
	try
	{
		theApp.ConfigFile.load(ConfigFileName);
	}
	catch (std::exception &e)
	{
		::MessageBox(parent, e.what(), appTitle, MB_ICONEXCLAMATION);
		exit(0);
	}
}


// ***************************************************************************
std::string CClient_background_downloaderApp::getLanguage()
{
	std::string language = "en";
	NLMISC::CConfigFile::CVar *languageVarPtr = ConfigFile.getVarPtr("LanguageCode");
	if (languageVarPtr)
	{
		language = languageVarPtr->asString();
	}
	else
	{
		nlwarning("Language not found in %s. Using english as a default.", ConfigFileName);
	}
	return language;
}

// ***************************************************************************
void CClient_background_downloaderApp::setCloseWarningFlag(BOOL enabled)
{
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, AppRegEntry, &hKey)==ERROR_SUCCESS)
	{		
		DWORD flag = (DWORD) enabled;
		RegSetValueEx(hKey, RegKeyDisplayCloseWarning, 0, REG_DWORD, (LPBYTE)&enabled, 4);		
	}
}

// ***************************************************************************
BOOL CClient_background_downloaderApp::getCloseWarningFlag() const
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, AppRegEntry, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD enabled = 0;
		DWORD type;
		ULONG len = 4;
		RegQueryValueEx(hKey, RegKeyDisplayCloseWarning, NULL, &type, (LPBYTE)&enabled, &len);
		return (BOOL) enabled;		
	}
	return TRUE;
}


// ***************************************************************************
uint CClient_background_downloaderApp::computePatchSize(const CPatchManager::SPatchInfo &infoOnPatch, bool patchMainland)
{
	uint32 nNonOptSize = 0;
	uint totalPatchSize = 0;
	std::vector<sint32> ReqCat;
	if (patchMainland) // 'optionnal category' will be the flag to indicate 'mainland'
	{
		for(uint i = 0; i < infoOnPatch.OptCat.size(); i++)
		{
			totalPatchSize += infoOnPatch.OptCat[i].Size;
			if (infoOnPatch.OptCat[i].Req != -1)
			{
				uint32 j;
				for (j = 0; j < ReqCat.size(); ++j)
					if (ReqCat[j] == infoOnPatch.OptCat[i].Req)
						break;
				// Add just once the req cat size to the non optional size
				if (j == ReqCat.size())
				{
					ReqCat.push_back(infoOnPatch.OptCat[i].Req);
					nNonOptSize += infoOnPatch.ReqCat[infoOnPatch.OptCat[i].Req].Size;
				}
			}			
		}
	}
	
	// Add non optional cats
	for (uint32 i = 0; i < infoOnPatch.NonOptCat.size(); ++i)
		nNonOptSize += infoOnPatch.NonOptCat[i].Size;	
	totalPatchSize += nNonOptSize;	
	return totalPatchSize;
}

