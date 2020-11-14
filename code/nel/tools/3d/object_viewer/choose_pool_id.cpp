// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "choose_pool_id.h"

/////////////////////////////////////////////////////////////////////////////
// CChoosePoolID dialog


CChoosePoolID::CChoosePoolID(bool freezeID , CWnd* pParent /*=NULL*/)
	: CDialog(CChoosePoolID::IDD, pParent),  _FreezeID(freezeID)
{
	//{{AFX_DATA_INIT(CChoosePoolID)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChoosePoolID::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChoosePoolID)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChoosePoolID, CDialog)
	//{{AFX_MSG_MAP(CChoosePoolID)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChoosePoolID message handlers

void CChoosePoolID::OnOK() 
{
	CString val;
	GetDlgItem(IDC_POOL_ID)->GetWindowText(val);

	if (NLMISC::fromString(NLMISC::tStrToUtf8(val), PoolID))
	{
		GetDlgItem(IDC_POOL_NAME)->GetWindowText(val);
		Name = NLMISC::tStrToUtf8(val);
		CDialog::OnOK();
	}
	else
	{
		MessageBox(_T("Invalid value"), _T("error"), MB_OK);
	}
}

BOOL CChoosePoolID::OnInitDialog() 
{
	CDialog::OnInitDialog();

	std::string val = NLMISC::toString(PoolID);

	GetDlgItem(IDC_POOL_ID)->SetWindowText(nlUtf8ToTStr(val));
	GetDlgItem(IDC_POOL_NAME)->SetWindowText(nlUtf8ToTStr(Name));

	if (_FreezeID)
	{
		GetDlgItem(IDC_POOL_ID)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
