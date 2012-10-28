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

// main_frm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"

#include "action.h"
#include "world_editor.h"
#include "world_editor_doc.h"
#include "display.h"
#include "primitive_view.h"
#include "tools_logic.h"
#include "tools_zone.h"
#include "main_frm.h"
#include "resource.h"
#include "generate_dlg.h"
#include "type_manager_dlg.h"
#include "move_dlg.h"
#include "name_dlg.h"
#include "custom_snapshot.h"
#include "export_dlg.h"
// #include "export_cb_dlg.h"
#include "project_settings.h"
#include "dialog_properties.h"
#include "generate_primitive.h"
#include "editor_primitive.h"
#include "select_by_location.h"
#include "find_primitive_dlg.h"
#include "primitive_configuration_dlg.h"
#include "pacs.h"
#include "editor_primitive.h"
#include "file_dialog_ex.h"
#include "nel/misc/mem_stream.h"

#include <string>

using namespace NLLIGO;
using namespace std;
using namespace NLMISC;
using namespace NL3D;

extern bool	DontUse3D;

#define TIMER_UPDATE_FILES 666
#define TIMER_PLUGINS		667
#define LOCATE_BORDER 50

uint32 getUniqueId ();


// ***************************************************************************
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_COMMAND(ID_APP_EXIT,				onMenuFileExit)
	ON_COMMAND(ID_EDIT_LOGIC,			onMenuModeLogic)
	ON_COMMAND(ID_EDIT_ZONE,			onMenuModeZone)
	ON_COMMAND(ID_EDIT_TRANSITION,		onMenuModeTransition)
	ON_COMMAND(ID_EDIT_UNDO,			onMenuModeUndo)
	ON_COMMAND(ID_EDIT_REDO,			onMenuModeRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO,	onUpdateModeUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO,	onUpdateModeRedo)
	ON_COMMAND(ID_PROJECT_NEWLANDSCAPE, onProjectNewlandscape)
	ON_COMMAND(ID_PROJECT_ADDLANDSCAPE, onProjectAddlandscape)
	ON_COMMAND(ID_PROJECT_SETTINGS,		onProjectSettings)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TRANSITION, OnUpdateEditTransition)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE, OnUpdateEditZone)
	ON_UPDATE_COMMAND_UI(ID_EDIT_LOGIC, OnUpdateEditLogic)
	ON_COMMAND(ID_PROJECT_IMPORT_PRIM, OnProjectImportPrim)
	ON_COMMAND(ID_PROJECT_ADDPRIMITIVE, OnProjectAddPrimitive)
	ON_COMMAND(ID_PROJECT_CLEARALLPRIMITIVE, OnProjectClearallprimitive)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_LOCK, OnEditLock)
	ON_COMMAND(ID_EDIT_SELECT, OnEditSelect)
	ON_COMMAND(ID_EDIT_DETAILS, OnEditDetails)
	ON_COMMAND(ID_VIEW_LANDSCAPE, OnViewLandscape)
	ON_COMMAND(ID_VIEW_LAYERS, OnViewLayers)
	ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
	ON_COMMAND(ID_VIEW_POINTS, OnViewPoints)
	ON_COMMAND(ID_VIEW_PACS, OnViewPACS)
	ON_COMMAND(ID_VIEW_PRIMITIVES, OnViewPrimitives)
	ON_COMMAND(ID_EDIT_TRANSLATE, OnEditTranslate)
	ON_COMMAND(ID_EDIT_ROTATE, OnEditRotate)
	ON_COMMAND(ID_EDIT_TURN, OnEditTurn)
	ON_COMMAND(ID_EDIT_RADIUS, OnEditRadius)
	ON_COMMAND(ID_EDIT_SCALE, OnEditScale)
	ON_COMMAND(ID_EDIT_ADD_POINT, OnEditAddPoint)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_PROJECT_NEWPRIMITIVE, OnProjectNewPrimitive)
	ON_COMMAND(ID_EDIT_PROPERTIES, OnEditProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_POINT, OnUpdateEditAddPoint)
	ON_UPDATE_COMMAND_UI(ID_EDIT_LOCK, OnUpdateEditLock)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTIES, OnUpdateEditProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ROTATE, OnUpdateEditRotate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SCALE, OnUpdateEditScale)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TRANSLATE, OnUpdateEditTranslate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TURN, OnUpdateEditTurn)
	ON_UPDATE_COMMAND_UI(ID_EDIT_RADIUS, OnUpdateEditRadius)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT, OnUpdateEditSelect)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DETAILS, OnUpdateEditDetails)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LANDSCAPE, OnUpdateViewLandscape)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIDE, OnUpdateViewHide)
	ON_COMMAND(ID_VIEW_HIDE, OnViewHide)
	ON_COMMAND(ID_VIEW_SHOW, OnViewShow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW, OnUpdateViewShow)
	ON_UPDATE_COMMAND_UI(ID_EDIT_EXPAND, OnUpdateEditExpand)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COLLAPSE, OnUpdateEditCollapse)
	ON_COMMAND(ID_EDIT_SELECT_CHILDREN, OnEditSelectChildren)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_CHILDREN, OnUpdateEditSelectChildren)
	ON_WM_TIMER()
	ON_COMMAND(ID_EDIT_EXPAND, OnEditExpand)
	ON_COMMAND(ID_EDIT_COLLAPSE, OnEditCollapse)
	ON_COMMAND(ID_HELP_FINDER, OnHelpFinder)
	ON_COMMAND(ID_VIEW_LOCATESELECTEDPRIMITIVES, OnViewLocateselectedprimitives)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOCATESELECTEDPRIMITIVES, OnUpdateViewLocateselectedprimitives)
	ON_COMMAND(ID_VIEW_LOCATESELECTEDPRIMITIVES_TREE, OnViewLocateselectedprimitivesTree)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOCATESELECTEDPRIMITIVES_TREE, OnUpdateViewLocateselectedprimitivesTree)
	ON_COMMAND(ID_EDIT_SELECT_BY_LOCATION, OnEditSelectByLocation)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_BY_LOCATION, OnUpdateEditSelectByLocation)
	ON_COMMAND(ID_PROJECT_RESETUNIQUEID, OnProjectResetuniqueid)
	ON_COMMAND(ID_PROJECT_GENERATENULLID, OnProjectGeneratenullid)
	ON_COMMAND(ID_PROJECT_CORRECT_ID, OnProjectForceiduniqueness)
	ON_COMMAND(ID_FIND, OnEditFind)
	ON_COMMAND(ID_GOTO, OnEditGoto)
	ON_UPDATE_COMMAND_UI(ID_FIND, OnUpdateFind)
	ON_COMMAND(ID_VIEW_COLLISIONS, OnViewCollisions)
	ON_COMMAND(ID_HELP_HISTORY, OnHelpHistory)
	ON_COMMAND(ID_EXPORT_SNAPSHOT, OnExportSnapshot)
	ON_COMMAND(ID_WINDOWS_PRIMITIVECONFIGURATION, OnWindowsPrimitiveconfiguration)
	ON_UPDATE_COMMAND_UI(ID_WINDOWS_PRIMITIVECONFIGURATION, OnUpdateWindowsPrimitiveconfiguration)
	ON_COMMAND(ID_PROJECT_RESET_PRIMITIVE_CONFIGURATION, OnProjectResetPrimitiveConfiguration)
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_EXPAND, OnEditExpand)
	ON_COMMAND(ID_EDIT_COLLAPSE, OnEditCollapse)
	ON_WM_PAINT()
	ON_WM_MENUSELECT()
	ON_COMMAND(ID_SAVPOS, OnSavePosition)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_MISSION_COMPILER, OnMissionCompiler)
	ON_COMMAND(ID_NAME_DLG, OnNameDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


static UINT indicators[] =
{
	ID_INDICATOR_ZONENAMENEL,
	ID_INDICATOR_ZONENAMEREF,
	ID_INDICATOR_COORDINATES,
	ID_INDICATOR_ZONEROT,
	ID_INDICATOR_ZONEFLIP,
	ID_INDICATOR_SELECTION,
	ID_INDICATOR_INFO,
};

// ***************************************************************************
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
	:
	_Mode(0),
	_SplitterCreated(false),
	_MasterCB(0),
	_CurrentPlugin(0)
{
	CreateX = CreateY = CreateCX = CreateCY = 0;
	_ZoneBuilder = NULL;
	_SelectionLocked = false;
	_TransformModes[0] = Select;
	_TransformModes[1] = Select;
	_ShowDetails = false;
	_ShowLandscape = true;
	_ShowGrid = true;
	_ShowPoints = true;
	_ShowPACS = false;
	_ShowPrimitives = true;
	_ShowLayers = true;
	_ValidLandscape = true;
	_ShowCollisions = false;
	m_FindPrimitiveDlg = NULL;
	_MustRefreshPropertyDialog = false;
}

CMainFrame::~CMainFrame()
{
	if (_ZoneBuilder)
		delete _ZoneBuilder;

	if (m_FindPrimitiveDlg)
	{
		if (::IsWindow(m_FindPrimitiveDlg->GetSafeHwnd()))
            m_FindPrimitiveDlg->EndDialog(IDCANCEL);
        delete m_FindPrimitiveDlg;
		m_FindPrimitiveDlg = NULL;
	}

	// Free the clipboard
	deletePrimitiveClipboard ();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
/*	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME, 
		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}*/

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) /*||
		!m_wndReBar.AddBar(&m_wndDlgBar)*/)
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	CMenu *menu = GetMenu ();
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_ZONE), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_TRANSITION), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_LOGIC), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_LOCK), TBBS_CHECKBOX);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_DETAILS), TBBS_CHECKBOX);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_SELECT), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_TRANSLATE), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_ROTATE), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_TURN), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_SCALE), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_RADIUS), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_EDIT_ADD_POINT), TBBS_CHECKGROUP);
	m_wndToolBar.SetButtonStyle (m_wndToolBar.CommandToIndex(ID_VIEW_LANDSCAPE), TBBS_CHECKBOX);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_LANDSCAPE);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_LAYERS);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_GRID);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_POINTS);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_PRIMITIVES);
	menu->CheckMenuItem (ID_VIEW_LANDSCAPE, MF_CHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_VIEW_LAYERS, MF_CHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_VIEW_GRID, MF_CHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_VIEW_POINTS, MF_CHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_VIEW_PRIMITIVES, MF_CHECKED|MF_BYCOMMAND);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_ZONE);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_SELECT);

	CDocument *pNewDoc = new CWorldEditorDoc;//CDocument;
	InitialUpdateFrame(pNewDoc, TRUE);

	// Default tool column size
	_ToolColumnSize = 300;

	// Init display
	SetActiveView((CView*)m_wndSplitter.GetPane (0,0));
	DispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));

	// Timer
	CFrameWnd::SetTimer (TIMER_UPDATE_FILES, 5000, NULL);
	CFrameWnd::SetTimer (TIMER_PLUGINS, 1000/10, NULL);

	// Init the main frame
	if (init ())
	{
		// Disable events
		CNoInteraction nointeraction;
		
		launchLoadingDialog("init display");
		DispWnd->setCellSize (theApp.Config.CellSize);
		DispWnd->init (this);
		terminateLoadingDialog();
		return 0;
	}
	else
		return -1;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
//	AFX_MANAGE_STATE (AfxGetStaticModuleState());
	// create a splitter with 1 row, 2 columns
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	// add the first splitter pane - the default view in column 0
	if (!m_wndSplitter.CreateView(0, 0, 
		RUNTIME_CLASS(CDisplay), CSize(640, 512), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	// add the second splitter pane - the default view in column 0
	if (!m_wndSplitter.CreateView(0, 1, 
		RUNTIME_CLASS(CPrimitiveView), CSize(200, 100), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	_SplitterCreated = true;

	return TRUE;
}

// ***************************************************************************

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	//AFX_MANAGE_STATE (AfxGetStaticModuleState());

	if ((CreateCX != 0)&&(CreateCY != 0))
	{
		cs.x = CreateX;
		cs.y = CreateY;
		cs.cx = CreateCX;
		cs.cy = CreateCY;
	}

	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

// ***************************************************************************

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

// ***************************************************************************

// CMainFrame message handlers








/* Old Trap begin */

// ***************************************************************************

// CType

// ***************************************************************************

void SType::serial (NLMISC::IStream&f)
{
	int version = f.serialVersion(0);
	f.serial (Name);
	f.serial (Color);
}

// ***************************************************************************

// CType

// ***************************************************************************

SEnvironnement::SEnvironnement ()
{
}

// ***************************************************************************

void SEnvironnement::serial (NLMISC::IStream&f)
{
	int version = f.serialVersion (1);

	f.serialCont (Types);
	f.serial (BackgroundColor);
	// f.serial (ExportOptions);

	if (version > 0)
		f.serial (DataDir);
}

// ***************************************************************************
// CMainFrame

// ***************************************************************************

NLMISC::CConfigFile &CMainFrame::getConfigFile()
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	// Old Trap return IWorldEditor::getInterface()->getConfigFile();
	CWinApp *theApp = AfxGetApp();
	CWorldEditorApp* wea = static_cast<CWorldEditorApp*>(theApp);

	return wea->PluginConfig;
}

void CMainFrame::onLogicChanged(const std::vector<NLLIGO::CPrimRegion*> &regions)
{
	/* const std::vector<IPluginCallback*> &plugins = IWorldEditor::getInterface()->getPlugins();	
	std::vector<IPluginCallback*>::const_iterator first(plugins.begin()), last(plugins.end());

	for (; first != last; ++first)
	{
		(*first)->primRegionChanged(regions);
	} */
}

// ***************************************************************************

bool CMainFrame::yesNoMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	return theApp.yesNoMessage (buffer);
}

// ***************************************************************************

void CMainFrame::errorMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	theApp.errorMessage (buffer);
}

// ***************************************************************************

void CMainFrame::infoMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	theApp.infoMessage (buffer);
}

// ***************************************************************************

