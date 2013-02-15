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

// ---------------------------------------------------------------------------
// MainFrm.h : interface of the CMainFrame class
// ---------------------------------------------------------------------------

#if !defined(AFX_MAINFRM_H__D9ABC57D_9514_49B1_A65F_1CC64C6D6BB6__INCLUDED_)
#define AFX_MAINFRM_H__D9ABC57D_9514_49B1_A65F_1CC64C6D6BB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ---------------------------------------------------------------------------

#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include "MasterTree.h"
#include <string>

#include "../export/export.h"

#include "../../3d/ligo/worldeditor/worldeditor_interface.h" // MasterCB

#include "easy_cfg.h"

// ---------------------------------------------------------------------------
// Interface to the tools
// ---------------------------------------------------------------------------

class IWorldEditor;
class IGeorges;
class ILogicEditor;
class CMainFrame;
class CExportCBDlg;
// ---------------------------------------------------------------------------

// Do not parse following directories
#define MAX_SYS_DIR 7
extern char *gSysDir[MAX_SYS_DIR];
// Do not display files with those extensions
#define MAX_INVALID_EXT 1
extern char *gInvalidExt[MAX_INVALID_EXT];

// ---------------------------------------------------------------------------

struct SEnvironnement : public IEasyCFG
{
	// Master params
	sint32 MasterX, MasterY;
	sint32 MasterTreeX, MasterTreeY;
	sint32 MasterTreeCX, MasterTreeCY;
	bool MasterTreeLocked;
	std::string ContinentsDir;
	std::string DefaultDFNDir;
	std::string DefaultGameElemDir;
	
	// WorldEditor params
	bool WorldEdOpened;
	sint32 WorldEdX, WorldEdY, WorldEdCX, WorldEdCY;

	// Georges params
	bool GeorgesOpened;
	sint32 GeorgesX, GeorgesY, GeorgesCX, GeorgesCY;

	// LogicEditor params
	bool LogicEditorOpened;
	sint32 LogicEditorX, LogicEditorY, LogicEditorCX, LogicEditorCY;

	// Export params
	SExportOptions ExportOptions;

	SEnvironnement ();

	bool load (const std::string &filename);
	bool save (const std::string &filename);
};


// ---------------------------------------------------------------------------

class CMasterCB : public IMasterCB
{
	CMainFrame					*_MainFrame;
	std::vector<std::string>	_PrimZoneList;
	std::string					_Text;
	std::vector<std::string>	_GroupPrimList;

public:
	
	CMasterCB ();
	void setMainFrame (CMainFrame*pMF); // Link to master

	// Accessors
	std::vector<std::string> &getAllPrimZoneNames ();

	// Overridables
	// setAllPrimZoneNames : called when the list of patatoid changes
	virtual void setAllPrimZoneNames (std::vector<std::string> &primZoneList);
	// transfert : called when WE want to transfert text to georges
	virtual void multiTransfert (const std::vector<std::string> &vText, bool append);
	virtual void transfert (const std::string &sText);
	// Selection line up and down
	virtual void lineUp ();
	virtual void lineDown ();
};

// ---------------------------------------------------------------------------

class CMainFrame : public CFrameWnd
{
	SEnvironnement	_Environnement;

	IWorldEditor	*_WorldEditor;
	HMODULE			_WorldEditorModule;

	IGeorges		*_Georges;
	HMODULE			_GeorgesModule;

	ILogicEditor	*_LogicEditor;
	HMODULE			_LogicEditorModule;

	CMasterTreeDlg	*_Tree;

	std::string		_ActivePath;
	CMasterCB		_MasterCB;

	std::string		_MasterExeDir;

	CExport			*_Export;
	CExportCBDlg	*_ExportCBDlg;

public:

	CMainFrame ();
	virtual ~CMainFrame ();

	void getAllInterfaces (); // Load all dlls and get tools interfaces
	void releaseAllInterfaces ();

	void openFile (const std::string &fname);
	void openDir (const std::string &fname);
	void openDirParse (const std::string &sBaseName, const std::string &sRelativeName);

	void openContinentCfgFile (const std::string &filename);

	// ***********
	// WORLDEDITOR
	// ***********

	void openWorldEditor ();
	void openWorldEditorFile (const std::string &fileName);
	void closeWorldEditor ();

	// *******
	// GEORGES
	// *******

	void openGeorges ();
	void openGeorgesFile (const std::string &fileName);
	void closeGeorges ();

	void georgesSetPathesFromActive (); // Set Path from _ActiveContinent/_ActiveWorld/etc...
	void georgesPutGroupText (const std::vector<std::string> &vText, bool append);
	void georgesPutText (const std::string &sText);
	void georgesLineUp ();
	void georgesLineDown ();
	void georgesCreatePlantName ();

	// ***********
	// LOGICEDITOR
	// ***********

	void openLogicEditor ();
	void openLogicEditorFile (const std::string &fileName);
	void closeLogicEditor ();

	// *****
	// TOOLS
	// *****

	// Tree and directories manipulation

	bool createDirIfNotExist (const std::string& dirName, const std::string& errorMsg);
	void deltree (const std::string &dirName);
	void copytree (const std::string &srcDir, const std::string &dstDir);
	void displayLastErrorDialog ();
	void updateTree ();

	// Continent methods 
	// -----------------

	void continentNew ();

	void continentOpen			(const std::string &contName);
	void continentProperties	(const std::string &contName);
	void continentNewRegion		(const std::string &contName);
	void continentDelete		(const std::string &contName);

	// Active path
	void setActivePath			(const std::string &contName);
	std::string getActivePath	()								{ return _ActivePath; }

	// Region methods
	// --------------

	void regionOpen			(const std::string &path);
	void regionRename		(const std::string &path);
	void regionNewPrim		(const std::string &path);
	void regionNewGeorges	(const std::string &path);
	void regionNewSubRegion (const std::string &path);
	void regionDelete		(const std::string &path);

	// File methods
	// ------------

	void fileOpen	(const std::string &sFileFullName);
	void fileRename (const std::string &sFileFullName);
	void fileDelete (const std::string &sFileFullName);

	void copy		(const std::string &sPathSrc, const std::string &sPathDst);

#ifdef _DEBUG
	virtual void AssertValid () const;
	virtual void Dump (CDumpContext& dc) const;
#endif

	virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage (MSG*pMsg);

protected:

	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg void OnMove (int x, int y);
	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd (CDC* pDC);

protected:

	// Menu methods

	afx_msg void onContinentSave ();
	afx_msg void onContinentExport ();

	afx_msg void OnContinentDelete ();

	afx_msg void onOptionsTreeLock ();
	afx_msg void onOptionsSetContinentsDir ();
	afx_msg void onOptionsSetDefaultDFNDir ();
	afx_msg void onOptionsSetDefaultGameElemDir ();


	afx_msg void onWindowsWorldEditor ();
	afx_msg void onWindowsGeorges ();
	afx_msg void onWindowsLogicEditor ();
	afx_msg void onWindowsReset ();

	afx_msg void OnClose ();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__D9ABC57D_9514_49B1_A65F_1CC64C6D6BB6__INCLUDED_)
