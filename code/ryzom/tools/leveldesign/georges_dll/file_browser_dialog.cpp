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

// file_browser_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "file_browser_dialog.h"
#include "file_browser_dialog.h"
#include "main_frm.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserDialog dialog


CFileBrowserDialog::CFileBrowserDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFileBrowserDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileBrowserDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CFileBrowserDialog::~CFileBrowserDialog()
{
	int toto=0;
}

void CFileBrowserDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileBrowserDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileBrowserDialog, CDialog)
	//{{AFX_MSG_MAP(CFileBrowserDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_TAB_FILE, OnClickTabFile)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_FILE, OnSelchangeTabFile)
	ON_WM_MOVE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserDialog message handlers

BOOL CFileBrowserDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect sz;
	GetClientRect(sz);
	// sz.DeflateRect(5,5);

	// The tab
	CTabCtrl *TabFile = (CTabCtrl*)GetDlgItem (IDC_TAB_FILE);
	nlassert (TabFile);
	int tabCount = 0;
	if (theApp.Superuser)
	{
		TabFile->InsertItem (0, "Type");
		TabFile->InsertItem (1, "Dfn");
		tabCount += 2;

		TreeCtrlType.create( sz, TabFile, 0);
		TreeCtrlType.ShowWindow (SW_HIDE);
		TreeCtrlType.addExclusiveExtFilter (".typ");
		TreeCtrlType.setRootDirectory ((theApp.RootSearchPath + theApp.TypeDfnSubDirectory).c_str ());
		TreeCtrlType.setNotifyWindow (m_hWnd, 0);

		TreeCtrlDfn.create( sz, TabFile, 1);
		TreeCtrlDfn.ShowWindow (SW_HIDE);
		TreeCtrlDfn.addExclusiveExtFilter (".dfn");
		TreeCtrlDfn.setRootDirectory ((theApp.RootSearchPath + theApp.TypeDfnSubDirectory).c_str ());
		TreeCtrlDfn.setNotifyWindow (m_hWnd, 1);
	}

	TabFile->InsertItem (tabCount, "Form");
	TabFile->SetCurSel (tabCount);

	TreeCtrlForm.create( sz, TabFile, 2);
	TreeCtrlForm.ShowWindow (SW_SHOW);
	if (theApp.Superuser)
	{
		TreeCtrlForm.addNegativeExtFilter (".typ");
		TreeCtrlForm.addNegativeExtFilter (".dfn");
		TreeCtrlForm.addNegativeExtFilter (".ico");
	}
	else
	{
		uint i;
		for (i=0; i<theApp.UserTypes.size (); i++)
		{
			TreeCtrlForm.addExclusiveExtFilter (("." + theApp.UserTypes[i]).c_str ());
		}
	}
	TreeCtrlForm.setRootDirectory (theApp.RootSearchPath.c_str ());
	TreeCtrlForm.setNotifyWindow (m_hWnd, 2);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFileBrowserDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (IsWindow (*this))
	{
		CRect sz;
		sz.left = 0;
		sz.top = 0;
		sz.right = cx;
		sz.bottom = cy;

		CTabCtrl *TabFile = (CTabCtrl*)GetDlgItem (IDC_TAB_FILE);
		if (TabFile)
		{
			sz.top += 5;
			sz.left += 5;
			sz.right -= 5;
			sz.bottom -= 5;

			TabFile->MoveWindow(sz);
		}
		if (IsWindow (TreeCtrlType))
		{
			TabFile->GetClientRect(sz);

			sz.top += 2;
			sz.left += 2;
			sz.right -= 2;
			sz.bottom -= 22;
			TreeCtrlType.MoveWindow(sz);
		}
		if (IsWindow (TreeCtrlDfn))
		{
			TabFile->GetClientRect(sz);

			sz.top += 2;
			sz.left += 2;
			sz.right -= 2;
			sz.bottom -= 22;
			TreeCtrlDfn.MoveWindow(sz);
		}
		if (IsWindow (TreeCtrlForm))
		{
			TabFile->GetClientRect(sz);

			sz.top += 2;
			sz.left += 2;
			sz.right -= 2;
			sz.bottom -= 22;
			TreeCtrlForm.MoveWindow(sz);
		}
	}
}

void CFileBrowserDialog::OnClickTabFile(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CTabCtrl *TabFile = (CTabCtrl*)GetDlgItem (IDC_TAB_FILE);

	// Hide every body
	if (theApp.Superuser)
	{
		TreeCtrlType.ShowWindow (SW_HIDE);
		TreeCtrlDfn.ShowWindow (SW_HIDE);
		TreeCtrlForm.ShowWindow (SW_HIDE);

		// Get the current selection
		uint sel = (uint)TabFile->GetCurSel ();
		if (sel == 0)
			TreeCtrlType.ShowWindow (SW_SHOW);
		else if (sel == 1)
			TreeCtrlDfn.ShowWindow (SW_SHOW);
		else if (sel == 2)
			TreeCtrlForm.ShowWindow (SW_SHOW);

		*pResult = 0;
	}
}

