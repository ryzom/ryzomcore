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

// error_box_dlg.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "error_box_dlg.h"
#include "nel/misc/win32_util.h"
#include "../client/browse_faq.h"


/////////////////////////////////////////////////////////////////////////////
// CErrorBoxDlg dialog


CErrorBoxDlg::CErrorBoxDlg(const ucstring &errorMsg, const ucstring &resumeMessage, CWnd* pParent /*=NULL*/)
	: CDialog(CErrorBoxDlg::IDD, pParent)
{
	_ErrorMsg = errorMsg;
	_ResumeMsg = resumeMessage;
	//{{AFX_DATA_INIT(CErrorBoxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CErrorBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CErrorBoxDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CErrorBoxDlg, CDialog)
	//{{AFX_MSG_MAP(CErrorBoxDlg)
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CErrorBoxDlg message handlers

BOOL CErrorBoxDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetIcon(AfxGetApp()->LoadIcon(IDI_MAIN_ICON), TRUE);
	NLMISC::CWin32Util::localizeWindow(this->m_hWnd);	
	SetWindowTextW(GetDlgItem(IDC_ERROR_MSG)->m_hWnd, (const WCHAR *) _ErrorMsg.c_str());
	SetWindowTextW(GetDlgItem(IDC_RESUME_MSG)->m_hWnd, (const WCHAR *) _ResumeMsg.c_str());
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CErrorBoxDlg::OnHelp() 
{
	browseFAQ(theApp.ConfigFile);
}
