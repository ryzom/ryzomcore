// nel_launcher.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "nel_launcher.h"
#include "nel_launcherDlg.h"
#include <WinReg.h>
#include "MsgDlg.h"

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	KEY_ROOT		_T("SOFTWARE\\Ryzom")
#define	KEY_MAX_LENGTH	1024

NLMISC::CFileDisplayer FileDisplayer;

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherApp

BEGIN_MESSAGE_MAP(CNel_launcherApp, CWinApp)
	//{{AFX_MSG_MAP(CNel_launcherApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherApp construction

CNel_launcherApp::CNel_launcherApp()
{
	m_bAuthWeb	= FALSE;
	m_bAuthGame	= FALSE;
	m_dVersion	= 0;
	m_bLog		= TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNel_launcherApp object

CNel_launcherApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherApp initialization

BOOL CNel_launcherApp::InitInstance()
{
	// use log.log if NEL_LOG_IN_FILE defined as 1
	NLMISC::createDebug(NULL, true, true);

	// filedisplayer only deletes the 001 etc
	if (NLMISC::CFile::isExists("nel_launcher_ext.log"))
		NLMISC::CFile::deleteFile("nel_launcher_ext.log");
	// initialize the log file
	FileDisplayer.setParam("nel_launcher_ext.log", true);
	NLMISC::DebugLog->addDisplayer(&FileDisplayer);
	NLMISC::InfoLog->addDisplayer(&FileDisplayer);
	NLMISC::WarningLog->addDisplayer(&FileDisplayer);
	NLMISC::AssertLog->addDisplayer(&FileDisplayer);
	NLMISC::ErrorLog->addDisplayer(&FileDisplayer);

	LoadVersion();
	m_config.Load();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CNel_launcherDlg dlg;
	m_pMainWnd = &dlg;

	m_hcPointer	= LoadCursor(IDC_POINTER);


	// ace: clear the cookies

	HKEY	hkey;
	TCHAR	*szKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
	CString	csRet;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		csRet	= ReadInfoFromRegistry("Cookies", hkey);
		RegCloseKey(hkey);

		string path = csRet.GetBuffer(0);

		nlinfo("Cookies are in directory '%s'", path.c_str());
		if(!path.empty())
		{
			vector<string> res;
			NLMISC::CPath::getPathContent(path, false, false, true, res);
			for(uint i = 0; i < res.size(); i++)
			{
				if(NLMISC::strlwr(res[i]).find("www_test") != string::npos)
				{
					nlinfo("Deleting cookie '%s'", res[i].c_str());
					NLMISC::CFile::deleteFile(res[i]);
				}
			}
		}
	}
	else
	{
		nlwarning("Can't find the cookies directory");
	}
	


	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CNel_launcherApp::GetWindowsVersion()
{
	DWORD	dwVersion = ::GetVersion();	
	DWORD	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD	dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
	DWORD	dwBuild;

	// Get the build number for Windows NT/Windows 2000 or Win32s.

	if(dwVersion < 0x80000000)                // Windows NT/2000
		dwBuild = (DWORD)(HIWORD(dwVersion));
	else if(dwWindowsMajorVersion < 4)        // Win32s
		dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
	else                  // Windows 95/98 -- No build number
	{
		dwBuild =  0;
		return FALSE;
	}
	return TRUE;
}

CString CNel_launcherApp::GetRegKeyValue(const char* lpszEntry)
{
    HKEY	hkey;
    TCHAR	szKey[KEY_MAX_LENGTH];
	CString	csRet;

	strcpy(szKey, KEY_ROOT);
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		csRet	= ReadInfoFromRegistry(lpszEntry, hkey);
		RegCloseKey(hkey);
	}
    return csRet;
}

void CNel_launcherApp::SetRegKey(char* lpszValueName, CString csValue)
{
	HKEY	hkey;
    TCHAR	szKey[KEY_MAX_LENGTH];
	DWORD	dwDisp;
	char	lpszBuffer[KEY_MAX_LENGTH*2];

	strcpy(szKey, KEY_ROOT);
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp) == ERROR_SUCCESS)
    {
		strcpy(lpszBuffer, csValue);
		RegSetValueEx(hkey, _T(lpszValueName), 0L, REG_SZ, (const BYTE *)lpszBuffer, strlen(lpszBuffer)+1);
		RegCloseKey(hkey);
	}
}

