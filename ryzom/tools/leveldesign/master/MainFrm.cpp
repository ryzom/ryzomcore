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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "master.h"

#include "MainFrm.h"
//#include "RegionPropertiesDlg.h"
//#include "NewGeorgesFormDlg.h"
//#include "PrimNameDlg.h"
#include "NameEditDlg.h"
#include "ChooseTag.h"
#include "ChooseDir.h"
#include "exportdlg.h"
#include "exportcbdlg.h"
#include "continentcfg.h"
#include "continentPropertiesDlg.h"

#include "../georges_dll/georges_interface.h"
#include "../logic_editor_dll/logic_editor_interface.h"

#include "nel/misc/file.h"
#include "nel/misc/stream.h"

using namespace NLMISC;
using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NL_NEW
	#undef new
#endif
	
/////////////////////////////////////////////////////////////////////////////

// Do not parse following directories
char *gSysDir[MAX_SYS_DIR] = 
{
	".",
	"..",
	"ZoneBitmaps",
	"ZoneLigos",
	"dfn",
	"tmp",
	"cvs"
};

// Do not display files with those extensions
#define MAX_INVALID_EXT 1
char *gInvalidExt[MAX_INVALID_EXT] = 
{
	".log"
};


/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
SEnvironnement::SEnvironnement()
{
	MasterX				= 0;
	MasterY				= 0;
	MasterTreeX			= 0;
	MasterTreeY			= 0;
	MasterTreeCX		= 100;
	MasterTreeCY		= 100;
	MasterTreeLocked	= true;

	char tmp[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, tmp);
	ContinentsDir	= tmp;
	ContinentsDir += "\\Continents\\";

	WorldEdOpened = false;
	WorldEdX = 50;
	WorldEdY = 50;
	WorldEdCX = 600;
	WorldEdCY = 400;

	GeorgesOpened = false;
	GeorgesX = 50;
	GeorgesY = 50;
	GeorgesCX = 300;
	GeorgesCY = 300;

	LogicEditorOpened = false;
	LogicEditorX = 50;
	LogicEditorY = 50;
	LogicEditorCX = 300;
	LogicEditorCY = 300;
}

// ---------------------------------------------------------------------------
bool SEnvironnement::load (const std::string &filename)
{
	if (!openRead(filename))
		return false;
	// Master
	MasterX =				getInt	("MASTA_PosX");
	MasterY =				getInt	("MASTA_PosY");
	MasterTreeX =			getInt	("MASTA_TreePosX");
	MasterTreeY =			getInt	("MASTA_TreePosY");
	MasterTreeCX =			getInt	("MASTA_TreeSizeX");
	MasterTreeCY =			getInt	("MASTA_TreeSizeY");
	MasterTreeLocked =		getBool ("MASTA_TreeLocked");
	ContinentsDir =			getStr	("MASTA_ContinentsDir");
	DefaultDFNDir =			getStr	("MASTA_DefaultDFNDir");
	DefaultGameElemDir =	getStr	("MASTA_DefaultGameElemDir");

	// WorldEditor
	WorldEdOpened =			getBool	("WE_Opened");
	WorldEdX =				getInt	("WE_PosX");
	WorldEdY =				getInt	("WE_PosY");
	WorldEdCX =				getInt	("WE_SizeX");
	WorldEdCY =				getInt	("WE_SizeY");

	// Georges
	GeorgesOpened =			getBool	("GG_Opened");
	GeorgesX =				getInt	("GG_PosX");
	GeorgesY =				getInt	("GG_PosY");
	GeorgesCX =				getInt	("GG_SizeX");
	GeorgesCY =				getInt	("GG_SizeY");

	// LogicEditor
	LogicEditorOpened =		getBool	("LE_Opened");
	LogicEditorX =			getInt	("LE_PosX");
	LogicEditorY =			getInt	("LE_PosY");
	LogicEditorCX =			getInt	("LE_SizeX");
	LogicEditorCY =			getInt	("LE_SizeY");

	// Export options
	ExportOptions.loadcf (*cf);
	close ();
	return true;
}

