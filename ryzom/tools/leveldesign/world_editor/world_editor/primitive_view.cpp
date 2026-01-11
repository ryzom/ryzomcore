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

// primitive_view.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "primitive_view.h"

// ***************************************************************************
// CPrimitiveView

IMPLEMENT_DYNCREATE(CPrimitiveView, CTreeView)

// ***************************************************************************

CPrimitiveView::CPrimitiveView()
{
}

// ***************************************************************************

CPrimitiveView::~CPrimitiveView()
{
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CPrimitiveView, CTreeView)
	//{{AFX_MSG_MAP(CPrimitiveView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CPrimitiveView drawing

void CPrimitiveView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

// ***************************************************************************
// CPrimitiveView diagnostics

#ifdef _DEBUG
void CPrimitiveView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CPrimitiveView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

// ***************************************************************************
// CPrimitiveView message handlers

void CPrimitiveView::OnSize(UINT nType, int cx, int cy) 
{
	CTreeView::OnSize(nType, cx, cy);
	
	// Resize list ctrl to fill the whole view.
	/*CRect iniRect;
	GetClientRect(&iniRect);*/

	CRect iniRect;
	iniRect.top = 0;
	iniRect.left = 0;
	iniRect.bottom = cy;
	iniRect.right = cx;
	MoveWindow (&iniRect, TRUE);	

	// We commented this line because it generates infinite loops on some computers
	// http://dev.ryzom.com/issues/show/310
	//GetTreeCtrl().MoveWindow (&iniRect);

	/* iniRect.top = 45; iniRect.left = 10;
	iniRect.right -= 10; iniRect.bottom -= 10; */	
}

// ***************************************************************************

int CPrimitiveView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect iniRect;
	GetClientRect (iniRect);

	ShowWindow (SW_SHOW);
	// GetTreeCtrl().Create (TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP, iniRect, this, 0);
	GetTreeCtrl().ShowWindow (SW_SHOW);
	
	insertItemUTF8 (GetTreeCtrl(), "Root");

	return 0;
}

// ***************************************************************************