void CMainFrame::startPositionControl(IPluginCallback *plugin, const CVector &initPos)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	if (_CurrentPlugin != 0)
		_CurrentPlugin->lostPositionControl();

	_CurrentPlugin = plugin;
	_PositionControl = initPos;

	if (_Mode != 3)
	{
		_LastMode = _Mode;
//		uninitTools();
		_Mode = 3;
//		initTools();
	}

	DispWnd->Invalidate();
}

// ***************************************************************************

void CMainFrame::stopPositionControl(IPluginCallback *plugin)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	if (_CurrentPlugin == plugin)
	{
		_CurrentPlugin = 0;
		if (_Mode == 3)
		{
	//		uninitTools();
			_Mode = _LastMode;
	//		initTools();
		}
	}
	DispWnd->Invalidate();
}

// ***************************************************************************
void CMainFrame::setExeDir (const char* str)
{
//	AFX_MANAGE_STATE (AfxGetStaticModuleState());
	_ExeDir = str;
	if ((str[strlen(str)-1] != '\\') || (str[strlen(str)-1] != '/'))
		_ExeDir += "\\";
}

// ***************************************************************************

void CMainFrame::uninitTools()
{
	if (_Mode == 0) // Mode Zone
	{
		CToolsZone *toolWnd = dynamic_cast<CToolsZone*>(m_wndSplitter.GetPane(0,1));
		toolWnd->uninit ();
	}
	if (_Mode == 1) // Mode Logic
	{
		// Invalidate primitives
		CWorldEditorDoc *doc = getDocument ();
		uint i;
		uint count = doc->getNumDatabaseElement ();
		for (i=0; i<count; i++)
		{
			// Invalidate it
			CDatabaseLocatorPointer locator;
			locator.getRoot (i);
			InvalidatePrimitiveRec (locator, LogicTreeStruct);
		}
	}
	if (_Mode == 3) // mode plugin position control
	{
		if (_CurrentPlugin != 0)
		{
			_CurrentPlugin->lostPositionControl();
			_CurrentPlugin = 0;
		}
	}
}

// ***************************************************************************

void CMainFrame::initTools()
{
	if (_Mode == 0) // Mode Zone
	{
		m_wndSplitter.DeleteView (0, 1);
		m_wndSplitter.CreateView (0, 1, RUNTIME_CLASS(CToolsZone), CSize(100, 100), NULL);
		SetActiveView ((CView*)m_wndSplitter.GetPane(0,1));
		CToolsZone *toolWnd = dynamic_cast<CToolsZone*>(m_wndSplitter.GetPane(0,1));
		toolWnd->init (this);
		adjustSplitter ();
	}
	if (_Mode == 1) // Mode Logic
	{
		m_wndSplitter.DeleteView (0, 1);
		m_wndSplitter.CreateView (0, 1, RUNTIME_CLASS(CToolsLogic), CSize(200, 100), NULL);
		SetActiveView ((CView*)m_wndSplitter.GetPane(0,1));
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
		toolWnd->init (this);
		adjustSplitter ();

		// Invalidate primitives
		CWorldEditorDoc *doc = getDocument ();
		uint i;
		uint count = doc->getNumDatabaseElement ();
		for (i=0; i<count; i++)
		{
			// Invalidate it
			CDatabaseLocatorPointer locator;
			locator.getRoot (i);
			InvalidatePrimitiveRec (locator, LogicTreeStruct);
		}
		// todo primitive init
	}
	if (_Mode == 3) // // mode plugin position control
	{
		// send an initial position report
		if (_CurrentPlugin != 0)
			_CurrentPlugin->positionMoved(_PositionControl);

	}
	SetActiveView ((CView*)m_wndSplitter.GetPane(0,0));

}

// ***************************************************************************

void CMainFrame::invalidateTools ()
{
	if (_Mode == 1) // Mode Logic
	{
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
	}
}

// ***************************************************************************

void CMainFrame::invalidateLandscape ()
{
	_ValidLandscape = false;
}

// ***************************************************************************

void CMainFrame::invalidateToolsParam ()
{
	if (_Mode == 1) // Mode Logic
	{
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
		toolWnd->Invalidate(FALSE);
	}
}

// ***************************************************************************

void CMainFrame::invalidateModification ()
{
	if (_Mode == 1) // Mode Logic
	{
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
	}
}

// ***************************************************************************

void CMainFrame::updateData ()
{
	// Disable events
	CNoInteraction nointeraction;
	
	// Update primitive quad grid
	UpdatePrimitives ();

	// Update the landscape
	if (!_ValidLandscape)
	{	
		// Init the landscape manager
		_ZoneBuilder->refresh ();

		_ValidLandscape = true;
	}

	if (_Mode == 1) // Mode Logic
	{
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
	}

	// Update the cursor
	getDisplay ()->updateCursor ();

	// Update status bar info
	displayStatusBarInfo ();
}

// ***************************************************************************
void CMainFrame::primZoneModified ()
{
	if (_MasterCB != NULL)
	{
		vector<string> allNames;
	}
}

// ***************************************************************************

void CMainFrame::displayStatusBarInfo ()
{
	std::string sTmp;

	// Position
	const CVector &v = DispWnd->_CurPos;

	// Write the zone name in NeL protocol
	sint32 x, y;

	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	x = (sint32)floor(v.x / dispWnd->_CellSize);
	y = (sint32)floor(v.y / dispWnd->_CellSize);
	if ((v.x >= 0) && (x <= 255) && (v.y <= 0) && (y >= -255))
	{
		getZoneNameFromXY(x, y, sTmp);
	}
	else
	{
		sTmp = "NOT A VALID ZONE";
	}

	string text;
	if (dispWnd->getActionHelp (text))
	{
		m_wndStatusBar.SetPaneText (0, text.c_str());
	}
	else
	{
		m_wndStatusBar.SetPaneText (0, sTmp.c_str());
	}
	//for (uint32 i = sTmp.size(); i < 10; ++i)
	//	sTmp += " ";

	// Write zone reference name 
	if (dispWnd->getActionText (text))
	{
		m_wndStatusBar.SetPaneText (1, text.c_str());
	}
	else
	{
		sTmp = _ZoneBuilder->getZoneName (x, y);
		m_wndStatusBar.SetPaneText (1, sTmp.c_str());
	}

	// Write coordinates
	char temp[1024];
	sprintf(temp, "(%.3f , %.3f)", v.x, v.y);
	sTmp = temp;
//	sTmp = "( " + toString(v.x) + " , " + toString(v.y) + " )";
	m_wndStatusBar.SetPaneText (2, sTmp.c_str());

	// Write rot
	sTmp = "Rot(" + toString(_ZoneBuilder->getRot(x, y)) + ")";
	m_wndStatusBar.SetPaneText (3, sTmp.c_str());

	// Write flip
	sTmp = "Flip(" + toString(_ZoneBuilder->getFlip(x, y)) + ")";
	m_wndStatusBar.SetPaneText (4, sTmp.c_str());

	// Write selection
	if (Selection.size ())
	{
		if (Selection.size ()>1)
			sTmp = toString (Selection.size ()) + " selected primitives";
		else
			sTmp = toString (Selection.size ()) + " selected primitive";
	}
	else
		sTmp = "No selected primitive";
	m_wndStatusBar.SetPaneText (5, sTmp.c_str());

	// Write path of selected primitive
	if (Selection.size())
		sTmp = getDocument()->getPathOfSelectedPrimitive();
	else
		sTmp = "";
	
	m_wndStatusBar.SetPaneText (6, sTmp.c_str());
}

// ***************************************************************************

void CMainFrame::displayInfo (const char *info)
{
	m_wndStatusBar.SetPaneText (6, info);
}

// ***************************************************************************
// ***************************************************************************
// CMainFrame diagnostics


// ***************************************************************************
bool CMainFrame::init (bool bMakeAZone)
{
	// Initialize the zoneBuilder (load bank and this kind of stuff)
	if (initLandscapeData ())
	{
		// Mode zone
		_Mode = 0;
		return true;
	}
	else
	{
		return false;
	}
}

// ***************************************************************************
void CMainFrame::uninit ()
{
	// Save the WorldEditor.cfg
	try
	{
		COFile fileOut;
		string sWorldEdCfg = _ExeDir;
		sWorldEdCfg += "WorldEditor.cfg";
		fileOut.open (sWorldEdCfg.c_str(), false, false, true);
		fileOut.serial(_Environnement);
		fileOut.close();
	}
	catch (Exception& e)
	{
		MessageBox (e.what(), "Warning");
	}
}

// ******************
// MESSAGES FROM MENU
// ******************

// ***************************************************************************
void CMainFrame::onMenuFileOpenLogic ()
{
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "prim", TRUE, "prim", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Primitives Files (*.prim)|*.prim||", this);
	if (dialog.DoModal() == IDOK)
	{
		launchLoadingDialog(string("loading prim zone ") + (LPCSTR)dialog.GetFileName());
		// todo primitive load
		primZoneModified ();
		terminateLoadingDialog();
	}
}

// ***************************************************************************

void CMainFrame::onMenuFileExit ()
{
	OnClose();
}

// ***************************************************************************
void CMainFrame::onMenuModeZone ()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem( ID_EDIT_ZONE, ID_EDIT_LOGIC, ID_EDIT_ZONE, MF_BYCOMMAND);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_ZONE);

	if (_Mode == 1) // if we were in Logic mode switch
	{
		// store the width of the logic tool panel
		if (_SplitterCreated)
		{
			RECT r;
			int cxCur, cxMin;
			GetClientRect (&r);
			m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
			_ToolColumnSize = r.right-r.left-cxCur;
		}
		
		uninitTools ();
		_Mode = 0;
		initTools ();
	}
	_Mode = 0;
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));

	// Update data
	updateData ();
}

// ***************************************************************************
void CMainFrame::onMenuModeTransition ()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem( ID_EDIT_ZONE, ID_EDIT_LOGIC, ID_EDIT_TRANSITION, MF_BYCOMMAND);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_TRANSITION);

	if (_Mode == 1) // if we were in Logic mode switch
	{
		// store the width of the logic tool panel
		if (_SplitterCreated)
		{
			RECT r;
			int cxCur, cxMin;
			GetClientRect (&r);
			m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
			_ToolColumnSize = r.right-r.left-cxCur;
		}

		uninitTools ();
		_Mode = 0;
		initTools ();
	}
	_Mode = 2;
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));

	// Update data
	updateData ();
}

// ***************************************************************************
void CMainFrame::onMenuModeLogic ()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem( ID_EDIT_ZONE, ID_EDIT_LOGIC, ID_EDIT_LOGIC, MF_BYCOMMAND);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_LOGIC);

	if (_Mode != 1) // Mode Logic
	{
		uninitTools();
		_Mode = 1;
		initTools();
	}
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::onMenuModeUndo ()
{
	getDocument ()->undo ();

	// Update data
	updateData ();
}

// ***************************************************************************
void CMainFrame::onMenuModeRedo ()
{
	getDocument ()->redo ();

	// Update data
	updateData ();
}

// ***************************************************************************
#if 0
void CMainFrame::onMenuModeMove ()
{
	CMoveDlg dialog;
	if (dialog.DoModal() == IDOK)
	if (getDocument ()->getNumLandscape ())
	{
		// Select the size
		const CZoneRegion &region = getDocument ()->getZoneRegion (_ZoneBuilder->_ZoneRegionSelected);

		_ZoneBuilder->move (dialog.XOffset, dialog.YOffset);
		CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	}
}
#endif 
// ***************************************************************************
/*
void CMainFrame::onMenuModeCountZones ()
{
	if (getDocument ()->getNumLandscape ())
	{
		uint32 nNbZones = _ZoneBuilder->countZones ();
		string tmp = "There are " + toString(nNbZones) + " zones";
		MessageBox(tmp.c_str(), "Information", MB_ICONINFORMATION|MB_OK);
	}
}
*/
// ***************************************************************************
void CMainFrame::onMenuViewGrid ()
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	dispWnd->setDisplayGrid (!dispWnd->getDisplayGrid());
	CMenu *menu = GetMenu();
	menu->CheckMenuItem (ID_VIEW_GRID, dispWnd->getDisplayGrid()?MF_CHECKED|MF_BYCOMMAND:MF_UNCHECKED|MF_BYCOMMAND);
}

// ***************************************************************************
void CMainFrame::onMenuViewBackground ()
{
	CColorDialog colDial;

	if (colDial.DoModal() == IDOK)
	{
		int r = GetRValue(colDial.GetColor());
		int g = GetGValue(colDial.GetColor());
		int b = GetBValue(colDial.GetColor());
		_Environnement.BackgroundColor = CRGBA(r,g,b,255);
		CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
		dispWnd->setBackgroundColor (_Environnement.BackgroundColor);
	}
}

// ***************************************************************************
void CMainFrame::OnClose ()
{
	_Exit = true;
	CFrameWnd::OnClose ();
}

// ***************************************************************************
void CMainFrame::OnSize (UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED)
	{
		if (cx < 100)
			cx = 100;
		if (cy < 100)
			cy = 100;
	}

	// store the width of the logic tool panel
	if (_SplitterCreated && _Mode == 1)
	{
		int cxCur, cxMin;
		m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
		_ToolColumnSize = cx - cxCur;
	}

	CFrameWnd::OnSize (nType, cx, cy);
	if (nType != SIZE_MINIMIZED)
		adjustSplitter ();
}

// ***************************************************************************
void CMainFrame::adjustSplitter ()
{
	if (_SplitterCreated)
	{
		RECT r;
		GetClientRect (&r);

		// Mode Zone
		if ((_Mode == 0) || (_Mode == 2) && (r.right-r.left > 380))
			m_wndSplitter.SetColumnInfo (0, r.right-r.left-380, 100); // 380 really experimental value
			
		// Mode Logic
		if (_Mode == 1) 
			if (r.right-r.left > _ToolColumnSize)
				m_wndSplitter.SetColumnInfo (0, r.right-r.left-_ToolColumnSize, 10);
			else
				m_wndSplitter.SetColumnInfo (0, 0, 10);


		m_wndSplitter.RecalcLayout ();
	}
}

