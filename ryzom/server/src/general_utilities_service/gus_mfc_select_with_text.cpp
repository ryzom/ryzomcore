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

// gus_mfc_select_with_text.cpp : implementation file
//

#include "stdafx.h"
#include "gus_mfc_select_with_text.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGusMfcSelectWithText dialog


CGusMfcSelectWithText::CGusMfcSelectWithText(CWnd* pParent /*=NULL*/)
	: CDialog(CGusMfcSelectWithText::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGusMfcSelectWithText)
	Text = _T("");
	ComboValue = _T("");
	//}}AFX_DATA_INIT
}


void CGusMfcSelectWithText::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGusMfcSelectWithText)
	DDX_Control(pDX, IDC_COMBO1, Combo);
	DDX_Text(pDX, IDC_EDIT1, Text);
	DDX_CBString(pDX, IDC_COMBO1, ComboValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGusMfcSelectWithText, CDialog)
	//{{AFX_MSG_MAP(CGusMfcSelectWithText)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGusMfcSelectWithText message handlers

void CGusMfcSelectWithText::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

BOOL CGusMfcSelectWithText::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	Combo.InsertString (-1, "aa");
	Combo.InsertString (-1, "bb");
	Combo.InsertString (-1, "cc");
	Combo.InsertString (-1, "dd");
	Combo.SetCurSel (1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