CString CNel_launcherApp::ReadInfoFromRegistry(const char *lpszEntry, HKEY hkey)
{
	DWORD	dwType  = 0L;
	DWORD	dwSize	= KEY_MAX_LENGTH;
	CString	csRet;

	if(RegQueryValueEx(hkey, _T(lpszEntry), NULL, &dwType, (unsigned char *)(csRet.GetBuffer(KEY_MAX_LENGTH)), &dwSize) == ERROR_SUCCESS)
		csRet.ReleaseBuffer();
	else
	{
		csRet.ReleaseBuffer();
		csRet.Empty();
	}
	return csRet;
}

int CNel_launcherApp::ExitInstance() 
{
	ResetConnection();
	
	return CWinApp::ExitInstance();
}

void CNel_launcherApp::ResetConnection()
{
	CString csUrl	= "http://" + m_config.m_csHost + m_config.m_csUrlMain;

	SetRegKey(_T("Login"), "");

/*	CInternetSession session("nel_launcher");
	session.SetCookie(csUrl, "login", "");
	session.SetCookie(csUrl, "password", "");
	session.SetCookie(csUrl, "digest", "");*/

	BOOL bResult = FALSE;
    BOOL bDone = FALSE;
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = NULL;  
 
    DWORD  dwTrySize, dwEntrySize = 4096; // start buffer size    
    HANDLE hCacheDir = NULL;    
    DWORD  dwError = ERROR_INSUFFICIENT_BUFFER;
    
    do 
    {                               
        switch (dwError)
        {
            // need a bigger buffer
            case ERROR_INSUFFICIENT_BUFFER: 
                delete [] lpCacheEntry;            
                lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
                lpCacheEntry->dwStructSize = dwEntrySize;
                dwTrySize = dwEntrySize;
                BOOL bSuccess;
                if (hCacheDir == NULL)                
                  
                    bSuccess = (hCacheDir 
                      = FindFirstUrlCacheEntry(NULL, lpCacheEntry,
                      &dwTrySize)) != NULL;
                else
                    bSuccess = FindNextUrlCacheEntry(hCacheDir, lpCacheEntry, &dwTrySize);

                if (bSuccess)
                    dwError = ERROR_SUCCESS;    
                else
                {
                    dwError = GetLastError();
                    dwEntrySize = dwTrySize; // use new size returned
                }
                break;

             // we are done
            case ERROR_NO_MORE_ITEMS:
                bDone = TRUE;
                bResult = TRUE;                
                break;

             // we have got an entry
            case ERROR_SUCCESS:                       
        
                // only cookies are concerned                 
                if((lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY))
				{
					CString csUser;
					DWORD	dwSize;

					GetUserName(csUser.GetBuffer(2048), &dwSize);
					csUser.ReleaseBuffer();
					if(!strcmp(lpCacheEntry->lpszSourceUrlName, "Cookie:"+csUser+"@"+m_config.m_csHost+"/www_test/"))
						DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
				}
                    
                // get ready for next entry
                dwTrySize = dwEntrySize;
                if (FindNextUrlCacheEntry(hCacheDir, lpCacheEntry, &dwTrySize))
                    dwError = ERROR_SUCCESS;          
     
                else
                {
                    dwError = GetLastError();
                    dwEntrySize = dwTrySize; // use new size returned
                }                    
                break;

            // unknown error
            default:
                bDone = TRUE;                
                break;
        }

        if (bDone)
        {   
            delete [] lpCacheEntry; 
            if (hCacheDir)
                FindCloseUrlCache(hCacheDir);         
                                  
        }
    } while (!bDone);

}

void CNel_launcherApp::LoadVersion()
{
	CString csVersion;

	m_dVersion	= 0;

	if(!csVersion.LoadString((HINSTANCE)GetModuleHandle(NULL), IDS_VERSION))
	{
		MSGBOX("Error", "Cannot load version resource");
	}
	else
	{
		nlinfo("Launcher' version %s", csVersion);
		NLMISC::fromString(csVersion, m_dVersion);
	}
}

//void CNel_launcherApp::EnableLog(BOOL bEnable)
//{
//	m_bLog	= bEnable;
//}
//
//void CNel_launcherApp::Log(CString cs)
//{
//	nlinfo(cs);
//
//	/*if(!m_bLog)
//		return;
//
//	CFile	f;
//
//	if(f.Open(LOGFILE, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate))
//	{
//		f.SeekToEnd();
//		cs	+= "\r\n";
//		f.Write(cs, cs.GetLength());
//		f.Close();
//	}
//	else
//		AfxMessageBox("Cannot access/create to log file");*/
//}
