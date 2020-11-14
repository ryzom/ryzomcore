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

// client_configDlg.cpp : implementation file
//

#include "stdafx.h"
#include "client_config.h"
#include "client_configDlg.h"
#include "cfg_file.h"
#include "database.h"

#include <nel/misc/debug.h>
#include <nel/misc/system_utils.h>

#define ICON_ZONE_WIDTH 128

#define LEFT_WIDTH 256
#define LEFT_HEIGHT 512

#define BAR_START_X (LEFT_WIDTH+ICON_ZONE_WIDTH)
#define BAR_WIDTH 448
#define BAR_HEIGHT 72
#define TREE_ZONE_WIDTH 150
#define TREE_OFFSET_LEFT 10
#define TREE_OFFSET_TOP 10

// Label
#define LARGE_LABEL_HEIGHT -22
#define LARGE_LABEL_START_X (LEFT_WIDTH+TREE_ZONE_WIDTH)
#define LARGE_LABEL_START_Y (15)
#define LARGE_LABEL_END_X (LEFT_WIDTH+ICON_ZONE_WIDTH+BAR_WIDTH)
#define LARGE_LABEL_END_Y BAR_HEIGHT

// Icon
#define ICON_WIDTH 64
#define ICON_HEIGHT 64
#define ICON_START_Y ((BAR_HEIGHT-ICON_HEIGHT)/2)
#define ICON_START_X (LEFT_WIDTH+ICON_START_Y)
#define ICON_START_Y ((BAR_HEIGHT-ICON_HEIGHT)/2)

// ***************************************************************************
// CAboutDlg dialog used for App About
// ***************************************************************************

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ***************************************************************************

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CClient_configDlg dialog
// ***************************************************************************

CClient_configDlg::CClient_configDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClient_configDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClient_configDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Init dialog pointers
	Dialogs[PageGeneral] = &GeneralDlg;
	Dialogs[PageDisplay] = &DisplayDlg;
	Dialogs[PageDisplayDetails] = &DisplayDetailsDlg;
	Dialogs[PageDisplayAdvanced] = &DisplayAdvancedDlg;
	Dialogs[PageDisplaySysInfo] = &SystemInformationDlg;
	Dialogs[PageDisplayOpenGLInfo] = &DisplayInformationGLDlg;
	Dialogs[PageDisplayOpenD3DInfo] = &DisplayInformationD3DDlg;
	Dialogs[PageSound] = &SoundDlg;
	_CurrentPage = 0;
}

// ***************************************************************************

