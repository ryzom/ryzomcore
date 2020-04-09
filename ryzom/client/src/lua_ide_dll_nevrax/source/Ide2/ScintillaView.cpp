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

// ScintillaView.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "ScintillaView.h"

#include "OutputWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScintillaView

IMPLEMENT_DYNCREATE(CScintillaView, CView)

CScintillaView::CScintillaView()
{
}

CScintillaView::~CScintillaView()
{
}


BEGIN_MESSAGE_MAP(CScintillaView, CView)
	//{{AFX_MSG_MAP(CScintillaView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScintillaView drawing

void CScintillaView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CScintillaView diagnostics

#ifdef _DEBUG
void CScintillaView::AssertValid() const
{
	CView::AssertValid();
}

void CScintillaView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CScintillaView message handlers

BOOL CScintillaView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CView::PreCreateWindow(cs);
}

int CScintillaView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	static int nCtrlID = ID_OUTPUT_CTRL1;

	// TODO: Add your specialized creation code here
	m_view.Create(this, nCtrlID++);
	m_view.SetReadOnly(TRUE);

	return 0;
}

void CScintillaView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if ( ::IsWindow(m_view.m_hWnd) )
		m_view.SetWindowPos(NULL, 0, 0, cx, cy, 0);	
}

void CScintillaView::Clear()
{
	m_view.SetReadOnly(FALSE);
	m_view.ClearAll();
	m_view.SetReadOnly(TRUE);
}

void CScintillaView::Write(CString strMsg)
{
	m_view.SetReadOnly(FALSE);
	m_view.AddText(strMsg);
	m_view.SetReadOnly(TRUE);
	m_view.GotoLastLine();
}


BOOL CScintillaView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR *lpnmhdr = (LPNMHDR) lParam; 

	if (lpnmhdr->hwndFrom == m_view.m_hWnd)
	{
		*pResult = ((CScintillaBar*)GetParent())->OnSci(this, (SCNotification*)lParam);
		return TRUE;
	}
	
	return CView::OnNotify(wParam, lParam, pResult);
}
