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

#if !defined(AFX_VIEWCOLUMN_H__FD1910A9_1F29_45A3_8583_D8AE242010BF__INCLUDED_)
#define AFX_VIEWCOLUMN_H__FD1910A9_1F29_45A3_8583_D8AE242010BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewColumn.h : header file
//

extern TCHAR *WndRegKeys[4][5];

#include "View.h"

/////////////////////////////////////////////////////////////////////////////
// ViewColumn dialog

class ViewColumn : public CDialog
{
// Construction
public:
	ViewColumn(CWnd* pParent = NULL);   // standard constructor
	
	TileInfo **list;
	int nTiles;
	int sizetile_y;
	int nTileInWnd;
	int pos; //(0,1,2,3) depending of window position (left, bottom, top, right)
	CDialog *parent;
	POINT MousePos;

// Dialog Data
	//{{AFX_DATA(ViewColumn)
	enum { IDD = IDD_COLUMN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ViewColumn)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ViewColumn)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWCOLUMN_H__FD1910A9_1F29_45A3_8583_D8AE242010BF__INCLUDED_)
