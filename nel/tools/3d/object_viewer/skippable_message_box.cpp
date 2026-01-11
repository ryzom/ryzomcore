// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// skippable_message_box.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "skippable_message_box.h"


/////////////////////////////////////////////////////////////////////////////
// CSkippableMessageBox dialog


CSkippableMessageBox::CSkippableMessageBox(const CString &caption, const CString &content, CWnd* pParent /*=NULL*/)
	: CDialog(CSkippableMessageBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSkippableMessageBox)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	_BypassFlag = false;
	_Caption = caption;
	_Content = content;
}


void CSkippableMessageBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkippableMessageBox)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSkippableMessageBox, CDialog)
	//{{AFX_MSG_MAP(CSkippableMessageBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkippableMessageBox message handlers

BOOL CSkippableMessageBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetWindowText((LPCTSTR) _Caption);
	GetDlgItem(IDC_MB_CONTENT)->SetWindowText((LPCTSTR) _Content);	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//*************************************************************************************
void CSkippableMessageBox::OnOK()
{
	_BypassFlag = ((CButton *) GetDlgItem(IDC_DONT_SHOW_AGAIN))->GetCheck() != 0;
	CDialog::OnOK();
}