void CClient_configDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClient_configDlg)
	DDX_Control(pDX, ID_APPLY, ApplyCtrl);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CClient_configDlg, CDialog)
	//{{AFX_MSG_MAP(CClient_configDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_APPLY, OnApply)
	ON_BN_CLICKED(ID_DEFAULT, OnDefault)
	ON_BN_CLICKED(ID_LAUNCH, OnLaunch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CClient_configDlg message handlers
// ***************************************************************************

BOOL CClient_configDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu = "uiConfigMenuAbout";
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Create bitmaps
	uint i;
	for (i=0; i<BitmapCount; i++)
	{
		Bitmaps[i].LoadBitmap (BitmapId[i].ResId);
	}
	
	// Create others controls
	nlverify (Left.Create (NULL, WS_CHILD|WS_VISIBLE|SS_BITMAP, CRect (0, 0, LEFT_WIDTH, LEFT_HEIGHT), this));
	Left.SetBitmap (LoadBitmap (theApp.m_hInstance, MAKEINTRESOURCE (IDB_LEFT_0)));
	nlverify (Icon.Create (NULL, WS_CHILD|WS_VISIBLE|SS_BITMAP, CRect (ICON_START_X, ICON_START_Y, ICON_START_X+ICON_WIDTH, ICON_START_Y+ICON_HEIGHT), this));
	Icon.SetBitmap (Bitmaps[BitmapWelcome]);

	// Create bars
	nlverify (Top.Create (CRect (LEFT_WIDTH, BAR_HEIGHT, LEFT_WIDTH+ICON_ZONE_WIDTH+BAR_WIDTH, BAR_HEIGHT+1), this));
	nlverify (Bottom.Create (CRect (0, LEFT_HEIGHT, LEFT_WIDTH+ICON_ZONE_WIDTH+BAR_WIDTH, LEFT_HEIGHT+1), this));

	// Create the tree
	nlverify (Tree.Create ( WS_CHILD|WS_VISIBLE|TVS_NOSCROLL|TVS_TRACKSELECT|TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS, 
		CRect (LEFT_WIDTH+TREE_OFFSET_LEFT, BAR_HEIGHT+2+TREE_OFFSET_TOP, 
		LEFT_WIDTH+TREE_ZONE_WIDTH, BAR_HEIGHT+LEFT_HEIGHT-BAR_HEIGHT), this, TreeId));
	Tree.SetFont (GetFont ());

	// Create fonts
	nlverify (BarFont.CreateFont (LARGE_LABEL_HEIGHT, 0, 0, 0, FW_BOLD, TRUE, FALSE, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 
		TMPF_TRUETYPE | DEFAULT_PITCH | FF_MODERN, "Arial"));

	// Create dialog
	for (i=0; i<PageCount; i++)
	{
		nlverify (Dialogs[i]->Create (Pages[i].ResId, this));
		nlverify (Dialogs[i]->SetWindowPos (NULL, LEFT_WIDTH+TREE_ZONE_WIDTH, BAR_HEIGHT+2, 0, 0, SWP_NOZORDER|SWP_NOSIZE));
	}

	// Build the tree
	CPage *page = Root.Children[0];
	HTREEITEM parent = TVI_ROOT;
	while (page)
	{
		// Add an item
		parent = Tree.InsertItem ("", parent);
		Tree.SetItemData (parent, page->PageId);
		if (page->Bold)
			Tree.SetItemState (parent, TVIS_BOLD, TVIS_BOLD);

		// Next
		if (page->Children.size ())
			page = page->Children[0];
		else
		{
			// Got a parent ?
			while (page->Parent)
			{
				// Brother ?
				if (page->ChildId < page->Parent->Children.size ()-1)
				{
					page = page->Parent->Children[page->ChildId+1];
					parent = Tree.GetParentItem (parent);
					break;
				}
				else
				{
					page = page->Parent;
					parent = Tree.GetParentItem (parent);
					Tree.Expand (parent, TVE_EXPAND);
				}
			}
			if (!page->Parent)
				break;
		}
	}

	ApplyCtrl.EnableWindow (FALSE);

	// Get from the config file
	GetFromConfigFile ();

	theApp.Modified = false;

	// Build the localisation map 
	backupWindowHandleRec ();

	// Localize the windows
	changeLanguage (::GetIntForceLanguage()?"en" : ::GetString ("LanguageCode").c_str());
	theApp.Localized = true;

	// Focus
	Tree.SelectItem(Tree.GetChildItem (Tree.GetRootItem()));
	Tree.SetFocus ();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ***************************************************************************

void CClient_configDlg::changeLanguage (const char *language)
{
	// Init CI18N
	NLMISC::CI18N::load(language);

	localizeWindowsRec ();
	translateTree ();

	// Set the general page
	setPage (_CurrentPage);

	DisplayDlg.updateState ();
	DisplayDlg.UpdateData (FALSE);
	DisplayDetailsDlg.updateState ();
	SoundDlg.updateState ();

	// Set the title
	setWindowText(*this, (WCHAR*)((NLMISC::CI18N::get ("uiConfigTitle") + (theApp.Modified?" *":"")).c_str()));

	// The menu
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		if (NLMISC::CSystemUtils::supportUnicode())
		{		
			nlverify (::ModifyMenuW(*pSysMenu, IDM_ABOUTBOX, MF_BYCOMMAND|MF_STRING, IDM_ABOUTBOX, (WCHAR*)NLMISC::CI18N::get ("uiConfigMenuAbout").c_str()));
		}
		else
		{
			nlverify (::ModifyMenu(*pSysMenu, IDM_ABOUTBOX, MF_BYCOMMAND|MF_STRING, IDM_ABOUTBOX, (LPCTSTR)NLMISC::CI18N::get ("uiConfigMenuAbout").toString().c_str()));
		}
	}
}

