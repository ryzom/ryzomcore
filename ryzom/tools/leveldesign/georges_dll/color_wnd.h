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

#if !defined(AFX_COLOR_WND_H__E8BA0021_DBC1_4E3C_9D8A_D13A8FCE7B0C__INCLUDED_)
#define AFX_COLOR_WND_H__E8BA0021_DBC1_4E3C_9D8A_D13A8FCE7B0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// color_wnd.h : header file
//

#include "nel/misc/rgba.h"

/////////////////////////////////////////////////////////////////////////////
// CColorWnd window

#define CL_CHANGED (WM_APP+0x3a)
#define CBN_CHANGED (WM_APP+0x3b)

class CColorWnd : public CWnd
{
// Construction
public:
	CColorWnd();

	void create (DWORD wStyle, RECT &pos, CWnd *window, uint dialogIndex);

	void setColor (const NLMISC::CRGBA &color);
	NLMISC::CRGBA getColor () const;
	void colorChanged ();
	void setEdit(CEdit *pEdit) { this->pEdit = pEdit; }
	void updateEdit();

// Attributes
public:

	NLMISC::CRGBA	Color;
	uint			Id;

private:
	CEdit*			pEdit;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorWnd)
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLOR_WND_H__E8BA0021_DBC1_4E3C_9D8A_D13A8FCE7B0C__INCLUDED_)
