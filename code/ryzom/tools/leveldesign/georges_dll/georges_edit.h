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

#if !defined(AFX_GEORGES_EDIT_H__F5294DDC_52AF_4A4A_BF08_7EAD2A36B084__INCLUDED_)
#define AFX_GEORGES_EDIT_H__F5294DDC_52AF_4A4A_BF08_7EAD2A36B084__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "nel/misc/rgba.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/config_file.h"
#include "plugin_interface.h"
#include "imagelist_ex.h"

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditApp:
// See georges_edit.cpp for the implementation of this class
//

#define GEORGES_EDIT_BASE_REG_KEY "Software\\Nevrax\\Georges Edit"
#define GEORGES_EDIT_BROWSE_LABEL "--- Browse..."

extern const char* TypeFilter;
extern const char* DfnFilter;

class CGeorgesEditDoc;

// Doc template used by Georges in User mode
class CMyMultiDocTemplate : public CMultiDocTemplate
{
public:

	// Constructor
	CMyMultiDocTemplate (const char *ext, UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

	// Get dfn name for this form
	void getDfnName (std::string &dfnName);

protected:

	// Get doc string
	virtual BOOL GetDocString(CString& rString, enum DocStringIndex index) const;

	std::string _Ext;
	std::string _ExtMaj;
};

class CGeorgesEditApp : public CWinApp, public NLGEORGES::IEdit
{
public:
	CGeorgesEditApp();
	~CGeorgesEditApp();

	// Is a super user ?
	bool			Superuser;

	// Path of georges_exe.exe
	std::string		ExePath;

	// Root of search path for the forms, dfn and types
	std::string		RootSearchPath;

	// Type and dfn sub directory relative to RootSearchPath
	std::string		TypeDfnSubDirectory;

	// Default type inserted in list
	std::string		DefaultType;

	// Default type inserted in list
	std::string		DefaultDfn;

	// Remember list size per widget
	uint			RememberListSize;

	// Max undo backup
	uint			MaxUndo;

	// If true, expand document's content node at loading
	bool			StartExpanded;

	// Georges for CVS
	bool			Georges4CVS;

	// Clipboards ID
	UINT			FormClipBoardFormatStruct;
	UINT			FormClipBoardFormatVirtualStruct;
	UINT			FormClipBoardFormatArray;
	UINT			FormClipBoardFormatType;

	// Plugins info
	std::vector<std::string>	PluginsNames;
	std::vector<std::string>	UserTypes;

	// Resize and save main window size
	bool			ResizeMain;
	bool			ExeStandalone;

	// The config file loaded.
	NLMISC::CConfigFile		ConfigFile;

public:
	// Memory stream
	NLMISC::CMemStream	MemStream;
	bool				FillMemStreamWithClipboard (const char *formName, CGeorgesEditDoc *doc, uint slot);
	void				FillMemStreamWithBuffer (const uint8 *buffer, uint size);

	bool				SerialIntoMemStream (const char *formName, CGeorgesEditDoc *doc, uint slot, bool copyToClipboard);
	bool				SerialFromMemStream (const char *formName, CGeorgesEditDoc *doc, uint slot);

	// Init
	BOOL initInstance (int nCmdShow, bool exeStandalone, int x, int y, int cx, int cy);

	// On new view updated
	void onNewDocView (CGeorgesEditDoc *doc);

	// Is the plugin activated ?
	bool isPluginActivated (NLGEORGES::IEditPlugin *plugin) const;

	// From IEdit
	NLGEORGES::IEditDocument *getActiveDocument ();
	NLGEORGES::IEditDocument *createDocument (const char *dfnName, const char *pathName);
	virtual void getSearchPath (std::string &searchPath);
	virtual NLMISC::CConfigFile		&getConfigFile()				{	return ConfigFile; }

	// Save / restaure state
	void	saveState ();
	void	loadState ();

	// Save the doc templace
	CMultiDocTemplate	*_TemplateForm;
	CMultiDocTemplate	*_TemplateType;
	CMultiDocTemplate	*_TemplateDfn;

	// The image list
	CImageListEx		ImageList;

	// Get a template form
	CMultiDocTemplate	*getFormDocTemplate (const char *dfnName);

	void	saveWindowState (const CWnd *wnd, const char *name, bool controlBar);
	void	loadWindowState (CWnd *wnd, const char *name, bool changeShowWindow, bool controlBar);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesEditApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	virtual BOOL OnDDECommand(LPTSTR lpszCommand);
	//}}AFX_VIRTUAL

	// Config functions
	bool	loadCfg ();
	bool	saveCfg ();
	void	initCfg ();
	void	loadPlugins ();
	void	releasePlugins ();

	// Dialog function
	void	outputError (const char* message);
	bool	yesNo (const char* message);
	bool	getColor (NLMISC::CRGBA &color);

	// Browse an URL
	void	gotoURL (LPCTSTR url);

	// Utility function
	static void	getConfigFilePath (std::string &output);

	// Plugin list
	class CPlugin
	{
	public:
		// Constructor
		CPlugin (HINSTANCE hModule, NLGEORGES::IEditPlugin *plugin);

		// Activated ?
		bool					Activated;

		// Module of the plugin
		HINSTANCE				PluginModule;

		// Plugin interface
		NLGEORGES::IEditPlugin	*PluginInterface;
	};
	std::vector<CPlugin>	PluginArray;


// Implementation
	//{{AFX_MSG(CGeorgesEditApp)
	afx_msg void OnAppAbout();
	afx_msg void OnViewRefresh();
	afx_msg void OnFileSaveAll();
	afx_msg void OnFileCloseAll();
	afx_msg void OnUpdateFileSaveAll(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CGeorgesEditApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGES_EDIT_H__F5294DDC_52AF_4A4A_BF08_7EAD2A36B084__INCLUDED_)