// ***************************************************************************

void CClient_configDlg::translateTree ()
{
	// Set the item text
	HTREEITEM item = Tree.GetRootItem ();
	while (item)
	{
		// Set the item text
		uint page = (uint)Tree.GetItemData (item);
		ucstring name = NLMISC::CI18N::get (Pages[page].Name);		
		if (NLMISC::CSystemUtils::supportUnicode())		
		{		
			TVITEMEXW itemDesc;
			memset (&itemDesc, 0, sizeof(TVITEMEXW));
			itemDesc.hItem = item;
			itemDesc.mask = TVIF_TEXT;
			itemDesc.pszText = (WCHAR*)name.c_str();
			nlverify (SendMessageW (Tree, TVM_SETITEMW, 0, (LPARAM)&itemDesc));
		}
		else
		{
			TVITEMEXA itemDesc;
			memset (&itemDesc, 0, sizeof(TVITEMEXA));
			itemDesc.hItem = item;
			itemDesc.mask = TVIF_TEXT;
			std::string tmpStr = name.toString();
			itemDesc.pszText = (LPSTR) tmpStr.c_str();			
			nlverify (::SendMessageA(Tree, TVM_SETITEMA, 0 , (LPARAM)&itemDesc));
		}
		// Next item
		HTREEITEM old = item;
		item = Tree.GetChildItem (item);
		if (!item)
		{
			// No more child, try a brother
			item = Tree.GetNextSiblingItem (old);
		}
		if (!item)
		{
			// No more brother, try an oncle
			item = Tree.GetParentItem (old);
			if (item)
				item = Tree.GetNextSiblingItem (item);
		}
	}
}

// ***************************************************************************

void CClient_configDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ***************************************************************************

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClient_configDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// Draw rect
		CPaintDC dc(this); // device context for painting
		dc.FillSolidRect( LEFT_WIDTH, BAR_HEIGHT+2, TREE_ZONE_WIDTH, LEFT_HEIGHT-BAR_HEIGHT-2, GetSysColor (COLOR_WINDOW));

		// Draw top bar
		dc.FillSolidRect( LEFT_WIDTH, 0, ICON_ZONE_WIDTH, BAR_HEIGHT, RGB (255,255,255));
		CDC bitmapDC;
		bitmapDC.CreateCompatibleDC (&dc);
		bitmapDC.SelectObject (&Bitmaps[BitmapTopRight]); 
		nlverify (dc.BitBlt( LEFT_WIDTH + ICON_ZONE_WIDTH, 0, BAR_WIDTH, BAR_HEIGHT, &bitmapDC, 0, 0, SRCCOPY));

		// Draw some text
		dc.SetBkMode (TRANSPARENT);
		dc.SelectObject (&BarFont);
		CRect labelRect (LARGE_LABEL_START_X, LARGE_LABEL_START_Y, LARGE_LABEL_END_X, LARGE_LABEL_END_Y);
		DrawTextW (&(*dc), (WCHAR*)TopLargeLabel.c_str (), (sint)TopLargeLabel.size (), &labelRect, DT_LEFT|DT_TOP);

		CDialog::OnPaint();
	}
}

// ***************************************************************************

HCURSOR CClient_configDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// ***************************************************************************