// ***************************************************************************
void CMainFrame::launchLoadingDialog (const std::string &sText)
{
	LoadingDialog = new CLoadingDialog(this);
	LoadingDialog->ShowWindow(SW_SHOW);
	LoadingDialog->setText(sText);
	LoadingDialog->setProgress (0);
	CWnd *wnd = LoadingDialog->GetDlgItem (IDC_PROGRESS1);
	nlassert (wnd);
	wnd->ShowWindow (SW_HIDE);
}

// ***************************************************************************

void CMainFrame::progressLoadingDialog (float progress, bool _flushMessages)
{
	nlassert (LoadingDialog);
	LoadingDialog->setProgress (progress);

	// Flush current messages
	if (_flushMessages)
	{
		flushMessages ();
	}
}

// ***************************************************************************

void CMainFrame::flushMessages ()
{
	// Pump others message for the windows
	MSG	msg;
	while (PeekMessage(&msg, *LoadingDialog, 0, 0, PM_REMOVE))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

// ***************************************************************************

void CMainFrame::terminateLoadingDialog ()
{
	delete LoadingDialog;
}

// ***************************************************************************

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (!DontUse3D && CNELU::Driver)
	{
		typedef void (*winProc)(NL3D::IDriver *drv, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		winProc	drvWndProc= (winProc)CNELU::Driver->getWindowProc();
		drvWndProc (CNELU::Driver, m_hWnd, message, wParam, lParam);
	}
		
	return CFrameWnd::WindowProc(message, wParam, lParam);
}

// ***************************************************************************
// ***************************************************************************
// CLoadingDialog
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************

CLoadingDialog::CLoadingDialog (CWnd *pParent)
{
	Create (IDD_LOADING, pParent);
}

// ***************************************************************************

void CLoadingDialog::setText (const std::string &text)
{
	CStatic *pS = (CStatic*)GetDlgItem (IDC_STATIC_TEXT_LOADING);
	string sTmp = string("Please wait while ") + text;
	setWindowTextUTF8 (*pS, sTmp.c_str());

	MSG	msg;
	while (PeekMessage(&msg, *this, 0, 0, PM_REMOVE))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

// ***************************************************************************

void CLoadingDialog::setProgress (float progress)
{
	CProgressCtrl *progressDlg = (CProgressCtrl *)GetDlgItem (IDC_PROGRESS1);
	nlassert (progressDlg);

	progressDlg->ShowWindow (SW_SHOW);
	progressDlg->SetRange (0, 32767);
	
	clamp (progress, 0.f, 1.f);
	progressDlg->SetPos ((int)(progress * 32767.f));
}

// ***************************************************************************

/* 
void CMainFrame::onMenuFileView ()
{
	viewLand(false);
}
*/

// ***************************************************************************

/*
void CMainFrame::onMenuFileViewWithIG()
{
	viewLand(true);
}
*/

// ***************************************************************************

struct CViewerConfig
{
	bool			Windowed;
	uint			Width;
	uint			Height;
	uint			Depth;
	CVector			Position;
	CVector			Heading;
	CVector			EyesHeight;
	CRGBA			Background;

	// Landscape
	bool			AutoLight;
	CVector			LightDir;
	string			ZonesPath;
	string			BanksPath;
	string			TilesPath;
	bool			UseDDS;
	bool			AllPathRelative;

	string			IgPath;
	string			ShapePath;
	string			MapsPath;
	string			Bank;
	string			FontPath;
	//CTextContext	TextContext;
	//CFontManager	FontManager;
	float			ZFar;
	float			LandscapeTileNear;
	float			LandscapeThreshold;
	vector<string>	Zones;
	vector<string>	Igs;

	// HeightField.
	string			HeightFieldName;
	float			HeightFieldMaxZ;
	float			HeightFieldOriginX;
	float			HeightFieldOriginY;
	float			HeightFieldSizeX;
	float			HeightFieldSizeY;

	// StaticLight
	CRGBA			LandAmbient;
	CRGBA			LandDiffuse;

	// -----------------------------------------------------------------------

	CViewerConfig()
	{
		Windowed = true;
		Width = 800;
		Height = 600;
		Depth = 32;
		Position = CVector( 1088.987793f, -925.732178f, 0.0f );
		Heading = CVector(0,1,0);
		EyesHeight = CVector(0,0,1.8f);
		Background = CRGBA(100,100,255);
		AutoLight = false;
		LightDir = CVector (1, 0, 0);
		ZonesPath = "./";
		BanksPath = "./";
		TilesPath = "./";
		UseDDS = false;
		AllPathRelative = false;
		IgPath = "./";
		ShapePath = "./";
		MapsPath = "./";
		Bank = "bank.bank";
		FontPath = "\\\\server\\code\\fonts\\arialuni.ttf";
		ZFar = 1000;
		LandscapeTileNear = 50.0f;
		LandscapeThreshold = 0.001f;

		HeightFieldName= "";
		HeightFieldMaxZ= 100;
		HeightFieldOriginX= 16000;
		HeightFieldOriginY= -24000;
		HeightFieldSizeX= 160;
		HeightFieldSizeY= 160;

		CRGBA diffuse (241, 226, 244);
		CRGBA ambiant  (17, 54, 100);
		LandDiffuse= diffuse;
		LandAmbient= ambiant;

	}

	// -----------------------------------------------------------------------
	void load (const string & configFileName)
	{
		FILE * f = fopen (configFileName.c_str(), "rt");
		if(f==NULL)
		{
			nlwarning("'%s' not found, default values used", configFileName.c_str());
			save (configFileName);
		}
		else fclose (f);
		
		try
		{
			CConfigFile cf;
		
			cf.load(configFileName);
		
			CConfigFile::CVar &cvFullScreen = cf.getVar("FullScreen");
			this->Windowed = cvFullScreen.asInt() ? false : true;
			
			CConfigFile::CVar &cvWidth = cf.getVar("Width");
			this->Width = cvWidth.asInt();

			CConfigFile::CVar &cvHeight = cf.getVar("Height");
			this->Height = cvHeight.asInt();

			CConfigFile::CVar &cvDepth = cf.getVar("Depth");
			this->Depth = cvDepth.asInt();

			CConfigFile::CVar &cvPosition = cf.getVar("Position");
			nlassert(cvPosition.size()==3);
			this->Position.x = cvPosition.asFloat(0);
			this->Position.y = cvPosition.asFloat(1);
			this->Position.z = cvPosition.asFloat(2);

			CConfigFile::CVar &cvEyesHeight = cf.getVar("EyesHeight");
			this->EyesHeight = CVector(0,0,cvEyesHeight.asFloat());

			CConfigFile::CVar &cvBackColor = cf.getVar("Background");
			nlassert(cvBackColor.size()==3);
			this->Background.R = cvBackColor.asInt(0);
			this->Background.G = cvBackColor.asInt(1);
			this->Background.B = cvBackColor.asInt(2);

			CConfigFile::CVar &cvZFar = cf.getVar("ZFar");
			this->ZFar = cvZFar.asFloat();

			CConfigFile::CVar &cvAutoLight = cf.getVar("AutoLight");
			this->AutoLight = cvAutoLight.asInt() ? true : false;

			CConfigFile::CVar &cvLightDir = cf.getVar("LightDir");
			nlassert(cvLightDir.size()==3);
			this->LightDir.x = cvLightDir.asFloat(0);
			this->LightDir.y = cvLightDir.asFloat(1);
			this->LightDir.z = cvLightDir.asFloat(2);

			CConfigFile::CVar &cvLandscapeTileNear = cf.getVar("LandscapeTileNear");
			this->LandscapeTileNear = cvLandscapeTileNear.asFloat();

			CConfigFile::CVar &cvLandscapeThreshold = cf.getVar("LandscapeThreshold");
			this->LandscapeThreshold = cvLandscapeThreshold.asFloat();

			CConfigFile::CVar &cvBanksPath = cf.getVar("BanksPath");
			this->BanksPath = cvBanksPath.asString();

			CConfigFile::CVar &cvTilesPath = cf.getVar("TilesPath");
			this->TilesPath = cvTilesPath.asString();

			CConfigFile::CVar &cvUseDDS = cf.getVar("UseDDS");
			this->UseDDS = cvUseDDS.asInt() ? true : false;

			CConfigFile::CVar &cvAllPathRelative = cf.getVar("AllPathRelative");
			this->AllPathRelative = cvAllPathRelative.asInt() ? true : false;

			CConfigFile::CVar &cvBank = cf.getVar("Bank");
			this->Bank = cvBank.asString();
			
			CConfigFile::CVar &cvZonesPath = cf.getVar("ZonesPath");
			this->ZonesPath = cvZonesPath.asString();

			CConfigFile::CVar &cvIgPath = cf.getVar("IgPath");
			this->IgPath = cvIgPath.asString();

			CConfigFile::CVar &cvShapePath = cf.getVar("ShapePath");
			this->ShapePath = cvShapePath.asString();

			CConfigFile::CVar &cvMapsPath = cf.getVar("MapsPath");
			this->MapsPath = cvMapsPath.asString();

			CConfigFile::CVar &cvHeightFieldName = cf.getVar("HeightFieldName");
			this->HeightFieldName = cvHeightFieldName.asString();
			
			CConfigFile::CVar &cvHeightFieldMaxZ = cf.getVar("HeightFieldMaxZ");
			this->HeightFieldMaxZ = cvHeightFieldMaxZ.asFloat();

			CConfigFile::CVar &cvHeightFieldOriginX = cf.getVar("HeightFieldOriginX");
			this->HeightFieldOriginX = cvHeightFieldOriginX.asFloat();

			CConfigFile::CVar &cvHeightFieldOriginY = cf.getVar("HeightFieldOriginY");
			this->HeightFieldOriginY = cvHeightFieldOriginY.asFloat();

			CConfigFile::CVar &cvHeightFieldSizeX = cf.getVar("HeightFieldSizeX");
			this->HeightFieldSizeX = cvHeightFieldSizeX.asFloat();

			CConfigFile::CVar &cvHeightFieldSizeY = cf.getVar("HeightFieldSizeY");
			this->HeightFieldSizeY = cvHeightFieldSizeY.asFloat();


			CConfigFile::CVar &cvLandAmb = cf.getVar("LandAmbient");
			nlassert(cvLandAmb.size()==3);
			this->LandAmbient.R = cvLandAmb.asInt(0);
			this->LandAmbient.G = cvLandAmb.asInt(1);
			this->LandAmbient.B = cvLandAmb.asInt(2);

			CConfigFile::CVar &cvLandDiff = cf.getVar("LandDiffuse");
			nlassert(cvLandDiff.size()==3);
			this->LandDiffuse.R = cvLandDiff.asInt(0);
			this->LandDiffuse.G = cvLandDiff.asInt(1);
			this->LandDiffuse.B = cvLandDiff.asInt(2);


			CConfigFile::CVar &cvZones = cf.getVar("Zones");
			for(uint i=0; i<cvZones.size(); i++)
			{
				this->Zones.push_back(cvZones.asString(i));
			}

			CConfigFile::CVar &cvIgs = cf.getVar("Ig");
			for(uint i=0; i<cvIgs.size(); i++)
			{
				this->Igs.push_back(cvIgs.asString(i));
			}

		}
		catch (EConfigFile &e)
		{
			printf ("Problem in config file : %s\n", e.what ());
		}
	}

	// -----------------------------------------------------------------------
	void save (const string &configFileName)
	{
		FILE * f = fopen (configFileName.c_str(), "wt");

		if(f==NULL)
		{
			fprintf(stderr,"can't open file '%s'\n",configFileName);
		}

		fprintf(f,"FullScreen = %d;\n",this->Windowed?0:1);
		fprintf(f,"Width = %d;\n",this->Width);
		fprintf(f,"Height = %d;\n",this->Height);
		fprintf(f,"Depth = %d;\n",this->Depth);
		fprintf(f,"Position = { %f, %f, %f };\n", this->Position.x,this->Position.y,this->Position.z);
		fprintf(f,"EyesHeight = %f;\n", this->EyesHeight.z);
		fprintf(f,"Background = { %d, %d, %d };\n", this->Background.R,this->Background.G,this->Background.B);
		fprintf(f,"ZFar = %f;\n", this->ZFar);

		fprintf(f,"AutoLight = %d;\n", this->AutoLight?1:0);
		fprintf(f,"LightDir = { %f, %f, %f };\n", this->LightDir.x, this->LightDir.y, this->LightDir.z);
		fprintf(f,"LandscapeTileNear = %f;\n", this->LandscapeTileNear);
		fprintf(f,"LandscapeThreshold = %f;\n", this->LandscapeThreshold);
		fprintf(f,"BanksPath = \"%s\";\n",this->BanksPath.c_str());
		fprintf(f,"TilesPath = \"%s\";\n",this->TilesPath.c_str());
		fprintf(f,"UseDDS = \"%d\";\n",this->UseDDS?1:0);
		fprintf(f,"AllPathRelative = \"%d\";\n",this->AllPathRelative?1:0);
		fprintf(f,"Bank = \"%s\";\n",this->Bank.c_str());
		fprintf(f,"ZonesPath = \"%s\";\n",this->ZonesPath.c_str());
		fprintf(f,"IgPath = \"%s\";\n",this->IgPath.c_str());
		fprintf(f,"ShapePath = \"%s\";\n",this->ShapePath.c_str());
		fprintf(f,"MapsPath = \"%s\";\n",this->MapsPath.c_str());
		


		fprintf(f,"HeightFieldName = \"%s\";\n", this->HeightFieldName.c_str());
		fprintf(f,"HeightFieldMaxZ = %f;\n", this->HeightFieldMaxZ);
		fprintf(f,"HeightFieldOriginX = %f;\n", this->HeightFieldOriginX);
		fprintf(f,"HeightFieldOriginY = %f;\n", this->HeightFieldOriginY);
		fprintf(f,"HeightFieldSizeX = %f;\n", this->HeightFieldSizeX);
		fprintf(f,"HeightFieldSizeY = %f;\n", this->HeightFieldSizeY);

		fprintf(f,"LandAmbient = { %d, %d, %d };\n", this->LandAmbient.R,this->LandAmbient.G,this->LandAmbient.B);
		fprintf(f,"LandDiffuse = { %d, %d, %d };\n", this->LandDiffuse.R,this->LandDiffuse.G,this->LandDiffuse.B);

		uint32 i;
		fprintf(f,"Zones = {\n");
		for (i = 0; i < Zones.size(); ++i)
			if (i < (Zones.size()-1))
				fprintf(f,"\"%s\",\n", Zones[i]);
			else
				fprintf(f,"\"%s\"\n", Zones[i]);
		fprintf(f,"};\n");

		fprintf(f,"Ig = {\n");
		for (i = 0; i < Igs.size(); ++i)
			if (i < (Igs.size()-1))
				fprintf(f,"\"%s\",\n", Igs[i]);
			else
				fprintf(f,"\"%s\"\n", Igs[i]);
		fprintf(f,"};\n");

		fclose(f);
	}
};

// ***************************************************************************

/*
void CMainFrame::viewLand(bool withIgs)
{
		// Modify the zviewer.cfg
	CViewerConfig cfgFile;
	SetCurrentDirectory (_ExeDir.c_str());
	string cfgFileName = "zviewer.cfg";
	cfgFile.load (cfgFileName);
	cfgFile.Zones.clear ();
	cfgFile.Igs.clear ();

	// Select the size
	const CZoneRegion &region = getDocument ()->getZoneRegion (_ZoneBuilder->_ZoneRegionSelected);

	sint32 nMinX = region.getMinX() < 0 ? 0 : region.getMinX();
	sint32 nMaxX = region.getMaxX() > 255 ? 255 : region.getMaxX();
	sint32 nMinY = region.getMinY() > 0 ? 0 : region.getMinY();
	sint32 nMaxY = region.getMaxY() < -255 ? -255 : region.getMaxY();

	if ((_Environnement.ExportOptions.ZoneMin != "") && (_Environnement.ExportOptions.ZoneMax != ""))
	{
		_Environnement.ExportOptions.ZoneMin = strupr (_Environnement.ExportOptions.ZoneMin);
		_Environnement.ExportOptions.ZoneMax = strupr (_Environnement.ExportOptions.ZoneMax);
		sint32 nNewMinX = CExport::getXFromZoneName (_Environnement.ExportOptions.ZoneMin);
		sint32 nNewMinY = CExport::getYFromZoneName (_Environnement.ExportOptions.ZoneMin);
		sint32 nNewMaxX = CExport::getXFromZoneName (_Environnement.ExportOptions.ZoneMax);
		sint32 nNewMaxY = CExport::getYFromZoneName (_Environnement.ExportOptions.ZoneMax);

		if (nNewMinX > nNewMaxX) 
			swap (nNewMinX, nNewMaxX);
		if (nNewMinY > nNewMaxY) 
			swap (nNewMinY, nNewMaxY);

		if (nNewMinX > nMinX)
			nMinX = nNewMinX;
		if (nNewMinY > nMinY)
			nMinY = nNewMinY;

		if (nNewMaxX < nMaxX)
			nMaxX = nNewMaxX;
		if (nNewMaxY < nMaxY)
			nMaxY = nNewMaxY;
	}


	for (sint32 j = nMinY; j <= nMaxY; ++j)
	for (sint32 i = nMinX; i <= nMaxX; ++i)
	{
		const string &SrcZoneName = region.getName (i,j);

		if ((SrcZoneName == STRING_OUT_OF_BOUND) ||
			(SrcZoneName == STRING_UNUSED))
			continue;

		string DstZoneFileName = CExport::getZoneNameFromXY (i,j);						
		if (withIgs)	
		{
			cfgFile.Igs.push_back (DstZoneFileName + ".ig");	
		}
		cfgFile.Zones.push_back (DstZoneFileName + ".zonel");		
	}

	cfgFile.ZonesPath = this->_Environnement.ExportOptions.OutZoneDir;
	cfgFile.IgPath = this->_Environnement.ExportOptions.OutIGDir;
	

	string sTmp = this->_Environnement.ExportOptions.TileBankFile;
	string sBanksPath, sBankName;
	int pos = sTmp.rfind('\\');
	for (int i = 0; i < pos; ++i)
		sBanksPath += sTmp[i];
	++i;
	for (; i < (int)sTmp.size(); ++i)
		sBankName += sTmp[i];

	cfgFile.BanksPath = sBanksPath;	
	cfgFile.TilesPath = sBanksPath;
	cfgFile.Bank = sBankName;

	
	

	//
	cfgFile.save (cfgFileName);

	// Launch the zviewer program
	const char *exeFileName = "zviewer.exe";
	if (NLMISC::CFile::fileExists(exeFileName))
	{
		_spawnl (_P_WAIT, exeFileName, "aze", NULL);
	}
	else
	{	
		MessageBox((std::string("unable to find ") + exeFileName).c_str(), "WorldEditor", MB_OK | MB_ICONEXCLAMATION);
	}
}
*/

// ***************************************************************************

/* Old Trap End */

void CMainFrame::onProjectAddlandscape() 
{
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "land", TRUE, "land", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "NeL Ligo Landscape Files (*.land)|*.land|All Files (*.*)|*.*||", this);
	if (dialog.DoModal() == IDOK)
	{
		// Add the landscape in the project
		getDocument ()->beginModification ();
		getDocument ()->addModification (new CActionAddLandscape (dialog.GetPathName()));
		getDocument ()->endModification ();

		// Update data
		updateData ();

		// If data direcrory is empty, auto-assign it to landscape directory
		if (getDocument()->getDataDir() != "")
			return;

		string path = dialog.GetPathName();

		uint pos = path.rfind("\\");
		if (pos == string::npos)
			return;

		path.erase(pos, path.size());
		getDocument()->setDataDir(path.c_str());

		// Init the landscape
		initLandscapeData();
	}
}

// ***************************************************************************

void CMainFrame::onProjectNewlandscape() 
{
	// Add the landscape in the project
	getDocument ()->beginModification ();
	getDocument ()->addModification (new CActionNewLandscape ());
	getDocument ()->endModification ();

	// Update data
	updateData ();
}

// ***************************************************************************

bool CMainFrame::initLandscapeData ()
{
	// Disable events
	CNoInteraction nointeraction;
	
	const string &dataDir = getDocument ()->getDataDir ();

	_Environnement.DataDir = dataDir;
	launchLoadingDialog ("loading ligo zones");
	if (_ZoneBuilder)
		delete _ZoneBuilder;
	_ZoneBuilder = new CBuilderZone (theApp.Config.ZoneSnapShotRes);
	_ZoneBuilder->init (dataDir, false, DispWnd);
	terminateLoadingDialog();

	// Get pointer on the document
	CWorldEditorDoc *doc = getDocument ();
	if (doc)
	{
		// For each landscape
		uint count = doc->getNumDatabaseElement ();
		for (uint land = 0; land < count; land++)
		{
			// Is a landscape
			if (doc->isLandscape (land))
			{
				// Get the landscape name
				const string &name = doc->getDatabaseElement (land);
				
				launchLoadingDialog (("loading landscape " + NLMISC::CFile::getFilename (name)).c_str ());

				// Load it
				_ZoneBuilder->refresh ();

				terminateLoadingDialog();
			}
		}
	}

	// Reinit the tools
	initTools();

	// Release and reload pacs if needed
	PacsManager.releasePacs();
	string dir;
	dir = getDocument ()->getDataDir ();
	if (dir == "")
	{
		if (_ExeDir == "")
		{
			dir = NLMISC::CPath::getCurrentPath();
		}
		else
		{
			dir = _ExeDir;
		}
	}
	dir += "/pacs";
	PacsManager.setDir(dir);
	// load PACS only if they are visible
	if (_ShowPACS)
	{
		launchLoadingDialog ("loading PACS");
		PacsManager.loadPacs();
		terminateLoadingDialog();
	}

	return true;
}

// ***************************************************************************

CWorldEditorDoc	*CMainFrame::getDocument ()
{
	return (CWorldEditorDoc	*)(GetActiveDocument ());
}

// ***************************************************************************

void CMainFrame::onProjectSettings() 
{
	CProjectSettings projectSettings;
	if (projectSettings.DoModal () == IDOK)
	{
		bool initLandscape = false;

		// Get the new data path
		std::string	oldDataDir = standardizePath (getDocument ()->getDataDir ().c_str ());
		std::string	newDataDir = standardizePath (projectSettings.DataDirectory);
		initLandscape |= (oldDataDir != newDataDir);
		getDocument ()->setDataDir (projectSettings.DataDirectory);

		// Init the landscape
		if (initLandscape)
			initLandscapeData ();
	}
}

// ***************************************************************************

void CMainFrame::OnUpdateEditTransition(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

// ***************************************************************************

void CMainFrame::OnUpdateEditZone(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

// ***************************************************************************

void CMainFrame::onUpdateModeUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getDocument ()->undoAvailable () ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::onUpdateModeRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getDocument ()->redoAvailable () ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditLogic(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

// ***************************************************************************

void CMainFrame::OnProjectImportPrim() 
{
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "prim", TRUE, "prim", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Old NeL Ligo Prim Files (*.prim)|*.prim|All Files (*.*)|*.*||", this);
	if (dialog.DoModal() == IDOK)
	{
		// Add the landscape in the project
		getDocument ()->beginModification ();
		getDocument ()->addModification (new CActionImportPrimitive (dialog.GetPathName()));
		getDocument ()->endModification ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnProjectAddPrimitive() 
{
	static char	buffer[32000];
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "primitive", TRUE, "primitive", NULL, OFN_ALLOWMULTISELECT|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "NeL Ligo Primitive Files (*.primitive)|*.primitive|All Files (*.*)|*.*||", this);
	// increase result buffer size (for multi select)
	memset(buffer, 0, 32000);
	dialog.m_ofn.lpstrFile = buffer;
	dialog.m_ofn.nMaxFile = 32000;
	if (dialog.DoModal() == IDOK)
	{
		// Add the landscape in the project
		getDocument ()->beginModification ();
		{
			POSITION pos;
			pos = dialog.GetStartPosition();

			while (pos!=NULL)
			{
				string path = dialog.GetNextPathName(pos);
				if (!getDocument()->isPrimitiveLoaded(path))
					getDocument ()->addModification (new CActionLoadPrimitive (path.c_str()));
				else
					infoMessage("Primitive already existing: %s", path.c_str());
			}
		}
		getDocument ()->endModification ();

		// Verify primitive structures
		VerifyPrimitivesStructures();

		// Update data
		updateData ();
	}

}

// ***************************************************************************

void CMainFrame::OnProjectClearallprimitive() 
{
	// Add the landscape in the project
	getDocument ()->beginModification ();
	getDocument ()->addModification (new CActionClearPrimitives ());
	getDocument ()->endModification ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::setSelectionLocked (bool lock)
{
	if (_SelectionLocked != lock)
	{
		CMenu *menu = GetMenu();
		_SelectionLocked = lock;
		if (_SelectionLocked)
		{
			// Update UI
			m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_LOCK);
			menu->CheckMenuItem (ID_EDIT_LOCK, MF_CHECKED|MF_BYCOMMAND);
		}
		else
		{
			// Update UI
			m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_LOCK, FALSE);
			menu->CheckMenuItem (ID_EDIT_LOCK, MF_UNCHECKED|MF_BYCOMMAND);
		}

		// Set the current mode
		setTransformMode (_TransformModes[(uint)_SelectionLocked]);

		// Invaludate view
		invalidateLeftView ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnEditSelectAll() 
{
	if (_Mode == 1) // Mode Logic
	{
		// Add the landscape in the project
		getDocument ()->beginModification ();
		getDocument ()->addModification (new CActionSelectAll ());
		getDocument ()->endModification ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnEditLock() 
{
	if (_Mode == 1) // Mode Logic
	{
		// Toggle lock
		_SelectionLocked ^= true;
		CMenu *menu = GetMenu();
		if (_SelectionLocked)
		{
			// Update UI
			m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_LOCK);
			menu->CheckMenuItem (ID_EDIT_LOCK, MF_CHECKED|MF_BYCOMMAND);
		}
		else
		{
			// Update UI
			m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_LOCK, FALSE);
			menu->CheckMenuItem (ID_EDIT_LOCK, MF_UNCHECKED|MF_BYCOMMAND);
		}

		// Set the current mode
		setTransformMode (_TransformModes[(uint)_SelectionLocked]);

		// Invalidate left view
		invalidateLeftView ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnEditDetails() 
{
	if (_Mode == 1) // Mode Logic
	{
		// Toggle lock
		_ShowDetails ^= true;
		CMenu *menu = GetMenu();
		if (_ShowDetails)
		{
			// Update UI
			m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_DETAILS);
			menu->CheckMenuItem (ID_EDIT_DETAILS, MF_CHECKED|MF_BYCOMMAND);
		}
		else
		{
			// Update UI
			m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_DETAILS, FALSE);
			menu->CheckMenuItem (ID_EDIT_DETAILS, MF_UNCHECKED|MF_BYCOMMAND);
		}

		// Invalidate left view
		invalidateLeftView ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnViewLandscape() 
{
	// Toggle lock
	_ShowLandscape ^= true;
	CMenu *menu = GetMenu();
	if (_ShowLandscape)
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_LANDSCAPE);
		menu->CheckMenuItem (ID_VIEW_LANDSCAPE, MF_CHECKED|MF_BYCOMMAND);
	}
	else
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_LANDSCAPE, FALSE);
		menu->CheckMenuItem (ID_VIEW_LANDSCAPE, MF_UNCHECKED|MF_BYCOMMAND);
	}

	// Invalidate left view
	invalidateLeftView ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnViewGrid() 
{
	// Toggle lock
	_ShowGrid ^= true;
	CMenu *menu = GetMenu();
	if (_ShowGrid)
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_GRID);
		menu->CheckMenuItem (ID_VIEW_GRID, MF_CHECKED|MF_BYCOMMAND);
	}
	else
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_GRID, FALSE);
		menu->CheckMenuItem (ID_VIEW_GRID, MF_UNCHECKED|MF_BYCOMMAND);
	}

	// Invalidate left view
	invalidateLeftView ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnViewPoints() 
{
	// Toggle lock
	_ShowPoints ^= true;
	CMenu *menu = GetMenu();
	if (_ShowPoints)
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_POINTS);
		menu->CheckMenuItem (ID_VIEW_POINTS, MF_CHECKED|MF_BYCOMMAND);
	}
	else
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_POINTS, FALSE);
		menu->CheckMenuItem (ID_VIEW_POINTS, MF_UNCHECKED|MF_BYCOMMAND);
	}

	// Invalidate left view
	invalidateLeftView ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnViewPACS() 
{
	// Toggle lock
	_ShowPACS ^= true;
	CMenu *menu = GetMenu();
	if (_ShowPACS)
	{
		// load PACS if it's not already done
		if(!PacsManager.areLoaded())
		{
			launchLoadingDialog ("loading PACS");
			PacsManager.loadPacs();
			terminateLoadingDialog();
		}

		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_PACS);
		menu->CheckMenuItem (ID_VIEW_PACS, MF_CHECKED|MF_BYCOMMAND);
	}
	else
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_PACS, FALSE);
		menu->CheckMenuItem (ID_VIEW_PACS, MF_UNCHECKED|MF_BYCOMMAND);
	}

	// Invalidate left view
	invalidateLeftView ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnViewLayers() 
{
	// Toggle lock
	_ShowLayers ^= true;
	CMenu *menu = GetMenu();
	if (_ShowLayers)
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_LAYERS);
		menu->CheckMenuItem (ID_VIEW_LAYERS, MF_CHECKED|MF_BYCOMMAND);
	}
	else
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_LAYERS, FALSE);
		menu->CheckMenuItem (ID_VIEW_LAYERS, MF_UNCHECKED|MF_BYCOMMAND);
	}

	// Invalidate left view
	invalidateLeftView ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnViewPrimitives () 
{
	// Toggle lock
	_ShowPrimitives ^= true;
	CMenu *menu = GetMenu();
	if (_ShowPrimitives)
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_PRIMITIVES);
		menu->CheckMenuItem (ID_VIEW_PRIMITIVES, MF_CHECKED|MF_BYCOMMAND);
	}
	else
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_PRIMITIVES, FALSE);
		menu->CheckMenuItem (ID_VIEW_PRIMITIVES, MF_UNCHECKED|MF_BYCOMMAND);
	}

	// Invalidate left view
	invalidateLeftView ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnEditTranslate() 
{
	setTransformMode (Move);
}

