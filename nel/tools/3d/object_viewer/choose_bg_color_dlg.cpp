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

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_bg_color_dlg.h"
#include "color_edit.h"



/////////////////////////////////////////////////////////////////////////////
// CChooseBGColorDlg dialog


CChooseBGColorDlg::CChooseBGColorDlg(CObjectViewer *objectViewer, CWnd* pParent)
	: CDialog(CChooseBGColorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseBGColorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	nlassert(objectViewer);
	_BGColorWrapper.OV = objectViewer;
	_ColorEdit = new CColorEdit(this);
	_ColorEdit->setWrapper(&_BGColorWrapper);
}

CChooseBGColorDlg::~CChooseBGColorDlg()
{
	_ColorEdit->DestroyWindow();
	delete _ColorEdit;
}



void CChooseBGColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseBGColorDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseBGColorDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseBGColorDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseBGColorDlg message handlers

BOOL CChooseBGColorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	RECT r;
	GetDlgItem(IDC_BG_COLOR)->GetWindowRect(&r);
	ScreenToClient(&r);
	_ColorEdit->init(r.left, r.top, this);		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseBGColorDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_CHOOSE_BG_COLOR_DLG);		
	CDialog::OnDestroy();	
}
