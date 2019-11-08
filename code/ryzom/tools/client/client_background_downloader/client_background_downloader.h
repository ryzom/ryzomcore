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

// client_background_downloader.h : main header file for the CLIENT_BACKGROUND_DOWNLOADER application
//

#if !defined(AFX_CLIENT_BACKGROUND_DOWNLOADER_H__C4688976_6D8B_4811_81AA_13C3C6CEC789__INCLUDED_)
#define AFX_CLIENT_BACKGROUND_DOWNLOADER_H__C4688976_6D8B_4811_81AA_13C3C6CEC789__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "../client/login_patch.h"

/////////////////////////////////////////////////////////////////////////////
// CClient_background_downloaderApp:
// See client_background_downloader.cpp for the implementation of this class
//

class CClient_background_downloaderApp : public CWinApp
{
public:
	CClient_background_downloaderApp();
	~CClient_background_downloaderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClient_background_downloaderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CClient_background_downloaderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void setCloseWarningFlag(BOOL enabled);
	BOOL getCloseWarningFlag() const;

	static uint computePatchSize(const CPatchManager::SPatchInfo &InfoOnPatch, bool patchMainland);

	bool parseCommandLine(const std::string &commandLine);
	// get language chosen from the config file
	std::string getLanguage();
	// Load the config file. Title is needed at that point because no translation file could possibly be loaded
	void loadConfigFile(HWND parent, const char *appTitle);
	

public:
	static const char *AppRegEntry;
	static const char *RegKeyDisplayCloseWarning;

	std::vector<std::string> PatchURIs;
	std::string				 ServerPath;
	std::string				 ServerVersion;

	NLMISC::CConfigFile		 ConfigFile;
private:
	void initLog(const char *appName);
};


extern CClient_background_downloaderApp theApp;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENT_BACKGROUND_DOWNLOADER_H__C4688976_6D8B_4811_81AA_13C3C6CEC789__INCLUDED_)
