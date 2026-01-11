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

// BreakPointWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "BreakPointWnd.h"
#include "MainFrame.h"
#include "BreakPointCondition.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBreakPointWnd dialog


CBreakPointWnd::CBreakPointWnd(CWnd* pParent /*=NULL*/)
	: CDialog(CBreakPointWnd::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBreakPointWnd)
	//}}AFX_DATA_INIT
}


void CBreakPointWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBreakPointWnd)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBreakPointWnd, CDialog)
	//{{AFX_MSG_MAP(CBreakPointWnd)
	ON_LBN_SELCHANGE(IDC_BP_LIST, OnSelchangeBpList)
	ON_BN_CLICKED(IDC_CONDITION, OnCondition)
	ON_CLBN_CHKCHANGE(IDC_BP_LIST, OnCheckBP)
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_DELETEALL, OnDeleteAll)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBreakPointWnd message handlers

BOOL CBreakPointWnd::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateContent();	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBreakPointWnd::UpdateContent()
{
	int oldCurSel = getBPListCtrl().GetCurSel();
	getBPListCtrl().ResetContent();
	for(int k = 0; k < (int) m_BP.size(); ++k)
	{
		
		CString bpDesc;		
		bpDesc.Format("%s, line = %d", (LPCTSTR) m_BP[k].m_File->GetNameExt(), m_BP[k].m_BP.m_Line);
		if (!m_BP[k].m_BP.m_Condition.IsEmpty())
		{
			CString cond;
			cond.Format(" (Condition = %s)", m_BP[k].m_BP.m_Condition);
			bpDesc += cond;
		}				
		getBPListCtrl().InsertString(k, bpDesc);
		getBPListCtrl().SetCheck(k, m_BP[k].m_BP.m_Enabled ? 1 : 0);
	}
	getBPListCtrl().SetCurSel(oldCurSel != LB_ERR ? oldCurSel : 0);
}

void CBreakPointWnd::UpdateButtons()
{
	BOOL conditionsOn = (getBPListCtrl().GetCurSel() == LB_ERR);
	GetDlgItem(IDC_CONDITION)->EnableWindow(IDC_CONDITION);
}


void CBreakPointWnd::OnSelchangeBpList() 
{
	UpdateButtons();	
}

void CBreakPointWnd::OnCondition() 
{
	CBreakPointCondition bpc;
	CFileBreakPoint &fbp = m_BP[getBPListCtrl().GetCurSel()];
	bpc.m_Condition = fbp.m_BP.m_Condition;
	if (bpc.DoModal() == IDOK)
	{
		CString errors;
		if (!bpc.m_Condition.IsEmpty())
		{
			if (!theApp.GetMainFrame()->GetDebugger()->GetLuaHelper().CheckSyntax("return (" + bpc.m_Condition + ")", errors))
			{
				CString caption;
				caption.LoadString(AFX_IDS_APP_TITLE);
				MessageBox((LPCTSTR) errors, (LPCTSTR) caption, MB_ICONEXCLAMATION|MB_OK);
			}
		}
		fbp.m_BP.m_Condition = bpc.m_Condition;
	}
	UpdateContent();
}

void CBreakPointWnd::OnDeleteAll() 
{
	// TODO: Add your control notification handler code here
	m_BP.clear();
	UpdateContent();
}

void CBreakPointWnd::OnCheckBP()
{
	int currSel = getBPListCtrl().GetCurSel();
	assert(currSel != LB_ERR);
	assert(currSel < (int) m_BP.size());
	m_BP[currSel].m_BP.m_Enabled = getBPListCtrl().GetCheck(currSel);
}

void CBreakPointWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_DELETE)
	{
		EraseSel();		
	}	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);	
}

void CBreakPointWnd::EraseSel() 
{
	int currSel = getBPListCtrl().GetCurSel();
	if (currSel != LB_ERR)
	{
		assert(currSel < (int) m_BP.size());
		m_BP.erase(m_BP.begin() + currSel);
		getBPListCtrl().DeleteString(currSel);
	}
}

void CBreakPointWnd::OnRemove() 
{
	EraseSel();	
}


CCheckListBox &CBreakPointWnd::getBPListCtrl()
{
	CWnd *lb = GetDlgItem(IDC_BP_LIST);
	assert(lb);
	return *(CCheckListBox *) lb;
}
