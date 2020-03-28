// Configuration.cpp: implementation of the CConfiguration class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "nel_launcher.h"
#include "Configuration.h"
#include <nel/misc/debug.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CONFIG_FILE		"nel_launcher.cfg"

#define KEY_HOST		"StartupHost"
#define KEY_VERSION		"Version"
#define KEY_URL_MAIN	"StartupPage"
#define KEY_URL_RN		"RNPage"
#define KEY_URL_NEWS	"NewsPage"
#define KEY_APPLICATION	"Application"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfiguration::CConfiguration()
{
}

CConfiguration::~CConfiguration()
{
}

BOOL CConfiguration::Load()
{
	CFile	f;
	DWORD	dwSize;
	CString	csBuffer;
	CString	csValue;

	if(f.Open(CONFIG_FILE, CFile::modeRead))
	{
		nlinfo("Opening configuration file...");

		dwSize	= f.GetLength();
		f.Read(csBuffer.GetBuffer(dwSize), dwSize);
		csBuffer.ReleaseBuffer();
		f.Close();

		// Reading the configuration file version
		GetValue(csBuffer, KEY_VERSION, csValue);
		NLMISC::fromString(csValue, m_dVersion);
		nlinfo("Config' version %s", csValue);

		if(m_dVersion < APP.m_dVersion)
		{
			nlinfo("Launcher version > config version, loading config from resources...");
			LoadFromResource();
		}
		else
		{
			nlinfo("Launcher version <= config version, loading config from configuration file...");

			HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);

			if(!GetValue(csBuffer, KEY_HOST, m_csHost))
				m_csHost.LoadString(hInst, IDS_HOST);

			if(!GetValue(csBuffer, KEY_URL_MAIN, m_csUrlMain))
				m_csUrlMain.LoadString(hInst, IDS_URLMAIN);

			if(!GetValue(csBuffer, KEY_URL_RN, m_csUrlRN))
				m_csUrlRN.LoadString(hInst, IDS_URLRN);

			if(!GetValue(csBuffer, KEY_URL_NEWS, m_csUrlNews))
				m_csUrlNews.LoadString(hInst, IDS_URLNEWS);

			if(!GetValue(csBuffer, KEY_APPLICATION, m_csApp, 0))
				m_csApp.LoadString(hInst, IDS_APPLICATION_APP);

			if(!GetValue(csBuffer, KEY_APPLICATION, m_csExe, 1))
				m_csExe.LoadString(hInst, IDS_APPLICATION_EXE);

			if(!GetValue(csBuffer, KEY_APPLICATION, m_csBasePath, 2))
				m_csBasePath.LoadString(hInst, IDS_APPLICATION_BASEPATH);

			if(!GetValue(csBuffer, KEY_APPLICATION, m_csAppBasePath, 3))
				m_csAppBasePath.LoadString(hInst, IDS_APPLICATION_APPBASEPATH);
		}
	}
	else
	{
		// No configuration file is available
		// Take configuration from exe resource
		nlinfo("No configuration file found, loading config from resources...");
		LoadFromResource();
	}
	nlinfo("Host %s", m_csHost);
	nlinfo("URL main page %s", m_csUrlMain);
	nlinfo("URL 'release notes' %s", m_csUrlRN);
	nlinfo("URL 'news' %s", m_csUrlNews);
	nlinfo("App %s", m_csApp);
	nlinfo("Exe %s", m_csExe);
	nlinfo("BasePath %s", m_csBasePath);
	nlinfo("AppBasePath %s", m_csAppBasePath);

	return TRUE;
}

void CConfiguration::LoadFromResource()
{
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);

	m_csHost.LoadString(hInst, IDS_HOST);
	m_csUrlMain.LoadString(hInst, IDS_URLMAIN);
	m_csUrlRN.LoadString(hInst, IDS_URLRN);
	m_csUrlNews.LoadString(hInst, IDS_URLNEWS);
	m_csApp.LoadString(hInst, IDS_APPLICATION_APP);
	m_csExe.LoadString(hInst, IDS_APPLICATION_EXE);
	m_csBasePath.LoadString(hInst, IDS_APPLICATION_BASEPATH);
	m_csAppBasePath.LoadString(hInst, IDS_APPLICATION_APPBASEPATH);
	m_csAppBasePath.TrimRight();
}

BOOL CConfiguration::GetValue(CString& csBuffer, CString csKey, CString& csValue, int iIndex)
{
	int		iOffset1	= 0;
	int		iOffset2;
	CString csRow;
	CString	csKeyTmp;

	while((iOffset2 = csBuffer.Find('\n', iOffset1)) != -1)
	{
		csRow	= csBuffer.Mid(iOffset1, iOffset2 - iOffset1);
		if(csRow.Right(1) == "\r")
			csRow	= csRow.Left(csRow.GetLength()-1);
		csRow.TrimLeft();
		csRow.TrimRight();

		if(csRow.Left(2) != "//" && csRow.GetLength() > 0 && csRow.Find('=') != -1)
		{
			csKeyTmp	= csRow.Mid(0, csRow.Find('='));
			csKeyTmp.TrimLeft();
			csKeyTmp.TrimRight();
			if(!csKeyTmp.CompareNoCase(csKey))
			{
				// This is the key we're looking for
				csValue	= csRow.Mid(csRow.Find('=')+1);
				csValue.TrimLeft();
				csValue.TrimRight();
				if(csValue.Left(1) == "\"")
					csValue.Delete(0);
				if(csValue.Right(1) == ";")
					csValue	= csValue.Left(csValue.GetLength()-1);
				if(csValue.Right(1) == "\"")
					csValue	= csValue.Left(csValue.GetLength()-1);
				
				if(csValue.Left(1) == "{")
				{
					// This is a key is multiple values, delete '{' and '}'
					csValue.Delete(0);
					csValue.TrimRight();
					if(csValue.Right(1) == "}")
						csValue	= csValue.Left(csValue.GetLength()-1);
					csValue.TrimLeft();
					csValue.TrimRight();

					for(int i = 0; i <= iIndex; i++)
					{
						if(csValue.IsEmpty())
							return FALSE;

						if(csValue.Left(1) == "\"")
							csValue.Delete(0);
						if(csValue.Find('"') != -1)
						{
							if(i == iIndex)
							{
								csValue	= csValue.Left(csValue.Find('\"'));
								return TRUE;
							}
							csValue	= csValue.Mid(csValue.Find('\"')+1);
							csValue.TrimLeft();
							if(csValue.Left(1) == ",")
								csValue.Delete(0);
							csValue.TrimLeft();
						}
					}
					return FALSE;
				}
				return TRUE;
			}
		}
		iOffset1	= iOffset2 + 1;
	}
	return FALSE;
}