// ---------------------------------------------------------------------------
bool SEnvironnement::save (const std::string &filename)
{
	if (!openWrite(filename))
		return false;

	putCommentLine ("------");
	putCommentLine ("Master");
	putCommentLine ("------");
	putInt	("MASTA_PosX",				MasterX);
	putInt	("MASTA_PosY",				MasterY);
	putInt	("MASTA_TreePosX",			MasterTreeX);
	putInt	("MASTA_TreePosY",			MasterTreeY);
	putInt	("MASTA_TreeSizeX",			MasterTreeCX);
	putInt	("MASTA_TreeSizeY",			MasterTreeCY);
	putBool	("MASTA_TreeLocked",		MasterTreeLocked);
	putStr	("MASTA_ContinentsDir",		ContinentsDir);
	putStr	("MASTA_DefaultDFNDir",		DefaultDFNDir);
	putStr	("MASTA_DefaultGameElemDir",DefaultGameElemDir);

	putCommentLine ("-----------");
	putCommentLine ("WorldEditor");
	putCommentLine ("-----------");
	putBool ("WE_Opened",	WorldEdOpened);
	putInt	("WE_PosX",		WorldEdX);
	putInt	("WE_PosY",		WorldEdY);
	putInt	("WE_SizeX",	WorldEdCX);
	putInt	("WE_SizeY",	WorldEdCY);

	putCommentLine ("-------");
	putCommentLine ("Georges");
	putCommentLine ("-------");
	putBool ("GG_Opened",	GeorgesOpened);
	putInt	("GG_PosX",		GeorgesX);
	putInt	("GG_PosY",		GeorgesY);
	putInt	("GG_SizeX",	GeorgesCX);
	putInt	("GG_SizeY",	GeorgesCY);

	putCommentLine ("-----------");
	putCommentLine ("LogicEditor");
	putCommentLine ("-----------");
	putBool ("LE_Opened",	LogicEditorOpened);
	putInt	("LE_PosX",		LogicEditorX);
	putInt	("LE_PosY",		LogicEditorY);
	putInt	("LE_SizeX",	LogicEditorCX);
	putInt	("LE_SizeY",	LogicEditorCY);

	// Export options
	ExportOptions.save (f);
	close();
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CMasterCB
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
CMasterCB::CMasterCB ()
{
	_MainFrame = NULL;
}

// ---------------------------------------------------------------------------
void CMasterCB::setMainFrame (CMainFrame*pMF)
{
	_MainFrame = pMF;
}

// ---------------------------------------------------------------------------
vector<string> &CMasterCB::getAllPrimZoneNames ()
{
	return _PrimZoneList;
}

// ---------------------------------------------------------------------------
void CMasterCB::setAllPrimZoneNames (vector<string> &primZoneList)
{
	_PrimZoneList = primZoneList;
	//_MainFrame->georgesUpdatePatatoid();
}

// ---------------------------------------------------------------------------
void CMasterCB::multiTransfert (const std::vector<std::string> &vText, bool append)
{
	_GroupPrimList = vText;
	_MainFrame->georgesPutGroupText (_GroupPrimList, append);
}

// ---------------------------------------------------------------------------
void CMasterCB::transfert (const string &sText)
{
	_Text = sText;
	_MainFrame->georgesPutText (_Text);
}

// ---------------------------------------------------------------------------
void CMasterCB::lineUp ()
{
	_MainFrame->georgesLineUp ();
}

// ---------------------------------------------------------------------------
void CMasterCB::lineDown ()
{
	_MainFrame->georgesLineDown ();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame
/////////////////////////////////////////////////////////////////////////////

//IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !

	ON_COMMAND(ID_CONTINENT_SAVE, onContinentSave)
	ON_COMMAND(ID_CONTINENT_EXPORT, onContinentExport)

	//ON_COMMAND(ID_CONTINENT_DELETE, OnContinentDelete)

	ON_COMMAND(ID_OPTIONS_TREELOCK, onOptionsTreeLock)

	ON_COMMAND(ID_OPTIONS_SETCONTINENTSDIR, onOptionsSetContinentsDir)
	ON_COMMAND(ID_OPTIONS_SETDEFAULTDFNDIR, onOptionsSetDefaultDFNDir)
	ON_COMMAND(ID_OPTIONS_SETDEFAULTGAMEELEMDIR, onOptionsSetDefaultGameElemDir)

	ON_COMMAND(ID_WINDOWS_WORLDEDITOR, onWindowsWorldEditor)
	ON_COMMAND(ID_WINDOWS_GEORGES, onWindowsGeorges)
	ON_COMMAND(ID_WINDOWS_LOGICEDITOR, onWindowsLogicEditor)
	ON_COMMAND(ID_WINDOWS_RESET, onWindowsReset)

	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

// ---------------------------------------------------------------------------
CMainFrame::CMainFrame()
{
	_WorldEditor = NULL;
	_Georges = NULL;
	_LogicEditor = NULL;
	_Tree = NULL;
	_Export = NULL;
	_ExportCBDlg = NULL;
}

// ---------------------------------------------------------------------------
CMainFrame::~CMainFrame()
{
	delete _Export;
}

// ---------------------------------------------------------------------------
// Used by getAllInterfaces method to retrieve all DLLs and tools Interfaces

typedef IWorldEditor* (*IWORLDEDITOR_GETINTERFACE)(int version);
const char *IWORLDEDITOR_GETINTERFACE_NAME = "IWorldEditorGetInterface";
typedef void (*IWORLDEDITOR_RELINTERFACE)(IWorldEditor*pWE);
const char *IWORLDEDITOR_RELINTERFACE_NAME = "IWorldEditorReleaseInterface";

typedef IGeorges* (*IGEORGES_GETINTERFACE)(int version);
const char *IGEORGES_GETINTERFACE_NAME = "IGeorgesGetInterface";
typedef void (*IGEORGES_RELINTERFACE)(IGeorges *pGeorges);
const char *IGEORGES_RELINTERFACE_NAME = "IGeorgesReleaseInterface";

typedef ILogicEditor* (*ILOGICEDITOR_GETINTERFACE)(int version);
const char *ILOGICEDITOR_GETINTERFACE_NAME = "ILogicEditorGetInterface";
typedef void (*ILOGICEDITOR_RELINTERFACE)(ILogicEditor *pLogicEditor);
const char *ILOGICEDITOR_RELINTERFACE_NAME = "ILogicEditorReleaseInterface";

// ---------------------------------------------------------------------------
void CMainFrame::getAllInterfaces ()
{
	SetCurrentDirectory (_MasterExeDir.c_str());

	// Get WorldEditor Interface
	if (_WorldEditor == NULL)
	{
		IWORLDEDITOR_GETINTERFACE IWEGetInterface = NULL;

		#if defined NL_RELEASE_DEBUG
			_WorldEditorModule = AfxLoadLibrary ("WorldEditor_rd.dll");
		#elif defined NL_DEBUG_FAST
			_WorldEditorModule = AfxLoadLibrary ("WorldEditor_df.dll");
		#elif defined _DEBUG
			_WorldEditorModule = AfxLoadLibrary ("WorldEditor_debug.dll");
		#elif defined NDEBUG
			_WorldEditorModule = AfxLoadLibrary ("WorldEditor.dll");
		#endif
		if (_WorldEditorModule != NULL)
		{
			IWEGetInterface = (IWORLDEDITOR_GETINTERFACE)::GetProcAddress (_WorldEditorModule, IWORLDEDITOR_GETINTERFACE_NAME);
			if (IWEGetInterface != NULL)
				_WorldEditor = IWEGetInterface (WORLDEDITOR_VERSION);
		}
		else
		{
			MessageBox("WorldEditor dll not Loaded", "Warning");
		}
	}

	// Get Georges Interface
	if (_Georges == NULL)
	{
		IGEORGES_GETINTERFACE IGGetInterface = NULL;

		#if defined NL_RELEASE_DEBUG
			_GeorgesModule = AfxLoadLibrary ("Georges_dll_rd.dll");
		#elif defined NL_DEBUG_FAST
			_GeorgesModule = AfxLoadLibrary ("Georges_dll_debug_fast.dll");
		#elif defined _DEBUG
			_GeorgesModule = AfxLoadLibrary ("Georges_dll_debug.dll");
		#elif defined NDEBUG
			_GeorgesModule = AfxLoadLibrary ("Georges_dll.dll");
		#endif
		if (_GeorgesModule != NULL)
		{
			IGGetInterface = (IGEORGES_GETINTERFACE)::GetProcAddress (_GeorgesModule, IGEORGES_GETINTERFACE_NAME);
			if (IGGetInterface != NULL)
				_Georges = IGGetInterface (GEORGES_VERSION);
		}
		else
		{
			MessageBox("Georges dll not Loaded", "Warning");
		}
	}

	// Get LogicEditor Interface
	if (_LogicEditor == NULL)
	{
		ILOGICEDITOR_GETINTERFACE ILEGetInterface = NULL;

		#if defined NL_RELEASE_DEBUG
			_LogicEditorModule = AfxLoadLibrary ("Logic_Editor_rd.dll");
		#elif defined NL_DEBUG_FAST
			_LogicEditorModule = AfxLoadLibrary ("Logic_Editor_df.dll");
		#elif defined _DEBUG
			_LogicEditorModule = AfxLoadLibrary ("Logic_Editor_debug.dll");
		#elif defined NDEBUG
			_LogicEditorModule = AfxLoadLibrary ("Logic_Editor.dll");
		#endif
		if (_LogicEditorModule != NULL)
		{
			ILEGetInterface = (ILOGICEDITOR_GETINTERFACE)::GetProcAddress (_LogicEditorModule, ILOGICEDITOR_GETINTERFACE_NAME);
			if (ILEGetInterface != NULL)
				_LogicEditor = ILEGetInterface (LOGIC_EDITOR_VERSION);
		}
		else
		{
			MessageBox("LogicEditor dll not Loaded", "Warning");
		}
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::releaseAllInterfaces()
{
	// Release the WorldEditor interface
	if (_WorldEditor != NULL)
	{
		IWORLDEDITOR_RELINTERFACE IWERelInterface = NULL;
		IWERelInterface = (IWORLDEDITOR_RELINTERFACE)::GetProcAddress (_WorldEditorModule, IWORLDEDITOR_RELINTERFACE_NAME);
		IWERelInterface (_WorldEditor);
	}

	// Release the Georges interface
	if (_Georges != NULL)
	{
		IGEORGES_RELINTERFACE IGRelInterface = NULL;
		IGRelInterface = (IGEORGES_RELINTERFACE)::GetProcAddress (_GeorgesModule, IGEORGES_RELINTERFACE_NAME);
		IGRelInterface (_Georges);
	}

	// Release the LogicEditor interface
	if (_LogicEditor != NULL)
	{
		ILOGICEDITOR_RELINTERFACE ILERelInterface = NULL;
		ILERelInterface = (ILOGICEDITOR_RELINTERFACE)::GetProcAddress (_LogicEditorModule, ILOGICEDITOR_RELINTERFACE_NAME);
		ILERelInterface (_LogicEditor);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::openFile (const string &fname)
{
	char curdir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curdir);

	string sFullName;
	sFullName = _Environnement.ContinentsDir;

	sFullName += fname;

	int size = fname.size();

	if ((size >= 13) && (stricmp(&fname[size-13],"continent.cfg") == 0))
	{
		openContinentCfgFile (sFullName.c_str());
	}
	else if ((stricmp(&fname[size-5],".prim") == 0) || (stricmp(&fname[size-5],".land") == 0))
	{
		openWorldEditor ();
		openWorldEditorFile (sFullName.c_str());
	}
	else if (stricmp(&fname[size-6],".logic") == 0)
	{
		openLogicEditor ();
		openLogicEditorFile (sFullName.c_str());
	}	
	else
	{
		openGeorges ();
		openGeorgesFile (sFullName.c_str());
	}

	SetCurrentDirectory (curdir);
}

// ---------------------------------------------------------------------------
void CMainFrame::openDir (const string &fname)
{
	if (_ActivePath == "") return;
	
	char curdir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curdir);

	string sBaseName;
	sBaseName = _Environnement.ContinentsDir;

	openDirParse (sBaseName, fname);

	SetCurrentDirectory (curdir);
}

// ---------------------------------------------------------------------------
void CMainFrame::openDirParse (const std::string &sBaseNameDir, const std::string &sRelativeNameDir)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;

	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	string sFullNamedDir = sBaseNameDir + sRelativeNameDir; 
	if (!SetCurrentDirectory (sFullNamedDir.c_str()))
	{
		SetCurrentDirectory (sCurDir);
		return;
	}
	
	hFind = FindFirstFile ("*.*", &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (GetFileAttributes(findData.cFileName)&FILE_ATTRIBUTE_DIRECTORY)
		{
			// Look if the name is a system directory
			bool bFound = false;
			for (uint32 i = 0; i < MAX_SYS_DIR; ++i)
				if (stricmp (findData.cFileName, gSysDir[i]) == 0)
				{
					bFound = true;
					break;
				}
			if (!bFound) // No, ok lets recurse it
			{
				string newPath = sRelativeNameDir + string("\\") + string(findData.cFileName);
				openDirParse (sBaseNameDir, newPath);
			}
		}
		else
		{
			// Check for invalid extension
			bool bFound = false;
			for (uint32 j = 0; j < MAX_INVALID_EXT; ++j)
				if (strlen(findData.cFileName) > strlen(gInvalidExt[j]))
				if (stricmp(&findData.cFileName[strlen(findData.cFileName)-strlen(gInvalidExt[j])], gInvalidExt[j]) == 0)
				{
					bFound = true;
					break;
				}
			// If the extension is an invalid one -> Do not open the file
			if (!bFound)
			{
				string fileName = sRelativeNameDir + "\\" + findData.cFileName;
				openFile (fileName.c_str());
			}
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);

	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::openContinentCfgFile (const string &filename)
{
	SContinentCfg cfg;
	cfg.load (filename);
	_WorldEditor->setDataDir (cfg.LandDir.c_str());
	openWorldEditor ();
	openWorldEditorFile (cfg.LandFile.c_str());
}

// ---------------------------------------------------------------------------
void CMainFrame::openWorldEditor()
{
	if (_Environnement.WorldEdOpened)
		return;
	_Environnement.WorldEdOpened = true;
	if (_WorldEditor != NULL)
		_WorldEditor->initUILight (_Environnement.WorldEdX, _Environnement.WorldEdY, 
								_Environnement.WorldEdCX, _Environnement.WorldEdCY);
	GetMenu()->CheckMenuItem (ID_WINDOWS_WORLDEDITOR, MF_CHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::openWorldEditorFile (const string &fileName)
{
	_WorldEditor->loadFile (fileName.c_str());
}

// ---------------------------------------------------------------------------
void CMainFrame::closeWorldEditor()
{
	if (_WorldEditor == NULL) return;
	if (!_Environnement.WorldEdOpened) return;

	//onContinentSave ();
	_Environnement.WorldEdOpened = false;
	RECT r;
	CFrameWnd *pFW = (CFrameWnd*)_WorldEditor->getMainFrame();
	pFW->GetWindowRect(&r);
	WINDOWPLACEMENT wp;
	pFW->GetWindowPlacement (&wp);
	if (wp.showCmd == SW_SHOWNORMAL)
	{
		_Environnement.WorldEdY = r.top;
		_Environnement.WorldEdX = r.left;
		_Environnement.WorldEdCY = r.bottom - r.top;
		_Environnement.WorldEdCX = r.right - r.left;
	}
	_WorldEditor->releaseUI ();
	GetMenu()->CheckMenuItem (ID_WINDOWS_WORLDEDITOR, MF_UNCHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::openGeorges ()
{
	if (_Georges == NULL) return;
	if (_Environnement.GeorgesOpened)
		return;
	_Environnement.GeorgesOpened = true;
	if (_Georges != NULL)
	{
		_Georges->initUILight (SW_SHOW, _Environnement.GeorgesX, _Environnement.GeorgesY, 
								_Environnement.GeorgesCX, _Environnement.GeorgesCY);
	}
	GetMenu()->CheckMenuItem (ID_WINDOWS_GEORGES, MF_CHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::openGeorgesFile (const string &fileName)
{
	if (_Georges == NULL) return;
	georgesSetPathesFromActive ();
	_Georges->LoadDocument (fileName);
}

// ---------------------------------------------------------------------------
void CMainFrame::closeGeorges ()
{
	if (_Georges == NULL) return;
	if (!_Environnement.GeorgesOpened) return;

	_Environnement.GeorgesOpened = false;
	RECT r;
	CFrameWnd *pFW = (CFrameWnd*)_Georges->getMainFrame();
	pFW->GetWindowRect(&r);
	WINDOWPLACEMENT wp;
	pFW->GetWindowPlacement (&wp);
	if (wp.showCmd == SW_SHOWNORMAL)
	{
		_Environnement.GeorgesY = r.top;
		_Environnement.GeorgesX = r.left;
		_Environnement.GeorgesCY = r.bottom - r.top;
		_Environnement.GeorgesCX = r.right - r.left;
	}
	_Georges->releaseUI ();
	GetMenu()->CheckMenuItem (ID_WINDOWS_GEORGES, MF_UNCHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::georgesSetPathesFromActive ()
{
	if (_Georges == NULL) return;
	if (_ActivePath == "")
	{
		string sDir;
		sDir = _Environnement.ContinentsDir;
		_Georges->SetDirLevel (sDir);

		_Georges->SetDirDfnTyp (_Environnement.DefaultDFNDir);
		_Georges->SetDirPrototype (_Environnement.DefaultGameElemDir);
	}
	else
	{
		string sDir;
	
		int i = 0;
		if (_ActivePath[i] == '\\') ++i;
		for (; i < (int)_ActivePath.size(); ++i)
		{
			if (_ActivePath[i] == '\\') break;
			sDir += _ActivePath[i];
		}

		sDir = _Environnement.ContinentsDir + sDir + "\\";

		_Georges->SetDirLevel (sDir);

		SContinentCfg cfg;
		string sTmp = sDir + "continent.cfg";
		cfg.load (sTmp.c_str());
		_Georges->SetDirDfnTyp (cfg.DfnDir);
		_Georges->SetDirPrototype (cfg.GameElemDir);
	}
}

// ---------------------------------------------------------------------------
/*
void CMainFrame::georgesUpdatePatatoid ()
{
	if (_Georges == NULL) return;
	if ((_ActiveWorld == "") || (_ActiveContinent == "") || (_ActiveRegion == "")) return;

	string sCurDir;
	if (_ActiveWorld == "Continents")
		sCurDir = _Environnement.ContinentsDir;
	else if (_ActiveWorld == "Trash")
		sCurDir = _Environnement.TrashDir;
	else if (_ActiveWorld == "Backup")
		sCurDir = _Environnement.BackupDir;
	sCurDir += _ActiveContinent + "\\" + _ActiveRegion + "\\";
	_Georges->SetDirLevel (sCurDir);

	if (_ActiveContinent != "")
		_Georges->SetTypPredef (sCurDir + "tmp\\patat_name.typ", _MasterCB.getAllPrimZoneNames());
}
*/

// ---------------------------------------------------------------------------
void CMainFrame::georgesPutGroupText (const std::vector<std::string> &vText, bool append)
{
	if (_Georges == NULL) return;
	_Georges->PutGroupText (vText, append);
}

// ---------------------------------------------------------------------------
void CMainFrame::georgesPutText (const std::string &sText)
{
	if (_Georges == NULL) return;
	_Georges->PutText (sText);
}

// ---------------------------------------------------------------------------
void CMainFrame::georgesLineUp ()
{
	if (_Georges == NULL) return;
	_Georges->LineUp ();
}

// ---------------------------------------------------------------------------
void CMainFrame::georgesLineDown ()
{
	if (_Georges == NULL) return;
	_Georges->LineDown ();
}

// ---------------------------------------------------------------------------
/*
void CMainFrame::georgesCreateFilesWhenNewRegion ()
{
	// When we arrive in this function the current directory is the new Continent being created
	if (_Georges == NULL)
		return;

	CreateDirectory ("tmp", NULL);
	char curdir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curdir);
	string fullname = string(curdir) + "\\tmp\\patat_name.typ";
	vector< pair< string, string > > lpsx;
	vector< pair< string, string > > lpsx2;
	lpsx.push_back (make_pair(string("PatatFrite"),	string("PatatFrite")));
	lpsx.push_back (make_pair(string("PatatVapeur"), string("PatatVapeur")));
	_Georges->MakeTyp (fullname, "String", "PATAT", "true", "", "", "PatatFrite", &lpsx, &lpsx2);
}
*/

// ---------------------------------------------------------------------------
void CMainFrame::georgesCreatePlantName ()
{
	if (_Georges == NULL) return;
	WIN32_FIND_DATA fdTmp;
	char curdir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curdir);

	try
	{
		string plantname = _Environnement.DefaultDFNDir;
		if (!SetCurrentDirectory(plantname.c_str()))
			return;

		plantname += "tmp\\plant_name.typ";

		// If plantname file do not already exists
		HANDLE hFind = FindFirstFile(plantname.c_str(), &fdTmp);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose (hFind);
			if (!DeleteFile (plantname.c_str()))
			{
				MessageBox (plantname.c_str(), "Cannot overwrite");
				return;
			}
		}
		
		vector< pair< string, string > > lpsx;
		vector< pair< string, string > > lpsx2;
		lpsx.push_back (std::make_pair(string("xxx.plant"),	string("xxx.plant")));
		lpsx.push_back (std::make_pair(string("yyy.plant"), string("yyy.plant")));
	 	_Georges->MakeTyp (plantname, String, FileBrowser, "", "", "xxx.plant", &lpsx);

		// Parse the plant directory and add all these predef
		string plantdir = _Environnement.DefaultGameElemDir;
		if (!SetCurrentDirectory (plantdir.c_str()))
			return;

		vector<string> allPlants;
		CExport::getAllFiles (".plant", allPlants);

		for (uint32 i = 0; i < allPlants.size(); ++i)
		{
			char fName[_MAX_FNAME];
			char ext[_MAX_FNAME];
			::_splitpath((const char*)allPlants[i].c_str(), NULL, NULL, fName, ext);
			allPlants[i] = string(fName) + string(ext);
		}
		
		// todo Hulud : handle this another way..
		// _Georges->SetTypPredef ("plant_name.typ", allPlants);
	}
	catch(NLMISC::Exception &e)
	{
		MessageBox (e.what(), "Warning", MB_ICONERROR|MB_OK);
	}
	SetCurrentDirectory (curdir);
}

// ---------------------------------------------------------------------------
void CMainFrame::openLogicEditor ()
{
	if (_Environnement.LogicEditorOpened)
		return;
	_Environnement.LogicEditorOpened = true;
	if (_LogicEditor != NULL)
		_LogicEditor->initUILight (_Environnement.LogicEditorX, _Environnement.LogicEditorY, 
								_Environnement.LogicEditorCX, _Environnement.LogicEditorCY);
	GetMenu()->CheckMenuItem (ID_WINDOWS_LOGICEDITOR, MF_CHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::openLogicEditorFile (const string &fileName)
{
	_LogicEditor->loadFile (fileName.c_str());
}

// ---------------------------------------------------------------------------
void CMainFrame::closeLogicEditor ()
{
	if (!_Environnement.LogicEditorOpened)
		return;
	_Environnement.LogicEditorOpened = false;
	RECT r;
	CFrameWnd *pFW = (CFrameWnd*)_LogicEditor->getMainFrame();
	pFW->GetWindowRect(&r);
	WINDOWPLACEMENT wp;
	pFW->GetWindowPlacement (&wp);
	if (wp.showCmd == SW_SHOWNORMAL)
	{
		_Environnement.LogicEditorY = r.top;
		_Environnement.LogicEditorX = r.left;
		_Environnement.LogicEditorCY = r.bottom - r.top;
		_Environnement.LogicEditorCX = r.right - r.left;
	}
	_LogicEditor->releaseUI ();
	GetMenu()->CheckMenuItem (ID_WINDOWS_LOGICEDITOR, MF_UNCHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::updateTree()
{
	_Tree->update (_Environnement.ContinentsDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::deltree (const std::string &dirName)
{
	// Get all directory object
	vector<string> toremovelist;
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	if (!SetCurrentDirectory (dirName.c_str()))
		return;
	hFind = FindFirstFile ("*.*", &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		// Look if the name is a system directory
		if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				// If this is a directory deltree and delete the directory
				deltree (findData.cFileName);
				toremovelist.push_back (findData.cFileName);
			}
			else
			{
				// If this is a file delete
				if (!DeleteFile (findData.cFileName))
					displayLastErrorDialog ();
			}
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	SetCurrentDirectory (dirName.c_str());
	for (uint32 i = 0; i < toremovelist.size(); ++i)
	{
		if (!RemoveDirectory (toremovelist[i].c_str()))
			displayLastErrorDialog ();
	}
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::displayLastErrorDialog ()
{
	LPVOID lpMsgBuf;
	FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf, 0, NULL);
	// Display the string.
	MessageBox ((LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONERROR);
	// Free the buffer.
	LocalFree (lpMsgBuf);
}

// ---------------------------------------------------------------------------
/*int getMaxTag(const std::string &dirName, const std::string &tagName)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	SetCurrentDirectory (dirName.c_str());
	string tmp = tagName + "*";
	hFind = FindFirstFile (tmp.c_str(), &findData);	
	int ret = 0;
	while (hFind != INVALID_HANDLE_VALUE)
	{
		int nb =	(findData.cFileName[tagName.size()+0]-'0')*100 + 
					(findData.cFileName[tagName.size()+1]-'0')*10 + 
					(findData.cFileName[tagName.size()+2]-'0')*1;
		if (nb > ret)
			ret = nb;
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	SetCurrentDirectory (sCurDir);
	return ret;
}*/

// ---------------------------------------------------------------------------
void CMainFrame::copytree (const string &srcDir, const string &dstDir)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	string tmp = srcDir + "\\*";
	hFind = FindFirstFile (tmp.c_str(), &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		bool bFound = false;
		for (uint32 i = 0; i < MAX_SYS_DIR; ++i)
			if (stricmp (findData.cFileName, gSysDir[i]) == 0)
			{
				bFound = true;
				break;
			}
		if (!bFound)
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				string newDirSrc = srcDir + "\\" + findData.cFileName;
				string newDirDst = dstDir + "\\" + findData.cFileName;
				if (!CreateDirectory(newDirDst.c_str(), NULL))
					displayLastErrorDialog ();
				copytree (newDirSrc, newDirDst);
			}
			else
			{
				string fnameSrc = srcDir + "\\" + findData.cFileName;
				string fnameDst = dstDir + "\\" + findData.cFileName;
				if (!CopyFile(fnameSrc.c_str(), fnameDst.c_str(), false))
					displayLastErrorDialog ();
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
}


// ---------------------------------------------------------------------------
void CMainFrame::continentDelete (const string &contName)
{
	string srcDir = _Environnement.ContinentsDir + contName;
	deltree (srcDir);
	RemoveDirectory (srcDir.c_str());
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::regionOpen (const string &path)
{
	setActivePath (path);
	openDir (path);
}

// ---------------------------------------------------------------------------
void CMainFrame::regionRename (const string &path)
{
	if (path == "") return;
	int pos = path.rfind('\\');
	if (pos == string::npos) return;
	string smallpath;
	int i;
	for (i = 0; i <= pos; ++i)
		smallpath += path[i];
	string dirname;
	for (; i < (int)path.size(); ++i)
		dirname += path[i];
	smallpath = _Environnement.ContinentsDir + smallpath;
	char curdir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curdir);
	SetCurrentDirectory (smallpath.c_str());

	CNameEditDlg dlg;
	dlg.Title = "Change the directory name";
	dlg.Comment = "Enter the new directory name";
	dlg.Name = dirname.c_str();

	if (dlg.DoModal() == IDOK)
	{
		if (!MoveFile (dirname.c_str(), dlg.Name))
			displayLastErrorDialog ();
	}

	SetCurrentDirectory (curdir);
}

// ---------------------------------------------------------------------------
void CMainFrame::regionNewPrim (const string &path)
{
	if (path == "") return;
	if (_WorldEditor == NULL) return;

	// Ask the name to the user
	CNameEditDlg dlg;
	dlg.Title = "New Prim File Name";
	dlg.Comment = "Enter the name of the new .PRIM file";
	dlg.Name = "NewPrim";
	if (dlg.DoModal() == IDOK)
	{
		string primname = dlg.Name;
		if (primname != "")
		{
			string sTmp = _Environnement.ContinentsDir + path + "\\" + primname;
			_WorldEditor->createEmptyPrimFile (sTmp.c_str());
			updateTree ();
		}
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::regionNewGeorges (const string &path)
{
	if (path == "")
		return;
	if (_Georges == NULL)
		return;

	// Ask the user the .dfn to use
	CNameEditDlg dlg;
	dlg.Title = "New Form File Name";
	dlg.Comment = "Enter the name of the new Form file";
	dlg.Name = "NewForm";
	if (dlg.DoModal() == IDOK)
	{
		char curdir[MAX_PATH];
		GetCurrentDirectory (MAX_PATH, curdir);

		// setActivePath (path);

		// \todo load from the continent.cfg
		string newDir = _Georges->GetDirDfnTyp ();

		CFileDialog fd (true, "*.dfn", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
						"Definition (*.dfn)|*.dfn", this);
		fd.m_ofn.lpstrInitialDir = newDir.c_str();
		if (fd.DoModal() == IDOK)
		{
			string sTmp = _Environnement.ContinentsDir + path + "\\" + (LPCSTR)dlg.Name;
			_Georges->createInstanceFile (sTmp, (LPCSTR)fd.GetFileName());
		}

		SetCurrentDirectory (curdir);
		updateTree ();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::regionNewSubRegion (const std::string &path)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	CNameEditDlg dlg(this);
	dlg.Title = "New Sub Region";
	dlg.Comment = "Enter the name of the new sub region";
	dlg.Name = "NewSubRegion";

	if (dlg.DoModal() == IDOK)
	{
		if (dlg.Name != "")
		{
			string tmpDir = _Environnement.ContinentsDir + path;
			SetCurrentDirectory (tmpDir.c_str());
			CreateDirectory ((LPCSTR)dlg.Name, NULL);
			SetCurrentDirectory (dlg.Name);
			updateTree ();
		}
	}

	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::regionDelete (const std::string &path)
{
	string srcDir = _Environnement.ContinentsDir + path;
	deltree (srcDir);
	if (!RemoveDirectory (srcDir.c_str()))
		displayLastErrorDialog ();
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::fileOpen (const std::string &sFileFullName)
{
	openFile (sFileFullName);
}

// ---------------------------------------------------------------------------
void CMainFrame::fileRename (const std::string &sFileFullName)
{
	if (sFileFullName == "") return;
	int pos = sFileFullName.rfind('\\');
	if (pos == string::npos) return;
	string smallpath;
	int i;
	for (i = 0; i <= pos; ++i)
		smallpath += sFileFullName[i];
	string filename;
	for (; i < (int)sFileFullName.size(); ++i)
		filename += sFileFullName[i];
	smallpath = _Environnement.ContinentsDir + smallpath;
	char curdir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curdir);
	SetCurrentDirectory (smallpath.c_str());

	CNameEditDlg dlg;
	dlg.Title = "Change the name of the file";
	dlg.Comment = "Enter the new file name";
	dlg.Name = filename.c_str();

	if (dlg.DoModal() == IDOK)
	{
		if (!MoveFile (filename.c_str(), dlg.Name))
		{
			MessageBox ("Cannot rename file", "Error", MB_ICONERROR|MB_OK);
		}		
	}

	SetCurrentDirectory (curdir);
}

// ---------------------------------------------------------------------------
void CMainFrame::fileDelete (const std::string &sFileFullName)
{
	if (sFileFullName.size() > 4)
		if (stricmp(&sFileFullName.c_str()[sFileFullName.size()-4], ".cfg") == 0)
			return;
	string fullname = _Environnement.ContinentsDir + sFileFullName;
	DeleteFile (fullname.c_str());
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::copy (const std::string &sPathSrc, const std::string &sPathDst)
{
	string sSrc = _Environnement.ContinentsDir + sPathSrc;
	string sDst = _Environnement.ContinentsDir + sPathDst;

	if (GetFileAttributes(sSrc.c_str())&FILE_ATTRIBUTE_DIRECTORY)
	{
		copytree (sSrc, sDst);
	}
	else
	{
		int zepos = sSrc.rfind('\\');
		for (int i = zepos; i < (int)sSrc.size(); ++i)
			sDst += sSrc[i];
		if (!CopyFile (sSrc.c_str(), sDst.c_str(), TRUE))
			displayLastErrorDialog ();
	}
	updateTree();
}

// ---------------------------------------------------------------------------
void CMainFrame::setActivePath (const std::string &pathName)
{
	_ActivePath = pathName;
	if (_ActivePath == "") return;
	string sContinentDir;
	int i = 0;
	if (pathName[i] == '\\') ++i;
	for (; i < (int)pathName.size(); ++i)
	{
		if (pathName[i] == '\\') break;
		sContinentDir += pathName[i];
	}

	sContinentDir = _Environnement.ContinentsDir + sContinentDir;

	if (_Georges != NULL)
	{
		georgesSetPathesFromActive ();
	}

	if ((_Environnement.WorldEdOpened) && (_ActivePath != ""))
	{
		// Open continent.cfg to load the .land
		SContinentCfg cfg;
		string sTmp = sContinentDir + "\\continent.cfg";
		cfg.load (sTmp.c_str());
		_WorldEditor->setDataDir(cfg.LandDir.c_str());
		_WorldEditor->loadFile (cfg.LandFile.c_str());
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnSize (UINT nType, int cx, int cy)
{
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMove (int x, int y)
{
	RECT r;
	GetWindowRect (&r);
	_Environnement.MasterY = r.top;
	_Environnement.MasterX = r.left;
	if ((_Tree != NULL) && (_Environnement.MasterTreeLocked))
	{
		_Tree->SetWindowPos (&wndTop, x, y, 0, 0, SWP_NOSIZE);
		SetFocus();
	}
}

// ---------------------------------------------------------------------------
int CMainFrame::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	// Create the menu
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CoInitialize (NULL);

	// Create the tree
	_Tree = new CMasterTreeDlg ();
	_Tree->Create (IDD_MASTERTREE, this);

	// Load tools and get interface
	getAllInterfaces ();

	// Restore all (windows position and size, menu checked, etc...)
	// TREE
	_Tree->SetWindowPos (&wndTop, _Environnement.MasterTreeX, _Environnement.MasterTreeY, 
						_Environnement.MasterTreeCX, _Environnement.MasterTreeCY, SWP_SHOWWINDOW);
	_Tree->ShowWindow (SW_SHOW);
	_Tree->update (_Environnement.ContinentsDir);
	if (_Environnement.MasterTreeLocked)
		GetMenu()->CheckMenuItem (ID_OPTIONS_TREELOCK, MF_CHECKED);

	// WORLDEDITOR
	if (_WorldEditor != NULL)
	{
		_MasterCB.setMainFrame (this);
		_WorldEditor->setMasterCB (&_MasterCB);
		_WorldEditor->setDataDir (_Environnement.ContinentsDir.c_str());
	}
	if (_Environnement.WorldEdOpened)
	{ 
		_Environnement.WorldEdOpened = false;
		openWorldEditor ();
	}

	// GEORGES
	if (_Georges != NULL)
	{
		_Georges->SetDirDfnTyp (_Environnement.DefaultDFNDir);
		_Georges->SetDirPrototype (_Environnement.DefaultGameElemDir);
		georgesCreatePlantName ();		
	}
	if (_Environnement.GeorgesOpened)
	{ 
		_Environnement.GeorgesOpened = false;
		openGeorges ();
	}

	// LOGICEDITOR
//	if (_LogicEditor != NULL)
//		_LogicEditor->setRootDir (_Environnement.RootDir.c_str());
	if (_Environnement.LogicEditorOpened)
	{ 
		_Environnement.LogicEditorOpened = false;
		openLogicEditor ();
	}

	return 0;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::PreCreateWindow (CREATESTRUCT& cs)
{
	// Load the config file
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	_MasterExeDir = string(sCurDir) + "\\";
	try
	{
		_Environnement.load (_MasterExeDir+"master.cfg");
	}
	catch (Exception&e)
	{
		MessageBox (e.what(), "Warning");
	}

	// Restore the master window position
	cs.x = _Environnement.MasterX;
	cs.y = _Environnement.MasterY;

	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	
	cs.style = WS_OVERLAPPED;	
	cs.cx = 360;
	cs.cy = GetSystemMetrics (SM_CYCAPTION) + 
			GetSystemMetrics (SM_CYMENU) + 
			GetSystemMetrics (SM_CYFRAME);

	return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::PreTranslateMessage (MSG*pMsg)
{
	if (_Georges != NULL)
	{
		CWnd *pWnd = (CWnd*)_Georges->getMainFrame();
		CWnd *pFocusWnd = CWnd::GetFocus();
		if (pWnd != NULL)
		{
			while (pFocusWnd != NULL)
			{
				if (pWnd->m_hWnd == pFocusWnd->m_hWnd)
					return _Georges->PreTranslateMessage (pMsg);
				pFocusWnd = pFocusWnd->GetParent();
			}
		}
	}
	return CFrameWnd::PreTranslateMessage (pMsg);
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::OnEraseBkgnd (CDC* pDC)
{
	return true;
}

// ---------------------------------------------------------------------------
bool CMainFrame::createDirIfNotExist (const string& dirName, const string& errorMsg)
{
	char sCurDir[MAX_PATH];
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = true;
	GetCurrentDirectory (MAX_PATH, sCurDir);
	if (!SetCurrentDirectory (dirName.c_str()))
	{
		if (!CreateDirectory (dirName.c_str(), &sa))
		{
			MessageBox (errorMsg.c_str(), "Error", MB_ICONERROR|MB_OK);
			SetCurrentDirectory (sCurDir);
			return false;
		}
	}
	SetCurrentDirectory (sCurDir);
	return true;
}
// ---------------------------------------------------------------------------
void CMainFrame::continentNew ()
{
	if (!createDirIfNotExist (_Environnement.ContinentsDir, "Cannot create Continents system directory"))
		return;

	// Create the new continent
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	SetCurrentDirectory (_Environnement.ContinentsDir.c_str());
	CreateDirectory ("NewContinent", NULL);
	SContinentCfg cfg;
	cfg.save ("NewContinent\\continent.cfg");

	SetCurrentDirectory (_Environnement.ContinentsDir.c_str());
	SetCurrentDirectory ("NewContinent");

	// Ok so now create default files
// _WorldEditor->createDefaultFiles (newDirName.c_str());
//	georgesCreateFilesWhenNewContinent ();

	continentProperties ("NewContinent");

	_Tree->update (_Environnement.ContinentsDir);
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::continentOpen (const std::string &contName)
{
	setActivePath (contName);
	openDir (contName);
}

// ---------------------------------------------------------------------------
void CMainFrame::continentProperties (const string &contName)
{
	CContinentPropertiesDlg cp(this);

	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	string worldDir;
	worldDir = _Environnement.ContinentsDir;

	if (!SetCurrentDirectory (worldDir.c_str()))
		return;
	if (!SetCurrentDirectory (contName.c_str()))
		return;

	SContinentCfg cfg;
	cfg.load ("continent.cfg");

	cp.ContinentName	= contName.c_str();

	cp.LandFile			= cfg.LandFile.c_str();
	cp.LandDir			= cfg.LandDir.c_str();
	cp.DfnDir			= cfg.DfnDir.c_str();
	cp.GameElemDir		= cfg.GameElemDir.c_str();

	cp.LandBankFile		= cfg.LandBankFile.c_str();
	cp.LandFarBankFile	= cfg.LandFarBankFile.c_str();
	cp.LandTileNoiseDir	= cfg.LandTileNoiseDir.c_str();
	cp.LandZoneWDir		= cfg.LandZoneWDir.c_str();
	cp.OutIGDir			= cfg.OutIGDir.c_str();

	if (cp.DoModal() == IDOK)
	{
		cfg.LandFile			= cp.LandFile;
		cfg.LandDir				= cp.LandDir;
		cfg.DfnDir				= cp.DfnDir;
		cfg.GameElemDir			= cp.GameElemDir;

		cfg.LandBankFile		= cp.LandBankFile;
		cfg.LandFarBankFile		= cp.LandFarBankFile;
		cfg.LandTileNoiseDir	= cp.LandTileNoiseDir;
		cfg.LandZoneWDir		= cp.LandZoneWDir;
		cfg.OutIGDir			= cp.OutIGDir;

		SetCurrentDirectory (worldDir.c_str());
		SetCurrentDirectory (contName.c_str());
		cfg.save ("continent.cfg");
		if (cp.ContinentName != CString(contName.c_str()))
		{
			// Rename the directory
			SetCurrentDirectory (worldDir.c_str());
			if (!MoveFile (contName.c_str(), cp.ContinentName))
			{
				MessageBox ("Cannot rename directory", "Error", MB_ICONERROR|MB_OK);
			}
		}
	}
	else
	{
		SetCurrentDirectory (_Environnement.ContinentsDir.c_str());
		deltree ("NewContinent");
		RemoveDirectory("NewContinent");
	}
	SetCurrentDirectory (sCurDir);
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::continentNewRegion (const string &contName)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	CNameEditDlg dlg(this);
	dlg.Title = "New Region";
	dlg.Comment = "Enter the name of the new region";
	dlg.Name = "NewRegion";

	if (dlg.DoModal() == IDOK)
	{
		if (dlg.Name != "")
		{
			string tmpDir = _Environnement.ContinentsDir + contName;
			SetCurrentDirectory (tmpDir.c_str());
			CreateDirectory ((LPCSTR)dlg.Name, NULL);
			SetCurrentDirectory (dlg.Name);
			updateTree ();
		}
	}

	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
// Save all in all modules
void CMainFrame::onContinentSave ()
{
	if (_Environnement.WorldEdOpened)
	{
		if (_WorldEditor)
			_WorldEditor->saveOpenedFiles();
	}
	if (_Environnement.GeorgesOpened)
	{
//		if (_Georges)
//			_Georges->saveOpenedFiles();
	}
	if (_Environnement.LogicEditorOpened)
	{
//		if (_LogicEditor)
//			_LogicEditor->saveOpenedFiles();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::onContinentExport ()
{
	onContinentSave ();

	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	string sTmp = _Environnement.ContinentsDir;
	SetCurrentDirectory (sTmp.c_str());

	vector<string> vRegNames;
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile ("*.*", &fd);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		bool bFound = false;
		for (uint32 i = 0; i < MAX_SYS_DIR; ++i)
			if (stricmp (fd.cFileName, gSysDir[i]) == 0)
			{
				bFound = true;
				break;
			}
		if (!bFound)
		{
			if ((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				vRegNames.push_back (string(fd.cFileName));
			}
		}
		if (FindNextFile (hFind, &fd) == 0)
			break;
	}
	FindClose (hFind);

	if (vRegNames.size() == 0)
		return;

	CExportDlg dlg (this);
	
//	_Environnement.ExportOptions.SourceDir = _ActiveContinent;
	dlg.setOptions (_Environnement.ExportOptions, vRegNames);
	if (dlg.DoModal() == IDOK)
	{
		if (_ExportCBDlg != NULL)
			_ExportCBDlg->DestroyWindow();
		_ExportCBDlg = new CExportCBDlg();
		//Check if new succeeded and we got a valid pointer to a dialog object
		if (_Export == NULL)
			_Export = new CExport;
		if (_ExportCBDlg != NULL)
		{
			BOOL ret = _ExportCBDlg->Create (IDD_EXPORTCB, this);
			if (!ret)   //Create failed.
			{
				delete _ExportCBDlg;
				_ExportCBDlg = NULL;
			}
			_ExportCBDlg->ShowWindow (SW_SHOW);
		}

		_Environnement.ExportOptions.PrimFloraDir = _Environnement.ContinentsDir + _Environnement.ExportOptions.PrimFloraDir;

		SetCurrentDirectory (_MasterExeDir.c_str());
		try
		{
			_Environnement.save (_MasterExeDir+"master.cfg");
		
			_Export->newExport (_Environnement.ExportOptions, _ExportCBDlg->getExportCB());
			
		}
		catch(Exception&e)
		{
			MessageBox (e.what(), "Error", MB_ICONERROR|MB_OK);
		}

		_ExportCBDlg->setFinishedButton ();
		while (_ExportCBDlg->getFinished () == false)
		{
			_ExportCBDlg->pump ();
		}
		_ExportCBDlg->DestroyWindow ();
	}
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
/*void CMainFrame::OnContinentDelete ()
{
	CChooseDir chooseDir(this);
	chooseDir.setPath (_Environnement.ContinentsDir);
	if (chooseDir.DoModal() == IDOK)
	{
		continentDelete (chooseDir.getSelected());
	}
}*/

// ---------------------------------------------------------------------------
void CMainFrame::onOptionsTreeLock ()
{
	_Environnement.MasterTreeLocked = !_Environnement.MasterTreeLocked;
	if (_Environnement.MasterTreeLocked)
	{
		GetMenu()->CheckMenuItem (ID_OPTIONS_TREELOCK, MF_CHECKED);
		_Tree->SetWindowPos (&wndTop, _Environnement.MasterX, _Environnement.MasterY+45, 0, 0, SWP_NOSIZE);
		SetFocus();
	}
	else
		GetMenu()->CheckMenuItem (ID_OPTIONS_TREELOCK, MF_UNCHECKED);
}

// ---------------------------------------------------------------------------
int CALLBACK BrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	switch(uMsg) 
	{
		case BFFM_INITIALIZED: 
			SendMessage (hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
		default:
		break;
	}
	return 0;
}

// ---------------------------------------------------------------------------
void CMainFrame::onOptionsSetContinentsDir ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the continents path";
	bi.ulFlags = 0;
	bi.lpfn = BrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, _Environnement.ContinentsDir.c_str());	
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	_Environnement.ContinentsDir = str;
	if (_Environnement.ContinentsDir[_Environnement.ContinentsDir.size()-1] != '\\')
		_Environnement.ContinentsDir += "\\";
	// TREE
	_Tree->update (_Environnement.ContinentsDir);
	// WORLDEDITOR
	if (_WorldEditor != NULL)
		_WorldEditor->setDataDir (_Environnement.ContinentsDir.c_str());
	// GEORGES
	if (_Georges != NULL)
	{
		//_Georges->SetRootDirectory (_Environnement.RootDir + "common");
		//_Georges->SetWorkDirectory (_Environnement.RootDir + "common\\dfn");
		georgesCreatePlantName ();
	}
	// LOGICEDITOR
//	if (_LogicEditor != NULL)
//		_LogicEditor->setRootDir (_Environnement.RootDir.c_str());
}

// ---------------------------------------------------------------------------
void CMainFrame::onOptionsSetDefaultDFNDir ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the default DFN path";
	bi.ulFlags = 0;
	bi.lpfn = BrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, _Environnement.DefaultDFNDir.c_str());	
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	_Environnement.DefaultDFNDir = str;
	if (_Environnement.DefaultDFNDir[_Environnement.DefaultDFNDir.size()-1] != '\\')
		_Environnement.DefaultDFNDir += "\\";

	// GEORGES
	if (_Georges != NULL)
	{
		_Georges->SetDirDfnTyp (_Environnement.DefaultDFNDir);
		georgesCreatePlantName ();
	}
	// LOGICEDITOR
//	if (_LogicEditor != NULL)
//		_LogicEditor->setRootDir (_Environnement.RootDir.c_str());
}

// ---------------------------------------------------------------------------
void CMainFrame::onOptionsSetDefaultGameElemDir ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the default GameElem path";
	bi.ulFlags = 0;
	bi.lpfn = BrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, _Environnement.DefaultGameElemDir.c_str());	
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	_Environnement.DefaultGameElemDir = str;
	if (_Environnement.DefaultGameElemDir[_Environnement.DefaultGameElemDir.size()-1] != '\\')
		_Environnement.DefaultGameElemDir += "\\";

	// GEORGES
	if (_Georges != NULL)
	{
		_Georges->SetDirPrototype (_Environnement.DefaultGameElemDir);
		georgesCreatePlantName ();
	}
	// LOGICEDITOR
//	if (_LogicEditor != NULL)
//		_LogicEditor->setRootDir (_Environnement.RootDir.c_str());
}

// ---------------------------------------------------------------------------
void CMainFrame::onWindowsWorldEditor ()
{
	if (!_Environnement.WorldEdOpened)
		openWorldEditor ();
	else
		closeWorldEditor ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onWindowsGeorges ()
{
	if (!_Environnement.GeorgesOpened)
		openGeorges ();
	else
		closeGeorges ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onWindowsLogicEditor ()
{
	if (!_Environnement.LogicEditorOpened)
		openLogicEditor ();
	else
		closeLogicEditor ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onWindowsReset ()
{
	bool redo = false;

	redo = (_Environnement.WorldEdOpened == true);
	closeWorldEditor ();
	_Environnement.WorldEdX = 50;
	_Environnement.WorldEdY = 50;
	_Environnement.WorldEdCX = 600;
	_Environnement.WorldEdCY = 400;
	if (redo)
		openWorldEditor ();

	redo = (_Environnement.GeorgesOpened == true);
	closeGeorges ();
	_Environnement.GeorgesX = 50;
	_Environnement.GeorgesY = 50;
	_Environnement.GeorgesCX = 300;
	_Environnement.GeorgesCY = 300;
	if (redo)
		openGeorges ();

	redo = (_Environnement.LogicEditorOpened == true);
	closeLogicEditor ();
	_Environnement.LogicEditorX = 50;
	_Environnement.LogicEditorY = 50;
	_Environnement.LogicEditorCX = 300;
	_Environnement.LogicEditorCY = 300;
	if (redo)
		openLogicEditor ();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnClose ()
{
	RECT r;

	// Master Tree saves
	_Tree->GetWindowRect (&r);
	_Environnement.MasterTreeX = r.left;
	_Environnement.MasterTreeY = r.top;
	_Environnement.MasterTreeCX = r.right-r.left;
	_Environnement.MasterTreeCY = r.bottom-r.top;

	// WorldEditor saves
	if (_Environnement.WorldEdOpened)
	{
		closeWorldEditor ();
		_Environnement.WorldEdOpened = true;
	}

	// Georges saves
	if (_Environnement.GeorgesOpened)
	{
		closeGeorges ();
		_Environnement.GeorgesOpened = true;
	}

	// LogicEditor saves
	if (_Environnement.LogicEditorOpened)
	{
		closeLogicEditor ();
		_Environnement.LogicEditorOpened = true;
	}

	// Save the environnement
	SetCurrentDirectory (_MasterExeDir.c_str());
	try
	{
		_Environnement.save (_MasterExeDir+"master.cfg");
	}
	catch(Exception&e)
	{
		MessageBox (e.what(), "Error", MB_ICONERROR|MB_OK);
	}

	releaseAllInterfaces ();

	DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

