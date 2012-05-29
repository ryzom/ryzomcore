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

// output_console_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "output_console_dlg.h"
#include "main_frm.h"

/////////////////////////////////////////////////////////////////////////////
// COutputConsoleDlg dialog


COutputConsoleDlg::COutputConsoleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COutputConsoleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COutputConsoleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COutputConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutputConsoleDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COutputConsoleDlg, CDialog)
	//{{AFX_MSG_MAP(COutputConsoleDlg)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutputConsoleDlg message handlers

void COutputConsoleDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (IsWindow (*this))
	{
		CRect sz;
		sz.left = 0;
		sz.top = 0;
		sz.right = cx;
		sz.bottom = cy;

		CEdit* edit = (CEdit*)GetDlgItem (IDC_OUTPUT_CONSOLE);
		if (edit)
		{
			sz.top += 5;
			sz.left += 5;
			sz.right -= 5;
			sz.bottom -= 5;

			edit->MoveWindow(sz);
		}
	}
}

BOOL COutputConsoleDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Create a font
	/*CFont font;
	font.CreateFont (8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FF_DONTCARE, "Ms Sans Sherif");*/

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COutputConsoleDlg::outputString (const char *message)
{
	CEdit* edit = (CEdit*)GetDlgItem (IDC_OUTPUT_CONSOLE);
	if (edit)
	{
		int index = edit->LineIndex(edit->GetLineCount( )-1) + edit->LineLength(edit->GetLineCount( )-1);
		edit->SetSel (index, index);
		edit->ReplaceSel (message);
	}
}

void COutputConsoleDlg::OnCancel ()
{
	// Select document
	CMDIChildWnd *child = ((CMainFrame*)theApp.m_pMainWnd)->MDIGetActive();
	if (child)
	{
		child->SetFocus ();
	}
}