// ***************************************************************************

void CMainFrame::OnEditRotate() 
{
	setTransformMode (Rotate);
}

// ***************************************************************************

void CMainFrame::OnEditTurn() 
{
	setTransformMode (Turn);
}

// ***************************************************************************

void CMainFrame::OnEditRadius() 
{
	setTransformMode (Radius);
}

// ***************************************************************************

void CMainFrame::OnEditScale() 
{
	setTransformMode (Scale);
}

// ***************************************************************************

void CMainFrame::OnEditAddPoint() 
{
	setTransformMode (AddPoint);
}

// ***************************************************************************

void CMainFrame::OnEditSelect() 
{
	setTransformMode (Select);
}

// ***************************************************************************

void CMainFrame::setTransformMode (TTransformMode mode)
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem( ID_EDIT_SELECT, ID_EDIT_ADD_POINT, ID_EDIT_SELECT+mode, MF_BYCOMMAND);
	m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_EDIT_SELECT+mode);

	_TransformModes[(uint)_SelectionLocked] = mode;
}

// ***************************************************************************

void CMainFrame::deletePrimitive (bool subDelete, const char *actionName)
{
	list<NLLIGO::IPrimitive*> oldSelection;

	list<CDialogProperties*>::iterator it = PropertiesDialogs.begin();
	while ( it != PropertiesDialogs.end() )
	{
		bool closeDialog = false;
		if ( (*it)->containsSelection( Selection ) )
		{
			closeDialog = (*it)->removePrimitives( Selection );
		}

		if ( closeDialog )
			it = PropertiesDialogs.begin(); // iterator became invalid
		else
			it++;
	}

	// Get pointer on the document
	CWorldEditorDoc *doc = getDocument ();

	// Begin modification
	doc->beginModification ();

	if (_Mode == 1) // Mode Logic
	{
		// Next primitive to select
		bool locatorValid = false;
		CDatabaseLocatorPointer locatorNext;

		// Delete
		if (subDelete)
		{
			// For all the selection
			list<IPrimitive*>::iterator ite = Selection.begin ();
			while (ite != Selection.end ())
			{
				CDatabaseLocatorPointer locator;
				doc->getLocator (locator, *ite);

				// Point primitive ?
				const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor *> (locator.Primitive);
				if (point)
				{
				}
				else
				{
					// Path
					const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> (locator.Primitive);
					const CPrimZoneEditor *zone = dynamic_cast<const CPrimZoneEditor *> (locator.Primitive);
					if (path || zone)
					{
						while (1)
						{
							// Get the good primitive
							const IPrimitive *primitive = (const IPrimitive *)path ? (const IPrimitive *)path : (const IPrimitive *)zone;
							uint count = primitive->getNumVector ();
							if (count)
							{
								const CPrimVector *primVector = primitive->getPrimVector ();
								uint i;
								for (i=0; i<count; i++)
								{
									// Selected ?
									if (primVector[i].Selected)
									{
										locator.XSubPrim = i;
										doc->addModification (new CActionDeleteSub (locator));
										break;
									}
								}

								// Done ?
								if (i == count)
								{
									break;
								}
							}
							else
								break;
						}
					}
				}

				ite++;
			}
		}
		else
		{
			// For all the selection
			list<IPrimitive*>::iterator ite;
			ite = Selection.begin ();
			while (ite != Selection.end ())
			{
				bool deleted = false;

				CDatabaseLocatorPointer locator;
				doc->getLocator (locator, *ite);

				// Node or prim file ?
				if (locator.Primitive->getParent ())
				{
					// Primitive deletable ?
					if (theApp.Config.isPrimitiveDeletable (*(locator.Primitive)))
					{
						// Select the next primitive after delete
						CDatabaseLocatorPointer locatorNext2 = locator;
						if (locatorNext2.nextChild ())
							locatorNext = locator;
						else
						{
							// Get previous node
							locatorNext2 = locator;
							locatorNext2.previousChild ();
							locatorNext = locatorNext2;
						}
						locatorValid = true;

						doc->addModification (new CActionDelete (locator));
						deleted = true;
					}
				}
				else
				{
					// Editable ?
					uint index = locator.getDatabaseIndex ();
					if (doc->_DataHierarchy[index].Editable)
					{
						// Select the next primitive after delete
						locatorValid = false;

						doc->addModification (new CActionDeleteDatabaseElement (locator.getDatabaseIndex ()));
						deleted = true;
					}
				}

				// Update selection, restart to the beginning
				if (deleted)
				{
					UpdateSelection ();
					ite = Selection.begin ();
				}
				else
					ite++;
			}
		}

		// Select a new primitive ?
		if (locatorValid)
			doc->addModification (new CActionSelect (locatorNext));
	}

	// End modification
	doc->endModification ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnEditDelete() 
{
	deletePrimitive (isSelectionLocked (), "Delete");
}

// ***************************************************************************

void CMainFrame::OnProjectNewPrimitive() 
{
	// Get pointer on the document
	CWorldEditorDoc *doc = getDocument ();

	// Add the landscape in the project
	doc->beginModification ();
	doc->addModification (new CActionNewPrimitive ());

	// Select it
	doc->addModification (new CActionUnselectAll ());

	// Locator
	CDatabaseLocatorPointer locator;
	locator.getRoot (doc->getNumDatabaseElement ()-1);
	doc->addModification (new CActionSelect (locator));

	doc->endModification ();

	// Update data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnEditProperties() 
{
	list<CDialogProperties*>::iterator it = PropertiesDialogs.begin();
	bool ok = false;
	::CRect dlgPos(0, 0, 0, 0);

	while ( !ok && ( it != PropertiesDialogs.end() ) )
	{
		if ( (*it)->equalsSelection( Selection ) )
		{
			(*it)->ShowWindow( SW_SHOW );
			(*it)->SetFocus();
			
			ok = true;
		}

		it++;
	}

	if ( !ok )
	{
		CDialogProperties *pDlg = new CDialogProperties( Selection );
		pDlg->Create( IDD_PROPERTIES );
		
		pDlg->GetWindowRect( &dlgPos );
		
		CPoint point = CDialogProperties::getLastPosition();
		
		if ( point != CPoint(-1, -1) )
		{
			int diffX = point.x - dlgPos.left;
			int diffY = point.y - dlgPos.top;
			dlgPos.OffsetRect( diffX, diffY );

			pDlg->MoveWindow( &dlgPos );
		}
		
		pDlg->ShowWindow( SW_SHOW );
		pDlg->SetFocus();

		PropertiesDialogs.push_back( pDlg );
	}
}

// ***************************************************************************

void CMainFrame::OnAddPrimitive (UINT nID)
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Begin modification
	doc->beginModification ();

	// Only one selection ?
	nlassert (Selection.size () == 1);
	nlassert (Selection.front ());

	// What class is it ?
	const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*(Selection.front ()));
	nlassert (primClass);
	uint childId = nID - ID_EDIT_CREATE_BEGIN;

	// Get the child class
	CDatabaseLocatorPointer insert;
	doc->getLocator (insert, Selection.front ());
	CDatabaseLocator dest;
	insert.appendChild (dest);

	// Position on the Display
	CDisplay *display = getDisplay ();
	float delta = (float)DELTA_POS_ADD_PRIMITIVE * (display->_CurViewMax.x - display->_CurViewMin.x) / (float)display->getWidth ();
	if (doc->addModification (new CActionAddPrimitiveByClass (dest, primClass->DynamicChildren[childId].ClassName.c_str (), getDisplay ()->_CurPos, 
		delta, primClass->DynamicChildren[childId].Parameters)))
	{
		// Unselect all
		doc->addModification (new CActionUnselectAll ());

		// Select it
		doc->addModification (new CActionSelect (dest));
	}

	// End modification
	doc->endModification ();

	// Update the data
	updateData ();
}