void CClient_configDlg::setPage (uint pageId)
{
	CPage &page = Pages[pageId];
	Icon.SetBitmap (Bitmaps[page.Icon]);
	TopLargeLabel = NLMISC::CI18N::get (std::string(page.Name));

	// Hide all the dialog
	uint i;
	for (i=0; i<PageCount; i++)
		Dialogs[i]->ShowWindow (SW_HIDE);

	// Show the dialog
	Dialogs[pageId]->ShowWindow (SW_SHOW);

	// Invalidate bar
	InvalidateBar ();
	_CurrentPage = pageId;
}

// ***************************************************************************

BOOL CClient_configDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
    int idCtrl = (int) wParam; 
    LPNMHDR pnmh = (LPNMHDR) lParam; 

	switch (idCtrl)
	{
	case TreeId:
		{
			switch (pnmh->code)
			{
			case TVN_SELCHANGED:
				{
					// Get the selection
					HTREEITEM item = Tree.GetSelectedItem ();
					if (item)
					{
						// Get the page
						uint pageId = (uint)Tree.GetItemData (item);
						setPage (pageId);
					}
				}
				break;
			}
		}
		break;
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}

// ***************************************************************************

void CClient_configDlg::InvalidateBar ()
{
	CRect rect (BAR_START_X, 0, BAR_START_X+BAR_WIDTH, BAR_HEIGHT);
	InvalidateRect (&rect);
}

// ***************************************************************************

BOOL CClient_configDlg::UpdateData ( BOOL bSaveAndValidate )
{
	// For each pages
	uint i;
	for (i=0; i<PageCount; i++)
	{
		Dialogs[i]->UpdateData (bSaveAndValidate);
	}

	return CDialog::UpdateData ( bSaveAndValidate );
}

// ***************************************************************************

void CClient_configDlg::OnApply() 
{
	// Update config file
	SetToConfigFile ();

	// Save the config file
	SaveConfigFile ();

	// Doc is validated
	theApp.Modified = false;
	ApplyCtrl.EnableWindow (FALSE);

	// Set the title
	setWindowText(*this, (WCHAR*)NLMISC::CI18N::get ("uiConfigTitle").c_str());
}

// ***************************************************************************

void CClient_configDlg::OnCancel() 
{
	// Modified ?
	if (theApp.Modified)
	{
		// Quit without saving ?
		if (theApp.yesNo (NLMISC::CI18N::get ("uiConfigQuitWithoutSaving")))
		{
			CDialog::OnCancel();
		}
	}
	else
		CDialog::OnCancel();
}

// ***************************************************************************

void CClient_configDlg::OnOK() 
{
	if (theApp.Modified)
	{
		// Update config file
		SetToConfigFile ();

		// Save the config file
		SaveConfigFile ();

		// Doc is validated
		theApp.Modified = false;
	}
	
	CDialog::OnOK();
}

// ***************************************************************************

void CClient_configDlg::OnDefault() 
{
	/* Yoyo: Don't use the preset dlg for now cause doesn't work everywhere
	// Open the preset dialog
	CPresetDlg preset;
	if (preset.DoModal () == IDOK)
	{
		// Reset the CFG to default
		ResetConfigFile();
	
		// invalidate
		InvalidateConfig ();
	  
		// Update widgets
		GetFromConfigFile ();
	}
	*/
	// Quit without saving ?
	if (theApp.yesNo (NLMISC::CI18N::get ("uiConfigRestaureDefault")))
	{
		// Reset the CFG to default
		ResetConfigFileToDefault();
		
		// invalidate
		InvalidateConfig ();

		// Update widgets
		GetFromConfigFile ();
	}
}

// ***************************************************************************

void CClient_configDlg::OnLaunch() 
{
	if (theApp.Modified)
	{
		// Update config file
		SetToConfigFile ();
		
		// Save the config file
		SaveConfigFile ();
		
		// Doc is validated
		theApp.Modified = false;
	}

	// launch ryzom
	NLMISC::launchProgram("client_ryzom_rd.exe","");

	CDialog::OnOK();
}

// ***************************************************************************

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	localizeWindowsRec (this, false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
