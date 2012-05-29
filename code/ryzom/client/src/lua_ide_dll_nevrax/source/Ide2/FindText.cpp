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

// FindText.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "FindText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindText dialog


CFindText::CFindText(CWnd* pParent /*=NULL*/)
	: CDialog(CFindText::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindText)
	m_MatchCase = FALSE;
	m_WholeWord = FALSE;
	m_RegExp = FALSE;
	m_TextToFind = _T("");
	//}}AFX_DATA_INIT
}


void CFindText::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindText)
	DDX_Control(pDX, IDC_TEXT_TO_FIND, m_TextToFindCtrl);
	DDX_Check(pDX, IDC_MATCH_CASE, m_MatchCase);
	DDX_Check(pDX, IDC_MATCH_WHOLE_WORD, m_WholeWord);
	DDX_Check(pDX, IDC_REGULAR_EXPRESSION, m_RegExp);
	DDX_Text(pDX, IDC_TEXT_TO_FIND, m_TextToFind);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindText, CDialog)
	//{{AFX_MSG_MAP(CFindText)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindText message handlers

void CFindText::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);	
	if (bShow)
	{
		m_TextToFindCtrl.SetSel(0, -1);
		m_TextToFindCtrl.SetFocus();	
	}
}
