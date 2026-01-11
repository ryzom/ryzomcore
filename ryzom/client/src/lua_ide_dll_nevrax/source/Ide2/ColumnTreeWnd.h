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

#if !defined(AFX_COLUMNTREEWND_H__0AEDF909_4AF6_4DE8_AF8C_4EB9E0843020__INCLUDED_)
#define AFX_COLUMNTREEWND_H__0AEDF909_4AF6_4DE8_AF8C_4EB9E0843020__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColumnTreeWnd.h : header file
//

#include "ColumnTreeCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CColumnTreeCtrl window

class CColumnTreeWnd : public CWnd
{
// Construction
public:	

// Attributes
public:

// Operations
public:
	BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColumnTreeWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	enum ChildrenIDs { HeaderID = 1, TreeID = 2 };

	void UpdateColumns();

	CTreeCtrl& GetTreeCtrl() { return m_Tree; }
	CHeaderCtrl& GetHeaderCtrl() { return m_Header; }

	

	// Generated message map functions
protected:
	//{{AFX_MSG(CColumnTreeCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void OnDraw(CDC* pDC) {}
	void UpdateScroller();
	void RepositionControls();

protected:
	CColumnTreeCtrl m_Tree;
	CHeaderCtrl m_Header;
	int m_cyHeader;
	int m_cxTotal;
	int m_xPos;
	int m_arrColWidths[16];
	int m_xOffset;

	
protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);	

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLUMNTREECTRL_H__0AEDF909_4AF6_4DE8_AF8C_4EB9E0843020__INCLUDED_)
