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

#include "stdafx.h"
#include "georges_edit.h"
#include "georges_edit_view.h"
#include "georges_edit_doc.h"
#include "settings_dialog.h"
#include "child_frm.h"

#include "main_frm.h"

#include "nel/misc/debug.h"

using namespace NLMISC;
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_SETTINGS, OnSettings)
	ON_WM_CLOSE()
	ON_COMMAND(ID_VIEW_DOCKINGDIALOGBAR, OnViewDockingdialogbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOCKINGDIALOGBAR, OnUpdateViewDockingdialogbar)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_OUTPUT_CONSOLE, OnViewOutputConsole)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT_CONSOLE, OnUpdateViewOutputConsole)
	ON_COMMAND(ID_VIEW_GOTO_FILE_BROWSER, OnViewGotoFileBrowser)
	ON_COMMAND(ID_VIEW_GOTO_OUTPUT_CONSOLE, OnViewGotoOutputConsole)
	ON_COMMAND(ID_MODULES_0, OnModules0)
	ON_UPDATE_COMMAND_UI(ID_MODULES_0, OnUpdateModules0)
	ON_COMMAND(ID_MODULES_1, OnModules1)
	ON_UPDATE_COMMAND_UI(ID_MODULES_1, OnUpdateModules1)
	ON_COMMAND(ID_MODULES_2, OnModules2)
	ON_UPDATE_COMMAND_UI(ID_MODULES_2, OnUpdateModules2)
	ON_COMMAND(ID_MODULES_3, OnModules3)
	ON_UPDATE_COMMAND_UI(ID_MODULES_3, OnUpdateModules3)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SETTINGS, OnUpdateViewSettings)
	ON_COMMAND(ID_BROWSER_SORTED_BY_TYPE, OnBrowserSortedByType)
	ON_WM_KEYDOWN()
	ON_UPDATE_COMMAND_UI(ID_BROWSER_SORTED_BY_TYPE, OnUpdateBrowserSortedByType)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// m_bAutoMenuEnable = FALSE;
	m_bDontClose = false;
	BrowserSortedByType = false;
}