LRESULT CFileBrowserDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_FILE_TREE_VIEW_LDBLCLICK)
	{
		// No folder
		if (wParam == 0)
		{
			openDocument ();
		}
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CFileBrowserDialog::OnSelchangeTabFile(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnClickTabFile(pNMHDR, pResult);
}

BOOL CFileBrowserDialog::PreTranslateMessage(MSG* pMsg) 
{
	if (theApp.m_pMainWnd->PreTranslateMessage(pMsg))
		return TRUE;

	if ((pMsg->message == WM_KEYDOWN) && ((int) pMsg->wParam == VK_TAB) )
	{
		// Tab focus ?
		CTabCtrl *TabFile = (CTabCtrl*)GetDlgItem (IDC_TAB_FILE);
		if (TabFile->GetFocus () == TabFile)
		{
			if (TreeCtrlType.IsWindowVisible ())
				TreeCtrlType.SetFocus ();
			else if (TreeCtrlDfn.IsWindowVisible ())
				TreeCtrlDfn.SetFocus ();
			else if (TreeCtrlForm.IsWindowVisible ())
				TreeCtrlForm.SetFocus ();
		}
		else
		{
			TabFile->SetFocus ();
		}
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CFileBrowserDialog::OnOK ()
{
	if (TreeCtrlType.IsWindowVisible () && TreeCtrlType.haveFocus () )
		openDocument ();
	else if (TreeCtrlDfn.IsWindowVisible () && TreeCtrlDfn.haveFocus () )
		openDocument ();
	else if (TreeCtrlForm.IsWindowVisible () && TreeCtrlForm.haveFocus () )
		openDocument ();
}

void CFileBrowserDialog::OnCancel ()
{
	// Select document
	CMDIChildWnd *child = ((CMainFrame*)theApp.m_pMainWnd)->MDIGetActive();
	if (child)
	{
		child->SetFocus ();
	}
}

void CFileBrowserDialog::openDocument ()
{
	// Get the file name
	string filename;
	if (IsWindow (TreeCtrlType) && (TreeCtrlType.IsWindowVisible ()))
	{
		if (TreeCtrlType.getCurrentFilename (filename) && !filename.empty ())
		{
			string pathName = CPath::lookup (filename.c_str (), false, false);
			if (pathName.empty ())
				pathName = filename;
			theApp.OpenDocumentFile (pathName.c_str());
		}
	}
	else if (IsWindow (TreeCtrlType) && TreeCtrlDfn.IsWindowVisible ())
	{
		if (TreeCtrlDfn.getCurrentFilename (filename) && !filename.empty ())
		{
			string pathName = CPath::lookup (filename.c_str (), false, false);
			if (pathName.empty ())
				pathName = filename;
			theApp.OpenDocumentFile (pathName.c_str());
		}
	}
	else if (TreeCtrlForm.IsWindowVisible ())
	{
		if (TreeCtrlForm.getCurrentFilename (filename) && !filename.empty ())
		{
			string pathName = CPath::lookup (filename.c_str (), false, false);
			if (pathName.empty ())
				pathName = filename;
			theApp.OpenDocumentFile (pathName.c_str());
		}
	}
}

void CFileBrowserDialog::OnSetFocus(CWnd* pNewWnd) 
{
	CDialog::OnSetFocus(pNewWnd);
	
	// Set the focus to the list
	if (TreeCtrlType.IsWindowVisible ())
		TreeCtrlType.SetFocus ();
	else if (TreeCtrlDfn.IsWindowVisible ())
		TreeCtrlDfn.SetFocus ();
	else if (TreeCtrlForm.IsWindowVisible ())
		TreeCtrlForm.SetFocus ();
}

void CFileBrowserDialog::refresh ()
{
	if (IsWindow (TreeCtrlType))
		TreeCtrlType.setRootDirectory ((theApp.RootSearchPath + theApp.TypeDfnSubDirectory).c_str ());
	if (IsWindow (TreeCtrlDfn))
		TreeCtrlDfn.setRootDirectory ((theApp.RootSearchPath + theApp.TypeDfnSubDirectory).c_str ());
	if (IsWindow (TreeCtrlForm))
		TreeCtrlForm.setRootDirectory (theApp.RootSearchPath.c_str ());
}

void CFileBrowserDialog::setSortedByType (bool sortedByType)
{
	CFileTreeCtrl::TArrange type = sortedByType ? CFileTreeCtrl::ByType : CFileTreeCtrl::ByName;
	TreeCtrlType.setArrangeMode (type);
	TreeCtrlDfn.setArrangeMode (type);
	TreeCtrlForm.setArrangeMode (type);
	refresh ();
}