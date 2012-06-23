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

// client_data_checkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "client_data_check.h"
#include "client_data_checkDlg.h"
#include "client_data_checkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClient_data_checkDlg dialog

CClient_data_checkDlg::CClient_data_checkDlg(class CClient_data_checkApp *appParent, CWnd* pParent /*=NULL*/)
	: _AppParent(appParent), CDialog(CClient_data_checkDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClient_data_checkDlg)
	TextStatus = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClient_data_checkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClient_data_checkDlg)
	DDX_Control(pDX, IDC_BUTTON_CANCEL, CancelButton);
	DDX_Control(pDX, IDC_LIST_LOG, ListBox);
	DDX_Text(pDX, IDC_STATIC_STATUS, TextStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClient_data_checkDlg, CDialog)
	//{{AFX_MSG_MAP(CClient_data_checkDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClient_data_checkDlg message handlers

BOOL CClient_data_checkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClient_data_checkDlg::OnPaint() 
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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClient_data_checkDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CClient_data_checkDlg::OnButtonCancel() 
{
	if(_AppParent)
		_AppParent->cancel();
}