CMainFrame::~CMainFrame()
{
	ErrorLog->removeDisplayer (&Displayer);
	WarningLog->removeDisplayer (&Displayer);
	InfoLog->removeDisplayer (&Displayer);
	DebugLog->removeDisplayer (&Displayer);
	AssertLog->removeDisplayer (&Displayer);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Icon bar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// Dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Create the file browser
	if (!FileBrowser.Create(this, &FileBrowserDlg, CString("File browser"), IDD_FILE_BROWSER))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

	// Docking bar
	ShowControlBar (&FileBrowser, TRUE, FALSE);
    FileBrowser.SetBarStyle (FileBrowser.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	FileBrowser.EnableDocking (CBRS_ALIGN_ANY);
	DockControlBar (&FileBrowser);

	// Menu file browser
	CMenu *menu = GetMenu();
	menu->CheckMenuItem(ID_VIEW_DOCKINGDIALOGBAR, MF_CHECKED);

	// Create the output console
	if (!OutputConsole.Create(this, &OutputConsoleDlg, CString("Output"), IDD_OUTPUT_CONSOLE))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}

	// Docking bar
	ShowControlBar (&OutputConsole, TRUE, FALSE);
    OutputConsole.SetBarStyle (OutputConsole.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	OutputConsole.EnableDocking (CBRS_ALIGN_ANY);
	DockControlBar (&OutputConsole, AFX_IDW_DOCKBAR_BOTTOM);

	// Menu file browser
	menu = GetMenu();
	menu->CheckMenuItem(ID_VIEW_OUTPUT_CONSOLE, MF_CHECKED);

	// Init displayer
	ErrorLog->addDisplayer (&Displayer);
	WarningLog->addDisplayer (&Displayer);
	InfoLog->addDisplayer (&Displayer);
	DebugLog->addDisplayer (&Displayer);
	AssertLog->addDisplayer (&Displayer);

	// JC: added LoadBarState
	LoadBarState("Georges");

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	if (createX != -1)
		cs.x = createX;
	if (createY != -1)
		cs.y = createY;
	if (createCX != -1)
		cs.cx = createCX;
	if (createCY != -1)
		cs.cy = createCY;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers



void CMainFrame::OnSettings() 
{
	if (theApp.Superuser)
	{
		CSettingsDialog dialog (this);
		dialog.DoModal ();
		theApp.OnViewRefresh ();
	}
}

bool CMainFrame::dataToClipboard (UINT format, void *data, uint len)
{
	// Open the clipboard
	if (OpenClipboard ())
	{
		if (data)
		{
			// Alloc a global memory
			HGLOBAL hData = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE, len);
			if (hData)
			{
				// Copy the string
				LPVOID dataPtr = GlobalLock (hData);
				nlverify (dataPtr);
				memcpy (dataPtr, data, len);

				// Release the pointer
				GlobalUnlock (hData);

				// Set the clipboard
				nlverify (SetClipboardData (format, hData));

				// Close the clipboard
				CloseClipboard ();

				// Ok
				return true;
			}
		}
		else
		{
			// Empty the clipboard
			nlverify (EmptyClipboard());
		}
 

		// Close the clipboard
		CloseClipboard ();
	}
	return false;
}

bool CMainFrame::dataFromClipboard (UINT format, void *data)
{
	// Open the clipboard
	if (OpenClipboard ())
	{
		// Get the clipboard data
		HANDLE hData = GetClipboardData (format);
		if (hData)
		{
			DWORD len = GlobalSize (hData);
 			
			// Get the string
			LPVOID dataPtr = GlobalLock (hData);
			nlverify (dataPtr);

			// Copy the string
			memcpy (data, dataPtr, len);

			// Close the clipboard
			CloseClipboard ();

			// Ok
			return true;
		}

		// Close the clipboard
		CloseClipboard ();
	}
	return false;
}

bool CMainFrame::clipboardSize (UINT format, uint &size)
{
	// Open the clipboard
	if (OpenClipboard ())
	{
		// Get the clipboard data
		HANDLE hData = GetClipboardData (format);
		if (hData)
		{
			size = GlobalSize (hData);

			// Close the clipboard
			CloseClipboard ();

			// Ok
			return true;
		}
		else
			size = 0;

		// Close the clipboard
		CloseClipboard ();
	}
	return false;
}

void CMainFrame::OnClose() 
{
	if (m_bDontClose) return;

	if (theApp.SaveAllModified())
	{
		// JC: added save bar state
		SaveBarState("Georges");
		// Save state
		theApp.saveState ();

		theApp.CloseAllDocuments (TRUE);

		// Release plugins
		theApp.releasePlugins ();

		PostQuitMessage(0);
	}
}

void CMainFrame::OnViewDockingdialogbar() 
{
	CMenu *menu = GetMenu();
	if(menu->GetMenuState(ID_VIEW_DOCKINGDIALOGBAR, MF_BYCOMMAND) & MF_CHECKED)
	{
		ShowControlBar (&FileBrowser, FALSE, FALSE);
	}
	else
	{
		ShowControlBar (&FileBrowser, TRUE, FALSE);
	}
}

void CMainFrame::OnUpdateViewDockingdialogbar(CCmdUI* pCmdUI) 
{
	if(FileBrowser.IsVisible())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIFrameWnd::OnSize(nType, cx, cy);
	
	RecalcLayout ();
}

void CMainFrame::OnViewOutputConsole() 
{
	CMenu *menu = GetMenu();
	if(menu->GetMenuState(ID_VIEW_OUTPUT_CONSOLE, MF_BYCOMMAND) & MF_CHECKED)
	{
		ShowControlBar (&OutputConsole, FALSE, FALSE);
	}
	else
	{
		ShowControlBar (&OutputConsole, TRUE, FALSE);
	}
}

void CMainFrame::OnUpdateViewOutputConsole(CCmdUI* pCmdUI) 
{
	if(OutputConsole.IsVisible())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CMainFrame::outputConsoleString (const char *message)
{
	OutputConsoleDlg.outputString (message);
}

void CMainFrame::OnViewGotoFileBrowser() 
{
	ShowControlBar (&FileBrowser, TRUE, FALSE);
	FileBrowserDlg.SetFocus ();
}

void CMainFrame::OnViewGotoOutputConsole() 
{
	ShowControlBar (&OutputConsole, TRUE, FALSE);
	OutputConsoleDlg.SetFocus ();
}

void CMainFrame::showOutputConsole (bool show)
{
	ShowControlBar (&OutputConsole, show?TRUE:FALSE, FALSE);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	// For each plugins
	uint i;
	for (i=0; i<theApp.PluginArray.size (); i++)
		if (theApp.PluginArray[i].Activated && theApp.PluginArray[i].PluginInterface->pretranslateMessage (pMsg))
			return TRUE;
	
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnModules0() 
{
	theApp.PluginArray[0].Activated ^= true;
	theApp.PluginArray[0].PluginInterface->activate (theApp.PluginArray[0].Activated);
	CChildFrame *pChild = (CChildFrame*)MDIGetActive ();
	if (pChild)
	{
		CGeorgesEditDoc *doc = (CGeorgesEditDoc *)pChild->GetActiveDocument();
		doc->onActivateView (true);
	}
}

void CMainFrame::OnUpdateModules0(CCmdUI* pCmdUI) 
{
	if (theApp.PluginArray.size () > 0)
	{
		pCmdUI->Enable ();
		string name;
		theApp.PluginArray[0].PluginInterface->getPluginName (name);
		pCmdUI->SetText (name.c_str ());
		pCmdUI->SetCheck (theApp.PluginArray[0].Activated);
	}
	else
	{
		pCmdUI->Enable (FALSE);
	}
}

void CMainFrame::OnModules1() 
{
	theApp.PluginArray[1].Activated ^= true;
	theApp.PluginArray[1].PluginInterface->activate (theApp.PluginArray[1].Activated);
	CChildFrame *pChild = (CChildFrame*)MDIGetActive ();
	if (pChild)
	{
		CGeorgesEditDoc *doc = (CGeorgesEditDoc *)pChild->GetActiveDocument();
		doc->onActivateView (true);
	}
}

void CMainFrame::OnUpdateModules1(CCmdUI* pCmdUI) 
{
	if (theApp.PluginArray.size () > 1)
	{
		pCmdUI->Enable ();
		string name;
		theApp.PluginArray[1].PluginInterface->getPluginName (name);
		pCmdUI->SetText (name.c_str ());
		pCmdUI->SetCheck (theApp.PluginArray[1].Activated);
	}
	else
	{
		pCmdUI->Enable (FALSE);
	}
}

void CMainFrame::OnModules2() 
{
	theApp.PluginArray[2].Activated ^= true;
	theApp.PluginArray[2].PluginInterface->activate (theApp.PluginArray[2].Activated);
	CChildFrame *pChild = (CChildFrame*)MDIGetActive ();
	if (pChild)
	{
		CGeorgesEditDoc *doc = (CGeorgesEditDoc *)pChild->GetActiveDocument();
		doc->onActivateView (true);
	}
}

void CMainFrame::OnUpdateModules2(CCmdUI* pCmdUI) 
{
	if (theApp.PluginArray.size () > 2)
	{
		pCmdUI->Enable ();
		string name;
		theApp.PluginArray[2].PluginInterface->getPluginName (name);
		pCmdUI->SetText (name.c_str ());
		pCmdUI->SetCheck (theApp.PluginArray[2].Activated);
	}
	else
	{
		pCmdUI->Enable (FALSE);
	}
}

void CMainFrame::OnModules3() 
{
	theApp.PluginArray[3].Activated ^= true;
	theApp.PluginArray[3].PluginInterface->activate (theApp.PluginArray[3].Activated);
	CChildFrame *pChild = (CChildFrame*)MDIGetActive ();
	if (pChild)
	{
		CGeorgesEditDoc *doc = (CGeorgesEditDoc *)pChild->GetActiveDocument();
		doc->onActivateView (true);
	}
}

void CMainFrame::OnUpdateModules3(CCmdUI* pCmdUI) 
{
	if (theApp.PluginArray.size () > 3)
	{
		pCmdUI->Enable ();
		string name;
		theApp.PluginArray[3].PluginInterface->getPluginName (name);
		pCmdUI->SetText (name.c_str ());
		pCmdUI->SetCheck (theApp.PluginArray[3].Activated);
	}
	else
	{
		pCmdUI->Enable (FALSE);
	}
}

void CMainFrame::OnUpdateViewSettings(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (theApp.Superuser);	
}

void CMainFrame::OnBrowserSortedByType() 
{
	BrowserSortedByType ^= true;
	FileBrowserDlg.setSortedByType (BrowserSortedByType);
}

void CMainFrame::OnUpdateBrowserSortedByType(CCmdUI* pCmdUI) 
{
	if(BrowserSortedByType)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}
