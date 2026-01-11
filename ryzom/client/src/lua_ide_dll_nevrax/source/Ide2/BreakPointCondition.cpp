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

// BreakPointCondition.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "BreakPointCondition.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBreakPointCondition dialog


CBreakPointCondition::CBreakPointCondition(CWnd* pParent /*=NULL*/)
	: CDialog(CBreakPointCondition::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBreakPointCondition)
	m_Condition = _T("");
	//}}AFX_DATA_INIT
}


void CBreakPointCondition::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBreakPointCondition)
	DDX_Control(pDX, IDC_CONDITION, m_ConditionCtrl);
	DDX_Text(pDX, IDC_CONDITION, m_Condition);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBreakPointCondition, CDialog)
	//{{AFX_MSG_MAP(CBreakPointCondition)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBreakPointCondition message handlers

BOOL CBreakPointCondition::OnInitDialog() 
{
	CDialog::OnInitDialog();		
	m_ConditionCtrl.SetFocus();
	m_ConditionCtrl.SetSel(0, -1);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