// ***************************************************************************

void CMainFrame::OnGeneratePrimitive (UINT nID)
{
	// Disable events
	CNoInteraction nointeraction;
	
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Begin modification
	doc->beginModification ();

	// Primitive generator
	CGeneratePrimitive generatePrimitive;

	// Load in primitive
	if (!Selection.empty ())
	{
		// Progress bar
		CWorldEditorProgressCallback callback;

		// In / out primitives
		std::vector< std::vector<NLLIGO::IPrimitive*> >		out;
		std::vector<const NLLIGO::IPrimitive*>				in;

		// Add primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
		while (ite != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite);
			in.push_back (locator.Primitive);
			ite++;
		}

		// What class is it ?
		const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*(Selection.front ()));
		nlassert (primClass);

		// Should exist
		nlassert ( (nID - ID_EDIT_GENERATE_BEGIN) < primClass->GeneratedChildren.size ());

		// Generate it
		if (generatePrimitive.generate (out, in, callback, theApp.Config, doc->getDataDir ().c_str (), 
			primClass->GeneratedChildren[nID-ID_EDIT_GENERATE_BEGIN].ClassName.c_str () ))
		{
			// Checks
			nlassert (out.size () == Selection.size ());

			// Add primitives
			uint i=0;
			std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
			while (ite != Selection.end ())
			{
				CDatabaseLocatorPointer locator;
				doc->getLocator (locator, *ite);
				for (uint j=0;j<out[i].size (); j++)
				{
					// Get the locator
					CDatabaseLocator newLocator;
					locator.appendChild (newLocator);

					// Add the primitives
					doc->addModification (new CActionAddPrimitive (out[i][j], newLocator));
				}

				ite++;
				i++;
			}
		}
	}

	// End modification
	doc->endModification ();

	// Update the data
	updateData ();
}

// ***************************************************************************

void CMainFrame::buildFilenameVector (const IPrimitive &primitive, std::vector<std::string> &dest)
{
	// What class is it ?
	const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (primitive);
	nlassert (primClass);

	// Look for files
	uint i;
	for (i=0; i<primClass->Parameters.size (); i++)
	{
		// File ?
		if (primClass->Parameters[i].Filename)
		{
			// Exist ?
			string name;
			if (primitive.getPropertyByName (primClass->Parameters[i].Name.c_str (), name) && !name.empty ())
			{
				// Add the file
				string ext;

				// Build an extension
				if (!primClass->Parameters[i].FileExtension.empty ())
					ext = "."+primClass->Parameters[i].FileExtension;

				// Lookup
				string filename = name+ext;
				if (primClass->Parameters[i].Lookup)
					filename = CPath::lookup (filename, false, false, false);
				if (!filename.empty ())
				{
					dest.push_back (filename);
				}
			}
		}
	}
}

// ***************************************************************************

void CMainFrame::OnOpenFile (UINT nID)
{
	// Single selection
	nlassert (Selection.size () == 1);

	// Good id
	nID -= ID_EDIT_OPEN_FILE_BEGIN;

	// Filenames
	std::vector<std::string> files;
	buildFilenameVector (*Selection.front (),  files);
	
	// File name must exist
	nlassert (nID<files.size ());

	// Open the file
	if	(!openFile (files[nID].c_str ()))
		theApp.errorMessage ("Can't open the file %s", files[nID].c_str ());
}

// ***************************************************************************

void CMainFrame::createContextMenu (CWnd *parent, const CPoint &point, bool transformMode)
{
	// Menu creation
	CMenu *pMenu = new CMenu;
	pMenu->CreatePopupMenu ();

	// Tranform mode ?
	if (transformMode)
	{
		if (_Mode == 1)
		{
			// Add commands
			pMenu->AppendMenu (MF_STRING, ID_EDIT_SELECT, "&Select\tF5");
			pMenu->AppendMenu (MF_STRING, ID_EDIT_TRANSLATE, "&Move\tF6");
			pMenu->AppendMenu (MF_STRING, ID_EDIT_ROTATE, "&Rotate\tF7");
			pMenu->AppendMenu (MF_STRING, ID_EDIT_TURN, "&Turn\tF8");
			pMenu->AppendMenu (MF_STRING, ID_EDIT_SCALE, "&Scale\tF9");
			pMenu->AppendMenu (MF_STRING, ID_EDIT_RADIUS, "&Radius\tF10");
			if (isSelectionLocked ())
				pMenu->AppendMenu (MF_STRING, ID_EDIT_ADD_POINT, "&Add points\tF11");

			// Check the good one
			pMenu->CheckMenuRadioItem (ID_EDIT_SELECT, isSelectionLocked ()?ID_EDIT_ADD_POINT:ID_EDIT_SCALE, ID_EDIT_SELECT+getTransformMode (),
										MF_BYCOMMAND);

			// Add separator
			pMenu->AppendMenu (MF_SEPARATOR);
		}
	}

	// Always a delete menu
	pMenu->AppendMenu (MF_STRING, ID_EDIT_DELETE, "&Delete\tDel");

	// Add properties menu
	pMenu->AppendMenu (MF_STRING, ID_EDIT_PROPERTIES, "&Properties\tAlt+Enter");

	// Select Children
	pMenu->AppendMenu (MF_STRING, ID_EDIT_SELECT_CHILDREN, "&Select Children\tC");

	// Tree help
	pMenu->AppendMenu (MF_STRING, ID_HELP_FINDER, "&Help\tF1");

	// Add separator
	pMenu->AppendMenu (MF_SEPARATOR);
	pMenu->AppendMenu (MF_STRING, ID_RENAME_SELECTED, "&Rename All Selected");
	pMenu->AppendMenu (MF_STRING, ID_REPAIR_SELECTED, "&Repair All Selected");

	// Add separator
	pMenu->AppendMenu (MF_SEPARATOR);

	// Add properties menu
	pMenu->AppendMenu (MF_STRING, ID_VIEW_SHOW, "&Show\tS");
	pMenu->AppendMenu (MF_STRING, ID_VIEW_HIDE, "&Hide\tH");

	// Add separator
	pMenu->AppendMenu (MF_SEPARATOR);

	// Add expand / collapse menu
	pMenu->AppendMenu (MF_STRING, ID_EDIT_EXPAND, "&Expand\tE");
	pMenu->AppendMenu (MF_STRING, ID_EDIT_COLLAPSE, "&Collapse\tR");

	// Only one selection ?
	if (Selection.size () == 1)
	{
		// Primitive ?
		if (Selection.front ())
		{
			// What class is it ?
			const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*(Selection.front ()));
			if (primClass && primClass->DynamicChildren.size ())
			{
				// Add separator
				pMenu->AppendMenu (MF_SEPARATOR);

				// For each child, add a create method
				for (uint i=0; i<primClass->DynamicChildren.size (); i++)
				{
					pMenu->AppendMenu (MF_STRING, ID_EDIT_CREATE_BEGIN+i, ("Add "+primClass->DynamicChildren[i].ClassName).c_str ());
				}
			}

			// What class is it ?
			if (primClass && primClass->GeneratedChildren.size ())
			{
				// Add separator
				pMenu->AppendMenu (MF_SEPARATOR);

				// For each child, add a create method
				for (uint i=0; i<primClass->GeneratedChildren.size (); i++)
				{
					pMenu->AppendMenu (MF_STRING, ID_EDIT_GENERATE_BEGIN+i, ("Generate "+primClass->GeneratedChildren[i].ClassName).c_str ());
				}
			}

			// What class is it ?
			if (primClass)
			{
				// Look for files
				vector<string> filenames;

				// Filenames
				buildFilenameVector (*Selection.front (),  filenames);

				// File names ?
				if (!filenames.empty ())
				{
					// Add separator
					pMenu->AppendMenu (MF_SEPARATOR);

					// Found ?
					uint i;
					for (i=0; i<filenames.size (); i++)
					{
						// Add a menu entry
						pMenu->AppendMenu (MF_STRING, ID_EDIT_OPEN_FILE_BEGIN+i, ("Open "+NLMISC::CFile::getFilename (filenames[i])).c_str ());
					}
				}
			}
		}
	}

	pMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, parent);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditAddPoint(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((_SelectionLocked && (_Mode==1)) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditLock(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isInteraction () && (_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditRotate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditScale(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isInteraction () && (_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditTranslate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditTurn(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditRadius(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditSelect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ((_Mode==1) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditDetails(CCmdUI* pCmdUI) 
{	
}

// ***************************************************************************

void CMainFrame::OnUpdateViewLandscape(CCmdUI* pCmdUI) 
{	
}

// ***************************************************************************

void CMainFrame::deletePrimitiveClipboard ()
{
	// Delete each clipboard entry
	for (uint i=0; i<_PrimitiveClipboard.size (); i++)
		delete _PrimitiveClipboard[i];
	_PrimitiveClipboard.clear ();
}

// ***************************************************************************

void CMainFrame::OnEditCopy() 
{
	// Only if one node selected
	if (!Selection.empty ())
	{
		// Free the clipboard
		deletePrimitiveClipboard ();

		// Get the document
		CWorldEditorDoc *doc = getDocument ();

		// Add primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
		while (ite != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite);

			// Copy it
			_PrimitiveClipboard.push_back (locator.Primitive->copy ());

			// Unselect it
			getPrimitiveEditor(_PrimitiveClipboard.back ())->setSelected(false);
//			_PrimitiveClipboard.back ()->removePropertyByName ("selected");

			ite++;
		}

		// Update data
		updateData ();
	}
}
// ***************************************************************************

void CMainFrame::OnEditCut() 
{
	// Only if one node selected
	if (!Selection.empty ())
	{
		// Copy
		OnEditCopy();

		// Delete
		deletePrimitive (false, "Cut");

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	// Something selected ?
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	// Something selected ?
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void CMainFrame::OnEditPaste() 
{
	// Only if one node selected
	if ((!Selection.empty ()) && (!_PrimitiveClipboard.empty ()))
	{
		// Get the document
		CWorldEditorDoc *doc = getDocument ();
		sint lastChildPos = -1; // position of the last child of the selection

		// Begin modification
		doc->beginModification ();

		for (uint i=0; i<_PrimitiveClipboard.size (); i++)
		{
			// Paste it ?
			CDatabaseLocatorPointer locator;
			bool paste = false;

			// Is this a root ?
			if (Selection.front ()->getParent ())
			{
				// Try to add it as children of the selection
				if (theApp.Config.canBeChild (*(_PrimitiveClipboard[i]), *(Selection.front ())))
				{
					// Get last children index (before insertion)
					if ( lastChildPos == -1 )
						lastChildPos = Selection.front()->getNumChildren()-1;
					
					if ( lastChildPos >= 0 )
					{
						IPrimitive* prim;
						Selection.front()->getChild( prim, lastChildPos );
						doc->getLocator ( locator, prim );

						if ( ! locator.nextChild() )
							locator.appendChild ( locator );
					}
					else
					{
						doc->getLocator ( locator, Selection.front() );
						locator.appendChild ( locator );
					}

					
					paste = true;
				}
				else
				{
					// Can be a child ?
					if (theApp.Config.canBeChild (*(_PrimitiveClipboard[i]), *(Selection.front ()->getParent ())))
					{
						// Paste after this locator
						doc->getLocator ( locator, Selection.front() );
						if	( !locator.nextChild () )
							locator.appendChild ( locator );
						paste = true;
					}
				}
			}
			else
			{
				// Try to add it as children of the selection
				if (theApp.Config.canBeRoot (*(_PrimitiveClipboard[i])))
				{
					// Get first children
					doc->getLocator (locator, Selection.front ());
					locator.appendChild (locator);
					paste = true;
				}
			}

			// Paste it ?
			if (paste)
			{
				// Add the primitive
				doc->addModification (new CActionAddPrimitive (*(_PrimitiveClipboard[i]), locator));

				// New locator
				CDatabaseLocator newLocator = locator;
				doc->getLocator (locator, newLocator);

				// Change its name
				string name;
				if (locator.Primitive->getPropertyByName ("name", name))
				{
					// Get children count
					if (locator.Primitive->getParent ())
					{
						const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*locator.Primitive);
						if ((primClass == NULL) || primClass->Numberize)
						{
							// Set the number to the number of brothers (-1 because the node has just been created)
							name = numberize (name.c_str (), locator.Primitive->getParent ()->getNumChildren ());

							// Change the name
							doc->addModification (new CActionSetPrimitivePropertyString (locator, "name", name.c_str (), false));
						}
					}
				}
			}
			else
			{
				//	special case.
				if (	_PrimitiveClipboard[i]
					&&	_PrimitiveClipboard[i]->getClassName()=="CPrimZone"
					&&	Selection.front ()
					&&	Selection.front ()->getClassName()=="CPrimZone"
					&&	Selection.front ()->getParent()	)
				{
					// Get the child class
					CDatabaseLocatorPointer insert;
					doc->getLocator (insert, Selection.front ()->getParent());
					CDatabaseLocator dest;
					insert.appendChild (dest);

					// Position on the Display
					CDisplay *display = getDisplay ();
					float delta = (float)DELTA_POS_ADD_PRIMITIVE * (display->_CurViewMax.x - display->_CurViewMin.x) / (float)display->getWidth ();
				
					string className;
					if	(Selection.front ()->getPropertyByName ("class", className))
					{
						std::vector<NLLIGO::CPrimitiveClass::CInitParameters>	empty;
						doc->addModification (new CActionAddPrimitiveByClass (dest, className.c_str (), getDisplay ()->_CurPos, 
						delta, empty));
					}

					//	Now copy the shape ..
					sint	nbVert=_PrimitiveClipboard[i]->getNumVector();
					const CPrimVector *vectors = _PrimitiveClipboard[i]->getPrimVector ();
					for (sint j=nbVert-1; j>=0; j--)
						doc->addModification (new	CActionAddVertex(dest, vectors[j]));
				}

			}

		}

		// Begin modification
		doc->endModification ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	// Only if one node selected and clipboard not empty
	pCmdUI->Enable (isInteraction () && ((!Selection.empty()) && (!_PrimitiveClipboard.empty ())) ? TRUE : FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateViewHide(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void CMainFrame::OnViewHide() 
{
	// Only if one node selected
	if (!Selection.empty ())
	{
		// Get the document
		CWorldEditorDoc *doc = getDocument ();

		// Begin modifications
		doc->beginModification ();

		// Copy selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
		while (ite != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite);

			// Add the modification
			doc->addModification (new CActionShowHide (locator, false));

			ite++;
		}

		// End modifications
		doc->endModification ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnViewShow() 
{
	// Only if one node selected
	if (!Selection.empty ())
	{
		// Get the document
		CWorldEditorDoc *doc = getDocument ();

		// Begin modifications
		doc->beginModification ();

		// Copy selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
		while (ite != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite);

			// Add the modification
			doc->addModification (new CActionShowHide (locator, true));

			ite++;
		}

		// End modifications
		doc->endModification ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnEditExpand() 
{
	// Only if one node selected
	if (!Selection.empty ())
	{
		// Get the document
		CWorldEditorDoc *doc = getDocument ();

		// Copy selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
		while (ite != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite);

			// Add the modification
			getPrimitiveEditor(locator.Primitive)->setExpanded(true);
//			locator.Primitive->
//			const_cast<IPrimitive*> (locator.Primitive)->Expanded = true;
			InvalidatePrimitiveRec (locator, LogicTreeStruct);

			ite++;
		}

		// Invalidate tools
		invalidateTools ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnEditCollapse() 
{
	// Only if one node selected
	if (!Selection.empty ())
	{
		// Get the document
		CWorldEditorDoc *doc = getDocument ();

		// Copy selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
		while (ite != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite);

			// Add the modification
			getPrimitiveEditor(locator.Primitive)->setExpanded(false);
//			const_cast<IPrimitive*> (locator.Primitive)->Expanded = false;
			InvalidatePrimitiveRec (locator, LogicTreeStruct);

			ite++;
		}

		// Invalidate tools
		invalidateTools ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnUpdateViewShow (CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditExpand (CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void CMainFrame::OnUpdateEditCollapse (CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void selectChildren (CWorldEditorDoc *doc, const CDatabaseLocatorPointer &locator)
{
	// Fopr each children
	CDatabaseLocatorPointer myLocator = locator;
	if (myLocator.firstChild ())
	{
		do
		{
			// Select it
			doc->addModification (new CActionSelect (myLocator));

			// Select its children
			selectChildren (doc, myLocator);
		}
		while (myLocator.nextChild ());
	}
}

// ***************************************************************************

void CMainFrame::OnEditSelectChildren() 
{
	// Only if one node selected
	if (!Selection.empty ())
	{
		// Get the document
		CWorldEditorDoc *doc = getDocument ();

		// Begin modifications
		doc->beginModification ();

		// Unselect all
		doc->addModification (new CActionUnselectAll ());

		// Copy selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
		while (ite != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite);

			// Select children
			selectChildren (doc, locator);

			ite++;
		}

		// End modifications
		doc->endModification ();

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnUpdateEditSelectChildren(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void CMainFrame::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == TIMER_UPDATE_FILES)
	{
		if (TimerEnabled)
		{
			TimerEnabled = false;
			// Check file modification
			getDocument ()->updateFiles ();
			TimerEnabled = true;
		}
	}
	if (nIDEvent == TIMER_PLUGINS)
	{
		for (uint i=0; i<theApp.Plugins.size(); ++i)
		{
			theApp.Plugins[i]->onIdle();
		}
		// check if plugin querried to refresh the property dialog
		if (_MustRefreshPropertyDialog)
		{
			std::list<CDialogProperties*>::iterator it = PropertiesDialogs.begin();

			while ( it != PropertiesDialogs.end() )
			{
				(*it)->updateModification();
				it++;
			}

			//PropertyDialog.changeSelection (Selection);
			_MustRefreshPropertyDialog = false;
		}
	}	
	CFrameWnd::OnTimer(nIDEvent);
}

// ***************************************************************************

void CMainFrame::OnHelpFinder()
{
	if (Selection.size () == 1)
	{
		CWorldEditorDoc *doc = getDocument ();
		
		// For each selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite1 = Selection.begin ();
		
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, *ite1);
		
		const IPrimitive *primitive = locator.Primitive;
		if (primitive)
		{
			string className;
			bool Success;
			Success = primitive->getPropertyByName ("class", className);
			if (!Success)
			{
				// is this the root node ?
				const IPrimitive *parent = primitive->getParent();
				if (!parent)
				{
					// if no parent, we have a root node
					className = "root";
					Success = true;
				}
			}
			if (Success)
			{
				//string filename = theApp.ExePath+"doc/"+className+".html";
				string filename = theApp.DocPath+"/"+className+".html";
				if (!NLMISC::CFile::fileExists(filename) || !openFile (filename.c_str ()))
				{
					//openFile ((theApp.ExePath+"world_editor.html").c_str ());
					theApp.errorMessage ("Can't open the file %s", filename.c_str ());
				}
			}
		}
	}
	else
	{
		openFile ((theApp.ExePath+"world_editor.html").c_str ());
	}
}

// ***************************************************************************

void CMainFrame::interaction (bool enable)
{
	CDisplay *display = getDisplay ();
	if (display)
		display->Interactif = enable;
}

// ***************************************************************************

bool CMainFrame::isInteraction () const
{
	CDisplay *display = getDisplay ();
	if (display)
		return display->Interactif;
	else
		return false;
}

// ***************************************************************************
// CWorldEditorProgressCallback
// ***************************************************************************

CWorldEditorProgressCallback::CWorldEditorProgressCallback ()
{
	getMainFrame ()->launchLoadingDialog ("");
}

// ***************************************************************************

CWorldEditorProgressCallback::~CWorldEditorProgressCallback ()
{
	getMainFrame ()->terminateLoadingDialog ();
}

// ***************************************************************************

void CWorldEditorProgressCallback::progress (float value)
{
	getMainFrame ()->LoadingDialog->setText (this->DisplayString);
	getMainFrame ()->progressLoadingDialog (value);
}

// ***************************************************************************

void CMainFrame::OnViewLocateselectedprimitives() 
{
	// Not empty ?
	if (!Selection.empty ())
	{
		// Selection bbox
		CAABBox box;
		bool first = true;

		// For each selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite = Selection.begin ();
		while (ite != Selection.end ())
		{
			// Extend
			uint vertices = (*ite)->getNumVector ();
			if (vertices)
			{
				uint i;
				const CPrimVector *v = (*ite)->getPrimVector ();
				for (i=0; i<vertices; i++)
				{
					// Extend the bbox
					if (first)
					{
						box.setCenter (v[i]);
						first = false;
					}
					else
						box.extend (v[i]);
				}
			}

			ite++;
		}

		// Change the display settings
		getDisplay ()->setDisplayRegion (box.getMin (), box.getMax ());

		// Add a border
		CVector locateBorder (0, 0, 0);
		CVector locateBorder2 (LOCATE_BORDER, LOCATE_BORDER, 0);
		getDisplay ()->pixelToWorld (locateBorder);
		getDisplay ()->pixelToWorld (locateBorder2);
		locateBorder = locateBorder2 - locateBorder;
		getDisplay ()->setDisplayRegion (box.getMin ()-locateBorder, box.getMax ()+locateBorder);

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnUpdateViewLocateselectedprimitives(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void CMainFrame::OnViewLocateselectedprimitivesTree() 
{
	if (!Selection.empty ())
	{
		CToolsLogic *const toolWnd = dynamic_cast<CToolsLogic*>(getMainFrame ()->m_wndSplitter.GetPane(0,1));
		if (toolWnd)
		{
			for (std::list<NLLIGO::IPrimitive*>::iterator it=Selection.begin(),itEnd=Selection.end();it!=itEnd;++it)
			{
				IPrimitiveEditor *const primEditor = dynamic_cast<IPrimitiveEditor*> (*it);
				toolWnd->ensureVisible (primEditor);				
			}			
		}
	}
}

// ***************************************************************************

void CMainFrame::OnUpdateViewLocateselectedprimitivesTree(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( isInteraction () && (Selection.empty () || (_Mode != 1)) ? FALSE : TRUE);
}

// ***************************************************************************

void CMainFrame::OnEditSelectByLocation() 
{
	// Only in primitive mode
	if ( (_Mode==1) && !isSelectionLocked () )
	{
		// Selection dialog
		static CSelectByLocation select;
		uint mode;
		switch (mode=select.DoModal ())
		{
		case IDOK:
		case IDMORE:
			{
				// Begin modification
				CWorldEditorDoc *doc = getDocument ();
				doc->beginModification ();

				// Unselect others primitives ?
				if (mode == IDOK)
					doc->addModification (new CActionUnselectAll ());

				// Make a bbox
				const float threshold = (float) fabs (select.Threshold);
				CVector min = CVector (select.X - threshold, select.Y - threshold, 0);
				CVector max = CVector (select.X + threshold, select.Y + threshold, 0);

				// Get the locator
				std::vector<CDatabaseLocatorPointer> result;
				getDisplay ()->pickRect (min, max, result, false);

				// Parse
				uint i;
				for (i=0; i<result.size (); i++)
				{
					// Select it
					doc->addModification (new CActionSelect (result[i]));
				}

				// End modification
				doc->endModification ();
			}
		}

		// Update data
		updateData ();
	}
}

// ***************************************************************************

void CMainFrame::OnUpdateEditSelectByLocation(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( (isInteraction () && (_Mode == 1)) ? TRUE : FALSE );
}

// ***************************************************************************

void CMainFrame::OnProjectResetuniqueid() 
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Begin modifications
	doc->beginModification ();
	
	uint count = doc->getNumDatabaseElement ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Primitive ?
		if (doc->isPrimitive (i))
		{
			const NLLIGO::CPrimitives &primitive = doc->getDatabaseElements (i);
			doc->resetUniqueID (*primitive.RootNode);
		}
	}

	// End modifications
	doc->endModification ();
}

// ***************************************************************************

void CMainFrame::OnProjectGeneratenullid() 
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Begin modifications
	doc->beginModification ();
	
	uint count = doc->getNumDatabaseElement ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Primitive ?
		if (doc->isPrimitive (i))
		{
			const NLLIGO::CPrimitives &primitive = doc->getDatabaseElements (i);
			doc->resetUniqueID (*primitive.RootNode, true);
		}
	}

	// End modifications
	doc->endModification ();

}

// ***************************************************************************

void CMainFrame::OnProjectForceiduniqueness()
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Begin modifications
	doc->beginModification ();

	CHashSet<std::string>			ids;
	std::vector<CWorldEditorDoc::TPropertyNonUnique>		nonUnique;
//	uint32	regenCount = 0;
	
	uint count = doc->getNumDatabaseElement ();
	uint i;

	// the operation is done in two step to avoid the assignation of already
	// assigned ID to the first non unique ID found, generating more non
	// unique ID when the loop reach the other primitives.

	// first loop to collect non properties with non unique ID
	for (i=0; i<count; i++)
	{
		// Primitive ?
		if (doc->isPrimitive (i))
		{
			const NLLIGO::CPrimitives &primitive = doc->getDatabaseElements (i);
			doc->forceIDUniqueness(*primitive.RootNode, ids, nonUnique);
//			doc->forceIDUniqueness(*primitive.RootNode, ids, regenCount);
		}
	}

	// second loop to regen the non unique IDs
	for (uint i=0; i<nonUnique.size(); ++i)
	{
		CWorldEditorDoc::TPropertyNonUnique &nu = nonUnique[i];

		const NLLIGO::CPrimitiveClass *_class = theApp.Config.getPrimitiveClass (*nu.Primitive);

		if (_class)
		{
			for (uint j=0; j<_class->Parameters.size(); ++j)
			{
				if (_class->Parameters[j].Name == nu.PropertyName)
				{
					if (_class->Parameters[j].Type == CPrimitiveClass::CParameter::String)
					{
						CDatabaseLocatorPointer locator;
						doc->getLocator(locator, nu.Primitive);
						
						doc->addModification (new CActionSetPrimitivePropertyString (locator, nu.PropertyName.c_str (), toString (getUniqueId ()).c_str (), false));
					}
					else if (_class->Parameters[j].Type == CPrimitiveClass::CParameter::StringArray)
					{
						uint k;
						for (k=0; j<_class->Parameters[j].DefaultValue.size (); j++)
						{
							// Unique Id ?
							if (_class->Parameters[j].DefaultValue[k].GenID)
							{
								std::vector<string> result;
								std::vector<string> *resultPtr = NULL;
								nu.Primitive->getPropertyByName (nu.PropertyName.c_str (), resultPtr);

								// Copy
								if (resultPtr)
									result = *resultPtr;

								// Resize
								if (result.size ()<=k)
									result.resize (k+1);

								// Set the value
								result[k] = toString (getUniqueId ());

								CDatabaseLocatorPointer locator;
								doc->getLocator (locator, nu.Primitive);
								doc->addModification (new CActionSetPrimitivePropertyStringArray (locator, nu.PropertyName.c_str (), result, false));
							}
						}
					}
					break;
				}
			}
		}
	}

	// End modifications
	doc->endModification ();

	AfxMessageBox(NLMISC::toString("%u ids checked, %u non unique ID regenerated", ids.size()+nonUnique.size(), nonUnique.size()).c_str(), MB_OK);
}

// ***************************************************************************

void CMainFrame::OnEditFind () 
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	if (doc->getNumDatabaseElement ())
	{
		// Find dialog
#if 0
		CFindPrimitiveDlg dlg;
		dlg.DoModal ();
#else
		if (!m_FindPrimitiveDlg)
			m_FindPrimitiveDlg = new CFindPrimitiveDlg;
		
		if (!::IsWindow(m_FindPrimitiveDlg->GetSafeHwnd()))
			m_FindPrimitiveDlg->Create(IDD_FIND_PRIMITIVE, this);
		
		m_FindPrimitiveDlg->ShowWindow(SW_SHOW); 
#endif
	}
}


// ***************************************************************************

void CMainFrame::OnEditGoto () 
{
	// goto dialog

	m_GotoDlg.DoModal();
}

// ***************************************************************************

void CMainFrame::OnUpdateFind(CCmdUI* pCmdUI) 
{
	CWorldEditorDoc *doc = getDocument ();
	pCmdUI->Enable ( (isInteraction () && (_Mode == 1) && doc->getNumDatabaseElement ()) ? TRUE : FALSE );
}

// ***************************************************************************

void CMainFrame::OnViewCollisions() 
{
	// Toggle Collisions View 
	_ShowCollisions ^= true;
	CMenu *menu = GetMenu();
	if (_ShowCollisions)
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_COLLISIONS);
		menu->CheckMenuItem (ID_VIEW_COLLISIONS, MF_CHECKED|MF_BYCOMMAND);
	}
	else
	{
		// Update UI
		m_wndToolBar.GetToolBarCtrl ().CheckButton (ID_VIEW_COLLISIONS, FALSE);
		menu->CheckMenuItem (ID_VIEW_COLLISIONS, MF_UNCHECKED|MF_BYCOMMAND);
	}
	
	// Invalidate left view
	invalidateLeftView ();
	
	// Update data
	updateData ();
}

// ***************************************************************************

// routines for the plugin
NLLIGO::IPrimitive *CMainFrame::createRootPluginPrimitive (const char *name)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	
	// Add the landscape in the project
	CWorldEditorDoc *doc = getDocument ();

	// Push back the primitive
	doc->_DataHierarchy.push_back (CWorldEditorDoc::CDatabaseElement (CWorldEditorDoc::CDatabaseElement::Primitive));

	// This primitive is not editable
	doc->_DataHierarchy.back().Editable = false;
	doc->_DataHierarchy.back().Filename = name;

	doc->_DataHierarchy.recomputePointerArray ();

	// Invalidate tools
	getMainFrame ()->invalidateTools ();

	// Invalidate pointers
	InvalidateAllPrimitives ();

	// Modify files
	uint index = doc->_DataHierarchy.size ()-1;
	doc->modifyDatabase (index);
	
	// Update data
	updateData ();

	return doc->_DataHierarchy[index].Primitives.RootNode;
}

// ***************************************************************************

void CMainFrame::deleteRootPluginPrimitive (void)
{

}

// ***************************************************************************
void CMainFrame::getAllRootPluginPrimitive (std::vector<NLLIGO::IPrimitive*> &prims)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	
	prims.clear();

	// Add the landscape in the project
	CWorldEditorDoc *doc = getDocument ();
	nlassert(doc);

	// For all editable primitive only
	for (uint j=0; j<doc->_DataHierarchy.size(); ++j)
	{
		if (doc->_DataHierarchy[j].Editable)
		{
			if (doc->_DataHierarchy[j].Type == CWorldEditorDoc::CDatabaseElement::Primitive)
				prims.push_back(doc->_DataHierarchy[j].Primitives.RootNode);
		}
	}
	
}

// ***************************************************************************

const NLLIGO::IPrimitive *CMainFrame::createPluginPrimitive
(
 const char *className, 
 const char *primName, 
 const NLMISC::CVector &initPos, 
 float deltaPos, 
 const std::vector<CPrimitiveClass::CInitParameters> &initParameters,
 NLLIGO::IPrimitive *parent
 )
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	CWorldEditorDoc *doc = getDocument ();
	
	// What class is it ?
	const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass(className);
	if (primClass)
	{
		// Begin modification
		nlassert (primClass);
		
		// Get the child class
		CDatabaseLocatorPointer	insert;
		doc->getLocator (insert, parent);

		// Locator for the child
		CDatabaseLocator child;
		insert.appendChild (child);
		
		// Get the class primitive
		IPrimitive *primitive = const_cast<IPrimitive*> (doc->createPrimitive (child, className, "", initPos, deltaPos, initParameters));
		if (primitive != NULL)
		{
			// Invalidate left view
			invalidateLeftView ();
			getMainFrame ()->invalidateTools ();

			// Modify files
			doc->modifyDatabase (child.getDatabaseIndex ());

			// Get a pointer on the new primitive
			primitive->removePropertyByName ("name");
			primitive->addPropertyByName ("name",new CPropertyString (primName));

			// Invalidate quad grid
			CDatabaseLocatorPointer childPtr;
			doc->getLocator (childPtr, child);
			InvalidatePrimitiveRec (childPtr, QuadTree|LogicTreeStruct|SelectionState);
		}
	
		// Update the data
		updateData ();
		return primitive;
	}
	else
		return NULL;
}

// ***************************************************************************

// delete a plugin primitive
void CMainFrame::deletePluginPrimitive (const NLLIGO::IPrimitive *primitive)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	// Get the parent
	IPrimitive *parent = const_cast<IPrimitive*>(primitive)->getParent ();
	nlassert (parent);
	
	// Get the child id
	uint childId;
	nlverify (parent->getChildId (childId, primitive));
	
	// Delete the child
	nlverify (parent->removeChild (childId));
	invalidateLeftView ();
}

// ***************************************************************************

// indicates to the WorldEditor that the primitive has changed
void CMainFrame::invalidatePluginPrimitive (const NLLIGO::IPrimitive *primitive, uint channels)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	if (primitive)
	{
		IPrimitiveEditor *primitiveEditor = dynamic_cast<IPrimitiveEditor*> (const_cast<IPrimitive*> (primitive));
		
		// No, link it
		if (primitiveEditor->_ModifiedIterator == ModifiedPrimitive.end ())
		{
			ModifiedPrimitive.push_front (const_cast<IPrimitive*> (primitive));
			primitiveEditor->_ModifiedIterator = ModifiedPrimitive.begin ();
		}
		
		// Add the channel flags
		primitiveEditor->_Channels |= channels;
	}
	if (channels | QuadTree)
	{
		invalidateLeftView ();
	}
}

// ***************************************************************************
void CMainFrame::invalidateLeftView()
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	::invalidateLeftView();
}

// ***************************************************************************

// indicates to the WorldEditor that the primitive has changed
void CMainFrame::getWindowCoordinates(NLMISC::CVector &vmin, NLMISC::CVector &vmax)
{
	vmin = DispWnd->_CurViewMin;
	vmax = DispWnd->_CurViewMax;
}

// ***************************************************************************

void CMainFrame::OnHelpHistory() 
{
	string filename = theApp.ExePath+"history.txt";
	if (!openFile (filename.c_str()))
	{
		theApp.errorMessage ("Can't open the file %s", filename.c_str ());
	}
}

// ***************************************************************************

void CMainFrame::OnExportSnapshot() 
{
	CCustomSnapshot snapShot (this);
	if (snapShot.DoModal () == IDOK)
	{
		CFileDialogEx dialog (BASE_REGISTRY_KEY, "image", FALSE, "tga", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Targa Files (*.tga)|*.tga|All Files (*.*)|*.*||", this);
		if (dialog.DoModal() == IDOK)
		{
			if (snapShot.FixedSize == 0)
				_ZoneBuilder->snapshot (dialog.GetPathName(), (uint)theApp.Config.CellSize, snapShot.OutputRGB == 1);
			else
				_ZoneBuilder->snapshotCustom (dialog.GetPathName(), snapShot.Width, snapShot.Height, snapShot.KeepRatio != FALSE, 
					(uint)theApp.Config.CellSize, snapShot.OutputRGB == 1);
		}
	}
}

// ***************************************************************************

void CMainFrame::OnWindowsPrimitiveconfiguration() 
{
	PrimitiveConfigurationDlg.ShowWindow (PrimitiveConfigurationDlg.IsWindowVisible()?SW_HIDE:SW_SHOW);
}

// ***************************************************************************

void CMainFrame::OnUpdateWindowsPrimitiveconfiguration(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(PrimitiveConfigurationDlg.IsWindowVisible()?1:0);
}

// ***************************************************************************

void CMainFrame::OnProjectResetPrimitiveConfiguration() 
{
	theApp.reloadPrimitiveConfiguration ();
}

// ***************************************************************************

const std::list<NLLIGO::IPrimitive*> &CMainFrame::getCurrentSelection()
{
	return Selection;
}

// ***************************************************************************
// Allow a plugin to modify the primitive selection

void CMainFrame::setCurrentSelection(std::vector<NLLIGO::IPrimitive*>& newSelection)
{
	CWorldEditorDoc *doc=getDocument();
	list<IPrimitive*>::iterator iteSelection;
	doc->beginModification();
	//unselect the curent list
	for (std::list<NLLIGO::IPrimitive*>::iterator it=Selection.begin(),itEnd=Selection.end();it!=itEnd;++it)
	{
		CDatabaseLocatorPointer locatorCurrent;
		doc->getLocator (locatorCurrent, (*it) );
		doc->addModification (new CActionUnselect (locatorCurrent));
	}

	//select the new selection
	for(uint i=0;i<newSelection.size();i++)
	{
		CDatabaseLocatorPointer locatorCurrent;
		doc->getLocator (locatorCurrent, newSelection.at(i));

		doc->addModification (new CActionSelect (locatorCurrent));
	}
	doc->endModification();

	getMainFrame()->updateData ();
	getMainFrame()->OnViewLocateselectedprimitives ();
	getMainFrame()->OnViewLocateselectedprimitivesTree ();
}

// ***************************************************************************
// use the root file name in order to retrieve the correct root node
const NLLIGO::IPrimitive* CMainFrame::getRootNode(const std::string& rootFileName)
{

	CDatabaseLocatorPointer locator;
	CWorldEditorDoc *doc=getDocument();
	doc->getFirstLocator(locator);
	
	locator.getRoot(rootFileName);

	return locator.Primitive;
}

// ***************************************************************************
// retrieve the file name associated with the primitive of the root node
// it must be the root node 
std::string& CMainFrame::getRootFileName(NLLIGO::IPrimitive* rootNode)
{
	
	CDatabaseLocatorPointer locator;
	CWorldEditorDoc *doc=getDocument();
	doc->getLocator(locator,rootNode);


	return locator.getRootFileName(rootNode);
}

// ***************************************************************************


void CMainFrame::registerPrimitiveDisplayer(IPrimitiveDisplayer *displayer, const std::vector<std::string> &primClassNames)
{
	for (uint i=0; i<primClassNames.size(); ++i)
		theApp.PrimitiveDisplayers[primClassNames[i]] = displayer;
}

// ***************************************************************************
std::string CMainFrame::sheetIdToSheetName(NLMISC::CSheetId sheetID) const
{
	return sheetID.toString();
}

// ***************************************************************************
const std::list<NLLIGO::IPrimitive*> &CMainFrame::getCurrentSelection() const
{
	return Selection;
}

// ***************************************************************************
bool CMainFrame::isSelected(const NLLIGO::IPrimitive &prim) const
{
	const IPrimitiveEditor *pe = dynamic_cast<const IPrimitiveEditor *>(&prim);
	nlassert(pe);
	return pe->getSelected();
}

// ***************************************************************************
void CMainFrame::setPrimitiveHideFlag(NLLIGO::IPrimitive &prim, bool hidden)
{
	getPrimitiveEditor (&prim)->setHidden(hidden);
}

// ***************************************************************************

CPrimTexture *CMainFrame::createTexture()
{
	return new CPrimTexture;
}

// ***************************************************************************

void CMainFrame::deleteTexture(CPrimTexture *tex)
{
	delete tex;
}

// ***************************************************************************

bool CMainFrame::buildNLBitmapFromTGARsc(HRSRC bm, HMODULE hm, NLMISC::CBitmap &dest)
{
	HGLOBAL rsc = LoadResource(hm, bm);
	if (rsc == NULL) return false;
	uint numBytes = SizeofResource(hm, bm);
	if (numBytes == 0) return false;
	LPVOID dataPtr = LockResource(rsc);
	if (!dataPtr) return false;
	NLMISC::CMemStream ms;
	ms.serialBuffer((uint8 *) dataPtr, numBytes);
	NLMISC::CBitmap tmpBitmap;
	ms.invert();
	try
	{
		ms.seek(0, NLMISC::IStream::begin);
		tmpBitmap.load(ms);
	}
	catch(EStream &)
	{
		return false;
	}
	dest.swap(tmpBitmap);
	return true;
}



// ***************************************************************************

void CMainFrame::OnDestroy() 
{
	theApp.deletePlugins();
	CFrameWnd::OnDestroy();		
}

// ***************************************************************************

bool CMainFrame::CanDrop()
{
	for (uint i=0; i<_PrimitiveClipboard.size (); i++)
	{
		bool paste = false;

		// Is this a root ?
		if ( Selection.front() )
		{
			if ( Selection.front()->getParent() )
			{
				// Try to add it has children of the selection
				if (theApp.Config.canBeChild (*(_PrimitiveClipboard[i]), *(Selection.front ())))
				{
					paste = true;
				}
				else
				{
					// Can be a child ?
					if (theApp.Config.canBeChild (*(_PrimitiveClipboard[i]), *(Selection.front ()->getParent ())))
					{
						paste = true;
					}
				}
			}
			else
			{
				// Try to add it has children of the selection
				if (theApp.Config.canBeRoot (*(_PrimitiveClipboard[i])))
				{
					paste = true;
				}
			}
		}

		if ( !paste )
			return false;
	}

	return true;
}

// ***************************************************************************

void CMainFrame::OnSavePosition() 
{
	::CStdioFile file;
	::CString str;
	::CTime t = ::CTime::GetCurrentTime();

	const CVector &v = DispWnd->_CurPos;

	// Write the zone name in NeL protocol
	sint32 x, y;

	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	x = (sint32)floor(v.x / dispWnd->_CellSize);
	y = (sint32)floor(v.y / dispWnd->_CellSize);
	
	str.Format( "[%s] X = %.3f, Y = %.3f\r\n", t.Format( "%m/%d/%y, %H:%M:%S" ), v.x, v.y );
	
	file.Open( "position.txt", ::CFile::modeCreate|::CFile::modeNoTruncate|::CFile::modeWrite|::CFile::typeText );
	file.SeekToEnd();
	file.WriteString( str );
	file.Close();

	MessageBox( "Current coordinates saved in file position.txt", "Position saved", MB_OK );
}

// ***************************************************************************

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	// hDropInfo contains infos about dropped files
	if (hDropInfo)
	{
		CString Filename;

		// Get the number of files dropped
		uint numFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
		
		for (uint i=0 ; i<numFiles ; i++)
		{
			// Get the path of this file
			char* pFilename = Filename.GetBuffer(_MAX_PATH);
            DragQueryFile(hDropInfo, i, pFilename, _MAX_PATH);
			string path(pFilename);
			
			// Add it if it's a primitive not already loaded
			if (path.find(".primitive") != string::npos)
			{
				getDocument()->beginModification ();

				if (!getDocument()->isPrimitiveLoaded(path))
					getDocument ()->addModification (new CActionLoadPrimitive (path.c_str()));
				else
					infoMessage("Primitive already existing: %s", path.c_str());

				getDocument()->endModification ();

				// Verify primitive structures
				VerifyPrimitivesStructures();

				// Update data
				updateData ();
			}
			// Can be extended to support more file types
			else
			{
				errorMessage("File not supported: %s", pFilename);
			}
		}
	}
	
	// Release memory
	DragFinish(hDropInfo);
}

// ***************************************************************************

void CMainFrame::OnMissionCompiler()
{
	// list of selected primitives
	list<NLLIGO::IPrimitive*> sel = getCurrentSelection();

	// list of primitive files to compile
	vector<std::string> files;
	
	// store selected primitive files : no duplicate file
	for (list<NLLIGO::IPrimitive*>::iterator itSel=sel.begin() ; itSel!=sel.end() ; ++itSel)
	{
		// Get file path of selected primitive
		string path;
		CDatabaseLocatorPointer locator;
		CWorldEditorDoc *doc = getDocument();
		doc->getLocator(locator, *itSel);
		doc->getFilePath(locator.getDatabaseIndex(), path);

		// Check if already inserted
		vector<string>::iterator it = find(files.begin(), files.end(), path);
		if (it == files.end())
			files.push_back(path);		
	}
	
	// write filenames to a temp file for the mission compiler
	if (files.size() > 0)
	{
		// use system temp directory
		char tmpPath[MAX_PATH];
		GetEnvironmentVariable("TMP", tmpPath, MAX_PATH);
		strcat(tmpPath, "\\tmptool.txt");

		FILE *f = fopen(tmpPath, "w");
		if (f==NULL)
			infoMessage("Can't open file for writing !\n%s", tmpPath);

		for (uint i=0 ; i<files.size() ; i++)
			fprintf(f, "%s\n", files[i].c_str());

		fclose(f);
	}

	// launch mission compiler executable: the path is found in the config file
	CConfigFile::CVar *var = getMainFrame ()->getConfigFile().getVarPtr ("MissionCompilerPath");
	if (!var)
	{
		errorMessage("Can't find variable : ""MissionCompilerPath"" in world_editor_plugin.cfg");
		return;
	}
	
	char path[MAX_PATH];
	strcpy(path, var->asString().c_str());

	SHELLEXECUTEINFO ExecuteInfo;    
	memset(&ExecuteInfo, 0, sizeof(ExecuteInfo));
    
	ExecuteInfo.cbSize       = sizeof(ExecuteInfo);
	ExecuteInfo.fMask        = 0;
	ExecuteInfo.hwnd         = 0;
	ExecuteInfo.lpVerb       = "open";
	ExecuteInfo.lpFile       = "mission_compiler_fe_r.exe";
	ExecuteInfo.lpParameters = 0;
	ExecuteInfo.lpDirectory  = path;
	ExecuteInfo.nShow        = SW_SHOW;
	ExecuteInfo.hInstApp     = 0;

	if(ShellExecuteEx(&ExecuteInfo) == FALSE)
		errorMessage("File not found : mission_compiler_fe_r.exe !");
}

// ***************************************************************************

void CMainFrame::OnNameDlg()
{
	CNameDlg dlg;
	dlg.setSelection(getCurrentSelection());
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
		infoMessage("Files saved !");
}

// ***************************************************************************
